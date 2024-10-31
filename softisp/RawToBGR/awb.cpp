#include "raw2bgr.hpp"

using namespace gxr;

#ifndef GXR_AWB
#  define GXR_AWB 1
#endif

#if GXR_AWB

#include "gxr_awb.hpp"

AWB::AWB(int lsc_rows, int lsc_cols)
	: handle(0)
{
	verbose = 0;
	handle = new AWBAlgo(lsc_rows * lsc_cols);
	CV_Assert(handle);
	AWBAlgo* awb_p = static_cast<AWBAlgo*>(handle);
	awb_p->config(nullptr);
}


AWB::~AWB()
{
	if (handle)
		delete static_cast<AWBAlgo*>(handle);
	handle = nullptr;
}


void AWB::feedstats(Mat& blksum_)
{
	blksum_.copyTo(blksum);
	static_cast<AWBAlgo*>(handle)->readStats(blksum.ptr<int>(), blksum.total());
}


void AWB::estimate()
{
	AWBAlgo* awb_p = static_cast<AWBAlgo*>(handle);
	awb_p->estimate();
}


void AWB::getwb(float* rgain, float* bgain)
{
	AWBAlgo* awb_p = static_cast<AWBAlgo*>(handle);
	*rgain = 1.f / awb_p->final_gain[0];
	*bgain = 1.f / awb_p->final_gain[1];
}


void AWB::getccm(float* ccm)
{
	AWBAlgo* awb_p = static_cast<AWBAlgo*>(handle);
	memcpy(ccm, awb_p->final_ccm, sizeof(awb_p->final_ccm));
}


void AWB::getlsc(array<Mat, 4>& lsctable)
{
	int lscsz = lsctable[0].total();
	AWBAlgo* awb_p = static_cast<AWBAlgo*>(handle);
	memcpy(lsctable[0].data, awb_p->final_lsctable, sizeof(float) * lscsz);
	memcpy(lsctable[1].data, awb_p->final_lsctable + lscsz, sizeof(float) * lscsz);
	memcpy(lsctable[2].data, awb_p->final_lsctable + lscsz, sizeof(float) * lscsz);
	memcpy(lsctable[3].data, awb_p->final_lsctable + lscsz * 2, sizeof(float) * lscsz);
}

#else

AWB::AWB(int lsc_rows, int lsc_cols) {}

AWB::~AWB() {}

void AWB::feedstats(Mat& blksum_) {}

void AWB::estimate() {}

void AWB::getwb(float* rgain, float* bgain) {}

void AWB::getccm(float ccm[9]) {}

void AWB::getlsc(array<Mat, 4>& lsctable) {}

#endif



