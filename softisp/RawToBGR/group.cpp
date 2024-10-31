#include "group.hpp"

static double const GroupPI = 3.1415926535897932384626433832795;
static int const GroupsInterpolationShift = 11;
static int const GroupsInterpolationScale = 1 << GroupsInterpolationShift;

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
		double y = -(x + 3 - i) * GroupPI * ratio;
		double z = y * 0.25;
		cbuf[i] = static_cast<float>(std::sin(y) * std::sin(z) / (y * z));
		//if (cbuf[i] <= -FLT_EPSILON)
		//	cbuf[i] = -cbuf[i] / fabs(3.5f - i);
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
		double y = -(x + 2 - i) * GroupPI * ratio;
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
		double y = -(x + 1 - i) * GroupPI * ratio;
		double z = y * 0.5;
		cbuf[i] = static_cast<float>(std::sin(y) * std::sin(z) / (y * z));
		sum += cbuf[i];
	}

	sum = 1.f / sum;
	for (int i = 0; i < 4; i++)
		cbuf[i] *= sum;
}

static void initKernel(short* GroupKernel)
{
	float cbuf[8] = { 0 };
	int sum = 0;
	interpolateLanczos2(0.25f, cbuf + 2, 1.f / 1.75f);
	short* K = GroupKernel;
	for (int k = 0; k < 8; k++)
	{
		K[k] = saturate_cast<short>(cbuf[k] * GroupsInterpolationScale);
		sum += K[k];
	}
	K[3] += saturate_cast<short>(GroupsInterpolationScale - sum);

	sum = 0;
	interpolateLanczos2(0.75f, cbuf + 2, 1.f / 1.75f);
	K = GroupKernel + 8;
	for (int k = 0; k < 8; k++)
	{
		K[k] = saturate_cast<short>(cbuf[k] * GroupsInterpolationScale);
		sum += K[k];
	}
	K[4] += saturate_cast<short>(GroupsInterpolationScale - sum);
}

namespace gxr
{
	RawGroup::ColPass::ColPass(RawGroup* rg_, Mat& src_, Mat& tmp_)
		: rg(rg_), src(src_), tmp(tmp_)
	{
		scols = src.cols;
		dcols = tmp.cols;
		sw0 = rg->sw0;
		sw1 = rg->sw1;
		dw0 = rg->dw0;
		dw1 = rg->dw1;
		GroupKernel = rg->GroupKernel;
	}

	void RawGroup::ColPass::operator ()(Range const& range) const
	{
		int xmin = gxr_min(4, dw0);
		int xmax = gxr_max(dcols - 4, dw1);
		short* K0 = GroupKernel;
		short* K1 = GroupKernel + 8;
		v_int32 vK0 = v_int32(K0[0], K0[1], K0[2], K0[3], K0[4], K0[5], K0[6], K0[7]);
		v_int32 vK1 = v_int32(K1[0], K1[1], K1[2], K1[3], K1[4], K1[5], K1[6], K1[7]);
		v_int16 vr, vg;
		for (int h = range.start; h < range.end; h++)
		{
			short const* S = src.ptr<short>(h);
			short* D = tmp.ptr<short>(h);
			int dx = 0, limit = xmin, xcur = 0;
			for (;;)
			{
				for (; dx < limit; dx += 2)
				{
					int sx = xcur - 6;
					int v0 = 0, v1 = 0;
					for (int j = 0; j < 8; j++)
					{
						int sxj = sx + j;
						if ((unsigned)sxj >= (unsigned)scols)
						{
							while (sxj < 0)
								sxj += 2;
							while (sxj >= scols)
								sxj -= 2;
						}
						v0 += S[sxj] * K0[j];
						v1 += S[sxj] * K1[j];
					}
					D[dx] = v0 >> GroupsInterpolationShift;
					D[dx + 1] = v1 >> GroupsInterpolationShift;
					xcur += 4;
				}
				if (limit == dcols)
					break;
				for (; dx < dw0; dx += 2)
				{
					v_load_deinterleave(S + xcur - 6, vr, vg);
					D[dx] = v_reduce_sum(v_expand_low(vr) * vK0) >> GroupsInterpolationShift;
					D[dx + 1] = v_reduce_sum(v_expand_low(vg) * vK1) >> GroupsInterpolationShift;
					xcur += 4;
				}
				memcpy(D + dw0, S + sw0, (sw1 - sw0) * sizeof(short));
				dx = dw1; xcur += (sw1 - sw0);
				for (; dx < xmax; dx += 2)
				{
					v_load_deinterleave(S + xcur - 6, vr, vg);
					D[dx] = v_reduce_sum(v_expand_low(vr) * vK0) >> GroupsInterpolationShift;
					D[dx + 1] = v_reduce_sum(v_expand_low(vg) * vK1) >> GroupsInterpolationShift;
					xcur += 4;
				}
				limit = dcols;
			}
		}
	}

	RawGroup::RowPass::RowPass(RawGroup* rg_, Mat& dst_, Mat& tmp_)
		: rg(rg_), dst(dst_), tmp(tmp_)
	{
		srows = tmp.rows; scols = tmp.cols;
		drows = dst.rows; dcols = dst.cols;
		sh0 = rg->sh0; sh1 = rg->sh1;
		dh0 = rg->dh0; dh1 = rg->dh1;
		memcpy(dst.ptr<short>(dh0), tmp.ptr<short>(sh0), (sh1 - sh0) * tmp.step[0]);
		GroupKernel = rg->GroupKernel;
	}

	void RawGroup::RowPass::operator ()(Range const& range) const
	{
		short* S0[8], * S1[8];
		short* K0 = GroupKernel;
		short* K1 = GroupKernel + 8;
		int const VSZ = v_int32::nlanes;
		int xlimit = dcols - VSZ;
		v_int32 vK00 = vx_setall_s32(K0[0]), vK01 = vx_setall_s32(K0[1]), vK02 = vx_setall_s32(K0[2]), vK03 = vx_setall_s32(K0[3]);
		v_int32 vK04 = vx_setall_s32(K0[4]), vK05 = vx_setall_s32(K0[5]), vK06 = vx_setall_s32(K0[6]), vK07 = vx_setall_s32(K0[7]);
		v_int32 vK10 = vx_setall_s32(K1[0]), vK11 = vx_setall_s32(K1[1]), vK12 = vx_setall_s32(K1[2]), vK13 = vx_setall_s32(K1[3]);
		v_int32 vK14 = vx_setall_s32(K1[4]), vK15 = vx_setall_s32(K1[5]), vK16 = vx_setall_s32(K1[6]), vK17 = vx_setall_s32(K1[7]);
		v_int32 vs0, vs1, vs2, vs3, vs4, vs5, vs6, vs7;
		v_int32 vm0, vm1;
		for (int y = range.start; y < range.end; y++)
		{
			int dy = y << 1;
			if (dy >= dh0 && dy < dh1)
				continue;
			int sy = (dy < dh0) ? (dy << 1) : sh1 + ((dy - dh1) << 1);
			for (int k = 0; k < 8; k++)
			{
				S0[k] = tmp.ptr<short>(gxr_clamp(sy - 6 + k * 2, 0, srows - 2));
				S1[k] = S0[k] + scols;
				//CV_Assert(S1[k] == tmp.ptr<short>(gxr_clamp(sy - 5 + k * 2, 1, srows - 1)));
			}
			short* D0 = dst.ptr<short>(dy);
			short* D1 = dst.ptr<short>(dy + 1);
			int x = 0;
			for (; x <= xlimit; x += VSZ)
			{
				v_pack_store(D0 + x, (vx_load_expand(S0[0] + x) * vK00 + vx_load_expand(S0[1] + x) * vK01 + vx_load_expand(S0[2] + x) * vK02 + vx_load_expand(S0[3] + x) * vK03 +
					vx_load_expand(S0[4] + x) * vK04 + vx_load_expand(S0[5] + x) * vK05 + vx_load_expand(S0[6] + x) * vK06 + vx_load_expand(S0[7] + x) * vK07) >> GroupsInterpolationShift);

				v_pack_store(D1 + x, (vx_load_expand(S1[0] + x) * vK10 + vx_load_expand(S1[1] + x) * vK11 + vx_load_expand(S1[2] + x) * vK12 + vx_load_expand(S1[3] + x) * vK13 +
					vx_load_expand(S1[4] + x) * vK14 + vx_load_expand(S1[5] + x) * vK15 + vx_load_expand(S1[6] + x) * vK16 + vx_load_expand(S1[7] + x) * vK17) >> GroupsInterpolationShift);
			}
			for (; x < dcols; x++)
			{
				D0[x] = ((S0[0][x] * K0[0] + S0[1][x] * K0[1] +
					S0[2][x] * K0[2] + S0[3][x] * K0[3] + S0[4][x] * K0[4] +
					S0[5][x] * K0[5] + S0[6][x] * K0[6] + S0[7][x] * K0[7])) >> GroupsInterpolationShift;

				D1[x] = ((S1[0][x] * K1[0] + S1[1][x] * K1[1] +
					S1[2][x] * K1[2] + S1[3][x] * K1[3] + S1[4][x] * K1[4] +
					S1[5][x] * K1[5] + S1[6][x] * K1[6] + S1[7][x] * K1[7])) >> GroupsInterpolationShift;
			}
		}
	}

	RawGroup::RawGroup(Mat& src_, Rect& roi_, Mat& buffer_)
		: src(src_), roi(roi_)
	{
		srows = src.rows; scols = src.cols;
		drows = (srows - roi.height) / 2 + roi.height;
		dcols = (scols - roi.width) / 2 + roi.width;

		buffer_.create(drows, dcols, CV_16SC3);

		sw0 = roi.x; sw1 = roi.x + roi.width;
		sh0 = roi.y; sh1 = roi.y + roi.height;
		dw0 = sw0 >> 1; dw1 = dw0 + roi.width;
		dh0 = sh0 >> 1; dh1 = dh0 + roi.height;

		tmp = Mat(srows, dcols, CV_16SC1, buffer_.data);
		CV_Assert(tmp.dataend <= buffer_.dataend);

		initKernel(GroupKernel);
	}

	void RawGroup::run()
	{
		parallel_for_(Range(0, src.rows), ColPass(this, src, tmp));
		src = Mat(drows, dcols, CV_16SC1, src.data);
		parallel_for_(Range(0, src.rows / 2), RowPass(this, src, tmp));
	}
}







