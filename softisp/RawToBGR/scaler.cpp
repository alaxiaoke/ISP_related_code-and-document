#include "scaler.hpp"
#include "raw2bgr.hpp"
#include "rawproc.hpp"

#define USEG 0

#define DEMOSAIC 1
#if DEMOSAIC
#include "demosaic.h"
#endif

static double const ScalerPI = 3.1415926535897932384626433832795;
static int const ScalerPhaseShift = 6;
static int const ScalerPhaseNum = 1 << ScalerPhaseShift;
static int const ScalersInterpolationShift = 11;
static int const ScalersInterpolationScale = 1 << ScalersInterpolationShift;
static double ScalerSmooth = 0.7;

static inline int clamp_ud(int v, int l1, int l2)
{
	return (l1 < l2) ? gxr::gxr_clamp(v, l1, l2) : gxr::gxr_clamp(v, l2, l1);
}

static inline int polar(int x, int y)
{
	return (x ^ y) & 1;
}

static inline void interpolateLanczos(float x, float* cbuf, float ratio = 1.f)
{
	if (x <= FLT_EPSILON)
	{
		for (int i = 0; i < 16; i++)
			cbuf[i] = 0;
		cbuf[7] = 1;
		return;
	}

	float sum = 0;
	for (int i = 0; i < 16; i++)
	{
		double y = -(x + 7 - i) * ScalerPI * ratio;
		double z = y * 0.25;
		cbuf[i] = static_cast<float>(std::sin(y) * std::sin(z) / (y * z));
		sum += cbuf[i];
	}

	sum = 1.f / sum;
	for (int i = 0; i < 16; i++)
		cbuf[i] *= sum;
}

static inline void interpolateLanczos4(float x, float* cbuf, float ratio = 1.f)
{
	if (x <= FLT_EPSILON)
	{
		for (int i = 0; i < 8; i++)
			cbuf[i] = 0;
		cbuf[3] = 1;
		return;
	}

	float sum = 0;
	for (int i = 0; i < 8; i++)
	{
		//double y = -(x + 3 - i) * ScalerPI * ratio;
		//double z = y * 0.25;
		//cbuf[i] = static_cast<float>(std::sin(y) * std::sin(z) / (y * z));
		//sum += cbuf[i];



		double s = -(x + 3 - i) * ratio;
		if (s < -4 || s > 4)
			cbuf[i] = 0;
		else
		{
			double y = -(x + 3 - i) * ScalerPI * ratio;
			double z = y * 0.25;
			cbuf[i] = static_cast<float>(std::sin(y) * std::sin(z) / (y * z));
		}
		sum += cbuf[i];
	}

	sum = 1.f / sum;
	for (int i = 0; i < 8; i++)
		cbuf[i] *= sum;
}

static inline void interpolateLanczos3(float x, float* cbuf, float ratio = 1.f)
{
	if (x <= FLT_EPSILON)
	{
		for (int i = 0; i < 6; i++)
			cbuf[i] = 0;
		cbuf[2] = 1;
		return;
	}

	float sum = 0;
	for (int i = 0; i < 6; i++)
	{
		double y = -(x + 2 - i) * ScalerPI * ratio;
		double z = y / 3.0;
		cbuf[i] = static_cast<float>(std::sin(y) * std::sin(z) / (y * z));
		sum += cbuf[i];
	}

	sum = 1.f / sum;
	for (int i = 0; i < 6; i++)
		cbuf[i] *= sum;
}

static inline void interpolateLanczos2(float x, float* cbuf, float ratio = 1.f)
{
	if (x <= FLT_EPSILON)
	{
		for (int i = 0; i < 4; i++)
			cbuf[i] = 0;
		cbuf[1] = 1;
		return;
	}

	float sum = 0;
	for (int i = 0; i < 4; i++)
	{
		double y = -(x + 1 - i) * ScalerPI * ratio;
		double z = y * 0.5;
		cbuf[i] = static_cast<float>(std::sin(y) * std::sin(z) / (y * z));
		sum += cbuf[i];
	}

	sum = 1.f / sum;
	for (int i = 0; i < 4; i++)
		cbuf[i] *= sum;
}

namespace gxr
{
	RawScaler::ColPass::ColPass(RawScaler* rs_, Mat& src_, Mat& tmp_)
		: rs(rs_), src(src_), tmp(tmp_)
	{
		srows = src.rows; scols = src.cols;
		drows = tmp.rows / 2; dcols = tmp.cols;
		HorCoef = rs->coef.ptr<short>(0);
		HorPhase = rs->phase.ptr<short>(0);
		pat = RawFilter[rs->rpat];

		coefgreen.create(ScalerPhaseNum, 8, CV_16SC1);
		double hratio = (double)(scols) / dcols;
		for (int i = 0; i < ScalerPhaseNum; i++)
		{
			float cbuf[8] = { 0 };
			int sum = 0;
#if USEG
			interpolateLanczos4((double)(i) / ScalerPhaseNum, cbuf, 1 / hratio * 2);
#else
			interpolateLanczos4((double)(i) / ScalerPhaseNum, cbuf, 1 / hratio * 2 * ScalerSmooth);
#endif
			short* K = coefgreen.ptr<short>(i);
			for (int k = 0; k < 8; k++)
			{
				K[k] = saturate_cast<short>(cbuf[k] * ScalersInterpolationScale);
				sum += K[k];
			}
			K[3] += saturate_cast<short>(ScalersInterpolationScale - sum);
		}
		CoefGreen = coefgreen.ptr<short>(0);
	}

	void RawScaler::ColPass::operator ()(Range const& range) const
	{
		for (int h = range.start; h < range.end; h++)
		{
			short* S = src.ptr<short>(h);
#if USEG
			short* DG = tmp.ptr<short>(h);
#else
			short* DG = tmp.ptr<short>(srows + h);
#endif
			short* DR = tmp.ptr<short>(srows + h);
			for (int w = 0; w < dcols; w++)
			{
				int c = fcol(h, w, pat);
				int p = w & 1;
				int dw = HorPhase[w * 2];
				int phase = HorPhase[w * 2 + 1] >> 1;
				phase += polar(dw, p) ? (ScalerPhaseNum >> 1) : 0;
				dw = dw - polar(dw, p);
				int sw = dw - 6;
				int wmin = 0 + polar(0, p), wmax = scols - 1 - polar(scols - 1, p);
				int sum = 0;
				for (int i = 0; i < 8; i++)
				{
					int k = gxr_clamp(sw + i * 2, wmin, wmax);
					sum += S[k] * (c == 1 ? CoefGreen[phase * 8 + i] : HorCoef[phase * 8 + i]);
				}
				if (c == 1)
					DG[w] = sum >> ScalersInterpolationShift;
				else
				{
					DR[w] = sum >> ScalersInterpolationShift;
#if USEG
					p = p ^ 1;
					dw = HorPhase[w * 2];
					phase = HorPhase[w * 2 + 1] >> 1;
					phase += polar(dw, p) ? (ScalerPhaseNum >> 1) : 0;
					dw = dw - polar(dw, p);
					sw = dw - 6;
					wmin = 0 + polar(0, p), wmax = scols - 1 - polar(scols - 1, p);
					sum = 0;
					for (int i = 0; i < 8; i++)
					{
						int k = gxr_clamp(sw + i * 2, wmin, wmax);
						sum += S[k] * CoefGreen[phase * 8 + i];
					}
					DG[w] = sum >> ScalersInterpolationShift;
#endif
				}
			}
//#if USEG
//			for (int w = 1; w < dcols - 1; w++)
//			{
//				int c = fcol(h, w, pat);
//				if (c != 1)
//					DG[w] = clamp_ud(DG[w], DG[w - 1], DG[w + 1]);
//			}
//#endif
		}
	}

	RawScaler::RowPass::RowPass(RawScaler* rs_, Mat& dst_, Mat& tmp_)
		: rs(rs_), dst(dst_), tmp(tmp_)
	{
		srows = tmp.rows / 2; scols = tmp.cols;
		drows = dst.rows; dcols = dst.cols;
		VerCoef = rs->coef.ptr<short>(ScalerPhaseNum);
		VerPhase = rs->phase.ptr<short>(0, dcols * 2);
		pat = RawFilter[rs->rpat];

		coefgreen.create(ScalerPhaseNum, 8, CV_16SC1);
		double vratio = (double)(srows) / drows;
		for (int i = 0; i < ScalerPhaseNum; i++)
		{
			float cbuf[8] = { 0 };
			int sum = 0;
#if USEG
			interpolateLanczos4((double)(i) / ScalerPhaseNum, cbuf, 1 / vratio);
#else
			interpolateLanczos4((double)(i) / ScalerPhaseNum, cbuf, 1 / vratio * 2 * ScalerSmooth);
#endif
			short* K = coefgreen.ptr<short>(i);
			for (int k = 0; k < 8; k++)
			{
				K[k] = saturate_cast<short>(cbuf[k] * ScalersInterpolationScale);
				sum += K[k];
			}
			K[3] += saturate_cast<short>(ScalersInterpolationScale - sum);
		}
		CoefGreen = coefgreen.ptr<short>(0);
	}

	void RawScaler::RowPass::operator ()(Range const& range) const
	{
		for (int h = range.start; h < range.end; h++)
		{
			int p = h & 1;
			int dh = VerPhase[h * 2];
			int phase = VerPhase[h * 2 + 1];
			int phase_g = phase;
			int phase_r = phase >> 1;
			phase_r += polar(dh, p) ? (ScalerPhaseNum >> 1) : 0;
			int dh_g = dh, sh_g = dh_g - 3;
			int dh_r = dh - polar(dh, p), sh_r = dh_r - 6;
			int hmin = 0 + polar(0, p), hmax = srows - 1 - polar(srows - 1, p);
			short* SG[8], *SR[8], *D = dst.ptr<short>(h);
			for (int i = 0; i < 8; i++)
			{
				int k = gxr_clamp(sh_g + i, 0, srows - 1);
				SG[i] = tmp.ptr<short>(k);

				k = gxr_clamp(sh_r + i * 2, hmin, hmax);
				SR[i] = tmp.ptr<short>(srows + k);
			}
			for (int w = 0; w < dcols; w++)
			{
				int c = fcol(h, w, pat);
				int sum = 0;
				if (c == 1)
				{
#if USEG
					for (int i = 0; i < 8; i++)
						sum += SG[i][w] * CoefGreen[phase_g * 8 + i];
					D[w] = sum >> ScalersInterpolationShift;
#else
					for (int i = 0; i < 8; i++)
						sum += SR[i][w] * CoefGreen[phase_r * 8 + i];
					D[w] = sum >> ScalersInterpolationShift;
#endif
				}
				else
				{
					for (int i = 0; i < 8; i++)
						sum += SR[i][w] * VerCoef[phase_r * 8 + i];
					D[w] = sum >> ScalersInterpolationShift;
				}
			}
		}
	}

	RawScaler::RawScaler(Mat& src_, Size dsize, Mat& buffer_, int rpat_)
		: src(src_), rpat(rpat_)
	{
		srows = src.rows; scols = src.cols;
		drows = dsize.height; dcols = dsize.width;
		buffer_.create(drows, dcols, CV_16SC3);
		tmp = Mat(srows * 2, dcols, CV_16SC1, buffer_.data);
		CV_Assert(tmp.dataend <= buffer_.dataend);

		coef.create(ScalerPhaseNum * 2, 8, CV_16SC1);
		phase.create(1, (dcols + drows) * 2, CV_16SC1);
		short* P = phase.ptr<short>(0);

		double hratio = (double)(scols) / dcols;
		for (int i = 0; i < ScalerPhaseNum; i++)
		{
			float cbuf[8] = { 0 };
			int sum = 0;
#if DEMOSAIC
			interpolateLanczos4((double)(i) / ScalerPhaseNum, cbuf, 1 / hratio);
#else
#if USEG
			interpolateLanczos4((double)(i) / ScalerPhaseNum, cbuf, 1 / hratio * 2 * ScalerSmooth);
#else
			interpolateLanczos4((double)(i) / ScalerPhaseNum, cbuf, 1 / hratio * 2 * ScalerSmooth);
#endif
#endif
			short* K = coef.ptr<short>(i);
			for (int k = 0; k < 8; k++)
			{
				K[k] = saturate_cast<short>(cbuf[k] * ScalersInterpolationScale);
				sum += K[k];
			}
			K[3] += saturate_cast<short>(ScalersInterpolationScale - sum);
		}
		for (int w = 0; w < dcols; w++)
		{
			double x = (w + 0.5) * hratio - 0.5;
			int s = floor(x);
			x = x - s;
			P[0] = s;
			P[1] = static_cast<int>(x * ScalerPhaseNum);
			P += 2;
		}

		double vratio = (double)(srows) / drows;
		for (int i = 0; i < ScalerPhaseNum; i++)
		{
			float cbuf[8] = { 0 };
			int sum = 0;
#if DEMOSAIC
			interpolateLanczos4((double)(i) / ScalerPhaseNum, cbuf, 1 / vratio);
#else
#if USEG
			interpolateLanczos4((double)(i) / ScalerPhaseNum, cbuf, 1 / vratio * 2 * ScalerSmooth);
#else
			interpolateLanczos4((double)(i) / ScalerPhaseNum, cbuf, 1 / vratio * 2 * ScalerSmooth);
#endif
#endif
			short* K = coef.ptr<short>(ScalerPhaseNum + i);
			for (int k = 0; k < 8; k++)
			{
				K[k] = saturate_cast<short>(cbuf[k] * ScalersInterpolationScale);
				sum += K[k];
			}
			K[3] += saturate_cast<short>(ScalersInterpolationScale - sum);
		}
		for (int h = 0; h < drows; h++)
		{
			double x = (h + 0.5) * vratio - 0.5;
			int s = floor(x);
			x = x - s;
			P[0] = s;
			P[1] = static_cast<int>(x * ScalerPhaseNum);
			P += 2;
		}
	}

	void RawScaler::run()
	{
#if DEMOSAIC
		Mat bgr(srows, scols, CV_16SC3), src_channels(srows * 3, scols, CV_16SC1);
		Mat dst_channels(drows * 3, dcols, CV_16SC1, bgr.data);
		Mat tmp_channel = Mat(srows, dcols, CV_16SC1, tmp.data);
		int pattern;
		if (rpat == Raw_RGGB)
			pattern = 0;
		else if (rpat == Raw_GRBG)
			pattern = 1;
		else if (rpat == Raw_GBRG)
			pattern = 2;
		else if (rpat == Raw_BGGR)
			pattern = 3;
		demosaic((ushort*)src.data, (ushort*)bgr.data, srows, scols, pattern, 1023);
		for (int h = 0; h < srows; h++)
		{
			short* S = bgr.ptr<short>(h);
			short* B = src_channels.ptr<short>(h);
			short* G = src_channels.ptr<short>(srows + h);
			short* R = src_channels.ptr<short>(srows * 2 + h);
			for (int w = 0; w < scols; w++)
			{
				B[w] = S[0];
				G[w] = S[1];
				R[w] = S[2];
				S += 3;
			}
		}
		for (int i = 0; i < 3; i++)
		{
			Mat src_channel = Mat(srows, scols, CV_16SC1, src_channels.ptr<short>(i * srows));
			Mat dst_channel = Mat(drows, dcols, CV_16SC1, dst_channels.ptr<short>(i * drows));
			for (int h = 0; h < srows; h++)
			{
				short* HorCoef = coef.ptr<short>(0);
				short* HorPhase = phase.ptr<short>(0);
				short* S = src_channel.ptr<short>(h);
				short* D = tmp_channel.ptr<short>(h);
				for (int w = 0; w < dcols; w++)
				{
					int dw = HorPhase[w * 2];
					int phase = HorPhase[w * 2 + 1];
					int sw = dw - 3;
					int sum = 0;
					for (int i = 0; i < 8; i++)
					{
						int k = gxr_clamp(sw + i, 0, scols - 1);
						sum += S[k] * HorCoef[phase * 8 + i];
					}
					D[w] = sum >> ScalersInterpolationShift;
				}
			}
			for (int h = 0; h < drows; h++)
			{
				short* VerCoef = coef.ptr<short>(ScalerPhaseNum);
				short* VerPhase = phase.ptr<short>(0, dcols * 2);
				int dh = VerPhase[h * 2];
				int phase = VerPhase[h * 2 + 1];
				int sh = dh - 3;
				short* S[8], * D = dst_channel.ptr<short>(h);
				for (int i = 0; i < 8; i++)
				{
					int k = gxr_clamp(sh + i, 0, srows - 1);
					S[i] = tmp_channel.ptr<short>(k);
				}
				for (int w = 0; w < dcols; w++)
				{
					int sum = 0;
					for (int i = 0; i < 8; i++)
						sum += S[i][w] * VerCoef[phase * 8 + i];
					D[w] = sum >> ScalersInterpolationShift;
				}
			}
		}
		src = Mat(drows, dcols, CV_16SC1, src.data);
		for (int h = 0; h < drows; h++)
		{
			short* S = src.ptr<short>(h);
			short* B = dst_channels.ptr<short>(h);
			short* G = dst_channels.ptr<short>(drows + h);
			short* R = dst_channels.ptr<short>(drows * 2 + h);
			short* BGR[3] = { B, G, R };
			unsigned pat = RawFilter[rpat];
			for (int w = 0; w < dcols; w++)
			{
				int c = fcol(h, w, pat);
				S[w] = BGR[c][w];
			}
		}
#if GXR_CV_DEBUG
		Mat rawbgr(drows, dcols, CV_16SC3);
		for (int h = 0; h < drows; h++)
		{
			short* BGR = rawbgr.ptr<short>(h);
			short* B = dst_channels.ptr<short>(h);
			short* G = dst_channels.ptr<short>(drows + h);
			short* R = dst_channels.ptr<short>(drows * 2 + h);
			for (int w = 0; w < dcols; w++)
			{
				BGR[0] = B[w];
				BGR[1] = G[w];
				BGR[2] = R[w];
				BGR += 3;
			}
		}
		Mat viz;
		rawbgr.convertTo(viz, CV_8U, 255.0 / 1023.0);
		imwrite("rawbgr.bmp", viz);
#endif
#else
		parallel_for_(Range(0, src.rows), ColPass(this, src, tmp));
		src = Mat(drows, dcols, CV_16SC1, src.data);
		parallel_for_(Range(0, src.rows), RowPass(this, src, tmp));
#endif
	}
}