#include <exception>
#include <stdio.h>
#include <iostream>
#include <string>

//#include "./Include/allib_ae.h"
//#include "../Include/allib_ae_errcode.h"
#include "aeHandle.h"

using namespace std;

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

#define BSWAP_32(x) (uint32_t)((((uint32_t)(x)&0xff000000)>>24) \
								|(((uint32_t)(x)&0x00ff0000)>>8) \
								|(((uint32_t)(x)&0x0000ff00)<<8) \
								|(((uint32_t)(x)&0x000000ff)<<24))

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
	clip_w += (clip_w % 2);
	clip_h += (clip_h % 2);
	if (clip_w < 0) {
		std::cout << "block width size set error , out of range";
		return -1;
	}
	if (clip_h < 0) {
		std::cout << "sblock height size set error , out of range";
		return -1;
	}
	uint16_t satValue = 1024 * 0.97;   //可调参数
	for (int y = clip_h; y + clip_h - (clip_h % 2) < imgH; y++) {
		int r = (y - clip_h) / blockH;
		uint16_t index_add = y * imgW;
		for (int x = clip_w; x + clip_w - (clip_w % 2) < imgW; x++) {
			int c = (x - clip_w) / blockW;
			uint16_t blockNum = r * cols + c;
			uint16_t pixel_index = index_add + x;
			uint32_t value = p[pixel_index] < 64 ? 0 : p[pixel_index] - 64;  // blc ?
			value *= (1023.0 / 959);
			//cout << value << "	;";
			if (value <= satValue) {
				pStats[(y % 2) * 2 + x % 2 + blockNum * 8] += value;   //r/gr/gb/b  max is 2^20  
				pStats[(y % 2) * 2 + x % 2 + blockNum * 8 + 4] += 1;

			}
		}
	}
	return 0;
}

int histProc(uint16_t* ptr, int* pHist, int rows, int cols, int clip_h, int clip_w)
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
		*(rggb + i) = (int**)malloc(rows / 2 * sizeof(int*));
		memset(*(rggb + i), 0, rows / 2 * sizeof(int*));
		//分配三维
		for (j = 0; j < rows / 2; j++)
		{
			*(*(rggb + i) + j) = (int*)malloc(4 * sizeof(int));
			memset(*(*(rggb + i) + j), 0, 4 * sizeof(int));
		}
	}

	// get rggb value
	for (int y = clip_h; y + clip_h < rows; y++)
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
	for (int y = clip_h / 2; y + clip_h / 2 < rows / 2; y++)
	{
		for (int x = clip_w / 2; x + clip_w / 2 < cols / 2; x++)
		{
			int avgY = (int)(rggb[x][y][0] * RGB2Y[0] + (rggb[x][y][1] + (rggb[x][y][2] - rggb[x][y][1]) / 2) * RGB2Y[1] + rggb[x][y][3] * RGB2Y[2]) >> 2;
			pHist[avgY] += 1;
		}
	}
	//pHist[0] -= clip_black;
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

uint16_t unpack(const uint8_t* data) {
	uint16_t result = 0;
	result |= static_cast<uint16_t>(data[1]) << 8;  //大端在后
	result |= data[0];
	return result;
}

void ReadRawFromFile(const char* filePath, uint16_t* bTemp)
{
	FILE* fp;
	fp = fopen(filePath, "rb");
	if (fp == NULL)
	{
		std::cout << "打开文件失败";
		return;
	}
	fpos_t startpos, endpos;
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &endpos);
	fseek(fp, 0, SEEK_SET);
	fgetpos(fp, &startpos);
	long file1en = (long)(endpos - startpos);
	uint8_t* data = new uint8_t[file1en];

	if (bTemp == NULL)
	{
		fclose(fp);
		return;
	}
	//memset(bTemp, 0, file1en);
	fread(reinterpret_cast<char*>(data), sizeof(char), file1en, fp);
	fclose(fp);

	// 遍历每个数据项，并调用readInt10函数将其解析为10位数据  
	for (size_t i = 0; i < file1en; i += 2) {
		uint16_t value = unpack(&data[i]);
		*(bTemp + i / 2) = value;
		//std::cout << "Value at offset " << i << ": " << value << std::endl;
	}

	// 释放内存并删除指针  
	delete[] data;
	data = nullptr;
	return;
}

int generateStats(string fileName,unsigned char* stats, unsigned int* hist)
{
	char file[512];
	char* file_name;
	strcpy(file, fileName.c_str());
	file_name = file;

	//STATS
	int rows = 16;
	int cols = 16;
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
	//int clip_black = 120186;

	//// read raw for sim
	int pixels = imgH * imgW;
	uint16_t* p = (uint16_t*)malloc(sizeof(uint16_t) * pixels);
	if (p == NULL) {
		std::cout << "p 指针内存分配失败";
		return 1;
	}

	memset(p, 0, pixels);
	ReadRawFromFile(file_name, p);
	statsProc(p, pStats, rows, cols, imgH, imgW);
	histProc(p, pHist, imgH, imgW, clip_h, clip_w);

	stats = (unsigned char*)pStats;
	hist = (unsigned int*)pHist;
	// conclusion: ori is big-endian need swap
	//if (stats[0] == 0)	printf("Little-endian \n");	
	//else 	printf("Big-endian \n");
	//printf("%#x\n", stats[0]);
	//printf("%#x\n", stats[1]);
	//printf("%#x\n", hist[2]);
	//printf("%#x\n", hist[3]);

	return 0;
}

int main(void)
{
	gxr_aehd gxr_aeParam;
	gxr_aeParam.ae_stats = NULL;   
	gxr_aeParam.yhis_stats = NULL; 

	double ispGain = 1.0;

	//构造数据  16 * 16 
	unsigned char*   ae_stats_buffer = (unsigned char*)malloc(8320);   // 16 * 16 * 32 + 128
	unsigned int*    yhis_stats_buffer = (unsigned int*)malloc(1152);  //256*4+128
	memset(ae_stats_buffer, 0, 8320);
	memset(yhis_stats_buffer, 0, 1152);
	//gxr_aeParam.ae_stats = ae_stats_buffer;
	//gxr_aeParam.yhis_stats = yhis_stats_buffer;

	for (int i = 0; i < 256; i++)
	{
		ae_stats_buffer[i * 32] = 127;
		ae_stats_buffer[i * 32 + 4] = 127;
		ae_stats_buffer[i * 32 + 8] = 127;
		ae_stats_buffer[i * 32 + 12] = 127;
		ae_stats_buffer[i * 32 + 16] = 100;
		ae_stats_buffer[i * 32 + 20] = 102;
		ae_stats_buffer[i * 32 + 24] = 102;
		ae_stats_buffer[i * 32 + 28] = 102;
		yhis_stats_buffer[i] = 100;
	}

	//string fileName = "D:\\work\\AE\\data\\4l\\0803\\7ms_1x_sun\\CAM1_[1407]_[2328_1744]_[141006_0458]_2350473.raw";
	string fileName = "D:\\work\\AE\\data\\ALTEK_EVK\\110.raw";
	//generateStats(fileName, ae_stats_buffer, yhis_stats_buffer);

	char file[512];
	char* file_name;
	strcpy(file, fileName.c_str());
	file_name = file;
	//STATS
	int rows = 16;
	int cols = 16;
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
	//int clip_black = 120186;
	//// read raw for sim
	int pixels = imgH * imgW;
	uint16_t* p = (uint16_t*)malloc(sizeof(uint16_t) * pixels);
	if (p == NULL) {
		std::cout << "p 指针内存分配失败";
		return 1;
	}
	memset(p, 0, pixels);
	ReadRawFromFile(file_name, p);
	statsProc(p, pStats, rows, cols, imgH, imgW);
	histProc(p, pHist, imgH, imgW, clip_h, clip_w);
	ae_stats_buffer = (unsigned char*)pStats;
	yhis_stats_buffer = (unsigned int*)pHist;

	//printf("%#x\n", ae_stats_buffer[0]);
	//printf("%#x\n", ae_stats_buffer[1]);
	//printf("%#x\n", yhis_stats_buffer[2]);
	//printf("%#x\n", yhis_stats_buffer[3]);

	// set calibration data for AE initial
	// min ISO gain, WB gain r/g/b, afd mode
	//gxr_aeParam.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.min_iso = al4a.lsc_get_para.para.default_ISO.min_iso;
	//gxr_aeParam.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_b_gain = al4a.lsc_get_para.para.default_ISO.gain_b;
	//gxr_aeParam.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain = al4a.lsc_get_para.para.default_ISO.gain_g;
	//gxr_aeParam.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_r_gain = al4a.lsc_get_para.para.default_ISO.gain_r;
	//gxr_aeParam.ae_set_para.set_param.ae_initial_setting.afd_flicker_mode = (enum ae_antiflicker_mode_t)al4a.afd_get_para.para.afd_setting.mode;

	gxr_aeParam.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.min_iso = 100;
	gxr_aeParam.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_b_gain = 1500;
	gxr_aeParam.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain = 1000;
	gxr_aeParam.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_r_gain = 1500;
	gxr_aeParam.ae_set_para.set_param.ae_initial_setting.afd_flicker_mode = ANTIFLICKER_50HZ;

	gxr_aeParam.ae_set_para.set_param.ae_enable = 1;

	gxr_aeParam.cameraId = 0;
	//gxr_aeParam.min_iso = 100;   // scale 100
	//gxr_aeParam.calib_b_gain = 1500;   // scale 1000  default or get from alsc
	//gxr_aeParam.calib_g_gain = 1000;
	//gxr_aeParam.calib_r_gain = 1500;
	//gxr_aeParam.atf_mode = (enum ae_antiflicker_mode_t)0;  // init default, run proc can modify timely by setting
	 //set AE enablet
	//gxr_aeParam.ae_enable = 1;

	unsigned int err = 0;

	err = ae_initial(&gxr_aeParam);
	if (err != 0x00) {
		printf("AE init error: %#x\r\n", err);
		return err;
	}

	// multi frame run
	int g_udsofidx = 0;
	do
	{
		// 更新 stats buff  & sensor exposure time、gain& sofidx(frameId)
		//gxr_aeParam.frameId = g_udsofidx;
		gxr_aeParam.ae_set_para.set_param.sof_notify_param.sys_sof_index = g_udsofidx;
		gxr_aeParam.ae_stats = ae_stats_buffer;   //get from isp aestats
		gxr_aeParam.yhis_stats = yhis_stats_buffer; // get from isp yhis

		////*********** must sensor driver  ****/
		//read sensor exp param
		//gxr_aeParam.ae_get_para.para.ae_get_init_expo_param.ad_gain = sensor.param.gain;
		//gxr_aeParam.ae_get_para.para.ae_get_init_expo_param.exp_time = sensor.param.exp_time;

		//gxr_aeParam.sensorgain = 100;
		//gxr_aeParam.shutter = 7000;

		//gxr_aeParam.ae_set_para.set_param.sof_notify_param.exp_time = 7000;
		//gxr_aeParam.ae_set_para.set_param.sof_notify_param.exp_adgain = 100;
		gxr_aeParam.ae_set_para.set_param.sof_notify_param.exp_time = gxr_aeParam.ae_get_para.para.ae_get_init_expo_param.exp_time;
		gxr_aeParam.ae_set_para.set_param.sof_notify_param.exp_adgain = gxr_aeParam.ae_get_para.para.ae_get_init_expo_param.ad_gain;
		gxr_aeParam.ae_set_para.set_param.sof_notify_param.midtones_gain = gxr_aeParam.ae_get_para.para.ae_get_init_expo_param.midtones_gain;

		if (g_udsofidx < 2) {
			//gxr_aeParam.atf_mode = 0;
			gxr_aeParam.ae_set_para.set_param.afd_flicker_mode = (enum ae_antiflicker_mode_t)0;
		}
		else {
			//gxr_aeParam.atf_mode = 1;  //get from atf algo out
			gxr_aeParam.ae_set_para.set_param.afd_flicker_mode = (enum ae_antiflicker_mode_t)1;  //get from atf algo out
		}

		gxr_aeParam.ae_set_para.set_param.afd_flicker_mode = ANTIFLICKER_50HZ;
		err = ae_run(&gxr_aeParam);
		if (err != 0x00) {
			printf("AE run error: %#x\r\n", err);
			return err;
		}

		printf("[alAELib] Process[%d]: Ymean %d, Lux %ld ExpT %ld(us), Gain %ld, converge %d\n", \
			g_udsofidx, gxr_aeParam.ae_output.ymean100_8bit, gxr_aeParam.ae_output.lux_result, gxr_aeParam.ae_output.udexposure_time, \
			gxr_aeParam.ae_output.udsensor_ad_gain, gxr_aeParam.ae_output.ae_converged);
		//printf("[alAELib] Process[%d]: Ymean %.2f, Lux %ld ExpT %.2f(us), Gain %.2f, converge %d\n", \
		//	g_udsofidx, gxr_aeParam.Ymean, gxr_aeParam.ae_lux_result, gxr_aeParam.ae_shutter, \
		//	gxr_aeParam.ae_adgain, gxr_aeParam.ae_converged);


		g_udsofidx++;
		//gxr_aeParam.ae_ispgain = max(1.0, gxr_aeParam.ae_adgain / 16 / 100.0);  //sensor adgain max is 16 
		//printf("ispGain is : %f \r\n", gxr_aeParam.ae_ispgain);
		// ispgain need trans for ISP input, with new expo param effect stats
		//write sensor exp param  gain& time
		//sensor.param.gain = gxr_aeParam.ae_output.udsensor_ad_gain /100 / ispGain; 
		//sensor.param.exp_time = gxr_aeParam.ae_output.udexposure_time;

	} while (g_udsofidx <= 5);

	err = ae_deinit(&gxr_aeParam);

	if (err != 0x00) {
		printf("AE deinit error: %#x\r\n", err);
		return err;
	}

	return err;
}
