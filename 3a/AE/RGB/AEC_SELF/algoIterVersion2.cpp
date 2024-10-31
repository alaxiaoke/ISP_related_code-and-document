#include <iostream>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include "aecAlgo.h"

using namespace std;

//global val


AECAlgo::AECAlgo()
{
}

AECAlgo::~AECAlgo()
{
}

double long log_2(double long x)
{
	return log(x) / log(2.0);
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

double lumaTable(float luma, int tableSelect) {
	double target = 200;
	switch (tableSelect)
	{
	case 1:
		//target table  1
		if (luma > 280)
		{
			target = 280;   //55   T1
		}
		else if (luma > 256 && luma <= 280)
		{
			//TARGETBRI = 256;   //55
			target = 5 * luma / 2 - 420;   //55
		}
		else if (luma > 220 && luma <= 256)
		{
			target = 220;   //55
		}
		else if (luma <= 220 && luma > 160)
		{
			//TARGETBRI = 200;   //50
			target = (4 * luma - 140) / 3;   //50
		}
		else if (luma <= 160 && luma > 100)
		{
			target = 150;   // 35  -->最终想使得目标亮度在200？
		}
		else
		{
			//TARGETBRI = 100;    //25
			target = 100 + 5 * luma / 14;    //25
		}
		break;

	case 2:
		//target table 2
		if (luma > 350)
		{
			target = 320;   //80   
		}
		else if (luma > 300 && luma <= 350)
		{
			target = 4 * luma / 5 + 40;   // 70-80
		}
		else if (luma > 260 && luma <= 300)
		{
			target = 280;   //70
		}
		else if (luma <= 260 && luma > 150)
		{
			target = (6 * luma + 1300) / 11;   // 50-65 
		}
		else if (luma <= 150)
		{
			target = 50 + luma;   // + 12.5  -->最终想使得目标亮度在50-65间？
		}
		break;

	default:
		target = 256;
		break;
	}
	//cout << "target cal is : " << luma << endl;
	//std::cout << "target select : " << target << endl;
	return target;
}

double LVTable(float lv) {
	if (lv > 700)
	{
		return 65.0 * 4;   //55
	}
	else if (lv > 500 && lv <= 700)
	{
		return 60 * 4 + (lv - 500) / 700.0 * 5; //60 -65
	}
	else if (lv <= 500 && lv > 200)
	{
		return 60.0 * 4 ;   //60
	}
	else
	{
		return 30 * 4 + lv / 200 * 120;  //30-60
	}
}

double LVTableHDR(float lv) {  // hdr case set lower target
	if (lv > 700)
	{
		return 45.0 * 4;   //45
	}
	else if (lv > 500 && lv <= 700)
	{
		return 40 * 4 + (lv - 500) / 700.0 * 5; //40 -45
	}
	else if (lv < 500 && lv >= 200)
	{
		return 40.0 * 4;   //40
	}
	else
	{
		return 20 * 4 + lv / 200 * 80;  //20-40
	}
}

int AECAlgo::init(InitParma* initParma)
{
	memset(&algoInput, 0, sizeof(AEC_algo_input));
	memset(&algoOut, 0, sizeof(AEC_algo_out));
	// APEX parameter initialization
		// Initial at LV 300    7ms 8X
	//apexStill.tv = 716;		//7ms
	//apexStill.sv = 644;		//8X   ISO = 8690
	//apexStill.lv = apexStill.tv + apexStill.av - apexStill.sv;    //300
	apexStill.av = 200 * log_2(initParma->F_number);		//F-number 2.2-->228

	memcpy(&imgInfo, &initParma->img, sizeof(ImgInfo));
	sumCnt = imgInfo.horBlockNum * imgInfo.verBlockNum;
	centerR = imgInfo.horBlockNum < imgInfo.verBlockNum ? imgInfo.horBlockNum : imgInfo.verBlockNum;  //modify by real statistic parameter
	bias = imgInfo.horBlockNum < imgInfo.verBlockNum ? (imgInfo.verBlockNum - imgInfo.horBlockNum) / 2 : (imgInfo.horBlockNum - imgInfo.verBlockNum) / 2;
	std::cout << "finished init" << endl;
	return 0;
}

void AECAlgo::setParam(AECSetParam* setParam)
{
	if (setParam == NULL)
	{
		std::cout << "setparam is null, error" << endl;
		return;
	}

	switch (setParam->type)
	{
	case EXPMODE:
		//set faceinfo
		memcpy(&algoInput.sCurExpoMode, setParam->param, sizeof(CurExpoMode));
		break;
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
		memcpy(&imgInfo, setParam->param, sizeof(FlickInfo));
		break;
	case SENSORINFO:
		//set sensorinfo
		memcpy(&algoInput.sensorInfo, setParam->param, sizeof(SensorInfo));
		break;
	case TUNINGDATA:
		
	default:
		break;
	}
}

void AECAlgo::setTuningData(AECTuningData* data)
{
	data->sigma = sigma;
	data->brightAreaPerc = brightAreaPrec;
	data->darkAreaPerc = darkAreaPrec;
	data->DRRatioTolerance = DRRatioTolerance;
	data->satNumberPercent = satNumberPercent;
	data->satRatioTolerance = satRatioTolerance;
	data->tableSelect = tableSelect;

	return;
}

void AECAlgo::getStats(int* ptr)
{
	for (int i = 0; i < sumCnt; i++)
	{
		algoInput.statsAE.sum[0][i] = ptr[i * 8];
		algoInput.statsAE.sum[1][i] = ptr[i * 8 + 1];
		algoInput.statsAE.sum[2][i] = ptr[i * 8 + 2];
		algoInput.statsAE.sum[3][i] = ptr[i * 8 + 3];
		algoInput.statsAE.non_satNum[0][i] = ptr[i * 8 + 4];
		algoInput.statsAE.non_satNum[1][i] = ptr[i * 8 + 5];
		algoInput.statsAE.non_satNum[2][i] = ptr[i * 8 + 6];
		algoInput.statsAE.non_satNum[3][i] = ptr[i * 8 + 7];
	}
	return;
}

void AECAlgo::getStatsFromRaw(AEC_stats* stats)
{
	memcpy(&algoInput.statsAE, stats, sizeof(AEC_stats));
	return;
}

void AECAlgo::getHist(int* ptr)
{
	memcpy(&algoInput.statsAE.histogram, ptr, sizeof(int)* 256);
	return;
}

/**************************************************
Function: CWtableCal()
Description: 计算中央重点权重测光表  --> 高斯函数权重
Input:  centerR----->权重中心半径
		bias --> 非正方形模版截取
		sigma  --> 标准差
return: 修改weight[BLOCKNUM_H_MAX][BLOCKNUM_W_MAX]  or weight[BLOCKNUM_H * BLOCKNUM_H_MAX]
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
				Weight[i * imgInfo.verBlockNum + j + bias] = guass[i][j];
			}
		}
	}
	return 0;
}

double AECAlgo::avglumaCal(AEC_stats* aecStats)
{
	int	satBlockCnt = 0;
	double sum = 0.0;
	double avg = 0.0;
	int pixelNum = imgInfo.blockHeight * imgInfo.blockWeight >> 2;
	int nonSatNum = pixelNum * satNumberPercent;
	double weiClip = 1.0;
	for (uint16_t i = 0; i < sumCnt; i++)
	{
		float avg_r = (float)aecStats->sum[0][i] / aecStats->non_satNum[0][i];  //over exposure pixel is not add.
		float avg_gr = (float)aecStats->sum[1][i] / aecStats->non_satNum[1][i];
		float avg_gb = (float)aecStats->sum[2][i] / aecStats->non_satNum[2][i];
		float avg_b = (float)aecStats->sum[3][i] / aecStats->non_satNum[3][i];
		//std::cout << i / imgInfo.verBlockNum << "," << i % imgInfo.verBlockNum << ":" << "avg_r gr gb b : " << avg_r << "  " << avg_gr << "  " << avg_gb << "  " << avg_b << endl;
		a[i] = (avg_r * RGB2Y[0] + (avg_gr + (avg_gb - avg_gr) / 2) * RGB2Y[1] + avg_b * RGB2Y[2]);
		//std::cout << i / imgInfo.verBlockNum << "," << i % imgInfo.verBlockNum << ":" << "avg_y:"<< a[i] << endl;
	}

	//all histogram avg， if weight table is ready, multi w[i]
	for (uint16_t i = 0; i < sumCnt; i++) {
		if (isnan(a[i]) || (((aecStats->non_satNum[2][i]) <= nonSatNum) && ((aecStats->non_satNum[0][i]) <= nonSatNum) && ((aecStats->non_satNum[3][i]) <= nonSatNum)))
		//if (isnan(a[i]))
		{
			satBlockCnt += 1;
			weiClip -= Weight[i];
			continue;
		}
		sum += a[i] * Weight[i];
		//sum += a[i];
	}
	//hdr judge
	double satRatio = (float)satBlockCnt / sumCnt;
	hdrFlag = false;
	//cout << "satRatio : " << satRatio << endl;
	//cout << "sum : " << sum << endl;
	//cout << "weiclip : " << weiClip << endl;
	if (satRatio >= satRatioTolerance) {
		hdrFlag = true;
	}
	//cal avgBri, sum piexl precent
	avg = sum / weiClip;
	return avg;
}

/**************Modify the target brightness according to the histogram **********************/
double AECAlgo::targetLumaModify(AEC_stats* aecStats)
{
	//tuning param
	float darkAreaTole = 0.3;
	float satAreaTole = 0.04;
	double bright_dark_ratioTole = 4;

	double sum = 0.0;
	double deltaTarget = 0.0;

	/*
	double brightAvg, darkAvg;
	////sort AvgY by down
	
	qsort(a, sumCnt, sizeof(float), compare);
	uint16_t cnt = 0;
	for (uint16_t i = 0; i < sumCnt * brightAreaPrec; i++) {
		if (isnan(a[i]))
		{
			continue;
		}
		cnt += 1;
		sum += a[i];
	}
	brightAvg = sum / cnt;
	sum = 0.0;
	cnt = 0;
	for (uint16_t i = sumCnt-1; i > sumCnt * (1-darkAreaPrec); i--) {
		if (isnan(a[i]))
		{
			continue;
		}
		cnt += 1;
		sum += a[i];
	}
	darkAvg = sum / cnt;
	double DRratio = log10(brightAvg / darkAvg);
		if (DRratio > 2 * DRRatioTolerance)
	{
		hdrFlag = true;
		deltaTarget = 15;
	}
	else if (DRratio > DRRatioTolerance) {
		hdrFlag = true;
		deltaTarget = (DRRatioTolerance - DRratio) / DRRatioTolerance * 10;
	}
	*/

	// sort by histgram
	uint64_t pixelNum = (1744 * 2328 >> 2) - 120186;  //histogram clip to 1428 * 1744 >>2 or clip_black 120186
	int brightSatvalue = 250;
	for (int i = brightSatvalue; i < 256; i++)
	{
		sum += aecStats->histogram[i];
	}
	double satAreaPrec = sum / pixelNum;

	if (satAreaPrec > satAreaTole)
	{
		hdrFlag = true;
		deltaTarget -= 5;
	}
	int darkSatvalue = 6;
	sum = 0.0;
	for (int i = 0; i < darkSatvalue; i++)
	{
		sum += aecStats->histogram[i];
	}
	double darkAreaPrec = sum / pixelNum;
	if (darkAreaPrec > darkAreaTole && hdrFlag == false)  //hdr case preserve bright area
	{
		deltaTarget += 5;
		return deltaTarget;
	}
	// super hdr --> over exposure 
	double BDRratio = satAreaPrec / darkAreaPrec;
#ifdef DEBUG
	std::cout << "satAreaPrec is : " << satAreaPrec << endl;
	std::cout << "darkAreaPrec is : " << darkAreaPrec << endl;
	std::cout << "BDRratio is : " << BDRratio << endl;
#endif // DEBUG

	if (BDRratio > 2 * bright_dark_ratioTole)
	{
		hdrFlag = true;
		deltaTarget -= 15;
	}
	else if (BDRratio > bright_dark_ratioTole) {
		hdrFlag = true;
		deltaTarget -= abs(bright_dark_ratioTole - BDRratio) / bright_dark_ratioTole * 10;
	}
	return deltaTarget;
}

int AECAlgo::aecProcess()
{
	int isStable = 0;
	double sum = 0.0;
	double avgY = 0.0;
	uint8_t total = 0;
	double avgBrightness, targetLuma, dRatio;
	double deltaEV, caldeltaEV;
	double tmpSensorGain, tmpExposureTime;
	uint32_t newExposureTime = algoInput.sensorInfo.sensorExposureTime;
	float newExposureGain = algoInput.sensorInfo.sensorExposureGain;

	// cal cur luma
	avgBrightness = avglumaCal(&algoInput.statsAE);
	std::cout << "avgBrightness :" << avgBrightness << endl;

	// target luma  lv-table
	double lumaModify = targetLumaModify(&algoInput.statsAE) * 4;
	//if (hdrFlag)
	//{
	//	TARGETBRI = LVTableHDR(apexStill.lv) + lumaModify;
	//}
	//else 
	TARGETBRI = LVTable(apexStill.lv) + lumaModify;
	//  luma table
	//targetLuma = avglumaCal(&algoInput.statsAE);
	//TARGETBRI = lumaTable(targetLuma, tableSelect);
	std::cout << "TARGETBRI select : " << TARGETBRI << endl;
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
	if (abs(avgBrightness - TARGETBRI) < AEC_COERD_TSHD && isStable && algoInput.sensorInfo.sensorExposureTime == newExposureTime)
	{
		std::cout << "exposure parames are suitable, needn't to modify" << endl;
		algoOut.exposureGain = newExposureGain;
		algoOut.exposureTime = newExposureTime;
		//std::cout << "current frame exposure parameteres: \n gain:" << algoInput.sensorInfo.sensorExposureGain << "; time:" << algoInput.sensorInfo.sensorExposureTime << endl;
		//std::cout << "next frame exposure parameteres: \n gain:" << newExposureGain << "; time:" << newExposureTime << endl;
		return 0;
	}
	// calculate the deltaEV
	deltaEV = 100 * (log_2(avgBrightness) - log_2(TARGETBRI));
	// save the previous LV
	//pprevious_LV = previous_LV;
	//previous_LV = apexStill.lv;
	// AE Step = 2
	if (abs(deltaEV) >= 300)
	{
		caldeltaEV = deltaEV / 8;
	}
	else if(abs(deltaEV) >= 150)
	{
		caldeltaEV = deltaEV / 4;
	}
	else
	{
		caldeltaEV = deltaEV / 2;
	}
	dRatio = pow(2, -(double)caldeltaEV / 100);
	tmpSensorGain = newExposureGain * dRatio;
	//std::cout << "need modify ratio is : " << dRatio << endl;

	if (avgBrightness > TARGETBRI)  // decrease the bri of next frame
	{
		if (tmpSensorGain >= MIN_SENSOR_GAIN) {
			newExposureGain = tmpSensorGain;
			apexStill.lv = apexStill.av + apexStill.tv - apexStill.sv + caldeltaEV;
			apexStill.sv -= caldeltaEV;
		}
		else {
			//调整曝光时间  简易lowlight\indoor\outdoor曝光策略表 --》 1、2、4、7ms  gain 变化
			if (caldeltaEV + apexStill.lv > 700) {
				newExposureTime = 1000;
				apexStill.tv = 997;
				newExposureGain = tmpSensorGain * algoInput.sensorInfo.sensorExposureTime / 1000.0;
				if (newExposureGain <= MIN_SENSOR_GAIN)
				{
					std::cout << "time is min 1ms,and A gain is min now, please modify d gain multiply:" << newExposureGain / MIN_SENSOR_GAIN << "else if d gain < 1,keep min dgain,here is a ultra bright case!" << endl;
					newExposureGain = MIN_SENSOR_GAIN;
				}
			}
			else if(caldeltaEV + apexStill.lv <= 700 && caldeltaEV + apexStill.lv >550)
			{
				newExposureTime = 2000;
				newExposureGain = tmpSensorGain * algoInput.sensorInfo.sensorExposureTime / 2000.0;
				apexStill.tv = 897;
			}
			else if (caldeltaEV + apexStill.lv <= 550 && caldeltaEV + apexStill.lv >400)	//   caldeltaEV + apexStill.lv < 600
			{
				newExposureTime = 4000;
				newExposureGain = tmpSensorGain * algoInput.sensorInfo.sensorExposureTime / 4000.0;
				apexStill.tv = 797;
			}
			else
			{
				newExposureTime = 7000;
				newExposureGain = tmpSensorGain * algoInput.sensorInfo.sensorExposureTime / 7000.0;
				apexStill.tv = 716;
			}
			apexStill.sv += 100 * log_2(newExposureGain / algoInput.sensorInfo.sensorExposureGain);
			apexStill.lv = apexStill.av + apexStill.tv - apexStill.sv;
		}
	}
	else   // increase the bri of next frame
	{
		if (tmpSensorGain <= 8) {  //1/2/4 ms max gain 8x,else add time;  time priorty in 7 ms
			newExposureGain = tmpSensorGain;
			apexStill.lv = apexStill.av + apexStill.tv - apexStill.sv + caldeltaEV;
			apexStill.sv -= caldeltaEV;
		}
		else if (newExposureTime ==7000) {   //7ms is max exposure time, keep add A gain
			newExposureGain = tmpSensorGain;
			if (newExposureGain >= MAX_SENSOR_GAIN)
			{
				std::cout << "time is max 7ms,and A gain is max now, please modify d gain multiply:" << newExposureGain / MAX_SENSOR_GAIN << endl;
				newExposureGain = MAX_SENSOR_GAIN;
			}
			apexStill.sv += 100 * log_2(newExposureGain / algoInput.sensorInfo.sensorExposureGain);
			apexStill.lv = apexStill.av + apexStill.tv - apexStill.sv;
		}
		else {
			//调整曝光时间
			if (caldeltaEV + apexStill.lv > 700) {
				newExposureTime = 1000;
				apexStill.tv = 997;
				newExposureGain = tmpSensorGain * algoInput.sensorInfo.sensorExposureTime / 1000.0;
			}
			else if (caldeltaEV + apexStill.lv <= 700 && caldeltaEV + apexStill.lv > 550)
			{
				newExposureTime = 2000;
				newExposureGain = tmpSensorGain * algoInput.sensorInfo.sensorExposureTime / 2000.0;
				apexStill.tv = 897;
			}
			else if (caldeltaEV + apexStill.lv <= 550 && caldeltaEV + apexStill.lv > 400)	//   caldeltaEV + apexStill.lv < 600
			{
				newExposureTime = 4000;
				newExposureGain = tmpSensorGain * algoInput.sensorInfo.sensorExposureTime / 4000.0;
				apexStill.tv = 797;
			}
			else
			{
				newExposureTime = 7000;
				apexStill.tv = 716;
				newExposureGain = tmpSensorGain * algoInput.sensorInfo.sensorExposureTime / 7000.0;
				if (newExposureGain >= MAX_SENSOR_GAIN)
				{
					std::cout << "time is max 7ms,and A gain is max now, please modify d gain multiply:" << newExposureGain / MAX_SENSOR_GAIN << endl;
					newExposureGain = MAX_SENSOR_GAIN;
				}
			}
			apexStill.sv += 100 * log_2(newExposureGain / algoInput.sensorInfo.sensorExposureGain);
			apexStill.lv = apexStill.av + apexStill.tv - apexStill.sv;
		}
	}

	algoOut.exposureGain = newExposureGain;
	algoOut.exposureTime = newExposureTime;
#ifdef DEBUG
	std::cout << "current frame exposure parameteres: \n gain:" << algoInput.sensorInfo.sensorExposureGain << "; time:" << algoInput.sensorInfo.sensorExposureTime << endl;
	std::cout << "next frame exposure parameteres: \n gain:" << newExposureGain << "; time:" << newExposureTime << endl;
#endif // DEBUG


	return 0;
}

void AECAlgo::algoCtrl()
{
	//skip setup stream
	//static int InitSkipFrame = 0;
	//if (InitSkipFrame < SKIP_FRAME)
	//{
	//	InitSkipFrame += 1;
	//	algoOut.exposureTime = algoInput.sensorInfo.sensorExposureTime;
	//	algoOut.exposureGain = algoInput.sensorInfo.sensorExposureGain;
	//	return;
	//}
	switch (algoInput.sCurExpoMode)
	{
	case MANU_EXP:
		algoOut.exposureGain = algoInput.sensorInfo.sensorExposureGain;
		algoOut.exposureTime = algoInput.sensorInfo.sensorExposureTime;
		break;
	case AntiFlickModeAE:
		break;
	default:
		AutoExp(&algoInput, &algoOut);
		break;
	}
	return;
}

void AECAlgo::getAEout(AEC_algo_out* out)
{
	memcpy(out, &algoOut, sizeof(AEC_algo_out));
	return;
}

int AECAlgo::AutoExp(AEC_algo_input* input, AEC_algo_out* out)
{
	//start = clock();
	// cal apexStill
	apexStill.tv = -100 * log_2(algoInput.sensorInfo.sensorExposureTime) + 2000;   // us
	apexStill.sv = 100 * log_2(algoInput.sensorInfo.sensorExposureGain * 10.86);   // 8X IS0 = 8690
	apexStill.lv = apexStill.tv + apexStill.av - apexStill.sv;

	// cal luma weight
	CWtableCal(centerR, bias, sigma);

	// call algo for first cal
	aecProcess();

	std::cout << "final results --> next frame exposure parameteres: \n gain:" << algoOut.exposureGain << "; time:" << algoOut.exposureTime << endl;

	// make sure does modify to sensor
	if (algoInput.sensorInfo.sensorExposureGain != algoOut.exposureGain) {
		std::cout << "gain is modify, please send the result to hardware" << endl;
	}
	if (algoInput.sensorInfo.sensorExposureTime != algoOut.exposureTime)
	{
		std::cout << "exposure time is modify, please check it is necessary?" << endl;
	}
	return 0;
}

int AECAlgo::ATFExp(AEC_algo_input* input, AEC_algo_out* out)
{
	return 0;
}

int AECAlgo::deinit()
{

	return 0;
}