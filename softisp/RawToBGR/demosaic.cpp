#include "demosaic.h"
#include "utility.hpp"

template<class Tp> inline
Tp const& clamp(Tp const& x, Tp const& lo, Tp const& hi)
{
	return x < lo ? lo : (hi < x ? hi : x);
}

static inline int color(unsigned h, unsigned w, int rpat)
{
	return (4 - ((((h & 1) << 1) + (w & 1)) ^ rpat)) >> 1;
}

static inline int clamp_ud(int v, int l1, int l2)
{
	return (l1 < l2) ? clamp(v, l1, l2) : clamp(v, l2, l1);
}

static inline void st(short& x, short& y)
{
	short t = x;
	x = gxr::gxr_min(x, y);
	y = gxr::gxr_max(y, t);
}

static void demosaic_fillcolor(ushort * Src, ushort * Dst, int rows, int cols, int rpat)
{
	for (int h = 0; h < rows; h++)
	{
		int i0 = color(h, 0, rpat);
		int i1 = color(h, 1, rpat) + 3;
		ushort* S = Src + h * cols;
		ushort* D = Dst + h * cols * 3;
		for (int w = 0; w < cols; w += 2)
		{
			D[i0] = S[0];
			D[i1] = S[1];
			S += 2;
			D += 6;
		}
	}
}

static void demosaic_border(ushort * Src, unsigned rows, unsigned cols, unsigned border, int rpat)
{
	int step = cols * 3;
	int S[8];
	for (unsigned h = 0; h < rows; ++h)
	{
		ushort* B = Src + h * step;
		for (unsigned w = 0; w < cols; ++w)
		{
			if ((w == border) && (h >= border) && (h < rows - border))
				w = cols - border;
			memset(S, 0, sizeof(S));
			for (unsigned y = h - 1; y != h + 2; ++y)
			{
				if (y >= rows) continue;
				ushort const* P = Src + y * step;
				for (unsigned x = w - 1; x != w + 2; ++x)
				{
					if (x >= cols) continue;
					int f = color(y, x, rpat);
					S[f] += P[x * 3 + f];
					++(S[f + 4]);
				}
			}
			int f = color(h, w, rpat);
			for (int c = 0; c < 3; ++c)
				if ((c != f) && S[c + 4])
					B[w * 3 + c] = static_cast<short>((S[c] + (S[c + 4] >> 1)) / S[c + 4]);
		}
	}
}

static void demosaic_inter(ushort * Src, int rows, int cols, int rpat, int sat)
{
	int const step = cols * 3;
	int const dir[5] = { 3, step, -3, -step, 3 };
	int wstop = cols - 3;
	int diff[2], guess[2];
	ushort* line, * vP;

	for (int h = 3; h < rows - 3; ++h)
	{
		line = Src + h * step;
		int w = 3 + (color(h, 3, rpat) & 1), c = color(h, w, rpat);
		int w3 = w * 3;
		for (; w < wstop; w += 2, w3 += 6)
		{
			vP = line + w3; int i, d, d2, d3;
			for (i = 0; i < 2; ++i)
			{
				d = dir[i]; d2 = d + d; d3 = d2 + d;
				guess[i]
					= 2 * ((int)(vP[-d + 1]) + vP[c] + vP[d + 1])
					- vP[-d2 + c] - vP[d2 + c];
				diff[i]
					= 3 * (abs((int)(vP[-d2 + c]) - vP[c])
						+ abs((int)(vP[d2 + c]) - vP[c])
						+ abs(vP[-d + 1] - vP[d + 1]))
					+ 2 * (abs((int)(vP[d3 + 1]) - vP[d + 1])
						+ abs((int)(vP[-d3 + 1]) - vP[-d + 1]));
			}
			i = diff[0] > diff[1];
			d = dir[i];
			if (vP[d + 1] < vP[-d + 1])
				vP[1] = clamp_ud(guess[i] / 4, vP[d + 1], vP[-d + 1]);
			else
				vP[1] = clamp_ud(guess[i] / 4, vP[-d + 1], vP[d + 1]);
		}
	}

	wstop = cols - 1;
	for (int h = 1; h < rows - 1; ++h)
	{
		line = Src + h * step;
		int w = 1 + (color(h, 2, rpat) & 1), c = color(h, w + 1, rpat);
		int w3 = w * 3, cc = 2 - c, s = 9 - w3 - w3;
		for (; w < wstop; w += 2, w3 += 6)
		{
			vP = line + w3; int i, d;
			for (i = 0; i < 2; c = 2 - c, ++i)
			{
				d = dir[i];
				d = ((int)(vP[-d + c]) + vP[d + c] + 2 * vP[1] - vP[-d + 1] - vP[d + 1]) / 2;
				vP[c] = clamp<int>(d, 0, sat);
			}

			vP += s;
			for (i = 0; i < 2; ++i)
			{
				d = dir[i] + dir[i + 1];
				diff[i] = abs((int)(vP[-d + cc]) - vP[d + cc]) + abs((int)(vP[-d + 1]) - vP[1]) + abs((int)(vP[d + 1]) - vP[1]);
				guess[i] = (int)(vP[-d + cc]) + vP[d + cc] + 2 * vP[1] - vP[-d + 1] - vP[d + 1];
			}
			if (diff[0] != diff[1])
				vP[cc] = clamp<int>((guess[diff[0] > diff[1]]) / 2, 0, sat);
			else
				vP[cc] = clamp<int>((guess[0] + guess[1]) / 4, 0, sat);
		}
	}
}

static void demosaic_median(ushort* Src, int rows, int cols, int sat)
{
	int step = cols * 3;
	for (int h = 0; h < rows; h++)
	{
		short* S = (short*)Src + h * step;
		int w = 0;
		for (; w < cols; w++)
		{
			S[0] -= S[1];
			S[2] -= S[1];
			S += 3;
		}
	}
	short* Tmp = (short*)malloc(rows * cols * 3 * sizeof(short));
	memcpy(Tmp, Src, rows * cols * 3 * sizeof(short));
	for (int h = 2; h < rows - 2; h++)
	{
		short* line[5];
		line[0] = Tmp + (h - 2) * step + 6;
		line[1] = Tmp + (h - 1) * step + 6;
		line[2] = Tmp + h * step + 6;
		line[3] = Tmp + (h + 1) * step + 6;
		line[4] = Tmp + (h + 2) * step + 6;
		short* lined = (short*)Src + h * step + 6;

		short b[25];
		short r[25];
		int w = 2;
		for (; w < cols - 2; w++)
		{
			for (int k = 0; k < 5; k++)
			{
				short const* rowk = line[k];
				short* rowb = b + k * 5;
				rowb[0] = rowk[-6];
				rowb[1] = rowk[-3];
				rowb[2] = rowk[0];
				rowb[3] = rowk[3];
				rowb[4] = rowk[6];

				short* rowr = r + k * 5;
				rowr[0] = rowk[-4];
				rowr[1] = rowk[-1];
				rowr[2] = rowk[2];
				rowr[3] = rowk[5];
				rowr[4] = rowk[8];
			}
			st(b[1], b[2]); st(b[0], b[1]); st(b[1], b[2]); st(b[4], b[5]); st(b[3], b[4]);
			st(b[4], b[5]); st(b[0], b[3]); st(b[2], b[5]); st(b[2], b[3]); st(b[1], b[4]);
			st(b[1], b[2]); st(b[3], b[4]); st(b[7], b[8]); st(b[6], b[7]); st(b[7], b[8]);
			st(b[10], b[11]); st(b[9], b[10]); st(b[10], b[11]); st(b[6], b[9]); st(b[8], b[11]);
			st(b[8], b[9]); st(b[7], b[10]); st(b[7], b[8]); st(b[9], b[10]); st(b[0], b[6]);
			st(b[4], b[10]); st(b[4], b[6]); st(b[2], b[8]); st(b[2], b[4]); st(b[6], b[8]);
			st(b[1], b[7]); st(b[5], b[11]); st(b[5], b[7]); st(b[3], b[9]); st(b[3], b[5]);
			st(b[7], b[9]); st(b[1], b[2]); st(b[3], b[4]); st(b[5], b[6]); st(b[7], b[8]);
			st(b[9], b[10]); st(b[13], b[14]); st(b[12], b[13]); st(b[13], b[14]); st(b[16], b[17]);
			st(b[15], b[16]); st(b[16], b[17]); st(b[12], b[15]); st(b[14], b[17]); st(b[14], b[15]);
			st(b[13], b[16]); st(b[13], b[14]); st(b[15], b[16]); st(b[19], b[20]); st(b[18], b[19]);
			st(b[19], b[20]); st(b[21], b[22]); st(b[23], b[24]); st(b[21], b[23]); st(b[22], b[24]);
			st(b[22], b[23]); st(b[18], b[21]); st(b[20], b[23]); st(b[20], b[21]); st(b[19], b[22]);
			st(b[22], b[24]); st(b[19], b[20]); st(b[21], b[22]); st(b[23], b[24]); st(b[12], b[18]);
			st(b[16], b[22]); st(b[16], b[18]); st(b[14], b[20]); st(b[20], b[24]); st(b[14], b[16]);
			st(b[18], b[20]); st(b[22], b[24]); st(b[13], b[19]); st(b[17], b[23]); st(b[17], b[19]);
			st(b[15], b[21]); st(b[15], b[17]); st(b[19], b[21]); st(b[13], b[14]); st(b[15], b[16]);
			st(b[17], b[18]); st(b[19], b[20]); st(b[21], b[22]); st(b[23], b[24]); st(b[0], b[12]);
			st(b[8], b[20]); st(b[8], b[12]); st(b[4], b[16]); st(b[16], b[24]); st(b[12], b[16]);
			st(b[2], b[14]); st(b[10], b[22]); st(b[10], b[14]); st(b[6], b[18]); st(b[6], b[10]);
			st(b[10], b[12]); st(b[1], b[13]); st(b[9], b[21]); st(b[9], b[13]); st(b[5], b[17]);
			st(b[13], b[17]); st(b[3], b[15]); st(b[11], b[23]); st(b[11], b[15]); st(b[7], b[19]);
			st(b[7], b[11]); st(b[11], b[13]); st(b[11], b[12]);

			st(r[1], r[2]); st(r[0], r[1]); st(r[1], r[2]); st(r[4], r[5]); st(r[3], r[4]);
			st(r[4], r[5]); st(r[0], r[3]); st(r[2], r[5]); st(r[2], r[3]); st(r[1], r[4]);
			st(r[1], r[2]); st(r[3], r[4]); st(r[7], r[8]); st(r[6], r[7]); st(r[7], r[8]);
			st(r[10], r[11]); st(r[9], r[10]); st(r[10], r[11]); st(r[6], r[9]); st(r[8], r[11]);
			st(r[8], r[9]); st(r[7], r[10]); st(r[7], r[8]); st(r[9], r[10]); st(r[0], r[6]);
			st(r[4], r[10]); st(r[4], r[6]); st(r[2], r[8]); st(r[2], r[4]); st(r[6], r[8]);
			st(r[1], r[7]); st(r[5], r[11]); st(r[5], r[7]); st(r[3], r[9]); st(r[3], r[5]);
			st(r[7], r[9]); st(r[1], r[2]); st(r[3], r[4]); st(r[5], r[6]); st(r[7], r[8]);
			st(r[9], r[10]); st(r[13], r[14]); st(r[12], r[13]); st(r[13], r[14]); st(r[16], r[17]);
			st(r[15], r[16]); st(r[16], r[17]); st(r[12], r[15]); st(r[14], r[17]); st(r[14], r[15]);
			st(r[13], r[16]); st(r[13], r[14]); st(r[15], r[16]); st(r[19], r[20]); st(r[18], r[19]);
			st(r[19], r[20]); st(r[21], r[22]); st(r[23], r[24]); st(r[21], r[23]); st(r[22], r[24]);
			st(r[22], r[23]); st(r[18], r[21]); st(r[20], r[23]); st(r[20], r[21]); st(r[19], r[22]);
			st(r[22], r[24]); st(r[19], r[20]); st(r[21], r[22]); st(r[23], r[24]); st(r[12], r[18]);
			st(r[16], r[22]); st(r[16], r[18]); st(r[14], r[20]); st(r[20], r[24]); st(r[14], r[16]);
			st(r[18], r[20]); st(r[22], r[24]); st(r[13], r[19]); st(r[17], r[23]); st(r[17], r[19]);
			st(r[15], r[21]); st(r[15], r[17]); st(r[19], r[21]); st(r[13], r[14]); st(r[15], r[16]);
			st(r[17], r[18]); st(r[19], r[20]); st(r[21], r[22]); st(r[23], r[24]); st(r[0], r[12]);
			st(r[8], r[20]); st(r[8], r[12]); st(r[4], r[16]); st(r[16], r[24]); st(r[12], r[16]);
			st(r[2], r[14]); st(r[10], r[22]); st(r[10], r[14]); st(r[6], r[18]); st(r[6], r[10]);
			st(r[10], r[12]); st(r[1], r[13]); st(r[9], r[21]); st(r[9], r[13]); st(r[5], r[17]);
			st(r[13], r[17]); st(r[3], r[15]); st(r[11], r[23]); st(r[11], r[15]); st(r[7], r[19]);
			st(r[7], r[11]); st(r[11], r[13]); st(r[11], r[12]);

			lined[0] = b[12]; lined[2] = r[12];

			line[0] += 3;
			line[1] += 3;
			line[2] += 3;
			line[3] += 3;
			line[4] += 3;
			lined += 3;
		}
	}
	for (int h = 0; h < rows; h++)
	{
		short* S = (short*)Src + h * step;
		int w = 0;
		for (; w < cols; w++)
		{
			S[0] = gxr::gxr_clamp(S[0] + S[1], 0, sat);
			S[2] = gxr::gxr_clamp(S[2] + S[1], 0, sat);
			S += 3;
		}
	}
	free(Tmp);
}

void demosaic(ushort* Src, ushort* Dst, int rows, int cols, int rpat, int sat)
{
	demosaic_fillcolor(Src, Dst, rows, cols, rpat);
	demosaic_border(Dst, rows, cols, 3, rpat);
	demosaic_inter(Dst, rows, cols, rpat, sat);
	//demosaic_median(Dst, rows, cols, sat);
}