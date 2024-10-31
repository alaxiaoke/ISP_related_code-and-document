#define GXR_RAW2BGR_Build
#include "gxr_raw2bgr.h"
#include "raw2bgr.hpp"
#include "ocl.hpp"
#include <cerrno>

#define VERSION_VERSION "GXR_RAW2BGR_3.5.9.0"

using namespace cv;
using namespace gxr;

static void gen_gammatable(const ushort* xtable, const ushort* ytable, uchar* gtab)
{
	int const len = LINEARSAT + 1;
	ushort xmax = xtable[256];
	float xratio = static_cast<float>(LINEARSAT) / xmax;

	ushort ymax = ytable[0];
	for (int i = 0; i < 257; i++)
	{
		ushort y = ytable[i];
		if (ymax < y)
			ymax = y;
	}
	float yratio = static_cast<float>(GAMMASAT) / ymax;

	vector<float> normXTable(len);
	vector<float> normYTable(len);
	for (int i = 0; i < 257; i++)
	{
		normXTable[i] = xtable[i] * xratio;
		normYTable[i] = ytable[i] * yratio;
	}

	for (int ind0 = 0; ind0 <= GAMMASAT; ind0++)
	{
		int ind1 = ind0 + 1;
		float x0 = normXTable[ind0], x1 = normXTable[ind1], ratio = 1.0 / (x1 - x0);
		int i0 = ceil(x0), i1 = floor(x1);
		for (int i = i0; i <= i1; i++)
		{
			float w1 = (i - x0) * ratio;
			gtab[i] = gxr_clamp<int>(normYTable[ind0] + (normYTable[ind1] - normYTable[ind0]) * w1 + 0.5f, 0, GAMMASAT);
		}
	}
}


static void raw2bgr_timerit(gxr_raw2bgr * raw2, char const* msg)
{
	raw2->ticksum += getTickCount() - raw2->tickpre;
	double cur = 1e3 * raw2->ticksum / getTickFrequency();
	if (raw2->verbose & VerbTimer)
		GXR_Printf("й└йд%d %9.3f %s\n", raw2->ntick, cur, msg);
	raw2->tickpre = getTickCount();
	++(raw2->ntick);
}


static void raw2bgr_impl(gxr_raw2bgr * raw2)
{
	int rpat = raw2->rpat, depth = raw2->depth;
	CV_Assert(raw2->impl);
	RAW2BGR* impl = static_cast<RAW2BGR*>(raw2->impl);

#define RAW2BGR_Args(var) impl->var = raw2->var
	RAW2BGR_Args(verbose); RAW2BGR_Args(rpat); RAW2BGR_Args(depth); RAW2BGR_Args(wben);  RAW2BGR_Args(expen);  RAW2BGR_Args(aesat);
	RAW2BGR_Args(blacklevel[0]); RAW2BGR_Args(blacklevel[1]); RAW2BGR_Args(blacklevel[2]); RAW2BGR_Args(blacklevel[3]);
	RAW2BGR_Args(wbgain[0]); RAW2BGR_Args(wbgain[1]); RAW2BGR_Args(wbgain[2]); RAW2BGR_Args(wbgain[3]);
	RAW2BGR_Args(sensorgain); RAW2BGR_Args(ispgain); RAW2BGR_Args(adrcgain); RAW2BGR_Args(shutter); RAW2BGR_Args(luxindex);
#undef RAW2BGR_Args
	//impl->verbose = 0;
	//impl->wben = !impl->wben;
	if (raw2->wben || impl->start)
		memcpy(impl->ccm, raw2->ccm, sizeof(raw2->ccm));

	float iso = log2f(raw2->sensorgain * gxr_max(raw2->ispgain, 1.f));
	int s0 = iso, s1 = gxr_min(s0 + 1, 7);
	float w = iso - s0;
	float* ynoise0 = &raw2->ynoise[s0 * 5], * ynoise1 = &raw2->ynoise[s1 * 5];
	float* cnoise0 = &raw2->cnoise[s0 * 5], * cnoise1 = &raw2->cnoise[s1 * 5];
	float* sharp0 = &raw2->sharp[s0 * 5], * sharp1 = &raw2->sharp[s1 * 5];
	for (int i = 0; i < 5; i++)
	{
		impl->ynoise[i] = ynoise0[i] + w * (ynoise1[i] - ynoise0[i]);
		impl->cnoise[i] = cnoise0[i] + w * (cnoise1[i] - cnoise0[i]);
		impl->sharp[i] = sharp0[i] + w * (sharp1[i] - sharp0[i]);
	}
	impl->bdlarge = raw2->bdlarge[s0] + w * (raw2->bdlarge[s1] - raw2->bdlarge[s0]);
	impl->roi.x = raw2->left; impl->roi.width = raw2->right - raw2->left;
	impl->roi.y = raw2->top; impl->roi.height = raw2->bottom - raw2->top;
	CV_Assert(raw2->height % 2 == 0 && raw2->width % 2 == 0);
	CV_Assert(impl->roi.height >= 256 && impl->roi.width >= 256);

	impl->raw = Mat(raw2->height, raw2->width, CV_16SC1, raw2->input);
	impl->dst = Mat(raw2->height, raw2->width, CV_8UC3, raw2->output);
	impl->awb = static_cast<AWB*>(raw2->handle[0]);
	if (!raw2->handle[1])
	{
		Size szblk(64, 64);
		int brows = ((raw2->height - impl->roi.height) >> 1) + impl->roi.height;
		int bcols = ((raw2->width - impl->roi.width) >> 1) + impl->roi.width;
		Size szsta = Size((bcols + szblk.width - 1) / szblk.width, (brows + szblk.height - 1) / szblk.height);
		raw2->handle[1] = new AE(szblk, szsta);
		impl->ae = static_cast<AE*>(raw2->handle[1]);
	}

	if (raw2->handle[2])
		impl->ocl = static_cast<OCL*>(raw2->handle[2]);

	size_t lscsz = raw2->lsc_height * raw2->lsc_width * sizeof(float);
	if (impl->wben || impl->start)
	{
		for (int cn = 0; cn < 4; ++cn)
		{
			// lsctable: R, Gr, Gb, B
			impl->lsctable[cn].create(raw2->lsc_height, raw2->lsc_width, CV_32FC1);
			memcpy(impl->lsctable[cn].data, raw2->lsctable[cn], lscsz);
		}
	}
	if (impl->start)
	{
		CV_Assert(raw2->gmtablex);
		CV_Assert(raw2->gmtabley);
		impl->gammatable.create(1, LINEARSAT + 1, CV_8UC1);
		memset(impl->gammatable.data, 0, LINEARSAT + 1);
		gen_gammatable(raw2->gmtablex, raw2->gmtabley, impl->gammatable.data);
	}

	if (impl->verbose * VerbParam)
	{
	#define RAW2BGR_Print(var, fmt) \
		GXR_Printf("%16s = " fmt "\n", #var, impl->var)
		RAW2BGR_Print(ynoise[0], "%f"); RAW2BGR_Print(ynoise[1], "%f"); RAW2BGR_Print(ynoise[2], "%f"); RAW2BGR_Print(ynoise[3], "%f"); RAW2BGR_Print(ynoise[4], "%f");
		RAW2BGR_Print(cnoise[0], "%f"); RAW2BGR_Print(cnoise[1], "%f"); RAW2BGR_Print(cnoise[2], "%f"); RAW2BGR_Print(cnoise[3], "%f"); RAW2BGR_Print(cnoise[4], "%f");
		RAW2BGR_Print(sharp[0], "%f"); RAW2BGR_Print(sharp[1], "%f"); RAW2BGR_Print(sharp[2], "%f"); RAW2BGR_Print(sharp[3], "%f"); RAW2BGR_Print(sharp[4], "%f");
		RAW2BGR_Print(bdlarge, "%f");
	#undef RAW2BGR_Print
	}
	impl->run_all();

	raw2->shutter = impl->shutter;
	raw2->sensorgain = impl->sensorgain;
	raw2->ispgain = impl->ispgain;
	if (impl->verbose & VerbParam)
		GXR_Printf("\tupdate shutter %u, sensorgain %.3f, ispgain %.3f\n", (unsigned)(raw2->shutter), raw2->sensorgain, raw2->ispgain);
}


int gxr_raw2bgr_initialize(gxr_raw2bgr * raw2)
{
	raw2->impl = NULL;
	raw2->lsc_height = 13;
	raw2->lsc_width = 17;
	raw2->lsctable[0] = raw2->lsctable[1] = raw2->lsctable[2] = raw2->lsctable[3] = NULL;
	raw2->gmtablex = raw2->gmtabley = NULL;
	raw2->verbose = 0;
	raw2->blacklevel[0] = raw2->blacklevel[1] = raw2->blacklevel[2] = raw2->blacklevel[3] = 0;
	raw2->wbgain[0] = raw2->wbgain[1] = raw2->wbgain[2] = raw2->wbgain[3] = 1.F;
	raw2->ccm[0] = +1.65625F; raw2->ccm[1] = -0.71875F; raw2->ccm[2] = 0.0625F;
	raw2->ccm[3] = -0.15625F; raw2->ccm[4] = +1.2890625F; raw2->ccm[5] = -0.1328125F;
	raw2->ccm[6] = +0.109375F; raw2->ccm[7] = -0.7265625F; raw2->ccm[8] = +1.6171875F;
	raw2->rpat = Raw_RGGB;
	raw2->bpc = 0;
	memset(raw2->ynoise, 0, sizeof(raw2->ynoise));
	memset(raw2->cnoise, 0, sizeof(raw2->cnoise));
	memset(raw2->sharp, 0, sizeof(raw2->sharp));
	memset(raw2->bdlarge, 0, sizeof(raw2->bdlarge));
	raw2->ispgain = raw2->sensorgain = raw2->adrcgain = 1.F;
	raw2->shutter = raw2->luxindex = 0.F;
	raw2->top = raw2->bottom = raw2->left = raw2->right = 0;
	memset(raw2->handle, 0, sizeof(raw2->handle));
	setNumThreads(getNumberOfCPUs());
	raw2->wben = 0;
	raw2->expen = 1;
	raw2->aesat = 4096;
	raw2->impl = new RAW2BGR();
	raw2->handle[0] = new AWB();
	raw2->handle[2] = new OCL();
	return 0;
}


void gxr_raw2bgr_release(gxr_raw2bgr * raw2)
{
	if (raw2->handle[0])
		delete static_cast<AWB*>(raw2->handle[0]);
	if (raw2->handle[1])
		delete static_cast<AE*>(raw2->handle[1]);
	if (raw2->handle[2])
		delete static_cast<OCL*>(raw2->handle[2]);
	memset(raw2->handle, 0, sizeof(raw2->handle));
	if (raw2->impl)
		delete static_cast<RAW2BGR*>(raw2->impl);
	raw2->impl = NULL;
}


void gxr_raw2bgr_run_all(gxr_raw2bgr * raw2)
{
	raw2->ticksum = raw2->ntick = 0;
	raw2->tickpre = getTickCount();
	//	if (raw2->verbose & VerbParam)
	//	{
	//#define RAW2BGR_Print(var, fmt) \
	//	GXR_Printf("%16s = " fmt "\n", #var, raw2->var)
	//		RAW2BGR_Print(verbose, "%hhd");
	//		GXR_Printf("%16s = %d, %d, %d, %d\n", "blacklevel", raw2->blacklevel[0], raw2->blacklevel[1], raw2->blacklevel[2], raw2->blacklevel[3]);
	//		GXR_Printf("%16s = %f, %f, %f, %f\n", "wbgain", raw2->wbgain[0], raw2->wbgain[1], raw2->wbgain[2], raw2->wbgain[3]);
	//		RAW2BGR_Print(sensorgain, "%f");     RAW2BGR_Print(ispgain, "%f");
	//		RAW2BGR_Print(adrcgain, "%f");     RAW2BGR_Print(luxindex, "%f"); RAW2BGR_Print(shutter, "%f");     RAW2BGR_Print(luxindex, "%f");
	//		RAW2BGR_Print(wben, "%d");     RAW2BGR_Print(expen, "%d");
	//		GXR_Printf("%16s = %dx%d\n",
	//			"input", raw2->width, raw2->height);
	//#undef RAW2BGR_Print
	//	}

	raw2bgr_impl(raw2);
	//raw2bgr_timerit(raw2, "raw2bgr impl");
}


GXR_RAW2BGR_VERSION gxr_raw2bgr_version()
{
	GXR_RAW2BGR_VERSION v;
	v.Version = VERSION_VERSION;
	return v;
}