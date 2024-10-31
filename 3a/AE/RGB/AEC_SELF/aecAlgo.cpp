#include <iostream>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <map>

using namespace std;

#include "aecAlgo.h"

#define SKIP_FRAME 3
#define MIN_SENSOR_EXP		10   //10us
#define MAX_SENSOR_EXP		10000  //10ms
#define MIN_SENSOR_GAIN		1
#define MAX_SENSOR_GAIN		64
#define HISNUM				6
#define	 AEC_COERD_TSHD		8
//global val
float a[BLOCKNUM_H * BLOCKNUM_W];
float Weight[BLOCKNUM_H * BLOCKNUM_W] = { 0.0 };
double TARGETBRI = 200;
double aHistoryY[HISNUM] = { -999999.0, -999999.0, -999999.0, -999999.0, -999999.0, -999999.0 };

AECAlgo::AECAlgo()
{

}

AECAlgo::~AECAlgo()
{
}

int AECAlgo::init(InitParma* initParma)
{

	cout << "finished init" << endl;
	return 0;
}

void AECAlgo::setParam(AECSetParam* setParam)
{
	if (setParam == NULL)
	{
		cout << "setparam is null, error" << endl;
		return;
	}

	switch (setParam->type)
	{
	case FACEINFO:
		//set faceinfo
		memcpy(&algoInput.faceInfo, setParam->param, sizeof(FaceInfo));
		break;
	case FLICKINFO:
		//set filckinfo
		memcpy(&algoInput.flickInfo, setParam->param, sizeof(FlickInfo));
		break;
	case IMGINFO:
		//set imginfo
		memcpy(&algoInput.imgInfo, setParam->param, sizeof(FlickInfo));
		break;
	case SENSORINFO:
		//set sensorinfo
		memcpy(&algoInput.sensorInfo, setParam->param, sizeof(SensorInfo));
		break;
	default:
		break;
	}
}

/**************************************************
Function: CWtableCal()
Description: 计算中央重点权重测光表  --> 高斯函数权重
Input:  centerR----->权重中心半径
		bias --> 非正方形模版截取
		sigma  --> 标准差
return: 修改weight[BLOCKNUM_H][BLOCKNUM_W]  or weight[BLOCKNUM_H * BLOCKNUM_W]
**************************************************/
/**************Calculate the target brightness weight table -- central weight **********************/
int AECAlgo::CWtableCal(int iSize, int bias, float sigma) {
	double** guass;
	double sum = 0;
	double x2 = 0;
	double y2 = 0;
	int center = (iSize - 1) / 2;
	guass = new double* [iSize];//注意，double*[k]表示一个有K个元素的指针数组

	for (int i = 0; i < iSize; ++i)
	{
		guass[i] = new double[iSize];
	}
	for (int i = 0; i < iSize; i++)
	{//使用x2，y2降低了运算速度，提高了程序的效率
		x2 = pow(double(i - center), 2);
		for (int j = 0; j < iSize; j++)
		{
			y2 = pow(double(j - center), 2);
			sum += guass[i][j] = exp(-(float)(x2 + y2) / (2 * sigma * sigma));
		}
	}
	if (sum != 0)
	{
		//归一化  
		for (int i = 0; i < iSize; i++)
		{
			for (int j = 0; j < iSize; j++)
			{
				guass[i][j] /= sum;
				Weight[i * BLOCKNUM_W + j + bias] = guass[i][j];
			}
		}
	}
	return 0;
}

int compare(const void* e1, const void* e2)
{
	int* p1 = (int*)e1;
	int* p2 = (int*)e2;
	if (*p1 > *p2)
	{
		return -1;
	}
	else if (*p1 == *p2)
	{
		return 0;
	}
	else if (*p1 < *p2)
	{
		return 1;
	}
}

/**************Calculate the target brightness according to the histogram **********************/
double AECAlgo::targetLumaCal(AEC_stats* aecStats)
{
	uint32_t i, j;
	double sum = 0.0;
	double target = 0.0;
	int sumCnt = BLOCKNUM_H * BLOCKNUM_W;

	for (i = 0; i < BLOCKNUM_H * BLOCKNUM_W; i++)
	{
		a[i] = aecStats->histogram[i];
	}
	//sort histogram by down
	qsort(a, BLOCKNUM_H * BLOCKNUM_W, sizeof(float), compare);
	//pich best segment for cal, tunable param or fixed empirical value 
	for (i = 1 * (BLOCKNUM_H * BLOCKNUM_W) / 18; i < 17 * (BLOCKNUM_H * BLOCKNUM_W) / 18; i++) {
		if (isnan(a[i]))
		{
			sumCnt -= 1;
			continue;
		}
		sum += a[i] * Weight[i];
	}
	//cal targetBri, divide sum piexl precent
	//target = sum / sumCnt * (9.0 / 8);
	target = sum * (9.0 / 8);
	return target;
}

double AECAlgo::avglumaCal(AEC_stats* aecStats)
{
	uint32_t i, j;
	double sum = 0.0;
	double avg = 0.0;
	int sumCnt = BLOCKNUM_H * BLOCKNUM_W;
	int pixelNum = BLOCK_H * BLOCK_W >> 2;
	double weiClip = 1.0;
	for (i = 0; i < BLOCKNUM_H * BLOCKNUM_W; i++)
	{
		a[i] = aecStats->histogram[i];
	}
	//all histogram avg， if weight table is ready, multi w[i]
	for (i = 0; i < BLOCKNUM_H * BLOCKNUM_W; i++) {
		if (isnan(a[i]) || (((aecStats->non_satNum[2][i]) <= pixelNum * 9 / 10) && ((aecStats->non_satNum[0][i]) <= pixelNum * 9 / 10) && ((aecStats->non_satNum[3][i]) <= pixelNum * 9 / 10)))
			//if (isnan(a[i]))
		{
			sumCnt -= 1;
			weiClip -= Weight[i];
			continue;
		}
		//sum += a[i];
		sum += a[i] * Weight[i];
	}
	//cal avgBri, sum piexl precent
	//avg = sum / sumCnt;
	avg = sum / weiClip;
	return avg;
}

int AECAlgo::aecProcess(AEC_algo_input* input, AEC_algo_out* out)
{
	int isStable = 0;
	double sum = 0.0;
	double avgY = 0.0;
	uint8_t total = 0;
	double avgBrightness, dRatio;
	double tmpSensorGain, tmpExposureTime;
	//uint32_t newExposureTime = 7000;  //fixed time is 7ms
	uint32_t newExposureTime = input->sensorInfo.sensorExposureTime;
	float newExposureGain = input->sensorInfo.sensorExposureGain;

	avgBrightness = avglumaCal(&input->statsAE);
	std::cout << "avgBrightness :" << avgBrightness << endl;
	for (int i = 0; i < HISNUM; i++) {
		avgY += aHistoryY[i];
	}
	//the gap of new bri with history bri is in the threshold range or not? flag of stable
	if (abs(avgBrightness * HISNUM - avgY) < AEC_COERD_TSHD * HISNUM && avgY >= 0) {
		//the avgBri is ok, right shift to save the avgBri in the first
		for (int i = HISNUM - 1; i > 0; i--) {
			aHistoryY[i] = aHistoryY[i - 1];
		}
		aHistoryY[0] = avgBrightness;
		isStable = 1;
	}
	else {
		// out of range, all the memory avg is replaced by new avgBri, stable is defined for bv change?
		for (int i = 0; i < HISNUM; i++)
		{
			aHistoryY[i] = avgBrightness;
		}
		isStable = 0;
	}
	//|target - avgBri| in the threshold range or not， fixed exposureTime
	if (abs(avgBrightness - TARGETBRI) < AEC_COERD_TSHD && isStable && input->sensorInfo.sensorExposureTime == newExposureTime)
	{
		std::cout << "exposure parames are suitable, needn't to modify" << endl;
		out->exposureGain = newExposureGain;
		out->exposureTime = newExposureTime;
		//std::cout << "current frame exposure parameteres: \n gain:" << input->sensorInfo.sensorExposureGain << "; time:" << input->sensorInfo.sensorExposureTime << endl;
		//std::cout << "next frame exposure parameteres: \n gain:" << newExposureGain << "; time:" << newExposureTime << endl;
		return 0;
	}
	// cal the new exposure param
	dRatio = (float)TARGETBRI / avgBrightness;
	cout << "need modify ratio is : " << dRatio << endl;
	tmpExposureTime = newExposureTime * dRatio;
	double tRatio = (float)newExposureTime / (float)input->sensorInfo.sensorExposureTime;
	tmpSensorGain = newExposureGain * dRatio * tRatio;
	if (avgBrightness > TARGETBRI)  // decrease the bri of next frame
	{
		if (tmpSensorGain >= MIN_SENSOR_GAIN) {
			newExposureGain = tmpSensorGain;
		}
		else {
			newExposureGain = MIN_SENSOR_GAIN;
			//gain is min, need to decrease time
			float exposureTime_ref = newExposureTime * (tmpSensorGain / MIN_SENSOR_GAIN);
			std::cout << "gain is minimized,and we need decreased the exposuretime, time cal ref is:" << exposureTime_ref << endl;
			//newExposureTime = tmpExposureTime;
		}
	}
	else   // increase the bri of next frame
	{
		if (tmpSensorGain <= MAX_SENSOR_GAIN) {
			newExposureGain = tmpSensorGain;
		}
		else {
			newExposureGain = MAX_SENSOR_GAIN;
			// gain is max, need to increase time, or again reach max, then assign to dgain
			float dgain = tmpSensorGain / MAX_SENSOR_GAIN;
			float exposureTime_ref = newExposureTime * dgain;
			std::cout << "sensor gain is maximum,and we need increased the exposuretime or dgain，time cal ref is:" << exposureTime_ref << "or make digtal gain is :" << dgain << endl;
			//newExposureTime = tmpExposureTime;
		}
	}

	out->exposureGain = newExposureGain;
	out->exposureTime = newExposureTime;
	std::cout << "current frame exposure parameteres: \n gain:" << input->sensorInfo.sensorExposureGain << "; time:" << input->sensorInfo.sensorExposureTime << endl;
	std::cout << "next frame exposure parameteres: \n gain:" << newExposureGain << "; time:" << newExposureTime << endl;
	return 0;
}

void AECAlgo::algoCtrl(AEC_algo_input* input, AEC_algo_out* out)
{
	//skip setup stream
	//static int InitSkipFrame = 0;
	//if (InitSkipFrame < SKIP_FRAME)
	//{
	//	InitSkipFrame += 1;
	//	out->exposureTime = input->sensorInfo.sensorExposureTime;
	//	out->exposureGain = input->sensorInfo.sensorExposureGain;
	//	return;
	//}

	//start = clock();
	// cal luma weight
	float sigma = 1;   // central circle raidus 3*sigma   在大概3σ距离之外的像素都可以看作不起作用
	int centerR = BLOCKNUM_H < BLOCKNUM_W ? BLOCKNUM_H : BLOCKNUM_W;;
	int bias = BLOCKNUM_H < BLOCKNUM_W ? (BLOCKNUM_W - BLOCKNUM_H) / 2 : (BLOCKNUM_H - BLOCKNUM_W) / 2;
	CWtableCal(centerR, bias, sigma);
	// cal target luma
	//double targetLuma = targetLumaCal(&algoInput.statsAE);
	double targetLuma = avglumaCal(&input->statsAE);
	int tableSelect = 2;
	switch (tableSelect)
	{
	case 1:
		//target table  1
		if (targetLuma > 280)
		{
			TARGETBRI = 280;   //55   T1
		}
		else if (targetLuma > 256 && targetLuma <= 280)
		{
			//TARGETBRI = 256;   //55
			TARGETBRI = 5 * targetLuma / 2 - 420;   //55
		}
		else if (targetLuma > 220 && targetLuma <= 256)
		{
			TARGETBRI = 220;   //55
		}
		else if (targetLuma <= 220 && targetLuma > 160)
		{
			//TARGETBRI = 200;   //50
			TARGETBRI = (4 * targetLuma - 140) / 3;   //50
		}
		else if (targetLuma <= 160 && targetLuma > 100)
		{
			TARGETBRI = 150;   // 35  -->最终想使得目标亮度在200？
		}
		else
		{
			//TARGETBRI = 100;    //25
			TARGETBRI = 100 + 5 * targetLuma / 14;    //25
		}
		break;

	case 2:
		//target table 2
		if (targetLuma > 350)
		{
			TARGETBRI = 320;   //80   
		}
		else if (targetLuma > 300 && targetLuma <= 350)
		{
			TARGETBRI = 4 * targetLuma / 5 + 40;   // 70-80
		}
		else if (targetLuma > 260 && targetLuma <= 300)
		{
			TARGETBRI = 280;   //70
		}
		else if (targetLuma <= 260 && targetLuma > 150)
		{
			TARGETBRI = (6 * targetLuma + 1300) / 11;   // 50-65 
		}
		else if (targetLuma <= 150)
		{
			TARGETBRI = 50 + targetLuma;   // + 12.5  -->最终想使得目标亮度在50-65间？
		}
		break;

	default:
		TARGETBRI = 256;
		break;
	}
	//cout << "target cal is : " << targetLuma << endl;
	cout << "TARGETBRI select : " << TARGETBRI << endl;

	// call algo for first cal
	aecProcess(input, out);

	std::cout << "final results --> next frame exposure parameteres: \n gain:" << out->exposureGain << "; time:" << out->exposureTime << endl;

	// make sure does modify to sensor
	if (input->sensorInfo.sensorExposureGain != out->exposureGain) {
		cout << "gain is modify, please send the result to hardware" << endl;
	}
	if (input->sensorInfo.sensorExposureTime != out->exposureTime)
	{
		cout << "exposure time is modify, please check it is necessary?" << endl;
	}
}
