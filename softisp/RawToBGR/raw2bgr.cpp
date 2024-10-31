#include "raw2bgr.hpp"
#include "group.hpp"
#include "scaler.hpp"
#include "gaussian.hpp"
#include "ocl.hpp"

using namespace gxr;

//==================== lens shading ====================//

class LensShadeSepCol : public ParallelLoopBody
{
	array<Mat, 4>& buf;
	Mat& img;
	float ymul;
	int border, rows, cols;
	int shrows, dh0, dh1, sh0, sh1;

public:

	LensShadeSepCol(array<Mat, 4>& rowbuf, Mat& raw, int shrows_, int dh0_, int dh1_, int sh0_, int sh1_)
		: buf(rowbuf), img(raw), shrows(shrows_), dh0(dh0_), dh1(dh1_), sh0(sh0_), sh1(sh1_)
	{
		border = rowbuf[0].rows - 1;
		rows = img.rows / 2;
		cols = img.cols / 2;
#if AltekLSC
		ymul = static_cast<float>(rowbuf[0].rows) / shrows;
#else
		ymul = static_cast<float>(border) / shrows;
#endif
		CV_Assert(rowbuf[0].cols == cols);
		CV_Assert(rowbuf[0].isContinuous());
	}

	void operator ()(Range const& range) const override
	{
		int const hborder = 0;
		for (int h = range.start; h < range.end; ++h)
		{
			int sh;
			if (h < dh0)
				sh = h << 1;
			else if (h < dh1)
				sh = h - dh0 + sh0;
			else
				sh = sh1 + ((h - dh1) << 1);
#if AltekLSC
			double yb = (sh + 0.5) * ymul - 0.5;
#else
			double yb = (sh + 0.5) * ymul;
#endif
			int y0 = gxr_clamp(static_cast<int>(yb), hborder, border - hborder - 1);
			yb = yb - y0;
			double const
				* R0 = buf[0].ptr<double>(y0),
				*R1 = buf[1].ptr<double>(y0),
				*R2 = buf[2].ptr<double>(y0),
				*R3 = buf[3].ptr<double>(y0);
			short
				*M0 = img.ptr<short>(h * 2),
				*M1 = M0 + img.cols;

			for (int w = 0; w < cols; ++w)
			{
				int x = w + cols;
				double r0 = R0[w] + yb * (R0[x] - R0[w]);
				double r1 = R1[w] + yb * (R1[x] - R1[w]);
				double r2 = R2[w] + yb * (R2[x] - R2[w]);
				double r3 = R3[w] + yb * (R3[x] - R3[w]);
				x = w * 2;
				M0[x + 0] = saturate_cast<short>(r0 * LSCVAL);
				M0[x + 1] = saturate_cast<short>(r1 * LSCVAL);
				M1[x + 0] = saturate_cast<short>(r2 * LSCVAL);
				M1[x + 1] = saturate_cast<short>(r3 * LSCVAL);
			}
		}
	}
};


static void lens_shade_sep_row(array<Mat, 4> & table, array<Mat, 4>& rowbuf, int shcols, int dw0, int dw1, int sw0, int sw1, float mul)
{
	int lscrow = table[0].rows;
	int lsccol = table[0].cols;
	int hcols = rowbuf[0].cols;
	int border = lsccol - 1;
	int const wborder = 0;
#if AltekLSC
	float xmul = static_cast<float>(lsccol) / shcols;
#else
	float xmul = static_cast<float>(border) / shcols;
#endif
	AutoBuffer<double, 1> _buffer(hcols * 2);
	double* beta = _buffer.data();
	int* xidx = reinterpret_cast<int*>(beta + hcols);
	for (int w = 0; w < hcols; ++w)
	{
		int sw;
		if (w < dw0)
			sw = w << 1;
		else if (w < dw1)
			sw = w - dw0 + sw0;
		else
			sw = sw1 + ((w - dw1) << 1);
#if AltekLSC
		double fx = (sw + 0.5) * xmul - 0.5;
#else
		double fx = (sw + 0.5) * xmul;
#endif
		int sx = gxr_clamp(static_cast<int>(fx), wborder, border - wborder - 1);
		beta[w] = fx - sx;
		xidx[w] = sx;
	}

	for (int h = 0; h < lscrow; ++h)
	{
		float const
			* L0 = table[0].ptr<float>(h),
			* L1 = table[1].ptr<float>(h),
			* L2 = table[2].ptr<float>(h),
			* L3 = table[3].ptr<float>(h);
		double
			* R0 = rowbuf[0].ptr<double>(h),
			* R1 = rowbuf[1].ptr<double>(h),
			* R2 = rowbuf[2].ptr<double>(h),
			* R3 = rowbuf[3].ptr<double>(h);
		for (int w = 0; w < hcols; ++w)
		{
			int x0 = xidx[w], x1 = x0 + 1;
			double xb = beta[w];
			R0[w] = (L0[x0] + xb * (L0[x1] - L0[x0])) * mul;
			R1[w] = (L1[x0] + xb * (L1[x1] - L1[x0])) * mul;
			R2[w] = (L2[x0] + xb * (L2[x1] - L2[x0])) * mul;
			R3[w] = (L3[x0] + xb * (L3[x1] - L3[x0])) * mul;
		}
	}
}


static void lens_shade_resize(array<Mat, 4>& table, Mat & raw, Rect roi, int srows, int scols, float mul)
{
	int sw0 = roi.x, sw1 = roi.x + roi.width;
	int sh0 = roi.y, sh1 = roi.y + roi.height;
	int dw0 = sw0 >> 1, dw1 = dw0 + roi.width;
	int dh0 = sh0 >> 1, dh1 = dh0 + roi.height;

	int lscrows = table[0].rows;
	int lsccols = table[0].cols;
	int rows = raw.rows / 2;
	int cols = raw.cols / 2;
	size_t pixel = lscrows * cols;
	AutoBuffer<double, 1> buffer(pixel * 4);
	array<Mat, 4> rowbuf;
	for (int c = 0; c < 4; ++c)
	{
		rowbuf[c] = Mat(lscrows, cols, CV_64FC1,
			buffer.data() + pixel * c);
	}
	lens_shade_sep_row(table, rowbuf, scols >> 1, dw0 >> 1, dw1 >> 1, sw0 >> 1, sw1 >> 1, mul);
	parallel_for_(Range(0, rows), LensShadeSepCol(rowbuf, raw, srows >> 1, dh0 >> 1, dh1 >> 1, sh0 >> 1, sh1 >> 1));
}


//==================== RAW2BGR ====================//

template<class Tp>
inline Tp set_interleave(int a, int b)
{
	typedef typename Tp::lane_type S;
	int const VSZ = Tp::nlanes;
	S CV_DECL_ALIGNED(32) buf[VSZ];
	for (int i = 0; i < VSZ; i += 2)
	{
		buf[i + 0] = static_cast<S>(a);
		buf[i + 1] = static_cast<S>(b);
	}
	return vx_load(buf);
}


static inline int clamp_ud(int v, int l1, int l2)
{
	return (l1 < l2) ? gxr_clamp(v, l1, l2) : gxr_clamp(v, l2, l1);
}


RAW2BGR::RAW2BGR()
	: old(1), start(1), ticktotal(0), framenum(0)
{
	szblk = Size(64, 64);
}


RAW2BGR::RAW2BGR(Size const& szorg, Size const& szroi, Size const& szlsc, Size const& szblk)
	: old(0), start(1), szorg(szorg), szroi(szroi), szlsc(szlsc), szblk(szblk)
{
	srows = szorg.height; scols = szorg.width;
	brows = ((srows - szroi.height) >> 1) + szroi.height; bcols = ((scols - szroi.width) >> 1) + szroi.width;
	hrows = brows >> 1; hcols = bcols >> 1;
	roi.width = szroi.width; roi.height = szroi.height;
	CV_Assert(szblk.height % 2 == 0 && szblk.width % 2 == 0);
	CV_Assert(2 <= szlsc.height && szlsc.height <= hrows);
	CV_Assert(2 <= szlsc.width && szlsc.width <= hcols);
	CV_Assert(srows % 4 == 0 && scols % 4 == 0);
	CV_Assert((brows % 4 == 0) && (bcols % 4 == 0));
	CV_Assert(brows >= 256 && bcols >= 256);
	CV_Assert(brows <= srows && bcols <= scols);
	szsta = Size((bcols + szblk.width - 1) / szblk.width, (brows + szblk.height - 1) / szblk.height);
	bgr.create(brows, bcols, CV_16SC3);
	awb_blockstats.create(szsta.height, szsta.width, CV_32SC3);
	ae_blockstats.create(szsta.height, szsta.width * 2, CV_32SC4);
	histgram.create(1, 256, CV_32SC1);
	gammatable.create(1, LINEARSAT + 1, CV_8UC1);
	for (int cn = 0; cn < 4; ++cn)
	{
		// lsctable: R, Gr, Gb, B
		lsctable[cn].create(szlsc.height, szlsc.width, CV_32FC1);
		float* Tab = lsctable[cn].ptr<float>();
		int lscsz = szlsc.height * szlsc.width;
		for (int w = 0; w < lscsz; w++)
			Tab[w] = 1.f;
	}
	lscflat.create(brows, bcols, CV_16SC1);
}


RAW2BGR::~RAW2BGR()
{}


void RAW2BGR::timer_step(char const* msg)
{
	ticksum += getTickCount() - tickprev;
	double curt = 1e3 * ticksum / getTickFrequency();
	if (verbose & VerbTimer)
		GXR_Printf("%2d: %9.3f | %s\n", ntick, curt, msg);
	tickprev = getTickCount();
	++ntick;
}


void RAW2BGR::run_all()
{
	prestats();

	poststats();
}


void RAW2BGR::prestats()
{
	rawgroup();
	timer_step("raw group");

	prepare();
	timer_step("prepare");

	TNR(this).run();
	timer_step("tnr");

	parallel_for_(Range(0, brows), LSC(this));
	timer_step("lsc");

	parallel_for_(Range(0, szsta.height), Stats(this));
	timer_step("stats");
}


void RAW2BGR::poststats()
{
	start = 0;

	if (!wben)
	{
		awb->feedstats(awb_blockstats);
		awb->estimate();
		awb->getwb(wbgain, wbgain + 3);
		awb->getccm(ccm);
		awb->getlsc(lsctable);
		if (verbose & VerbParam)
		{
			GXR_Printf("\testimate wbgain: %.3f,%.3f\n", wbgain[0], wbgain[3]);
			GXR_Printf("\testimate ccm:\n"
				"\t%.3f,%.3f,%.3f\n"
				"\t%.3f,%.3f,%.3f\n"
				"\t%.3f,%.3f,%.3f\n",
				ccm[0], ccm[1], ccm[2],
				ccm[3], ccm[4], ccm[5],
				ccm[6], ccm[7], ccm[8]);
			GXR_Printf("\testimate lsctable: %.3f,%.3f,%.3f,%.3f\n",
				*lsctable[0].ptr<float>(5, 6), *lsctable[1].ptr<float>(5, 6), *lsctable[2].ptr<float>(5, 6), *lsctable[3].ptr<float>(5, 6));
		}
		//FILE* fp = fopen("lsctable.bin", "wb");
		//fwrite(lsctable[0].ptr<float>(), 1, 17*13*sizeof(float), fp);
		//fwrite(lsctable[1].ptr<float>(), 1, 17*13*sizeof(float), fp);
		//fwrite(lsctable[2].ptr<float>(), 1, 17*13*sizeof(float), fp);
		//fwrite(lsctable[3].ptr<float>(), 1, 17*13*sizeof(float), fp);
		//fclose(fp);
		timer_step("awb");
	}

	if (!expen)
	{
		unsigned shutter_n;
		float totalgain_n;
		ae->feedstats(ae_blockstats, histgram, shutter, sensorgain * ispgain, 90);
		ae->estimate();
		ae->getexp(&shutter_n, &totalgain_n);
		ispgain = shutter_n * totalgain_n / (shutter * sensorgain);

		shutter = shutter_n;
		sensorgain = gxr_min(totalgain_n, 16.f);
		ispgain = totalgain_n / sensorgain;

		if (verbose & VerbParam)
		{
			GXR_Printf("\testimate shutter: %u\n", shutter_n);
			GXR_Printf("\testimate gain: %.3f\n", totalgain_n);
			GXR_Printf("\twill use ispgain: %.3f\n", ispgain);
		}
		timer_step("ae");
	}

	parallel_for_(Range(0, brows), Gain(this));
	timer_step("gain");

	GetColor getcolor(this);
	parallel_for_(Range(3, brows - 3), getcolor);
	getcolor.nextpass();
	parallel_for_(Range(1, brows - 1), getcolor);
	timer_step("getcolor");

	parallel_for_(Range(0, brows), CCMGamma(this));
	timer_step("ccm gamma");
#if GXR_CV_DEBUG
	imwrite("bgr.bmp", dst);
#endif

	parallel_for_(Range(0, hrows), BGR2YCbCr(this));
	timer_step("bgr2ycbcr");

	Mat hsrc((brows >> 2) << 1, (bcols >> 2) << 1, CV_8UC1);
	Mat hdst((brows >> 2) << 1, (bcols >> 2) << 1, CV_8UC1);
	Mat luma_dst(brows, bcols, CV_8UC1);
	Denoiser(ocl, luma, luma_dst).run();
	Downer(ocl, luma_dst, hsrc).run();
	Denoiser(ocl, hsrc, hdst).run();
	Uper(ocl, luma_dst, luma, hsrc, hdst).run();

	Mat chroma_hsrc((hrows >> 2) << 1, (hcols >> 2) << 1, CV_8UC2);
	Mat chroma_hdst((hrows >> 2) << 1, (hcols >> 2) << 1, CV_8UC2);
	Mat chroma_dst(hrows, hcols, CV_8UC2);
	Denoiser(ocl, chroma, chroma_dst).run();
	Downer(ocl, chroma_dst, chroma_hsrc).run();
	Denoiser(ocl, chroma_hsrc, chroma_hdst).run();
	Uper(ocl, chroma_dst, chroma, chroma_hsrc, chroma_hdst).run();

	YNR(this).run();
	timer_step("ynr sharpen");

	//ocl->run(luma.data, bgr.ptr<short>(brows / 2), brows, bcols);
	//memcpy(luma.data, bgr.ptr<short>(brows / 2), brows * bcols * sizeof(uchar));
	//timer_step("cl denoise");

	CNR(this).run();
	timer_step("cnr");

	parallel_for_(Range(0, hrows), YCbCr2BGR(this));
	timer_step("ycbcr2bgr");

	Ungroup(this).run();
	timer_step("ungroup");

	framenum++;
	ticktotal += ticksum;
	double curt = 1e3 * ticktotal / getTickFrequency();
	if (verbose & VerbTimer)
		GXR_Printf("%3d: %9.3f | mean time\n", framenum, curt / framenum);
}


void RAW2BGR::rawgroup()
{
	tickprev = getTickCount();
	ticksum = 0; ntick = 0;
	srows = raw.rows; scols = raw.cols;
	if (roi.x == 0 && roi.width == scols && roi.y == 0 && roi.height == srows)
		return;
	CV_Assert(roi.y >= 0 && roi.x >= 0);
	CV_Assert(roi.y % 32 == 0 && roi.x % 32 == 0);
	CV_Assert((roi.height % 16 == 0 || roi.y + roi.height == srows) && (roi.width % 16 == 0 || roi.x + roi.width == scols));
	CV_Assert(roi.y + roi.height <= srows && roi.x + roi.width <= scols);
	RawGroup(raw, roi, bgr).run();



	//tickprev = getTickCount();
	//ticksum = 0; ntick = 0;
	//srows = raw.rows; scols = raw.cols;
	//if (roi.width == scols && roi.height == srows)
	//	return;
	//CV_Assert(roi.width % 4 == 0 && roi.height % 4 == 0);
	//RawScaler(raw, Size(roi.width, roi.height), bgr, rpat).run();
}


void RAW2BGR::prepare()
{
	if (old)
	{
		awb->verbose = verbose & VerbParam;
		brows = raw.rows; bcols = raw.cols;
		hrows = brows >> 1; hcols = bcols >> 1;
		szsta = Size((bcols + szblk.width - 1) / szblk.width, (brows + szblk.height - 1) / szblk.height);
		CV_Assert((brows % 2 == 0) && (bcols % 2 == 0));
		CV_Assert(2 <= lsctable[0].rows && lsctable[0].rows <= hrows);
		CV_Assert(2 <= lsctable[0].cols && lsctable[0].cols <= hcols);
		pre.create(brows, bcols, CV_16SC1);
		bgr.create(brows, bcols, CV_16SC3);
		dst = Mat(brows, bcols, CV_8UC3, dst.data);
		lscflat.create(raw.size(), CV_16SC1);
		awb_blockstats.create(szsta.height, szsta.width, CV_32SC3);
		ae_blockstats.create(szsta.height, szsta.width * 2, CV_32SC4);
		histgram.create(1, 256, CV_32SC1);
	}

	//CV_Assert(depth >= 10 && depth < 16);
	for (int i = 0; i < 4; ++i)
		blacklevel[i] <<= (LINEARBIT - 10);

	switch (rpat)
	{
	case Raw_RGGB: cfa = Vec4i(0, 1, 2, 3); break;
	case Raw_BGGR: cfa = Vec4i(3, 2, 1, 0); break;
	case Raw_GRBG: cfa = Vec4i(1, 0, 3, 2); break;
	case Raw_GBRG: cfa = Vec4i(2, 3, 0, 1); break;
	default: CV_Error(Error::BadOrder, "wrong bayer CFA type");
	}
	for (int i = 0; i < 4; ++i)
	{
		wbcfa[i] = wbgain[cfa[i]];
		darkcfa[i] = blacklevel[cfa[i]];
	}

	array<Mat, 4> tab;
	int sat = ((1 << depth) - 1) << (LINEARBIT - depth);
	float ratio = ispgain * LINEARSAT / (sat - blacklevel[1]);
	for (int i = 0; i < 4; ++i)
		tab[i] = lsctable[cfa[i]];
	lens_shade_resize(tab, lscflat, roi, srows, scols, ratio);
}


//==================== TNR ====================//

//RAW2BGR::TNR::TNR(RAW2BGR* r)
//	: raw2(r), cur(r->raw), pre(r->pre), firstframe(r->start)
//{
//	pat = RawFilter[raw2->rpat];
//	brows = cur.rows;
//	bcols = cur.cols;
//	hrows = brows >> 1;
//	hcols = bcols >> 1;
//}
//
//
//void RAW2BGR::TNR::run()
//{
//	SpaFilter(cur).run();
//
//
//	//Mat lumi(hrows, hcols, CV_16SC1);
//	//Mat lumi_mean(hrows, hcols, CV_16SC1);
//	//Mat lumi_mae(hrows, hcols, CV_8UC1);
//	//for (int h = 0; h < brows; h += 2)
//	//{
//	//	int weight = pucMAE_LUT[a_puwMAEmap[sepchid]];
//	//	udTh = aTh_G[meanBayer[outid]] * weight >> 4;
//	//	udTh = clip(udTh, 0, 127);
//	//	udN = aN[meanBayer[outid] >> 8];
//	//	udN = clip(udN, 0, 8);
//
//	//	wDiff = a_puwImg[y[i] * ((INT32)a_udWidth) + x[i]] - *a_puwImg;
//	//	dWeight = (INT32)(a_pudAltekThY_L)+dN - abs(wDiff);
//	//	dWeight = max(dWeight, 0);
//	//	dWeight = min(dWeight, dN);
//	//	dSum += ((dWeight * wDiff) >> (a_pudAltekThY_H));
//	//}
//}


//==================== LSC ====================//

RAW2BGR::LSC::LSC(RAW2BGR* r)
	:raw2(r), rows(r->brows), cols(r->bcols), 
	lscflat(r->lscflat), raw(r->raw)
{
	slli = LINEARBIT - raw2->depth;
	pat = RawFilter[raw2->rpat];
	for (int i = 0; i < 4; ++i)
		kcfa[i] = raw2->darkcfa[i];
}


void RAW2BGR::LSC::operator()(Range const& range) const
{
	int const rlo = 0;
	int const rhi = LINEARSAT;
	int const bit = LSCBIT;
	int const VSZ = v_int16::nlanes;
	v_int16 vlo = vx_setall_s16(rlo);
	v_int16 vhi = vx_setall_s16(rhi);
	int rk[2], wstop = cols - VSZ;

	for (int h = range.start; h < range.end; ++h)
	{
		int w = (h & 1) << 1;
		rk[0] = kcfa[w];
		rk[1] = kcfa[w + 1];
		short const* L = lscflat.ptr<short>(h);
		short* S = raw.ptr<short>(h);
		v_int32 m0, m1;
		v_int16 vk = set_interleave<v_int16>(rk[0], rk[1]);
		w = 0;
		for (; w <= wstop; w += VSZ)
		{
			v_int16 v0 = (vx_load(S + w) << slli) - vk;
			v_mul_expand(v0, vx_load(L + w), m0, m1);
			v0 = v_pack(v_shr<bit>(m0), v_shr<bit>(m1));
			vx_store(S + w, v_min(v_max(v0, vlo), vhi));
		}
		for (; w < cols; w += 2)
		{
			int r0 = (((S[w + 0] << slli) - rk[0]) * L[w + 0]) >> bit;
			int r1 = (((S[w + 1] << slli) - rk[1]) * L[w + 1]) >> bit;
			S[w + 0] = static_cast<short>(gxr_clamp(r0, rlo, rhi));
			S[w + 1] = static_cast<short>(gxr_clamp(r1, rlo, rhi));
		}
	}
}


//==================== Stats ====================//

#define AESTATS 1
RAW2BGR::Stats::Stats(RAW2BGR* r)
	:raw2(r), aesat(r->aesat), rows(r->brows), cols(r->bcols), raw(r->raw), 
	awb_blockstats(r->awb_blockstats), ae_blockstats(r->ae_blockstats), histgram(r->histgram)
{
	pat = RawFilter[raw2->rpat];
	switch (raw2->rpat)
	{
	case Raw_RGGB: cfa = Vec4i(0, 1, 2, 3); break;
	case Raw_BGGR: cfa = Vec4i(3, 2, 1, 0); break;
	case Raw_GRBG: cfa = Vec4i(1, 0, 3, 2); break;
	case Raw_GBRG: cfa = Vec4i(2, 3, 0, 1); break;
	default: CV_Error(Error::BadOrder, "wrong bayer CFA type");
	}
	blksz_y = raw2->szblk.height;
	blksz_x = raw2->szblk.width;
	blknum_y = raw2->szsta.height;
	blknum_x = raw2->szsta.width;		
	memset(awb_blockstats.data, 0, awb_blockstats.rows * awb_blockstats.step[0]);
	memset(ae_blockstats.data, 0, ae_blockstats.rows * ae_blockstats.step[0]);
	memset(histgram.data, 0, histgram.rows * histgram.step[0]);

#if AESTATS
	int* Hist = histgram.ptr<int>();
	int const ShitBit = LINEARBIT - 8 + 2;
	int const VSZ = v_int16::nlanes;
	int const VSZ2 = VSZ * 2;
	v_int16 v00, v01, v10, v11;
	short CV_DECL_ALIGNED(32) buf[VSZ];
	for (int h = 0; h < rows; h += 2)
	{
		short* S0 = raw.ptr<short>(h);
		short* S1 = raw.ptr<short>(h + 1);
		int w = 0, wlimit = cols - VSZ2;
		for (; w <= wlimit; w += VSZ2)
		{
			v_load_deinterleave(S0, v00, v01);
			v_load_deinterleave(S1, v10, v11);
			v_store_aligned(buf, (v00 + v01 + v10 + v11) >> ShitBit);
			Hist[buf[0]]++; Hist[buf[1]]++; Hist[buf[2]]++; Hist[buf[3]]++;
			Hist[buf[4]]++; Hist[buf[5]]++; Hist[buf[6]]++; Hist[buf[7]]++;
			Hist[buf[8]]++; Hist[buf[9]]++; Hist[buf[10]]++; Hist[buf[11]]++;
			Hist[buf[12]]++; Hist[buf[13]]++; Hist[buf[14]]++; Hist[buf[15]]++;
			S0 += VSZ2; S1 += VSZ2;
		}
		for (; w < cols; w += 2)
		{
			Hist[(S0[0] + S0[1] + S1[0] + S1[1]) >> ShitBit]++;
			S0 += 2; S1 += 2;
		}
	}
	//{
	//	Mat viz(256, 256, CV_8UC1);
	//	uchar* Viz = viz.data;
	//	memset(Viz, 0, 256 * 256);
	//	double inv = 255.0 / (rows * cols / 4);
	//	for (int i = 0; i < 256; i++)
	//	{
	//		int d = Hist[i] * inv;
	//		for (int j = 0; j <= d; j++)
	//			Viz[((255 - j) << 8) + i] = 255;
	//	}
	//	imwrite("hist.bmp", viz);
	//}
#endif
}


void RAW2BGR::Stats::operator()(Range const& range) const
{
#define SATSKIP 0
	int const VSZ = v_int16::nlanes * 2, shift = LINEARBIT - 10;
	v_int16 v0, v1, vsat = vx_setall_s16(aesat), vzero = vx_setall_s16(0), vone = vx_setall_s16(1), vm0, vm1, vt0, vt1;
	for (int ih = range.start; ih < range.end; ih++)
	{
		int h = ih * blksz_y, hstop = gxr_min(h + blksz_y, rows);
		for (; h < hstop; h++)
		{
			int i0 = fcol(h, 0, pat);
			int i1 = fcol(h, 1, pat);
			int c0 = cfa[(h & 1) << 1];
			int c1 = cfa[((h & 1) << 1) + 1];
			int c2 = c0 + 4;
			int c3 = c1 + 4;
			short* S = raw.ptr<short>(h);
			int* Sum = awb_blockstats.ptr<int>(ih);
			int* Sum_ae = ae_blockstats.ptr<int>(ih);
			for (int iw = 0; iw < blknum_x; iw++)
			{
				int w = iw * blksz_x, wstop = gxr_min(w + blksz_x, cols), wlimit = wstop - VSZ;
				int sum0 = 0, sum1 = 0;
				int sum0_ae = 0, sum1_ae = 0;
				int nonsatn0 = 0, nonsatn1 = 0;
				int s0, s1;
				for (; w <= wlimit; w += VSZ)
				{
					v_load_deinterleave(S + w, v0, v1);
					sum0 += v_reduce_sum(v0); sum1 += v_reduce_sum(v1);
#if AESTATS
					vm0 = v0 < vsat; vm1 = v1 < vsat;
					vt0 = v_select(vm0, v0, vzero); vt1 = v_select(vm1, v1, vzero);
					sum0_ae += v_reduce_sum(vt0); sum1_ae += v_reduce_sum(vt1);
					vt0 = v_select(vm0, vone, vzero); vt1 = v_select(vm1, vone, vzero);
					nonsatn0 += v_reduce_sum(vt0); nonsatn1 += v_reduce_sum(vt1);
#endif
				}
				for (; w < wstop; w += 2)
				{
					s0 = S[w + 0]; s1 = S[w + 1];
					sum0 += s0; sum1 += s1;
#if AESTATS
					s0 < aesat ? sum0_ae += s0, nonsatn0 += 1 : SATSKIP;
					s1 < aesat ? sum1_ae += s1, nonsatn1 += 1 : SATSKIP;
#endif
				}
				Sum[i0] += sum0; Sum[i1] += sum1;
#if AESTATS
				Sum_ae[c0] += (sum0_ae >> shift); Sum_ae[c1] += (sum1_ae >> shift);
				Sum_ae[c2] += nonsatn0; Sum_ae[c3] += nonsatn1;
#endif
				//if (h == hstop - 1)
				//{
				//	printf("%d %d %d, %d %d %d %d, %d %d %d %d\n", Sum[0], Sum[1], Sum[2],
				//		Sum_ae[0], Sum_ae[1], Sum_ae[2], Sum_ae[3], Sum_ae[4], Sum_ae[5], Sum_ae[6], Sum_ae[7]);
				//	//CV_Assert(Sum[0] == Sum_ae[3] && Sum[2] == Sum_ae[0] && (Sum[1] == Sum_ae[1] + Sum_ae[2]));
				//	//CV_Assert(Sum_ae[4] == Sum_ae[5] && Sum_ae[5] == Sum_ae[6] && Sum_ae[6] == Sum_ae[7] && Sum_ae[7] == ((wstop - iw * blksz_x) * (hstop - ih * blksz_y) / 4));
				//}
				Sum += 3; Sum_ae += 8;
			}
		}
	}
#undef SATSKIP
}
#undef AESTATS


//==================== Gain ====================//

RAW2BGR::Gain::Gain(RAW2BGR* r)
	:raw2(r), rows(r->brows), cols(r->bcols), raw(r->raw), bgr(r->bgr)
{
	pat = RawFilter[raw2->rpat];
	for (int i = 0; i < 4; ++i)
	{
		raw2->wbcfa[i] = raw2->wbgain[raw2->cfa[i]];
		gcfa[i] = saturate_cast<short>(raw2->wbcfa[i] * LSCVAL);
	}

}


void RAW2BGR::Gain::operator()(Range const& range) const
{
	int const rlo = 0;
	int const rhi = LINEARSAT;
	int const bit = LSCBIT;
	int const VSZ = v_int16::nlanes;
	v_int16 vlo = vx_setall_s16(rlo);
	v_int16 vhi = vx_setall_s16(rhi);
	int g[2], i0, i1, wstop = cols - VSZ;

	for (int h = range.start; h < range.end; ++h)
	{
		int w = (h & 1) << 1;
		g[0] = gcfa[w];
		g[1] = gcfa[w + 1];
		i0 = fcol(h, 0, pat);
		i1 = fcol(h, 1, pat) + 3;
		short* S = raw.ptr<short>(h);
		short* C = bgr.ptr<short>(h);
		v_int32 m0, m1;
		v_int16 vg = set_interleave<v_int16>(g[0], g[1]);
		for (w = 0; w <= wstop; w += VSZ)
		{
			v_int16 v0 = vx_load(S + w);
			v_mul_expand(v0, vg, m0, m1);
			v0 = v_pack(v_shr<bit>(m0), v_shr<bit>(m1));
			vx_store(S + w, v_min(v_max(v0, vlo), vhi));
		}
		for (; w < cols; w += 2)
		{
			int r0 = (S[w + 0] * g[0]) >> bit;
			int r1 = (S[w + 1] * g[1]) >> bit;
			S[w + 0] = static_cast<short>(gxr_clamp(r0, rlo, rhi));
			S[w + 1] = static_cast<short>(gxr_clamp(r1, rlo, rhi));
		}
		for (w = 0; w < cols; w += 2)
		{
			C[i0] = S[w];
			C[i1] = S[w + 1];
			C += 6;
		}
	}
}


//==================== GetColor ====================//

static void border_demosaic(Mat& raw3, unsigned pat, unsigned border)
{
	unsigned const height = raw3.rows;
	unsigned const width = raw3.cols;
	int S[8];

	for (unsigned h = 0; h < height; ++h)
	{
		Vec3s* B = raw3.ptr<Vec3s>(h);
		for (unsigned w = 0; w < width; ++w)
		{
			if ((w == border) && (h >= border) && (h < height - border))
				w = width - border;
			memset(S, 0, sizeof(S));
			for (unsigned y = h - 1; y != h + 2; ++y)
			{
				if (y >= height) continue;
				Vec3s const* P = raw3.ptr<Vec3s>(y);
				for (unsigned x = w - 1; x != w + 2; ++x)
				{
					if (x >= width) continue;
					int f = fcol(y, x, pat);
					S[f] += P[x][f];
					++(S[f + 4]);
				}
			}
			int f = fcol(h, w, pat);
			for (int c = 0; c < 3; ++c)
				if ((c != f) && S[c + 4])
					B[w][c] = static_cast<short>((S[c] + (S[c + 4] >> 1)) / S[c + 4]);
		}
	}
}


RAW2BGR::GetColor::GetColor(RAW2BGR* r)
	:raw2(r), rows(r->brows), cols(r->bcols), bgr(r->bgr)
{
	pat = RawFilter[raw2->rpat];
	border_demosaic(bgr, pat, 3);
	pass = 0;
}


void RAW2BGR::GetColor::nextpass()
{
	pass++;
}


void RAW2BGR::GetColor::operator()(Range const& range) const
{
	if (pass == 0)
	{
		int wstop = cols - 3;
		int dir[5] = { 1, cols, -1, -cols, 1 };
		int diff[2], guess[2];
		Vec3s* line, * P;
		for (int h = range.start; h < range.end; ++h)
		{
			line = bgr.ptr<Vec3s>(h);
			int w = 3 + (fcol(h, 3, pat) & 1);
			int c = fcol(h, w, pat);

			short* rline = raw2->raw.ptr<short>(h) - 3, * R;
			int const VSZ = v_int16::nlanes;
			int const VSZ2 = VSZ << 1;
			short CV_DECL_ALIGNED(32) buf[VSZ];
			int wlimit = wstop - VSZ2;
			int cols2 = cols + cols, cols3 = cols2 + cols;
			v_int16 v0, v1, v2, v3, v_1, v_2, v_3;
			v_int16 vdiff[2], vguess[2], vmn[2], vmx[2], vres;
			v_int16 vc3 = vx_setall_s16(3), vz = vx_setall_s16(0);
			for (; w <= wlimit; w += VSZ2)
			{
				R = rline + w;
				P = line + w;

				v_load_deinterleave(R, v_3, v_2);
				v_load_deinterleave(R + 2, v_1, v0);
				v_load_deinterleave(R + 4, v1, v2);
				v_load_deinterleave(R + 6, v3, vres);
				vguess[0] = (((v_1 + v0 + v1) << 1) - (v_2 + v2)) >> 2;
				vdiff[0] = vc3 * (v_absdiffs(v_2, v0) +
					v_absdiffs(v2, v0) +
					v_absdiffs(v_1, v1))
					+ ((v_absdiffs(v3, v1) +
						v_absdiffs(v_3, v_1)) << 1);
				vmn[0] = v_min(v_1, v1);
				vmx[0] = v_max(v_1, v1);

				R = R + 3;
				v_load_deinterleave(R - cols3, v_3, vres);
				v_load_deinterleave(R - cols2, v_2, vres);
				v_load_deinterleave(R - cols, v_1, vres);
				v_load_deinterleave(R + cols, v1, vres);
				v_load_deinterleave(R + cols2, v2, vres);
				v_load_deinterleave(R + cols3, v3, vres);
				vguess[1] = (((v_1 + v0 + v1) << 1) - (v_2 + v2)) >> 2;
				vdiff[1] = vc3 * (v_absdiffs(v_2, v0) +
					v_absdiffs(v2, v0) +
					v_absdiffs(v_1, v1))
					+ ((v_absdiffs(v3, v1) +
						v_absdiffs(v_3, v_1)) << 1);
				vmn[1] = v_min(v_1, v1);
				vmx[1] = v_max(v_1, v1);

				vres = v_select(vdiff[0] > vdiff[1], vguess[1], vguess[0]);
				vres = v_min(v_max(vres, v_select(vdiff[0] > vdiff[1], vmn[1], vmn[0])), v_select(vdiff[0] > vdiff[1], vmx[1], vmx[0]));
				vx_store(buf, vres);
				R = &P[0][1];
				R[0] = buf[0]; R[6] = buf[1]; R[12] = buf[2]; R[18] = buf[3]; R[24] = buf[4]; R[30] = buf[5]; R[36] = buf[6]; R[42] = buf[7];
				R[48] = buf[8]; R[54] = buf[9]; R[60] = buf[10]; R[66] = buf[11]; R[72] = buf[12]; R[78] = buf[13]; R[84] = buf[14]; R[90] = buf[15];
			}

			for (; w < wstop; w += 2)
			{
				P = line + w;
				guess[0]
					= 2 * (P[-1][1] + P[0][c] + P[1][1])
					- P[-2][c] - P[2][c];
				diff[0]
					= 3 * (abs(P[-2][c] - P[0][c])
						+ abs(P[2][c] - P[0][c])
						+ abs(P[-1][1] - P[1][1]))
					+ 2 * (abs(P[3][1] - P[1][1])
						+ abs(P[-3][1] - P[-1][1]));
				guess[1]
					= 2 * (P[-cols][1] + P[0][c] + P[cols][1])
					- P[-cols2][c] - P[cols2][c];
				diff[1]
					= 3 * (abs(P[-cols2][c] - P[0][c])
						+ abs(P[cols2][c] - P[0][c])
						+ abs(P[-cols][1] - P[cols][1]))
					+ 2 * (abs(P[cols3][1] - P[cols][1])
						+ abs(P[-cols3][1] - P[-cols][1]));
				P[0][1] = static_cast<short>(diff[0] > diff[1] ? clamp_ud((guess[1]) / 4, P[cols][1], P[-cols][1]) : clamp_ud((guess[0]) / 4, P[1][1], P[-1][1]));
			}
		}
	}
	else
	{
		int wstop = cols - 1;
		int const dir[5] = { 1, cols, -1, -cols, 1 };
		int diff[2], guess[2];
		Vec3s* line, * P;
		for (int h = range.start; h < range.end; ++h)
		{
			line = bgr.ptr<Vec3s>(h);
			int w = 1 + (fcol(h, 2, pat) & 1), c = fcol(h, w + 1, pat), cc = 2 - c, s = 3 - w - w;
			for (; w < wstop; w += 2)
			{
				P = line + w;
				int d, cur, cur2 = P[0][1] << 1;

				d = dir[0];
				d = (P[-d][c] + P[d][c] + cur2
					- P[-d][1] - P[d][1]) >> 1;
				P[0][c] = gxr_clamp<short>(d, 0, LINEARSAT);

				d = dir[1];
				d = (P[-d][cc] + P[d][cc] + cur2
					- P[-d][1] - P[d][1]) >> 1;
				P[0][cc] = gxr_clamp<short>(d, 0, LINEARSAT);

				P += s;
				cur = P[0][1], cur2 = cur << 1;
				d = dir[0] + dir[1];
				diff[0] = abs(P[-d][cc] - P[d][cc])
					+ abs(P[-d][1] - cur) + abs(P[d][1] - cur);
				guess[0] = P[-d][cc] + P[d][cc] + cur2 - P[-d][1] - P[d][1];

				d = dir[1] + dir[2];
				diff[1] = abs(P[-d][cc] - P[d][cc])
					+ abs(P[-d][1] - cur) + abs(P[d][1] - cur);
				guess[1] = P[-d][cc] + P[d][cc] + cur2 - P[-d][1] - P[d][1];

				if (diff[0] != diff[1])
					P[0][cc] = gxr_clamp<short>((guess[diff[0] > diff[1]]) >> 1, 0, LINEARSAT);
				else
					P[0][cc] = gxr_clamp<short>((guess[0] + guess[1]) >> 2, 0, LINEARSAT);
			}
		}
	}
}


//==================== CCMGamma ====================//

RAW2BGR::CCMGamma::CCMGamma(RAW2BGR* r)
	: raw2(r)
{
	rows = raw2->brows;
	cols = raw2->bcols * 3;
	T = raw2->gammatable.ptr<uchar>(0);
	for (int i = 0; i < 9; i++)
		ccm[i] = gxr_clamp<int>(raw2->ccm[i] * 4096 + 0.5f, -16383, 16383);
}


void RAW2BGR::CCMGamma::operator()(Range const& range) const
{
	short* S = raw2->bgr.ptr<short>(range.start);
	short* E = S + ((size_t)range.end - (size_t)range.start) * raw2->bgr.cols * raw2->bgr.channels();
	uchar* D = raw2->dst.ptr<uchar>(range.start);

	int const VSZ = 3 * v_int16::nlanes;
	short* limit = E - VSZ;
	v_int16 lo = vx_setall_s16(0);
	v_int16 hi = vx_setall_s16(LINEARSAT);
	v_int16 ccm00 = vx_setall_s16(ccm[0]), ccm01 = vx_setall_s16(ccm[1]), ccm02 = vx_setall_s16(ccm[2]);
	v_int16 ccm10 = vx_setall_s16(ccm[3]), ccm11 = vx_setall_s16(ccm[4]), ccm12 = vx_setall_s16(ccm[5]);
	v_int16 ccm20 = vx_setall_s16(ccm[6]), ccm21 = vx_setall_s16(ccm[7]), ccm22 = vx_setall_s16(ccm[8]);
	short CV_DECL_ALIGNED(32) buf[VSZ];
	while (S <= limit)
	{
		v_int16 bs, gs, rs;
		v_int32 bd, gd, rd;
		v_int32 bd_, gd_, rd_;
		v_load_deinterleave(S, bs, gs, rs);

		v_mul_expand(rs, ccm00, rd, rd_);
		v_mul_expand(gs, ccm01, gd, gd_);
		v_mul_expand(bs, ccm02, bd, bd_);
		v_int16 r = v_min(v_max(v_pack((rd + gd + bd) >> 12, (rd_ + gd_ + bd_) >> 12), lo), hi);

		v_mul_expand(rs, ccm10, rd, rd_);
		v_mul_expand(gs, ccm11, gd, gd_);
		v_mul_expand(bs, ccm12, bd, bd_);
		v_int16 g = v_min(v_max(v_pack((rd + gd + bd) >> 12, (rd_ + gd_ + bd_) >> 12), lo), hi);

		v_mul_expand(rs, ccm20, rd, rd_);
		v_mul_expand(gs, ccm21, gd, gd_);
		v_mul_expand(bs, ccm22, bd, bd_);
		v_int16 b = v_min(v_max(v_pack((rd + gd + bd) >> 12, (rd_ + gd_ + bd_) >> 12), lo), hi);

		v_store_interleave(buf, b, g, r);
		for (int i = 0; i < VSZ; i++)
			D[i] = T[buf[i]];
		S += VSZ; D += VSZ;
	}

	while (S < E)
	{
		short b = S[0], g = S[1], r = S[2];
		int r_ = gxr_clamp<int>(r * raw2->ccm[0] + g * raw2->ccm[1] + b * raw2->ccm[2], 0, LINEARSAT);
		int g_ = gxr_clamp<int>(r * raw2->ccm[3] + g * raw2->ccm[4] + b * raw2->ccm[5], 0, LINEARSAT);
		int b_ = gxr_clamp<int>(r * raw2->ccm[6] + g * raw2->ccm[7] + b * raw2->ccm[8], 0, LINEARSAT);
		D[0] = T[b_]; D[1] = T[g_]; D[2] = T[r_];
		S += 3; D += 3;
	}
}


//==================== BGR2YCbCr ====================//

RAW2BGR::BGR2YCbCr::BGR2YCbCr(RAW2BGR* r)
	: raw2(r), bgr(r->dst), luma(r->luma), chroma(r->chroma)
{
	brows = raw2->brows; bcols = raw2->bcols;
	hrows = brows / 2; hcols = bcols / 2;
	luma = Mat(brows, bcols, CV_8UC1, raw2->bgr.ptr<short>(0));
	chroma = Mat(hrows, hcols, CV_8UC2, raw2->bgr.ptr<short>(brows / 4));

	y0 = 0.114f; y0s = cvRound(y0 * LSCVAL);
	y1 = 0.587f; y1s = cvRound(y1 * LSCVAL);
	y2 = 0.299f; y2s = cvRound(y2 * LSCVAL);
	cb0 = 0.564f; cb0s = cvRound(cb0 * LSCVAL);
	cr0 = 0.713f; cr0s = cvRound(cr0 * LSCVAL);
}


void RAW2BGR::BGR2YCbCr::operator()(Range const& range) const
{
	int hi = GAMMASAT;
	int half = (hi + 1) / 2;
	int const VSZ = v_uint8::nlanes;
	int const VSZHALF = VSZ / 2;
	int const VSZ3 = VSZ * 3;
	v_int16 vhi = vx_setall_s16(hi);
	v_int16 vhalf = vx_setall_s16(half);
	v_int32 vcoefhalf = vx_setall_s32(LSCVAL >> 1);
	v_int16 vy0s = vx_setall_s16(y0s);
	v_int16 vy1s = vx_setall_s16(y1s);
	v_int16 vy2s = vx_setall_s16(y2s);
	v_int16 vcb0s = vx_setall_s16(cb0s);
	v_int16 vcr0s = vx_setall_s16(cr0s);
	short CV_DECL_ALIGNED(32) vbuf_cb_row0[VSZ];
	short CV_DECL_ALIGNED(32) vbuf_cb_row1[VSZ];
	short CV_DECL_ALIGNED(32) vbuf_cr_row0[VSZ];
	short CV_DECL_ALIGNED(32) vbuf_cr_row1[VSZ];
	short* vbuf_cblow_row0 = vbuf_cb_row0, *vbuf_cbhigh_row0 = vbuf_cb_row0 + VSZHALF;
	short* vbuf_crlow_row0 = vbuf_cr_row0, *vbuf_crhigh_row0 = vbuf_cr_row0 + VSZHALF;
	short* vbuf_cblow_row1 = vbuf_cb_row1, * vbuf_cbhigh_row1 = vbuf_cb_row1 + VSZHALF;
	short* vbuf_crlow_row1 = vbuf_cr_row1, * vbuf_crhigh_row1 = vbuf_cr_row1 + VSZHALF;
	for (int h = range.start; h < range.end; h++)
	{
		int h2 = (h << 1);
		uchar* BGR0 = bgr.ptr<uchar>(h2);
		uchar* BGR1 = bgr.ptr<uchar>(h2 + 1);
		uchar* Y0 = luma.ptr<uchar>(h2);
		uchar* Y1 = luma.ptr<uchar>(h2 + 1);
		uchar* CbCr = chroma.ptr<uchar>(h);
		int w = 0;
		float b, g, r;
		float y, cb, cr;

		int wstop = bcols - VSZ;
		v_uint8 vb_, vg_, vr_;
		v_int16 vb, vg, vr, vtmp;
		v_int16 vy, vcb, vcr;
		v_int32 vlo0, vhi0;
		v_int32 vlo1, vhi1;
		v_int32 vlo2, vhi2;

		for (; w <= wstop; w += VSZ)
		{
			v_load_deinterleave(BGR0, vb_, vg_, vr_);
			vb = v_reinterpret_as_s16(v_expand_low(vb_));
			vg = v_reinterpret_as_s16(v_expand_low(vg_));
			vr = v_reinterpret_as_s16(v_expand_low(vr_));
			v_mul_expand(vy0s, vb, vlo0, vhi0);
			v_mul_expand(vy1s, vg, vlo1, vhi1);
			v_mul_expand(vy2s, vr, vlo2, vhi2);
			vy = v_pack((vlo0 + vlo1 + vlo2 + vcoefhalf) >> LSCBIT, (vhi0 + vhi1 + vhi2 + vcoefhalf) >> LSCBIT);
			v_pack_u_store(Y0, vy);
			vtmp = (vb - vy);
			v_mul_expand(vtmp, vcb0s, vlo0, vhi0);
			vcb = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vx_store(vbuf_cblow_row0, vcb);
			vtmp = (vr - vy);
			v_mul_expand(vtmp, vcr0s, vlo0, vhi0);
			vcr = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vx_store(vbuf_crlow_row0, vcr);

			vb = v_reinterpret_as_s16(v_expand_high(vb_));
			vg = v_reinterpret_as_s16(v_expand_high(vg_));
			vr = v_reinterpret_as_s16(v_expand_high(vr_));
			v_mul_expand(vy0s, vb, vlo0, vhi0);
			v_mul_expand(vy1s, vg, vlo1, vhi1);
			v_mul_expand(vy2s, vr, vlo2, vhi2);
			vy = v_pack((vlo0 + vlo1 + vlo2 + vcoefhalf) >> LSCBIT, (vhi0 + vhi1 + vhi2 + vcoefhalf) >> LSCBIT);
			v_pack_u_store(Y0 + VSZHALF, vy);
			vtmp = (vb - vy);
			v_mul_expand(vtmp, vcb0s, vlo0, vhi0);
			vcb = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vx_store(vbuf_cbhigh_row0, vcb);
			vtmp = (vr - vy);
			v_mul_expand(vtmp, vcr0s, vlo0, vhi0);
			vcr = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vx_store(vbuf_crhigh_row0, vcr);



			v_load_deinterleave(BGR1, vb_, vg_, vr_);
			vb = v_reinterpret_as_s16(v_expand_low(vb_));
			vg = v_reinterpret_as_s16(v_expand_low(vg_));
			vr = v_reinterpret_as_s16(v_expand_low(vr_));
			v_mul_expand(vy0s, vb, vlo0, vhi0);
			v_mul_expand(vy1s, vg, vlo1, vhi1);
			v_mul_expand(vy2s, vr, vlo2, vhi2);
			vy = v_pack((vlo0 + vlo1 + vlo2 + vcoefhalf) >> LSCBIT, (vhi0 + vhi1 + vhi2 + vcoefhalf) >> LSCBIT);
			v_pack_u_store(Y1, vy);
			vtmp = (vb - vy);
			v_mul_expand(vtmp, vcb0s, vlo0, vhi0);
			vcb = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vx_store(vbuf_cblow_row1, vcb);
			vtmp = (vr - vy);
			v_mul_expand(vtmp, vcr0s, vlo0, vhi0);
			vcr = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vx_store(vbuf_crlow_row1, vcr);

			vb = v_reinterpret_as_s16(v_expand_high(vb_));
			vg = v_reinterpret_as_s16(v_expand_high(vg_));
			vr = v_reinterpret_as_s16(v_expand_high(vr_));
			v_mul_expand(vy0s, vb, vlo0, vhi0);
			v_mul_expand(vy1s, vg, vlo1, vhi1);
			v_mul_expand(vy2s, vr, vlo2, vhi2);
			vy = v_pack((vlo0 + vlo1 + vlo2 + vcoefhalf) >> LSCBIT, (vhi0 + vhi1 + vhi2 + vcoefhalf) >> LSCBIT);
			v_pack_u_store(Y1 + VSZHALF, vy);
			vtmp = (vb - vy);
			v_mul_expand(vtmp, vcb0s, vlo0, vhi0);
			vcb = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vx_store(vbuf_cbhigh_row1, vcb);
			vtmp = (vr - vy);
			v_mul_expand(vtmp, vcr0s, vlo0, vhi0);
			vcr = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vx_store(vbuf_crhigh_row1, vcr);



			v_load_deinterleave(vbuf_cb_row0, vb, vg);
			v_load_deinterleave(vbuf_cb_row1, vr, vcb);
			vcb = ((vb + vg + vr + vcb) >> 2) + vhalf;
			v_load_deinterleave(vbuf_cr_row0, vb, vg);
			v_load_deinterleave(vbuf_cr_row1, vr, vcr);
			vcr = ((vb + vg + vr + vcr) >> 2) + vhalf;
			v_store_interleave(vbuf_cb_row0, vcb, vcr);
			vx_store(CbCr, v_pack_u(vx_load(vbuf_cblow_row0), vx_load(vbuf_cbhigh_row0)));
			BGR0 += VSZ3; Y0 += VSZ;
			BGR1 += VSZ3; Y1 += VSZ;
			CbCr += VSZ;
		}

		for (; w < bcols; w += 2)
		{
			b = BGR0[0];
			g = BGR0[1];
			r = BGR0[2];
			y = y0 * b + y1 * g + y2 * r;
			Y0[0] = gxr_min<int>(cvRound(y), hi);
			cb = (b - y) * cb0;
			cr = (r - y) * cr0;

			b = BGR1[0];
			g = BGR1[1];
			r = BGR1[2];
			y = y0 * b + y1 * g + y2 * r;
			Y1[0] = gxr_min<int>(cvRound(y), hi);
			cb += (b - y) * cb0;
			cr += (r - y) * cr0;

			BGR0 += 3; BGR1 += 3;
			Y0 += 1; Y1 += 1;

			b = BGR0[0];
			g = BGR0[1];
			r = BGR0[2];
			y = y0 * b + y1 * g + y2 * r;
			Y0[0] = gxr_min<int>(cvRound(y), hi);
			cb += (b - y) * cb0;
			cr += (r - y) * cr0;

			b = BGR1[0];
			g = BGR1[1];
			r = BGR1[2];
			y = y0 * b + y1 * g + y2 * r;
			Y1[0] = gxr_min<int>(cvRound(y), hi);
			cb += (b - y) * cb0;
			cr += (r - y) * cr0;
			CbCr[0] = gxr_clamp<int>(cvRound(cb * 0.25f) + half, 0, hi);
			CbCr[1] = gxr_clamp<int>(cvRound(cr * 0.25f) + half, 0, hi);

			BGR0 += 3; BGR1 += 3;
			Y0 += 1; Y1 += 1; CbCr += 2;
		}
	}
}


//==================== Denoise ====================//

static inline short calcTunedDif(short diff, short sgn, short noise)
{
	return gxr_clamp<short>(noise - (diff >> 1), 0, diff) * sgn;
}


static inline v_int16 vcalcTunedDif(v_int16 & vdiffs, v_int16 & vnoise, v_int16 & v0, v_int16 & v1, v_int16 & v_1)
{
	v_int16 vdiff = v_reinterpret_as_s16(v_abs(vdiffs));
	v_int16 vsgn = v_select(vdiffs >= v0, v1, v_1);
	return v_max(v_min(vnoise - (vdiff >> 1), vdiff), v0) * vsgn;
}


RAW2BGR::DenoiseImpl::DenoiseImpl(Mat& img_, Mat& buffer_, int noise_, float bdlarge_)
	: img(img_), buffer(buffer_), noise(noise_), bdlarge(bdlarge_)
{
	rows = img.rows;
	cols = img.cols;
	chs = img.channels();
	step = img.cols * chs;
	memcpy(buffer.ptr<uchar>(0), img.ptr<uchar>(0), step * 2);
	memcpy(buffer.ptr<uchar>(rows - 2), img.ptr<uchar>(rows - 2), step * 2);
}


void RAW2BGR::DenoiseImpl::operator()(Range const& range) const
{
	int const chs2 = chs * 2;
	int const border = chs * 2;
	int const wlimit = step - border;
	float shrows = (rows - 1) * 0.5f; float shcols = (step - 1) * 0.5f;
	float rc = bdlarge / (shrows * shrows + shcols * shcols);
	short wt[5] = { 3, 3, 4, 3, 3 };
	v_int16 vwt[5] = { vx_setall_s16(3), vx_setall_s16(3), vx_setall_s16(4), vx_setall_s16(3), vx_setall_s16(3) };
	v_int16 v2048 = vx_setall_s16(2048);
	for (int h = range.start; h < range.end; h++)
	{
		float hrc = (h - shrows) * (h - shrows);
		uchar * S[5];
		uchar * D = buffer.ptr<uchar>(h);
		S[0] = img.ptr<uchar>(h - 2, 2);
		S[1] = img.ptr<uchar>(h - 1, 2);
		S[2] = img.ptr<uchar>(h, 2);
		S[3] = img.ptr<uchar>(h + 1, 2);
		S[4] = img.ptr<uchar>(h + 2, 2);
		memcpy(buffer.ptr<uchar>(h), img.ptr<uchar>(h), chs2);
		memcpy(buffer.ptr<uchar>(h, cols - 2), img.ptr<uchar>(h, cols - 2), chs2);
		int w = 2 * chs;

		int const VSZ = v_uint16::nlanes;
		int wstop = wlimit - VSZ;
		v_int16 v0 = vx_setall_s16(0);
		v_int16 v1 = vx_setall_s16(1);
		v_int16 v_1 = vx_setall_s16(-1);
		v_int16 v3 = vx_setall_s16(3);
		v_int16 vm = vx_setall_s16(4369);
		v_int32 v32768 = vx_setall_s32(32768), vlo, vhi;
		for (; w <= wstop; w += VSZ)
		{
			short noisescale = gxr_clamp<int>((1.f + ((hrc + (w - shcols) * (w - shcols)) * rc)) * noise, 0, GAMMASAT);
			v_int16 vnoisescale = vx_setall_s16(noisescale);
			v_int16 vsum = vx_setall_s16(0);
			v_int16 vcur = v_reinterpret_as_s16(vx_load_expand(S[2]));
			for (int i = 0; i < 5; i++)
			{
				v_int16 diff0 = v_reinterpret_as_s16(vx_load_expand(S[i] - chs2)) - vcur;
				v_int16 diff1 = v_reinterpret_as_s16(vx_load_expand(S[i] - chs)) - vcur;
				v_int16 diff2 = v_reinterpret_as_s16(vx_load_expand(S[i])) - vcur;
				v_int16 diff3 = v_reinterpret_as_s16(vx_load_expand(S[i] + chs)) - vcur;
				v_int16 diff4 = v_reinterpret_as_s16(vx_load_expand(S[i] + chs2)) - vcur;
				v_int16 tunediff0 = vcalcTunedDif(diff0, vnoisescale, v0, v1, v_1);
				v_int16 tunediff1 = vcalcTunedDif(diff1, vnoisescale, v0, v1, v_1);
				v_int16 tunediff2 = vcalcTunedDif(diff2, vnoisescale, v0, v1, v_1);
				v_int16 tunediff3 = vcalcTunedDif(diff3, vnoisescale, v0, v1, v_1);
				v_int16 tunediff4 = vcalcTunedDif(diff4, vnoisescale, v0, v1, v_1);
				vsum += ((((tunediff0 + tunediff1 + tunediff3 + tunediff4) * v3 + (tunediff2 << 2)) * vwt[i]) >> 4);
				S[i] += VSZ;
			}
			v_mul_expand(vsum, vm, vlo, vhi);
			v_pack_u_store(D + w, v_pack((vlo + v32768) >> 16, (vhi + v32768) >> 16) + vcur);
		}
		for (; w < wlimit; w++)
		{
			short noisescale = gxr_clamp<int>((1.f + ((hrc + (w - shcols) * (w - shcols)) * rc)) * noise, 0, GAMMASAT);
			short sum = 0;
			short cur = S[2][0];
			for (int i = 0; i < 5; i++)
			{
				short diff0 = S[i][-chs2] - cur;
				short diff1 = S[i][-chs] - cur;
				short diff2 = S[i][0] - cur;
				short diff3 = S[i][chs] - cur;
				short diff4 = S[i][chs2] - cur;
				short tunediff0 = calcTunedDif(abs(diff0), diff0 >= 0 ? 1 : -1, noisescale);
				short tunediff1 = calcTunedDif(abs(diff1), diff1 >= 0 ? 1 : -1, noisescale);
				short tunediff2 = calcTunedDif(abs(diff2), diff2 >= 0 ? 1 : -1, noisescale);
				short tunediff3 = calcTunedDif(abs(diff3), diff3 >= 0 ? 1 : -1, noisescale);
				short tunediff4 = calcTunedDif(abs(diff4), diff4 >= 0 ? 1 : -1, noisescale);
				sum += ((((tunediff0 + tunediff1 + tunediff3 + tunediff4) * 3 + (tunediff2 << 2)) * wt[i]) >> 4);
				S[i]++;
			}
			D[w] = ((sum * 4369 + 32768) >> 16) + cur;
		}
	}
}


RAW2BGR::FilterUp::FilterUp(RAW2BGR* raw2_, Mat& hsrc_, Mat& hdst_, Mat& dst_)
	: raw2(raw2_), hsrc(hsrc_), hdst(hdst_), dst(dst_)
{
	int cn = hsrc.channels();
	int hcols = hsrc.cols * cn;
	buffer = Mat(2 * hsrc.rows, hcols, CV_16SC1, raw2->dst.data);
}


void RAW2BGR::FilterUp::operator()(Range const& range) const
{
	int cn = hsrc.channels();
	int hcols = hsrc.cols * cn;
	int const VSZ = v_int16::nlanes;
	int hcolsstop = hcols - VSZ;
	v_int16 vs0, vs1_, vs1, vt, v0 = vx_setall_s16(0), vsat = vx_setall_s16(GAMMASAT);
	for (int h = range.start; h < range.end; h++)
	{
		short* B0 = buffer.ptr<short>(h << 1);
		short* B1 = buffer.ptr<short>((h << 1) + 1);
		uchar* hS0 = hsrc.ptr<uchar>(h), * hD0 = hdst.ptr<uchar>(h);
		uchar* hS1_ = hsrc.ptr<uchar>(h - 1), * hD1_ = hdst.ptr<uchar>(h - 1);
		uchar* hS1 = hsrc.ptr<uchar>(h + 1), * hD1 = hdst.ptr<uchar>(h + 1);
		int w = 0;
		for (; w <= hcolsstop; w += VSZ)
		{
			vs1_ = v_reinterpret_as_s16(vx_load_expand(hD1_ + w)) - v_reinterpret_as_s16(vx_load_expand(hS1_ + w));
			vs0 = v_reinterpret_as_s16(vx_load_expand(hD0 + w)) - v_reinterpret_as_s16(vx_load_expand(hS0 + w));
			vs1 = v_reinterpret_as_s16(vx_load_expand(hD1 + w)) - v_reinterpret_as_s16(vx_load_expand(hS1 + w));
			vt = vs0 << 2;
			vx_store(B0 + w, vs1_ + vt + (vs0 << 1) + vs1);
			vx_store(B1 + w, (vs0 + vs1) << 2);
		}
		for (; w < hcols; w++)
		{
			short diff1_ = hD1_[w] - hS1_[w];
			short diff0 = hD0[w] - hS0[w];
			short diff1 = hD1[w] - hS1[w];
			B0[w] = diff1_ + diff0 * 6 + diff1;
			B1[w] = (diff0 + diff1) << 2;
		}
		if (cn == 1)
		{
			w = 1;
			int hcoslimit = hcols - 1;
			uchar* D0 = dst.ptr<uchar>(h << 1, 2);
			uchar* D1 = dst.ptr<uchar>((h << 1) + 1, 2);
			for (; w < hcoslimit; w++)
			{
				short s1_ = B0[w - 1];
				short s0 = B0[w];
				short s1 = B0[w + 1];
				D0[0] = gxr_clamp(D0[0] + ((s1_ + s0 * 6 + s1 + 32) >> 6), 0, 255);
				D0[1] = gxr_clamp(D0[1] + ((s0 + s1 + 8) >> 4), 0, 255);

				s1_ = B1[w - 1];
				s0 = B1[w];
				s1 = B1[w + 1];
				D1[0] = gxr_clamp(D1[0] + ((s1_ + s0 * 6 + s1 + 32) >> 6), 0, 255);
				D1[1] = gxr_clamp(D1[1] + ((s0 + s1 + 8) >> 4), 0, 255);
				D0 += 2;
				D1 += 2;
			}
		}
		else
		{
			w = 2;
			int hcoslimit = hcols - 2;
			uchar* D0 = dst.ptr<uchar>(h << 1, 2);
			uchar* D1 = dst.ptr<uchar>((h << 1) + 1, 2);
			short* T0 = B0 + 2;
			short* T1 = B1 + 2;
			for (; w < hcoslimit; w += 2)
			{
				short s1_ = T0[-2];
				short s0 = T0[0];
				short s1 = T0[2];
				D0[0] = gxr_clamp(D0[0] + ((s1_ + s0 * 6 + s1 + 32) >> 6), 0, 255);
				D0[2] = gxr_clamp(D0[2] + ((s0 + s1 + 8) >> 4), 0, 255);
				s1_ = T0[-1];
				s0 = T0[1];
				s1 = T0[3];
				D0[1] = gxr_clamp(D0[1] + ((s1_ + s0 * 6 + s1 + 32) >> 6), 0, 255);
				D0[3] = gxr_clamp(D0[3] + ((s0 + s1 + 8) >> 4), 0, 255);

				s1_ = T1[-2];
				s0 = T1[0];
				s1 = T1[2];
				D1[0] = gxr_clamp(D1[0] + ((s1_ + s0 * 6 + s1 + 32) >> 6), 0, 255);
				D1[2] = gxr_clamp(D1[2] + ((s0 + s1 + 8) >> 4), 0, 255);
				s1_ = T1[-1];
				s0 = T1[1];
				s1 = T1[3];
				D1[1] = gxr_clamp(D1[1] + ((s1_ + s0 * 6 + s1 + 32) >> 6), 0, 255);
				D1[3] = gxr_clamp(D1[3] + ((s0 + s1 + 8) >> 4), 0, 255);

				T0 += 2; T1 += 2;
				D0 += 4; D1 += 4;
			}
		}
	}
}


RAW2BGR::YNR::YNR(RAW2BGR* r)
	: raw2(r), luma(r->luma)
{}


void RAW2BGR::YNR::run_each(Mat* srcs, Mat* dsts, int* noise, int i)
{
	if (noise[i] > 0)
		parallel_for_(Range(2, srcs[i].rows - 2), DenoiseImpl(srcs[i], dsts[i], noise[i], raw2->bdlarge));
	else
		srcs[i].copyTo(dsts[i]);
	if (i < 4 && noise[i + 1] > 0)
	{
		gauDown(dsts[i], srcs[i + 1], Size(srcs[i + 1].cols, srcs[i + 1].rows));
		run_each(srcs, dsts, noise, i + 1);
		parallel_for_(Range(1, srcs[i + 1].rows - 1), FilterUp(raw2, srcs[i + 1], dsts[i + 1], dsts[i]));
	}
	if (raw2->sharp[i] > 0)
		parallel_for_(Range(2, dsts[i].rows - 2), Sharpen(raw2, dsts[i], raw2->sharp[i]));
}


void RAW2BGR::YNR::run()
{
	int noise[5];
	uchar* pbuf = (uchar*)raw2->bgr.ptr<short>(raw2->brows / 2);
	Mat srcs[5], dsts[5];
	srcs[0] = luma; dsts[0] = Mat(srcs[0].rows, srcs[0].cols, CV_8UC1, pbuf); pbuf += dsts[0].rows * dsts[0].step[0];
	noise[0] = gxr_clamp<int>(raw2->ynoise[0], 0, GAMMASAT);
	for (int i = 1; i < 5; i++)
	{
		noise[i] = gxr_clamp<int>(raw2->ynoise[i], 0, GAMMASAT);
		srcs[i] = Mat((srcs[i - 1].rows + 1) / 2, (srcs[i - 1].cols + 1) / 2, CV_8UC1, pbuf); pbuf += srcs[i].rows * srcs[i].step[0];
		dsts[i] = Mat((dsts[i - 1].rows + 1) / 2, (dsts[i - 1].cols + 1) / 2, CV_8UC1, pbuf); pbuf += dsts[i].rows * dsts[i].step[0];
		CV_Assert(pbuf <= raw2->bgr.dataend);
	}
	run_each(srcs, dsts, noise, 0);
	dsts[0].copyTo(srcs[0]);
}


RAW2BGR::CNR::CNR(RAW2BGR* r)
	: raw2(r), chroma(r->chroma)
{}


void RAW2BGR::CNR::run_each(Mat* srcs, Mat* dsts, int* noise, int i)
{
	if (noise[i] > 0)
		parallel_for_(Range(2, srcs[i].rows - 2), DenoiseImpl(srcs[i], dsts[i], noise[i], raw2->bdlarge));
	else
		srcs[i].copyTo(dsts[i]);
	if (i < 4 && noise[i + 1] > 0)
	{
		gauDown(dsts[i], srcs[i + 1], Size(srcs[i + 1].cols, srcs[i + 1].rows));
		run_each(srcs, dsts, noise, i + 1);
		parallel_for_(Range(1, srcs[i + 1].rows - 1), FilterUp(raw2, srcs[i + 1], dsts[i + 1], dsts[i]));
	}
}


void RAW2BGR::CNR::run()
{
#if !RefineCbCr
	int noise[5];
	uchar* pbuf = (uchar*)raw2->bgr.ptr<short>(raw2->brows * 3 / 8);
	Mat srcs[5], dsts[5];
	srcs[0] = chroma;
	noise[0] = gxr_clamp<int>(raw2->cnoise[0], 0, GAMMASAT);
	for (int i = 1; i < 5; i++)
	{
		noise[i] = gxr_clamp<int>(raw2->cnoise[i], 0, GAMMASAT);
		srcs[i] = Mat((srcs[i - 1].rows + 1) / 2, (srcs[i - 1].cols + 1) / 2, CV_8UC2, pbuf); pbuf += srcs[i].rows * srcs[i].step[0];
		dsts[i] = Mat((srcs[i - 1].rows + 1) / 2, (srcs[i - 1].cols + 1) / 2, CV_8UC2, pbuf); pbuf += dsts[i].rows * dsts[i].step[0];
	}
	gauDown(srcs[0], srcs[1], Size(srcs[1].cols, srcs[1].rows));
	run_each(srcs, dsts, noise, 1);
	gauUp(dsts[1], srcs[0], Size(srcs[0].cols, srcs[0].rows));
#else
	int noise[5];
	uchar* pbuf = (uchar*)raw2->bgr.ptr<short>(raw2->brows * 3 / 8);
	Mat srcs[5], dsts[5];
	srcs[0] = chroma; dsts[0] = Mat(srcs[0].rows, srcs[0].cols, CV_8UC2, raw2->raw.data);
	noise[0] = gxr_clamp<int>(raw2->cnoise[0], 0, GAMMASAT);
	for (int i = 1; i < 5; i++)
	{
		noise[i] = gxr_clamp<int>(raw2->cnoise[i], 0, GAMMASAT);
		srcs[i] = Mat((srcs[i - 1].rows + 1) / 2, (srcs[i - 1].cols + 1) / 2, CV_8UC2, pbuf); pbuf += srcs[i].rows * srcs[i].step[0];
		dsts[i] = Mat((dsts[i - 1].rows + 1) / 2, (dsts[i - 1].cols + 1) / 2, CV_8UC2, pbuf); pbuf += dsts[i].rows * dsts[i].step[0];
		CV_Assert(pbuf <= raw2->bgr.dataend);
	}
	run_each(srcs, dsts, noise, 0);
	dsts[0].copyTo(srcs[0]);
#endif
}


//==================== Sharpen ====================//

RAW2BGR::Sharpen::Sharpen(RAW2BGR * r, Mat& src_, float sharpscale_)
	: raw2(r), src(src_)
{
	cols = src.cols;
	sharpscale = gxr_clamp<int>(sharpscale_, 0, 32);
	buffer = Mat(src.rows * 2, src.cols, CV_8UC1, raw2->dst.data);
	memcpy(buffer.data, src.data, src.rows * src.step[0]);
}


void RAW2BGR::Sharpen::operator()(Range const& range) const
{
	int wlimit = cols - 2;
	v_int16 vsharpscale = vx_setall_s16(sharpscale);
	v_int16 vlo = vx_setall_s16(0);
	v_int16 vhi = vx_setall_s16(GAMMASAT);

	for (int h = range.start; h < range.end; h++)
	{
		uchar* S0 = buffer.ptr<uchar>(h - 2);
		uchar * S1 = S0 + cols;
		uchar * S2 = S1 + cols;
		uchar * S3 = S2 + cols;
		uchar * S4 = S3 + cols;
		uchar* T = buffer.ptr<uchar>(src.rows + h);

		int w = 0;
		int const VSZ = v_uint8::nlanes;
		int wstop_ = cols - VSZ;
		int wstop = wlimit - VSZ;
		v_uint8 vg_2, vg_1, vg0, vg1, vg2;
		v_uint8 vg;
		for (; w <= wstop_; w += VSZ)
		{
			vg_2 = vx_load(S0 + w);
			vg_1 = vx_load(S1 + w);
			vg0 = vx_load(S2 + w);
			vg1 = vx_load(S3 + w);
			vg2 = vx_load(S4 + w);
			vg = v_pack(v_reinterpret_as_u16(v_max(v_min(v_reinterpret_as_s16(v_expand_low(vg0)) + (((v_reinterpret_as_s16(v_expand_low(vg0) << 2) - v_reinterpret_as_s16(v_expand_low(vg_2) + v_expand_low(vg_1) + v_expand_low(vg1) + v_expand_low(vg2))) * vsharpscale) >> 5), vhi), vlo)),
				v_reinterpret_as_u16(v_max(v_min(v_reinterpret_as_s16(v_expand_high(vg0)) + (((v_reinterpret_as_s16(v_expand_high(vg0) << 2) - v_reinterpret_as_s16(v_expand_high(vg_2) + v_expand_high(vg_1) + v_expand_high(vg1) + v_expand_high(vg2))) * vsharpscale) >> 5), vhi), vlo)));
			vx_store(T + w, vg);
		}
		for (; w < cols; w++)
			T[w] = gxr_clamp<int>(S2[w] + ((((S2[w] << 2) - (S0[w] + S1[w] + S3[w] + S4[w])) * sharpscale) >> 5), 0, GAMMASAT);

		uchar* D = src.ptr<uchar>(h);
		w = 2;
		for (; w <= wstop; w += VSZ)
		{
			uchar* T0 = T + w;
			vg_2 = vx_load(T0 - 2);
			vg_1 = vx_load(T0 - 1);
			vg0 = vx_load(T0);
			vg1 = vx_load(T0 + 1);
			vg2 = vx_load(T0 + 2);
			vg = v_pack(v_reinterpret_as_u16(v_max(v_min(v_reinterpret_as_s16(v_expand_low(vg0)) + (((v_reinterpret_as_s16(v_expand_low(vg0) << 2) - v_reinterpret_as_s16(v_expand_low(vg_2) + v_expand_low(vg_1) + v_expand_low(vg1) + v_expand_low(vg2))) * vsharpscale) >> 5), vhi), vlo)),
				v_reinterpret_as_u16(v_max(v_min(v_reinterpret_as_s16(v_expand_high(vg0)) + (((v_reinterpret_as_s16(v_expand_high(vg0) << 2) - v_reinterpret_as_s16(v_expand_high(vg_2) + v_expand_high(vg_1) + v_expand_high(vg1) + v_expand_high(vg2))) * vsharpscale) >> 5), vhi), vlo)));
			vx_store(D + w, vg);
		}
		for (; w < wlimit; w++)
			D[w] = gxr_clamp<int>(T[w] + ((((T[w] << 2) - (T[w - 2] + T[w - 1] + T[w + 1] + T[w + 2])) * sharpscale) >> 5), 0, GAMMASAT);
	}
}


//==================== YCbCr2BGR ====================//

RAW2BGR::YCbCr2BGR::YCbCr2BGR(RAW2BGR* r)
	: raw2(r), bgr(r->dst), luma(r->luma), chroma(r->chroma)
{
	brows = raw2->brows; bcols = raw2->bcols;
	hrows = brows / 2; hcols = bcols / 2;
	b0 = 1.773f; g0 = -0.344f; g1 = -0.714f; r0 = 1.403f;
	b0s = cvRound(b0 * LSCVAL); g0s = cvRound(g0 * LSCVAL); g1s = cvRound(g1 * LSCVAL); r0s = cvRound(r0 * LSCVAL);
	tmp = Mat(brows, bcols * 2, CV_8UC1, raw2->raw.data);
}


void RAW2BGR::YCbCr2BGR::operator()(Range const& range) const
{
	int hi = GAMMASAT;
	int half = (hi + 1) / 2;
	int const VSZ = v_uint8::nlanes;
	int const VSZ3 = VSZ * 3;
	int const VSZHALF = VSZ / 2;
	v_int16 vlo = vx_setall_s16(0);
	v_int16 vhi = vx_setall_s16(hi);
	v_int16 vhalf = vx_setall_s16(half);
	v_int16 vb0s = vx_setall_s16(b0s);
	v_int16 vg0s = vx_setall_s16(g0s);
	v_int16 vg1s = vx_setall_s16(g1s);
	v_int16 vr0s = vx_setall_s16(r0s);
	v_int32 vcoefhalf = vx_setall_s32(LSCVAL >> 1);
	short CV_DECL_ALIGNED(32) vbufcb[VSZHALF];
	short CV_DECL_ALIGNED(32) vbufcr[VSZHALF];
#if !RefineCbCr
	for (int h = range.start; h < range.end; h++)
	{
		int h2 = (h << 1);
		uchar * BGR0 = bgr.ptr<uchar>(h2);
		uchar * BGR1 = bgr.ptr<uchar>(h2 + 1);
		uchar * Y0 = luma.ptr<uchar>(h2);
		uchar * Y1 = luma.ptr<uchar>(h2 + 1);
		uchar * CbCr = chroma.ptr<uchar>(h);
		int w = 0;

		int wstop = bcols - VSZ;
		v_uint8 vy0_, vy1_;
		v_int16 vy0, vy1, vcb, vcr, vdelta;
		v_int16 vb_low0, vg_low0, vr_low0;
		v_int16 vb_low1, vg_low1, vr_low1;
		v_int16 vb_high0, vg_high0, vr_high0;
		v_int16 vb_high1, vg_high1, vr_high1;
		v_int32 vlo0, vhi0;
		v_int32 vlo1, vhi1;
		for (; w <= wstop; w += VSZ)
		{
			vy0_ = vx_load(Y0);
			vy1_ = vx_load(Y1);

			vy0 = v_reinterpret_as_s16(v_expand_low(vy0_));
			vy1 = v_reinterpret_as_s16(v_expand_low(vy1_));
			for (int i = 0, j = 1; i < VSZHALF; i += 2, j += 2)
			{
				vbufcb[i] = vbufcb[j] = CbCr[i];
				vbufcr[i] = vbufcr[j] = CbCr[j];
			}
			CbCr += VSZHALF;
			vcb = vx_load(vbufcb) - vhalf;
			vcr = vx_load(vbufcr) - vhalf;

			v_mul_expand(vb0s, vcb, vlo0, vhi0);
			vdelta = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vb_low0 = vy0 + vdelta;
			vb_low1 = vy1 + vdelta;

			v_mul_expand(vg0s, vcb, vlo0, vhi0);
			v_mul_expand(vg1s, vcr, vlo1, vhi1);
			vdelta = v_pack((vlo0 + vlo1 + vcoefhalf) >> LSCBIT, (vhi0 + vhi1 + vcoefhalf) >> LSCBIT);
			vg_low0 = vy0 + vdelta;
			vg_low1 = vy1 + vdelta;

			v_mul_expand(vr0s, vcr, vlo0, vhi0);
			vdelta = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vr_low0 = vy0 + vdelta;
			vr_low1 = vy1 + vdelta;



			vy0 = v_reinterpret_as_s16(v_expand_high(vy0_));
			vy1 = v_reinterpret_as_s16(v_expand_high(vy1_));
			for (int i = 0, j = 1; i < VSZHALF; i += 2, j += 2)
			{
				vbufcb[i] = vbufcb[j] = CbCr[i];
				vbufcr[i] = vbufcr[j] = CbCr[j];
			}
			CbCr += VSZHALF;
			vcb = vx_load(vbufcb) - vhalf;
			vcr = vx_load(vbufcr) - vhalf;

			v_mul_expand(vb0s, vcb, vlo0, vhi0);
			vdelta = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vb_high0 = vy0 + vdelta;
			vb_high1 = vy1 + vdelta;

			v_mul_expand(vg0s, vcb, vlo0, vhi0);
			v_mul_expand(vg1s, vcr, vlo1, vhi1);
			vdelta = v_pack((vlo0 + vlo1 + vcoefhalf) >> LSCBIT, (vhi0 + vhi1 + vcoefhalf) >> LSCBIT);
			vg_high0 = vy0 + vdelta;
			vg_high1 = vy1 + vdelta;

			v_mul_expand(vr0s, vcr, vlo0, vhi0);
			vdelta = v_pack((vlo0 + vcoefhalf) >> LSCBIT, (vhi0 + vcoefhalf) >> LSCBIT);
			vr_high0 = vy0 + vdelta;
			vr_high1 = vy1 + vdelta;



			v_store_interleave(BGR0, v_pack_u(vb_low0, vb_high0), v_pack_u(vg_low0, vg_high0), v_pack_u(vr_low0, vr_high0));
			v_store_interleave(BGR1, v_pack_u(vb_low1, vb_high1), v_pack_u(vg_low1, vg_high1), v_pack_u(vr_low1, vr_high1));
			Y0 += VSZ; BGR0 += VSZ3;
			Y1 += VSZ; BGR1 += VSZ3;
		}

		for (; w < bcols; w += 2)
		{
			float cb = CbCr[0] - half;
			float cr = CbCr[1] - half;
			float bcoef = b0 * cb;
			float gcoef = g0 * cb + g1 * cr;
			float rcoef = r0 * cr;
			CbCr += 2;

			float y = Y0[0];
			BGR0[0] = gxr_clamp(cvRound(y + bcoef), 0, hi);
			BGR0[1] = gxr_clamp(cvRound(y + gcoef), 0, hi);
			BGR0[2] = gxr_clamp(cvRound(y + rcoef), 0, hi);
			Y0 += 1;
			BGR0 += 3;

			y = Y0[0];
			BGR0[0] = gxr_clamp(cvRound(y + bcoef), 0, hi);
			BGR0[1] = gxr_clamp(cvRound(y + gcoef), 0, hi);
			BGR0[2] = gxr_clamp(cvRound(y + rcoef), 0, hi);
			Y0 += 1;
			BGR0 += 3;

			y = Y1[0];
			BGR1[0] = gxr_clamp(cvRound(y + bcoef), 0, hi);
			BGR1[1] = gxr_clamp(cvRound(y + gcoef), 0, hi);
			BGR1[2] = gxr_clamp(cvRound(y + rcoef), 0, hi);
			Y1 += 1;
			BGR1 += 3;

			y = Y1[0];
			BGR1[0] = gxr_clamp(cvRound(y + bcoef), 0, hi);
			BGR1[1] = gxr_clamp(cvRound(y + gcoef), 0, hi);
			BGR1[2] = gxr_clamp(cvRound(y + rcoef), 0, hi);
			Y1 += 1;
			BGR1 += 3;
		}
	}
#else
	v_int16 v454 = vx_setall_s16(454);
	v_int16 v88 = vx_setall_s16(88);
	v_int16 v183 = vx_setall_s16(183);
	v_int16 v359 = vx_setall_s16(359);
	int const VSZ2 = VSZ * 2;
	for (int h = range.start; h < range.end; h++)
	{
		int h2 = h << 1;
		uchar* BGR0 = bgr.ptr<uchar>(h2);
		uchar* BGR1 = bgr.ptr<uchar>(h2 + 1);
		uchar * Y0 = luma.ptr<uchar>(h2);
		uchar * Y1 = luma.ptr<uchar>(h2 + 1);
		uchar * CbCr0 = chroma.ptr<uchar>(h);
		uchar * CbCr1 = chroma.ptr<uchar>(gxr_min(h + 1, hrows - 1));
		uchar * U0 = tmp.ptr<uchar>(h2), *U1 = tmp.ptr<uchar>(h2 + 1);
		uchar * V0 = U0 + bcols, *V1 = U1 + bcols;
		int wlimit0 = bcols - 2;
		int wlimit1 = bcols - 1;
		int x = 0;
		int xstop = wlimit0 - VSZ2;
		for (; x <= xstop; x += VSZ2)
		{
			v_uint8 u00_, v00_;
			v_uint8 u01_, v01_;
			v_uint8 u10_, v10_;
			v_uint8 u11_, v11_;
			v_load_deinterleave(CbCr0 + x, u00_, v00_); v_load_deinterleave(CbCr0 + x + 2, u01_, v01_);
			v_load_deinterleave(CbCr1 + x, u10_, v10_); v_load_deinterleave(CbCr1 + x + 2, u11_, v11_);

			v_uint16 u00_low, v00_low, u01_low, v01_low, u10_low, v10_low, u11_low, v11_low;
			v_uint16 u00_high, v00_high, u01_high, v01_high, u10_high, v10_high, u11_high, v11_high;
			v_expand(u00_, u00_low, u00_high); v_expand(v00_, v00_low, v00_high);
			v_expand(u01_, u01_low, u01_high); v_expand(v01_, v01_low, v01_high);
			v_expand(u10_, u10_low, u10_high); v_expand(v10_, v10_low, v10_high);
			v_expand(u11_, u11_low, u11_high); v_expand(v11_, v11_low, v11_high);

			v_store_interleave(U0, u00_, v_pack((u00_low + u01_low) >> 1, (u00_high + u01_high) >> 1));
			v_store_interleave(V0, v00_, v_pack((v00_low + v01_low) >> 1, (v00_high + v01_high) >> 1));
			v_store_interleave(U1, v_pack((u00_low + u10_low) >> 1, (u00_high + u10_high) >> 1), v_pack((u00_low + u01_low + u10_low + u11_low) >> 2, (u00_high + u01_high + u10_high + u11_high) >> 2));
			v_store_interleave(V1, v_pack((v00_low + v10_low) >> 1, (v00_high + v10_high) >> 1), v_pack((v00_low + v01_low + v10_low + v11_low) >> 2, (v00_high + v01_high + v10_high + v11_high) >> 2));

			v_uint16 vt0, vt1;
			v_int16 vy_low, vu_low, vv_low;
			v_int16 vy_high, vu_high, vv_high;
			v_int32 vm0, vm1, vm2, vm3;
			v_int16 vb_low, vb_high, vg_low, vg_high, vr_low, vr_high;

			v_expand(vx_load(Y0), vt0, vt1); vy_low = v_reinterpret_as_s16(vt0); vy_high = v_reinterpret_as_s16(vt1);
			v_expand(vx_load(U0), vt0, vt1); vu_low = v_reinterpret_as_s16(vt0) - vhalf; vu_high = v_reinterpret_as_s16(vt1) - vhalf;
			v_expand(vx_load(V0), vt0, vt1); vv_low = v_reinterpret_as_s16(vt0) - vhalf; vv_high = v_reinterpret_as_s16(vt1) - vhalf;

			v_mul_expand(vu_low, v454, vm0, vm1);
			vb_low = v_min(v_max(vy_low + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);
			v_mul_expand(vu_high, v454, vm0, vm1);
			vb_high = v_min(v_max(vy_high + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);

			v_mul_expand(vu_low, v88, vm0, vm1);
			v_mul_expand(vv_low, v183, vm2, vm3);
			vg_low = v_min(v_max(vy_low - v_rshr_pack<GAMMABIT>(vm0 + vm2, vm1 + vm3), vlo), vhi);
			v_mul_expand(vu_high, v88, vm0, vm1);
			v_mul_expand(vv_high, v183, vm2, vm3);
			vg_high = v_min(v_max(vy_high - v_rshr_pack<GAMMABIT>(vm0 + vm2, vm1 + vm3), vlo), vhi);

			v_mul_expand(vv_low, v359, vm0, vm1);
			vr_low = v_min(v_max(vy_low + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);
			v_mul_expand(vv_high, v359, vm0, vm1);
			vr_high = v_min(v_max(vy_high + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);

			v_store_interleave(BGR0, v_pack_u(vb_low, vb_high), v_pack_u(vg_low, vg_high), v_pack_u(vr_low, vr_high));
			BGR0 += VSZ3;



			v_expand(vx_load(Y0 + VSZ), vt0, vt1); vy_low = v_reinterpret_as_s16(vt0); vy_high = v_reinterpret_as_s16(vt1);
			v_expand(vx_load(U0 + VSZ), vt0, vt1); vu_low = v_reinterpret_as_s16(vt0) - vhalf; vu_high = v_reinterpret_as_s16(vt1) - vhalf;
			v_expand(vx_load(V0 + VSZ), vt0, vt1); vv_low = v_reinterpret_as_s16(vt0) - vhalf; vv_high = v_reinterpret_as_s16(vt1) - vhalf;

			v_mul_expand(vu_low, v454, vm0, vm1);
			vb_low = v_min(v_max(vy_low + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);
			v_mul_expand(vu_high, v454, vm0, vm1);
			vb_high = v_min(v_max(vy_high + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);

			v_mul_expand(vu_low, v88, vm0, vm1);
			v_mul_expand(vv_low, v183, vm2, vm3);
			vg_low = v_min(v_max(vy_low - v_rshr_pack<GAMMABIT>(vm0 + vm2, vm1 + vm3), vlo), vhi);
			v_mul_expand(vu_high, v88, vm0, vm1);
			v_mul_expand(vv_high, v183, vm2, vm3);
			vg_high = v_min(v_max(vy_high - v_rshr_pack<GAMMABIT>(vm0 + vm2, vm1 + vm3), vlo), vhi);

			v_mul_expand(vv_low, v359, vm0, vm1);
			vr_low = v_min(v_max(vy_low + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);
			v_mul_expand(vv_high, v359, vm0, vm1);
			vr_high = v_min(v_max(vy_high + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);

			v_store_interleave(BGR0, v_pack_u(vb_low, vb_high), v_pack_u(vg_low, vg_high), v_pack_u(vr_low, vr_high));
			BGR0 += VSZ3;



			v_expand(vx_load(Y1), vt0, vt1); vy_low = v_reinterpret_as_s16(vt0); vy_high = v_reinterpret_as_s16(vt1);
			v_expand(vx_load(U1), vt0, vt1); vu_low = v_reinterpret_as_s16(vt0) - vhalf; vu_high = v_reinterpret_as_s16(vt1) - vhalf;
			v_expand(vx_load(V1), vt0, vt1); vv_low = v_reinterpret_as_s16(vt0) - vhalf; vv_high = v_reinterpret_as_s16(vt1) - vhalf;

			v_mul_expand(vu_low, v454, vm0, vm1);
			vb_low = v_min(v_max(vy_low + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);
			v_mul_expand(vu_high, v454, vm0, vm1);
			vb_high = v_min(v_max(vy_high + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);

			v_mul_expand(vu_low, v88, vm0, vm1);
			v_mul_expand(vv_low, v183, vm2, vm3);
			vg_low = v_min(v_max(vy_low - v_rshr_pack<GAMMABIT>(vm0 + vm2, vm1 + vm3), vlo), vhi);
			v_mul_expand(vu_high, v88, vm0, vm1);
			v_mul_expand(vv_high, v183, vm2, vm3);
			vg_high = v_min(v_max(vy_high - v_rshr_pack<GAMMABIT>(vm0 + vm2, vm1 + vm3), vlo), vhi);

			v_mul_expand(vv_low, v359, vm0, vm1);
			vr_low = v_min(v_max(vy_low + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);
			v_mul_expand(vv_high, v359, vm0, vm1);
			vr_high = v_min(v_max(vy_high + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);

			v_store_interleave(BGR1, v_pack_u(vb_low, vb_high), v_pack_u(vg_low, vg_high), v_pack_u(vr_low, vr_high));
			BGR1 += VSZ3;



			v_expand(vx_load(Y1 + VSZ), vt0, vt1); vy_low = v_reinterpret_as_s16(vt0); vy_high = v_reinterpret_as_s16(vt1);
			v_expand(vx_load(U1 + VSZ), vt0, vt1); vu_low = v_reinterpret_as_s16(vt0) - vhalf; vu_high = v_reinterpret_as_s16(vt1) - vhalf;
			v_expand(vx_load(V1 + VSZ), vt0, vt1); vv_low = v_reinterpret_as_s16(vt0) - vhalf; vv_high = v_reinterpret_as_s16(vt1) - vhalf;

			v_mul_expand(vu_low, v454, vm0, vm1);
			vb_low = v_min(v_max(vy_low + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);
			v_mul_expand(vu_high, v454, vm0, vm1);
			vb_high = v_min(v_max(vy_high + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);

			v_mul_expand(vu_low, v88, vm0, vm1);
			v_mul_expand(vv_low, v183, vm2, vm3);
			vg_low = v_min(v_max(vy_low - v_rshr_pack<GAMMABIT>(vm0 + vm2, vm1 + vm3), vlo), vhi);
			v_mul_expand(vu_high, v88, vm0, vm1);
			v_mul_expand(vv_high, v183, vm2, vm3);
			vg_high = v_min(v_max(vy_high - v_rshr_pack<GAMMABIT>(vm0 + vm2, vm1 + vm3), vlo), vhi);

			v_mul_expand(vv_low, v359, vm0, vm1);
			vr_low = v_min(v_max(vy_low + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);
			v_mul_expand(vv_high, v359, vm0, vm1);
			vr_high = v_min(v_max(vy_high + v_rshr_pack<GAMMABIT>(vm0, vm1), vlo), vhi);

			v_store_interleave(BGR1, v_pack_u(vb_low, vb_high), v_pack_u(vg_low, vg_high), v_pack_u(vr_low, vr_high));
			BGR1 += VSZ3;

			Y0 += VSZ2; U0 += VSZ2; V0 += VSZ2;
			Y1 += VSZ2; U1 += VSZ2; V1 += VSZ2;
		}
		for (; x < wlimit0; x += 2)
		{
			int y00 = Y0[0] << 8;
			int y01 = Y0[1] << 8;
			int y10 = Y1[0] << 8;
			int y11 = Y1[1] << 8;
			int u00 = CbCr0[x] - 128;
			int u01 = CbCr0[x + 2] - 128;
			int u10 = CbCr1[x] - 128;
			int u11 = CbCr1[x + 2] - 128;
			int v00 = CbCr0[x + 1] - 128;
			int v01 = CbCr0[x + 3] - 128;
			int v10 = CbCr1[x + 1] - 128;
			int v11 = CbCr1[x + 3] - 128;

			int u, v;
			u = u00, v = v00;
			BGR0[0] = gxr_clamp<int>((y00 + u * 454 + 128) >> 8, 0, GAMMASAT);
			BGR0[1] = gxr_clamp<int>((y00 - u * 88 - v * 183 + 128) >> 8, 0, GAMMASAT);
			BGR0[2] = gxr_clamp<int>((y00 + v * 359 + 128) >> 8, 0, GAMMASAT);
			BGR0 += 3;

			u = (u00 + u01) >> 1, v = (v00 + v01) >> 1;
			BGR0[0] = gxr_clamp<int>((y01 + u * 454 + 128) >> 8, 0, GAMMASAT);
			BGR0[1] = gxr_clamp<int>((y01 - u * 88 - v * 183 + 128) >> 8, 0, GAMMASAT);
			BGR0[2] = gxr_clamp<int>((y01 + v * 359 + 128) >> 8, 0, GAMMASAT);
			BGR0 += 3;

			u = (u00 + u10) >> 1, v = (v00 + v10) >> 1;
			BGR1[0] = gxr_clamp<int>((y10 + u * 454 + 128) >> 8, 0, GAMMASAT);
			BGR1[1] = gxr_clamp<int>((y10 - u * 88 - v * 183 + 128) >> 8, 0, GAMMASAT);
			BGR1[2] = gxr_clamp<int>((y10 + v * 359 + 128) >> 8, 0, GAMMASAT);
			BGR1 += 3;

			u = (u00 + u01 + u10 + u11) >> 2, v = (v00 + v01 + v10 + v11) >> 2;
			BGR1[0] = gxr_clamp<int>((y11 + u * 454 + 128) >> 8, 0, GAMMASAT);
			BGR1[1] = gxr_clamp<int>((y11 - u * 88 - v * 183 + 128) >> 8, 0, GAMMASAT);
			BGR1[2] = gxr_clamp<int>((y11 + v * 359 + 128) >> 8, 0, GAMMASAT);
			BGR1 += 3;

			Y0 += 2;
			Y1 += 2;
		}
		for (; x < bcols; x += 2)
		{
			int y00 = Y0[0] << 8;
			int y01 = Y0[1] << 8;
			int y10 = Y1[0] << 8;
			int y11 = Y1[1] << 8;
			int u00 = CbCr0[x] - 128;
			int u01 = CbCr0[wlimit0] - 128;
			int u10 = CbCr1[x] - 128;
			int u11 = CbCr1[wlimit0] - 128;
			int v00 = CbCr0[x + 1] - 128;
			int v01 = CbCr0[wlimit1] - 128;
			int v10 = CbCr1[x + 1] - 128;
			int v11 = CbCr1[wlimit1] - 128;

			int u, v;
			u = u00, v = v00;
			BGR0[0] = gxr_clamp<int>((y00 + u * 454 + 128) >> 8, 0, GAMMASAT);
			BGR0[1] = gxr_clamp<int>((y00 - u * 88 - v * 183 + 128) >> 8, 0, GAMMASAT);
			BGR0[2] = gxr_clamp<int>((y00 + v * 359 + 128) >> 8, 0, GAMMASAT);
			BGR0 += 3;

			u = (u00 + u01) >> 1, v = (v00 + v01) >> 1;
			BGR0[0] = gxr_clamp<int>((y01 + u * 454 + 128) >> 8, 0, GAMMASAT);
			BGR0[1] = gxr_clamp<int>((y01 - u * 88 - v * 183 + 128) >> 8, 0, GAMMASAT);
			BGR0[2] = gxr_clamp<int>((y01 + v * 359 + 128) >> 8, 0, GAMMASAT);
			BGR0 += 3;

			u = (u00 + u10) >> 1, v = (v00 + v10) >> 1;
			BGR1[0] = gxr_clamp<int>((y10 + u * 454 + 128) >> 8, 0, GAMMASAT);
			BGR1[1] = gxr_clamp<int>((y10 - u * 88 - v * 183 + 128) >> 8, 0, GAMMASAT);
			BGR1[2] = gxr_clamp<int>((y10 + v * 359 + 128) >> 8, 0, GAMMASAT);
			BGR1 += 3;

			u = (u00 + u01 + u10 + u11) >> 2, v = (v00 + v01 + v10 + v11) >> 2;
			BGR1[0] = gxr_clamp<int>((y11 + u * 454 + 128) >> 8, 0, GAMMASAT);
			BGR1[1] = gxr_clamp<int>((y11 - u * 88 - v * 183 + 128) >> 8, 0, GAMMASAT);
			BGR1[2] = gxr_clamp<int>((y11 + v * 359 + 128) >> 8, 0, GAMMASAT);
			BGR1 += 3;

			Y0 += 2;
			Y1 += 2;
		}
	}
#endif
}


//==================== Ungroup ====================//

RAW2BGR::Ungroup::Ungroup(RAW2BGR* r)
	:raw2(r), dst(r->dst)
{
	srows = raw2->srows;
	scols = raw2->scols;
	brows = raw2->brows;
	bcols = raw2->bcols;
	tmp = Mat(brows, scols, CV_8UC3, raw2->bgr.data);

	sh0 = raw2->roi.y >> 1;
	sh1 = sh0 + raw2->roi.height;
	sw0 = raw2->roi.x >> 1;
	sw1 = sw0 + raw2->roi.width;

	dh0 = raw2->roi.y;
	dh1 = dh0 + raw2->roi.height;
	dw0 = raw2->roi.x;
	dw1 = dw0 + raw2->roi.width;
}


void RAW2BGR::Ungroup::run()
{
	//tmp = Mat(brows, bcols, CV_8UC3, raw2->bgr.data);
	//dst.copyTo(tmp);
	//dst = Mat(srows, scols, CV_8UC3, dst.data);
	//resize(tmp, dst, Size(scols, srows), 0, 0, INTER_LANCZOS4);
	//return;

	class ColPass : public ParallelLoopBody
	{	
	public:
		Ungroup* ug;
		Mat& src, & dst;

		ColPass(Ungroup* u)
			: ug(u), src(u->dst), dst(u->tmp)
		{
		}

		void operator ()(Range const& range) const override
		{
			for (int h = range.start; h < range.end; h++)
			{
				uchar* S = src.ptr<uchar>(h);
				uchar* D = dst.ptr<uchar>(h);
				int w = 0;
				for (; w < ug->dw0; w += 2)
				{
					D[0] = D[3] = S[0];
					D[1] = D[4] = S[1];
					D[2] = D[5] = S[2];
					S += 3; D += 6;
				}
				memcpy(D, S, (ug->dw1 - ug->dw0) * 3);
				S += (ug->dw1 - ug->dw0) * 3;
				D += (ug->dw1 - ug->dw0) * 3;
				for (w = ug->dw1; w < dst.cols; w += 2)
				{
					D[0] = D[3] = S[0];
					D[1] = D[4] = S[1];
					D[2] = D[5] = S[2];
					S += 3; D += 6;
				}
			}
		}
	};

	class RowPass : public ParallelLoopBody
	{
	public:
		Ungroup* ug;
		Mat& src, & dst;

		RowPass(Ungroup* u)
			: ug(u), src(u->tmp), dst(u->dst)
		{
			memcpy(dst.ptr<uchar>(ug->dh0), src.ptr<uchar>(ug->sh0), (ug->dh1 - ug->dh0) * dst.step[0]);
		}

		void operator ()(Range const& range) const override
		{
			for (int dy = range.start; dy < range.end; dy++)
			{
				if (dy >= ug->dh0 && dy < ug->dh1)
					continue;
				int sy = (dy < ug->dh0) ? (dy >> 1) : ug->sh1 + ((dy - ug->dh1) >> 1);
				uchar* S = src.ptr<uchar>(sy);
				uchar* D = dst.ptr<uchar>(dy);
				memcpy(D, S, src.step[0]);
			}
		}
	};

	if (srows == brows && scols == bcols)
		return;

	parallel_for_(Range(0, brows), ColPass(this));
	dst = Mat(srows, scols, CV_8UC3, dst.data);
	parallel_for_(Range(0, srows), RowPass(this));
}