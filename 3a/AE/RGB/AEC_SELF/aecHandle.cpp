#include <iostream>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <direct.h>
#include <string>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "aecHandle.h"

// block size
#define BLOCK_W  64
#define BLOCK_H  64
// img size
#define IMG_W	2328
#define IMG_H	1744
//#define IMG_H	1748
//block number
#define BLOCKNUM_W  36		//max
#define BLOCKNUM_H  27		//max

using namespace cv;
using namespace std;

AEC::AEC()
{
	int verbose = 0;
	handle = nullptr;
	cout << "<-- AEC start -->" << endl;
}

AEC::~AEC()
{
	if (handle)
		delete static_cast<AECAlgo*>(handle);
	handle = nullptr;
}

void AEC::init(InitParma* initParma)
{
	if (handle == nullptr) {
		handle = new AECAlgo();
		// CV_Assert(handle);
		AECAlgo* aec_p = static_cast<AECAlgo*>(handle);
		aec_p->init(initParma);
	}
}

void AEC::setParam(AECSetParam* setParam)
{
	AECAlgo* aec_p = static_cast<AECAlgo*>(handle);
	aec_p->setParam(setParam);
}

void AEC::getStats(int* p)
{
	AECAlgo* aec_p = static_cast<AECAlgo*>(handle);
	aec_p->getStats(p);
	return;
}

void AEC::getStatsFromRaw(AEC_stats* stats)
{
	AECAlgo* aec_p = static_cast<AECAlgo*>(handle);
	aec_p->getStatsFromRaw(stats);
	return;
}

void AEC::getHist(int* p)
{
	AECAlgo* aec_p = static_cast<AECAlgo*>(handle);
	aec_p->getHist(p);
	return;
}

void AEC::process()
{
	AECAlgo* aec_p = static_cast<AECAlgo*>(handle);
	aec_p->algoCtrl();
}

void AEC::getAEOut(AEC_algo_out* out)
{
	AECAlgo* aec_p = static_cast<AECAlgo*>(handle);
	aec_p->getAEout(out);
}

int AECStatsProcess(uint16_t* p, AEC_stats* stats)
{
	if (p ==NULL)
	{
		return -1;
	}
	if (stats == NULL)
	{
		return -1;
	}
	memset(stats, 0, sizeof(AEC_stats));
	float RGB2Y[3] = { 0.299, 0.587, 0.114 };
	if (p == NULL) {
		std::cout << "source file open is null";
		return 1;
	}
	int clip_w = (IMG_W - (BLOCKNUM_W * BLOCK_W)) / 2;
	int clip_h = (IMG_H - (BLOCKNUM_H * BLOCK_H)) / 2;
	if (clip_w < 0) {
		std::cout << "block width size set error , out of range";
		return 1;
	}
	if (clip_h < 0) {
		std::cout << "sblock height size set error , out of range";
		return 1;
	}
	uint16_t satValue = 1024 * 0.97;   //可调参数
	for (int y = clip_h; y + clip_h < IMG_H; y++) {
		int r = (y - clip_h) / BLOCK_H;
		int index_add = y * IMG_W;
		for (int x = clip_w; x + clip_w < IMG_W; x++) {
			int c = (x - clip_w) / BLOCK_W;
			int blockNum = r * BLOCKNUM_W + c;
			uint16_t pixel_index = index_add + x;
			uint16_t value = p[pixel_index] < 64 ? 0 : p[pixel_index] - 64;  // blc ?
			//uint16_t value = ((p[2 * pixel_index + 1] & 0x3) << 8) + p[2 * pixel_index] -64;  // blc ?
			value *= (1023.0 / 959);
			//cout << value << "	;";
			if (value <= satValue) {
				stats->sum[(y % 2) * 2 + x % 2][blockNum] += value;   //r/gr/gb/b
				stats->non_satNum[(y % 2) * 2 + x % 2][blockNum] += 1;
			}
		}
	}

	return 0;
}

int statsProc(uint16_t* p, int* pStats, int rows, int cols, int imgH, int imgW) {
	if (p == NULL) {
		std::cout << "source file open is null";
		return -1;
	}
	memset(pStats, 0, sizeof(int) * rows * cols);

	int blockW = imgW / cols;
	int blockH = imgH / rows;
	int clip_w = (imgW - (cols * blockW)) / 2;
	int clip_h = (imgH - (rows * blockH)) / 2;
	if (clip_w < 0) {
		std::cout << "block width size set error , out of range";
		return -1;
	}
	if (clip_h < 0) {
		std::cout << "sblock height size set error , out of range";
		return -1;
	}
	uint16_t satValue = 1024 * 0.97;   //可调参数
	for (int y = clip_h; y + clip_h < imgH; y++) {
		int r = (y - clip_h) / blockH;
		int index_add = y * imgW;
		for (int x = clip_w; x + clip_w < imgW; x++) {
			int c = (x - clip_w) / blockW;
			int blockNum = r * cols + c;
			uint16_t pixel_index = index_add + x;
			uint16_t value = p[pixel_index] < 64 ? 0 : p[pixel_index] - 64;  // blc ?
			value *= (1023.0 / 959);
			//cout << value << "	;";
			if (value <= satValue) {
				pStats[(y % 2) * 2 + x % 2 + blockNum * 8] += value;   //r/gr/gb/b
				pStats[(y % 2) * 2 + x % 2 + blockNum * 8 + 4] += 1;
			}
		}
	}
	return 0;
}

int histProc(uint16_t* ptr, int* pHist, int rows, int cols,int clip_h, int clip_w, int clip_black) 
{	
	float RGB2Y[3] = { 0.299, 0.587, 0.114 };
	if (pHist == NULL) {
		std::cout << "Hist ptr is null";
		return -1;
	}
	memset(pHist, 0, sizeof(int) * 256);
	//创建三维数组
	int*** rggb;  // R:0, Gr:1, Gb:2, B:3
	int i, j, k;
	// 自己分配一个三维的内存
	rggb = (int***)malloc(cols / 2 * sizeof(int**));    //第一维
	memset(rggb, 0, cols / 2 * sizeof(int**));
	//分配二维的
	for (i = 0; i < cols / 2; i++)
	{
		*(rggb + i) = (int**) malloc(rows / 2 * sizeof(int*));
		memset(*(rggb + i), 0, rows / 2 * sizeof(int*));
		//分配三维
		for (j = 0; j < rows / 2; j++)
		{
			*(*(rggb + i) + j) = (int*)malloc(4 * sizeof(int));
			memset(*(*(rggb + i) + j), 0, 4 * sizeof(int));
		}
	}

	// get rggb value
	for (int y = clip_h; y + clip_h< rows; y++)
	{
		for (int x = clip_w; x + clip_w < cols; x++)
		{
			uint16_t value = ptr[y * cols + x] < 64 ? 0 : ptr[y * cols + x] - 64;
			value *= (1023.0 / 959);
			rggb[x / 2][y / 2][(y % 2) * 2 + x % 2] = value;
		}
	}
	//get piexl level luma --> resize()-->1/4
	//pHist[0] -= ((clip_h * cols + clip_w * rows - clip_w * clip_h) * 2 >> 2);  // or x、y起始roi修改
	//减去clip掉的像素,rggb初始值  
	for (int y = clip_h / 2; y + clip_h / 2 < rows/2; y++)
	{
		for (int x = clip_w/2; x + clip_w / 2 < cols/2; x++)
		{
			int avgY = (int)(rggb[x][y][0] * RGB2Y[0] + (rggb[x][y][1] + (rggb[x][y][2] - rggb[x][y][1]) / 2) * RGB2Y[1] + rggb[x][y][3] * RGB2Y[2]) >> 2;
			pHist[avgY] += 1;
		}
	}
	pHist[0] -= clip_black;
	//释放内存  3->2->1  与创建相反
	for (i = 0; i < cols / 2; i++)
	{
		for (j = 0; j < rows / 2; j++)
		{
			free((rggb[i][j]));
		}
		free(rggb[i]);
	}
	free(rggb);
	return 0;
}

int showHist(int h, int w, int*pHist) {

	Mat src_hist = Mat::zeros(h + 5, w + 5, CV_8UC3);//直方图初始化，+3是为了防止像素0和255数量失真
	int c_max = 0;
	int c_min = 10000000;
	//比较每个像素对应个数得最大值，最小值
	for (int k = 0; k < 256; k++)
	{
		c_min = (c_min <= pHist[k]) ? c_min : pHist[k];
		c_max = (c_max >= pHist[k]) ? c_max : pHist[k];
	}
	//将像素个数的大小，（缩放）自适应至直方图的高
	for (int k = 0; k < 256; k++)
	{
		pHist[k] = h * (pHist[k] - c_min) / (c_max - c_min);
	}
	//将0-255（缩放）自适应至直方图的宽
	int x[256];
	for (int i = 0; i < 256; i++)
	{
		x[i] = w * i / 255;
	}

	//在直方图中画线
#if 0
	for (int i = 0; i < 255; i++)
	{
		cv::line(src_hist, Point(x[i], h - pHist[i]), Point(x[i + 1], h - pHist[i + 1]), Scalar(0, 0, 128), 1.5, LINE_AA);
	}
	//输出直方图
	cv::imshow("src_hist_512*512", src_hist);
	cv::waitKey(0);
#endif // 0


	return 0;
}

void saveRaw(uint16_t* p, char* savePath) {
	if (p == NULL)
	{
		std::cout << "raw path is error" << endl;
		return;
	}
	FILE* fp;
	fp = fopen(savePath, "wb");
	if (fp == NULL)
	{
		std::cout << "file point create fail" << endl;
		return;
	}
	fwrite(p, IMG_H * IMG_W, sizeof(int16_t), fp);
	fclose(fp);
	return;
}

uint16_t* ReadRawFromFile(const char* filePath)
{
	FILE* fp;
	fp = fopen(filePath, "rb");
	if (fp == NULL)
	{
		std::cout << "打开文件失败";
		return NULL;
	}
	fpos_t startpos, endpos;
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &endpos);
	fseek(fp, 0, SEEK_SET);
	fgetpos(fp, &startpos);
	long file1en = (long)(endpos - startpos);
	uint16_t* bTemp = NULL;
	bTemp = (uint16_t*)malloc(file1en);
	if (bTemp == NULL)
	{
		fclose(fp);
		return NULL;
	}
	memset(bTemp, 0, file1en);
	fread(bTemp, 1, file1en, fp);
	fclose(fp);
	return bTemp;
}

int rawModify(uint16_t* p, float gain)
{
	int length = IMG_H * IMG_W;
	for (int i = 0; i < length; i++)
	{
		*(p + i) *= gain;
	}
	return 0;
}

int main() {
	AEC aec_p;
	InitParma* initParma = (InitParma*)malloc(sizeof(InitParma));
	memset(initParma, 0, sizeof(InitParma));
	// set init fo statis value
	initParma->F_number = 2.2;
	initParma->img.imgHeight = 1744;
	initParma->img.imgWeight = 2328;
	initParma->img.blockHeight = 64;
	initParma->img.blockWeight = 64;
	initParma->img.horBlockNum = 27;
	initParma->img.verBlockNum = 36;
	aec_p.init(initParma);

	AECSetParam* setParam = (AECSetParam*)malloc(sizeof(AECSetParam));
	memset(setParam, 0, sizeof(AECSetParam));
	setParam->param = (setParamUnion*)malloc(sizeof(setParamUnion));
	//set ae mode
	setParam->type = EXPMODE;
	CurExpoMode expMode = AUTO_EXP;
	memcpy(setParam->param, &expMode, sizeof(CurExpoMode));
	aec_p.setParam(setParam);

	// set set paramet  face\ flick\ sensor
	setParam->type = SENSORINFO;
	SensorInfo sensorinfo;
	sensorinfo.sensorExposureGain = 10.3164;
	sensorinfo.sensorExposureTime = 7000;
	sensorinfo.frameRate = 90;
	memcpy(setParam->param, &sensorinfo, sizeof(SensorInfo));
	aec_p.setParam(setParam);

	//setParam->type = FACEINFO;
	//FaceInfo faceinfo;
	//memset(&faceinfo, 0, sizeof(FaceInfo));
	//memcpy(setParam->param, &faceinfo, sizeof(FaceInfo));
	//aec_p.setParam(setParam);

	//setParam->type = FLICKINFO;
	//FlickInfo filckinfo;
	//memset(&filckinfo, 0, sizeof(FlickInfo));
	//memcpy(setParam->param, &filckinfo, sizeof(FlickInfo));
	//aec_p.setParam(setParam);

	//// read raw for sim
	int pixels = IMG_H * IMG_W;
	uint16_t* p = (uint16_t*)malloc(sizeof(uint16_t) * pixels);
	if (p == NULL) {
		std::cout << "p 指针内存分配失败";
		return 1;
	}
	AEC_stats* stats = (AEC_stats *)malloc(sizeof(AEC_stats));

	char file[512];
	char* file_name;
	char* savePath;
	string pathName, fileName;
	//// get statistic from isp
	//STATS
	int rows = 27;
	int cols = 36;
	int imgH = 1744;
	int imgW = 2328;
	int blknum = rows * cols;
	int* pStats = (int*)malloc(sizeof(int) * blknum * 8);
	memset(pStats, 0, sizeof(int) * blknum * 8);
	//HIST
	int* pHist = (int*)malloc(sizeof(int) * 256);
	memset(pHist, 0, sizeof(int) * 256);
	//内接矩形计算直方图 or clip black-->FOV Histogram
	int clip_h = 0;
	int clip_w = 0;
	int clip_black = 120186;

	bool converFlag = 0;   //single frame convergence or not
	bool all_file_flag = 0;  //dir loop or not
	if (!all_file_flag)
	{
		//fileName = "D:\\work\\AE\\data\\4l\\convergence\\7ms_12x_globalMotion\\L_1\\CAM1_[153]_[2328_1744]_[152243_0702]_1699851.raw";
		fileName = "D:\\work\\ISP\\gravityxr\\v3.5.8.7.1_ae_add\\CAM1_[17303]_[2328_1744]_[141303_0024]_2350473.raw";
		//fileName = "D:\\work\\AE\\data\\4l\\convergence\\7ms_12x_globalMotion\\L\\CAM1_[350]_[2328_1744]_[152245_0893]_3888634.raw";
		//fileName = "D:\\work\\AE\\data\\4l\\convergence\\7ms_12x_globalMotion\\L\\CAM1_[27]_[2328_1744]_[152242_0302]_299919.raw";
		//fileName = "D:\\work\\AE\\data\\4l\\convergence\\7ms_8x_globalMotion\\L\\CAM1_[377]_[2328_1744]_[152114_0627]_4199306.raw";
		//fileName = "D:\\work\\AE\\data\\4l\\0803\\7ms_1x_sun\\CAM1_[1407]_[2328_1744]_[141006_0458]_2350473.raw";
		//fileName = "D:\\work\\AE\\data\\4l\\7ms_12x\\CAM1_[1361]_[2328_1744]_[153108_0788]_15131846.raw";
		//fileName = "D:\\work\\AE\\data\\CAM1_[117137]_[2328_1744]_[164756_0130]_1670075.raw";
		strcpy(file, fileName.c_str());
		file_name = file;
		p = ReadRawFromFile(file_name);

		//statistic total
		//AECStatsProcess(p, stats);
		//aec_p.getStatsFromRaw(stats);

		if (statsProc(p, pStats, rows, cols, imgH, imgW) == 0)
		{
			aec_p.getStats(pStats);
		}

		//内接矩形计算直方图-->fov Histogram
		if (histProc(p, pHist, imgH, imgW,clip_h,clip_w, clip_black) == 0)
		{
			//int h = 300;
			//int w = 256;
			//showHist(h, w, pHist);
			aec_p.getHist(pHist);
		}

		//process 
		aec_p.process();

		//get result
		AEC_algo_out* out = (AEC_algo_out*)malloc(sizeof(AEC_algo_out));
		aec_p.getAEOut(out);

		std::cout << "Time:" << out->exposureTime << endl;
		std::cout << "Gain:" << out->exposureGain << endl;

		vector<int> result;
		int saveFlag;
		std::cout << "please check is save the new raw by d gain or not? print 1 for yes and 0 for no:" ;
		while (cin >> saveFlag)
		{
			result.push_back(saveFlag);
			if (getchar() == '\n')
			{
				break;
			}
		}

		if (saveFlag)
		{
		//save raw 
			char* savePath;
			string subffix = ".raw";
			string replaceChar = "_aec_LVtable.raw";
			int pos = fileName.find(subffix);
			//modify the raw by AE result to save   simulate
			float missGain = out->exposureGain / sensorinfo.sensorExposureGain;
			cout << "missGain is " << missGain << endl;
			rawModify(p, missGain);
			strcpy(file, fileName.replace(pos, subffix.length(), replaceChar).c_str());
			savePath = file;
			saveRaw(p, savePath);
		}
		free(pStats);
		system("pause");
		free(pHist);
		free(p);
		free(setParam);
		free(initParma);
		return 0;
	}
	
	//loop dir raw to simu
	intptr_t hFile = 0;
	struct _finddata_t fileInfo;
	//string path = "D:\\work\\AE\\data\\songbin";
	//string path = "D:\\work\\AE\\data\\4l\\convergence\\7ms_12x_globalMotion\\L";
	//string path = "D:\\work\\AE\\data\\4l\\7ms_16x";
	//string path = "D:\\work\\AE\\data\\4l\\0803\\7ms_4x_globalmotion";
	string path = "D:\\work\\AE\\data\\4l\\0803\\convergence_test\\7ms_2x";
	string rawpath = path;
	//cout << pathName.assign(path).append("\\*.raw").c_str() << endl;
	hFile = _findfirst(pathName.assign(path).append("\\*.raw").c_str(), &fileInfo);
	if (hFile == -1) {
		std::cout << "there is no flie to read" << endl;
		return 1;
	}
	//mkdir new path file to save aec_raw
	string subffix = "\\";
	int pos2 = path.rfind(subffix);
	path.replace(pos2, subffix.length(), "\\HistClip_");
	//strcpy(file, path.c_str());
	if (_access(path.c_str(), 0) == -1)
	{
		_mkdir(path.c_str());
	}
	do {
		fileName = rawpath + "\\" + fileInfo.name;
		if (fileName.find("_aec") < fileName.length() || fileName.find("\\new") < fileName.length())
		{
			continue;
		}
		std::cout << "-----------------------start  ------------------------------" << endl;
		std::cout << "--------- fileName: " << fileName << " ------------" << endl;
		if (converFlag)
		{
			int loopCnt = 0;
			while (++loopCnt < 10)
			{
				strcpy(file, fileName.c_str());
				file_name = file;
				p = ReadRawFromFile(file_name);

				//statistic total
				AECStatsProcess(p, stats);
				aec_p.getStatsFromRaw(stats);

				if (histProc(p, pHist, imgH, imgW, clip_h, clip_w, clip_black) == 0)
				{
					//int h = 300;
					//int w = 256;
					//showHist(h, w, pHist);
					aec_p.getHist(pHist);
				}

				//process 
				aec_p.process();

				//get result
				AEC_algo_out* out = (AEC_algo_out*)malloc(sizeof(AEC_algo_out));
				aec_p.getAEOut(out);
	#ifdef DEBUG
				std::cout << "Time:" << out->exposureTime << endl;
				std::cout << "Gain:" << out->exposureGain << endl;
	#endif // DEBUG

				//modify the raw by AE result to save   simulate
				float missGain = out->exposureGain / sensorinfo.sensorExposureGain;
				cout << "missGain is " << missGain << endl;
				rawModify(p, missGain);

				fileName = path + "\\" + fileInfo.name;
				subffix = ".raw";
				string replaceChar = "_aec_lv_table"+to_string(loopCnt) + ".raw";
				int pos1 = fileName.find(subffix);
				strcpy(file, fileName.replace(pos1, subffix.length(), replaceChar).c_str());
				savePath = file;
				saveRaw(p, savePath);
				std::cout << "-----------------------end ------------------------------" << endl;

				setParam->type = SENSORINFO;

				sensorinfo.sensorExposureGain = out->exposureGain;
				sensorinfo.sensorExposureTime = out->exposureTime;
				sensorinfo.frameRate = 90;
				memcpy(setParam->param, &sensorinfo, sizeof(SensorInfo));
				aec_p.setParam(setParam);
			}
		}
		else
		{
			strcpy(file, fileName.c_str());
			file_name = file;
			p = ReadRawFromFile(file_name);

			//statistic total
			AECStatsProcess(p, stats);
			aec_p.getStatsFromRaw(stats);

			if (histProc(p, pHist, imgH, imgW, clip_h, clip_w, clip_black) == 0)
			{
				//int h = 300;
				//int w = 256;
				//showHist(h, w, pHist);
				aec_p.getHist(pHist);
			}

			//process 
			aec_p.process();

			//get result
			AEC_algo_out* out = (AEC_algo_out*)malloc(sizeof(AEC_algo_out));
			aec_p.getAEOut(out);
#ifdef DEBUG
			std::cout << "Time:" << out->exposureTime << endl;
			std::cout << "Gain:" << out->exposureGain << endl;
#endif // DEBUG

			//modify the raw by AE result to save   simulate
			float missGain = out->exposureGain / sensorinfo.sensorExposureGain;
			cout << "missGain is " << missGain << endl;
			rawModify(p, missGain);

			fileName = path + "\\" + fileInfo.name;
			subffix = ".raw";
			string replaceChar = "_aec_lv_table.raw";
			int pos1 = fileName.find(subffix);
			strcpy(file, fileName.replace(pos1, subffix.length(), replaceChar).c_str());
			savePath = file;
			saveRaw(p, savePath);
			std::cout << "-----------------------end ------------------------------" << endl;
		}
	} while (!_findnext(hFile, &fileInfo));    //_findnext()返回类型为intptr_t而非long型
	_findclose(hFile);

	system("pause");
	free(pHist);
	free(p);
	free(setParam);
	free(initParma);
	return 0;
}