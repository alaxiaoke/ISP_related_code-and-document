#include "raw2bgr.hpp"

using namespace gxr;

#ifndef GXR_AE
#  define GXR_AE 1
#endif

#if GXR_AE

#include "aecHandle.h"

AE::AE(Size szblk, Size szsta)
	: handle(0)
{
	verbose = 0;
	memset(param, 0, sizeof(param));
	handle = new AEC();
	CV_Assert(handle);
	AEC* aec_p = static_cast<AEC*>(handle);
	
	InitParma* initParma = (InitParma*)malloc(sizeof(InitParma));
	param[0] = initParma;
	memset(initParma, 0, sizeof(InitParma));
	// set init fo statis value
	initParma->F_number = 2.2;
	initParma->img.blockHeight = szblk.height;
	initParma->img.blockWeight = szblk.width;
	initParma->img.horBlockNum = szsta.width;
	initParma->img.verBlockNum = szsta.height;
	aec_p->init(initParma);

	AECSetParam* setParam = (AECSetParam*)malloc(sizeof(AECSetParam));
	param[1] = setParam;
	memset(setParam, 0, sizeof(AECSetParam));
	setParam->param = (setParamUnion*)malloc(sizeof(setParamUnion));
	param[2] = setParam->param;
	//set ae mode
	setParam->type = EXPMODE;
	CurExpoMode expMode = AUTO_EXP;
	memcpy(setParam->param, &expMode, sizeof(CurExpoMode));
	aec_p->setParam(setParam);
}

AE::~AE()
{
	for (int i = 0; i < 8; i++)
		if (param[i])
			free(param[i]);
	memset(param, 0, sizeof(param));
	if (handle)
		delete static_cast<AEC*>(handle);
	handle = nullptr;
}

void AE::feedstats(Mat& blkstats_, Mat& histgram_, unsigned shutter, float gain, int fps)
{
	blkstats_.copyTo(blkstats);
	histgram_.copyTo(histgram);
	AEC* aec_p = static_cast<AEC*>(handle);

	// set set paramet  face\ flick\ sensor
	AECSetParam* setParam = (AECSetParam*)param[1];
	setParam->type = SENSORINFO;
	SensorInfo sensorinfo;
	sensorinfo.sensorExposureGain = gain;
	sensorinfo.sensorExposureTime = shutter;
	sensorinfo.frameRate = fps;
	memcpy(setParam->param, &sensorinfo, sizeof(SensorInfo));
	aec_p->setParam(setParam);

	// get statistic from isp
	aec_p->getStats(blkstats.ptr<int>());
	aec_p->getHist(histgram.ptr<int>());
}

void AE::estimate()
{
	AEC* aec_p = static_cast<AEC*>(handle);
	aec_p->process();
}

void AE::getexp(unsigned* shutter, float* gain)
{
	AEC* aec_p = static_cast<AEC*>(handle);
	AEC_algo_out out;
	aec_p->getAEOut(&out);
	*shutter = out.exposureTime;
	*gain = out.exposureGain;
}

#else

AE::AE(Size szblk, Size szsta) {}

AE::~AE() {}

void AE::feedstats(Mat& blkstats_, Mat& histgram_, unsigned shutter, float gain, int fps) {}

void AE::estimate() {}

void AE::getexp(unsigned* shutter, float* gain) {}

#endif



