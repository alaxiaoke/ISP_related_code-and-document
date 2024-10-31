#include <cstring>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <queue>
#include <corecrt_io.h>
#include <string>

#include "IspSoftAeApi.h"
#include "IspSoftAwbApi.h"
#include "IspSoftAtfApi.h"
#include "IspSoftIspApi.h"
#include "out.h"
#include <iostream>

using namespace std;


enum owner_mode {
	ISP_SOFT_ISP = 0,
	ISP_SOFT_AE,
	ISP_SOFT_ATF,
	ISP_SOFT_AWB,
};

typedef struct IspFrameData
{
	uint32_t            frame_id;
	uint64_t            timestamp;

	IspSoftBuffer       out_buffer;
} IspFrameData;

queue<IspFrameData> AeOutEnquene;
queue<IspFrameData> AwbOutEnquene;
queue<IspFrameData> AtfOutEnquene;

queue<IspFrameData> AeInEnquene;
queue<IspFrameData> AwbInEnquene;
queue<IspFrameData> AtfInEnquene;


owner_mode get_owner_mode(void* owner) 
{

	return ISP_SOFT_ATF;
}

bool DispathIspMessage(uint32_t postType, void* data)
{
	switch (postType)
	{
	case ISP_SOFT_ISP_POST_OUT_STATS_READY:
	{
		auto ready = reinterpret_cast<IspSoftIspStatusReady *>(data);
		IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
		frameData.out_buffer = ready->ae_stats;
		AeInEnquene.push(frameData);

		frameData.out_buffer = ready->awb_stats;
		AwbInEnquene.push(frameData);

		frameData.out_buffer = ready->atf_stats;
		AtfInEnquene.push(frameData);

		return true;
	}
	case ISP_SOFT_ISP_POST_OUT_IMAGE_READY:
	{


		return true;
	}
	default:

		return false;
	}
}

bool DispathAeMessage(uint32_t postType, void* data)
{
	switch (postType)
	{
	case ISP_SOFT_AE_POST_OUT_READY:
	{
		auto ready = reinterpret_cast<IspSoftAeOutReady*>(data);
		IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
		frameData.out_buffer = ready->ae_out;

		AeOutEnquene.push(frameData);
		return true;
	}
	default:
		return false;
	}
}

bool DispathAtfMessage(uint32_t postType, void* data)
{
	switch (postType)
	{
	case ISP_SOFT_ATF_POST_OUT_READY:
	{
		auto ready = reinterpret_cast<IspSoftAtfOutReady*>(data);
		IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
		frameData.out_buffer = ready->atf_out;

		AtfOutEnquene.push(frameData);
		return true;
	}
	default:
		return false;
	}
}

bool DispathAwbMessage(uint32_t postType, void* data)
{
	switch (postType)
	{
	case ISP_SOFT_AWB_POST_OUT_READY:
	{
		auto ready = reinterpret_cast<IspSoftAwbOutReady*>(data);
		IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
		frameData.out_buffer = ready->awb_out;

		AwbOutEnquene.push(frameData);
		return true;
	}
	default:
		return false;
	}
}

int32_t post_message(void* owner, uint32_t postType, void* data)
{
	assert(owner);
	assert(data);

	//get module
	owner_mode isp_mode = get_owner_mode(owner);
	switch (isp_mode)
	{
	case ISP_SOFT_ISP:
	{
		return DispathIspMessage(postType, data);
	}
	case ISP_SOFT_AE:
	{
		return DispathAeMessage(postType, data);

	}
	case ISP_SOFT_ATF:
	{
		return DispathAtfMessage(postType, data);

	}
	case ISP_SOFT_AWB:
	{
		return DispathAwbMessage(postType, data);

	}
	default:
		break;
	}

}

static int height = 2460;
static int width = 3280;
static unsigned short* input_buffer;
static unsigned char* output_buffer;


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


int atfStatsProc(uint16_t* p, unsigned int* pStats, int imgH, int imgW, int clip_h, int clip_w) {
	if (p == NULL) {
		printf( "source file open is null");
		return -1;
	}
	if (clip_w < 0) {
		printf("block width size set error , out of range");
		return -1;
	}
	if (clip_h < 0) {
		printf("block height size set error , out of range");
		return -1;
	}
	// default fovea area is center 1/4 area
	int top = imgH / 4;
	int left = imgW / 4;
	int sub_h = imgH / 2;
	int sub_w = imgW / 2;

	for (int y = clip_h; y + clip_h < imgH; y++) {
		int r = (y - clip_h);
		uint32_t index_add = y * imgW;
		int cnt = 0;
		uint32_t sum = 0;
		int sub_cnt = 0;
		uint32_t sub_sum_h = 0;
		for (int x = clip_w; x + clip_w < imgW; x++) {

			uint32_t pixel_index = index_add + x;
			uint32_t value = p[pixel_index] < 64 ? 0 : p[pixel_index] - 64;  // blc 
			value *= (1023.0 / 959);
			//cout << value << "	;";

			if ((y % 2 == 0 && x % 2 == 1) || (y % 2 == 1 && x % 2 == 0))  //rggb --> get g
			{
				//if (y>=top && y < top + sub_h && x >= left && x < left + sub_w)   //sub area row stats forvea
				//{
				//	sub_sum_h += (value >> 2);
				//	sub_sum_w[(x-left) >> 1] += (vlaue >> 2);
				//	sub_cnt++;
				//}
				//else
				//{

				//}
				sum += (value / 4.0);    // 10 >> 8bit
				cnt++;
			}

		}
		pStats[r] = (int)(sum / cnt);

	}
	return 0;
}


void getAtfStats(string fileName, uint16_t* p, unsigned int* atf_stats, int imgH, int imgW, int woi_h, int woi_w)
{
	char file[512];
	char* file_name;
	strcpy(file, fileName.c_str());
	file_name = file;
	ReadRawFromFile(file_name, p);
	atfStatsProc(p, atf_stats, imgH, imgW, woi_h, woi_w);
}


int main(int argc, char** argv) 
{
	/* buffer*/
	input_buffer = (unsigned short*)malloc(height * width * sizeof(unsigned short));
	output_buffer = (unsigned char*)malloc(height * width * 3 * sizeof(unsigned char));


	//IspSoftModule isp = IspSoftIspLoad();
	//IspSoftModule awb = IspSoftAwbLoad();
	//IspSoftModule ae = IspSoftAeLoad();
	IspSoftModule atf = IspSoftAtfLoad();
	void* owner;
	
	uint8_t cameraId = 0;
	/* isp init param*/
	IspSoftIspInitParam isp_init;
	isp_init.camera_id = cameraId;

	//IspSoftHandle isp_handle = isp.Init(owner, post_message, &isp_init);

	/* ae init param*/
	IspSoftAeInitParam ae_init;
	ae_init.camera_id = cameraId;
	ae_init.initial_sensor_adgain = 400;
	ae_init.initial_sensor_shutter_us = 7000;

	//IspSoftHandle ae_handle = ae.Init(owner, post_message, &ae_init);


	/* awb init param*/
	IspSoftAwbInitParam awb_init;
	awb_init.camera_id = cameraId;
	
	//IspSoftHandle awb_handle = awb.Init(owner, post_message, &awb_init);

	/* ATF init param*/
	IspSoftAtfInitParam atf_init;
	atf_init.line_time_ns = 3500;
	atf_init.sensor_height = 3496;
	atf_init.sensor_width = 4656;
	//als
	atf_init.als_config.bin_num = 0;
	atf_init.als_config.freq_step = 0;
	atf_init.als_config.sample_rate = 0;
	// tuning bin
	atf_init.tuning_bin.Data = NULL;
	atf_init.tuning_bin.Length = 0;

	atf_init.config_path = "./";

	owner = &atf_init;
	IspSoftHandle atf_handle = atf.Init(owner, post_message, &atf_init);

	uint32_t frameId = 0;
	//out buffer init
	IspFrameData init;
	init.frame_id = 0;
	init.timestamp = 0;

	init.out_buffer.Data = NULL;
	init.out_buffer.Length = 0;
	AtfOutEnquene.push(init);
	AwbOutEnquene.push(init);
	AeOutEnquene.push(init);

	/* fake isp stats out*/
	// atf
	uint16_t groupLine[ATF_STATS_MAX_LINE_SENSOR] = { 1 };
	uint32_t atfStats[ATF_STATS_MAX_LINE_AFTER_GROUP] = { 1 };
	
	//read groupline
	string pathName, fileName;
	fileName = "D:\\work\\PCMR\\data\\atf\\16M_8M\\100hz_group\\line.txt";
	FILE* fp;
	fp = fopen(fileName.c_str(), "rt");
	if (fp == NULL)
	{
		std::cout << "open file failed!";
		return 1;
	}
	char tmp[512];
	int num = 0;
	int len;

	while (fgets(tmp, 512, fp) != NULL)
	{
		len = strlen(tmp);
		tmp[len - 1] = '\0';  /*去掉换行符*/
		//printf("%s %d \n", tmp, len - 1);

		groupLine[num] = atoi(tmp);
		num++;
	}
	fclose(fp);

	/* process*/
	while (frameId < 50)
	{
		
		//read file
		int clip_h = 0;
		int clip_w = 0;
		intptr_t hFile = 0;
		struct _finddata_t fileInfo;
		string path = "D:\\work\\PCMR\\data\\atf\\16M_8M\\100hz_group";
		string rawpath = path;
		hFile = _findfirst(pathName.assign(path).append("\\*.raw").c_str(), &fileInfo);
		if (hFile == -1) {
			printf("there is no flie to read");
			return 1;
		}

		do
		{
			fileName = rawpath + "\\" + fileInfo.name;
			printf("%s\n", fileName.c_str());

			IspSoftBuffer atf_out = AtfOutEnquene.back().out_buffer;
			IspSoftBuffer awb_out = AwbOutEnquene.back().out_buffer;
			IspSoftBuffer ae_out = AeOutEnquene.back().out_buffer;

			// pop 
			AtfOutEnquene.pop();
			//AwbOutEnquene.pop();
			//AeOutEnquene.pop();

			/* ISP*/
			//IspSoftIspInImage isp_in;
			//isp_in.frame_id = frameId;
			//isp_in.timestamp = 100;
			//isp_in.sensor_adgain = 400;
			//isp_in.sensor_shutter_us = 7000;
			//isp_in.rawdata.Data = (uint8_t*)input_buffer;
			//isp_in.awb_out = awb_out;
			//isp_in.ae_out = ae_out;

			//isp.Process(isp_handle, ISP_SOFT_ISP_PROCESS_IN_IMAGE, &isp_in);

			/* AWB*/
			//IspSoftBuffer awb_stats = AwbInEnquene.front().out_buffer;
			//IspSoftAwbInImage awb_in;
			//awb_in.frame_id = frameId;
			//awb_in.timestamp = 1000;
			//awb_in.ae_out = ae_out;
			//awb_in.isp_out = awb_stats;

			//awb.Process(owner, ISP_SOFT_AWB_PROCESS_IN_IMAGE, &awb_in);

			/* AE*/
			//IspSoftBuffer ae_stats = AeInEnquene.front().out_buffer;
			//IspSoftAeInImage ae_in;
			//ae_in.frame_id = frameId;
			//ae_in.timestamp = 1000;
			//ae_in.atf_out = atf_out;
			//ae_in.awb_out = awb_out;
			//ae_in.isp_out = ae_stats;

			//ae.Process(owner, ISP_SOFT_AE_PROCESS_IN_IMAGE, &ae_in);

			/* ATF*/
			//stats proc
			getAtfStats(fileName, input_buffer, atfStats, height, width, clip_h, clip_w);

			IspSoftIspAtfStatusOut atf_stats;
			atf_stats.sensor_adgain = 400;
			atf_stats.sensor_shutter_us = 7000;
			atf_stats.atf_stats_line = 2460;
			atf_stats.stats_woi_height = height - clip_h * 2;
			atf_stats.stats_woi_width = width - clip_w * 2;
			memcpy(&atf_stats.atf_stats, &atfStats, sizeof(uint32_t) * ATF_STATS_MAX_LINE_AFTER_GROUP);
			memcpy(&atf_stats.group_line, &groupLine, sizeof(uint16_t) * ATF_STATS_MAX_LINE_SENSOR);


			IspFrameData isp_out_atf;
			isp_out_atf.frame_id = frameId;
			isp_out_atf.timestamp = frameId;
			isp_out_atf.out_buffer.Data = (uint8_t*)&atf_stats;
			isp_out_atf.out_buffer.Length = sizeof(atf_stats);
			AtfInEnquene.push(isp_out_atf);

			IspFrameData atfIn = AtfInEnquene.front();
			printf("stats_id:%d\n", atfIn.frame_id);
			AtfInEnquene.pop();
			//AtfInEnquene.push(atfIn);

			FS_Result fs_result;
			memset(&fs_result, 0, sizeof(FS_Result));

			IspSoftAtfInImage atf_in;
			atf_in.frame_id = frameId;
			atf_in.timestamp = 1000 + frameId;
			atf_in.ae_out = ae_out;
			atf_in.isp_out = atfIn.out_buffer;
			atf_in.fs_out = fs_result;

			atf.Process(atf_handle, ISP_SOFT_ATF_PROCESS_IN_IMAGE, &atf_in);

			/* callback func get atf out print*/
			IspFrameData atfOut = AtfOutEnquene.back();
			IspSoftAtfOut* atf_out_print = (IspSoftAtfOut*)atfOut.out_buffer.Data;
			printf("[%02d] enable:%d, flag_banding: %d , atf_mode: %d, stable_mode: %d \n", atfOut.frame_id, atf_out_print->flag_enable, atf_out_print->flag_bending, atf_out_print->atf_mode, atf_out_print->atf_mode_stable);
			frameId++;

		} while (!_findnext(hFile, &fileInfo));

		atf.Process(atf_handle, ISP_SOFT_ATF_PROCESS_IN_TUNING, NULL);

	}

	atf.Deinit(atf_handle);
	//ae.Deinit(ae_handle);
	//awb.Deinit(awb_handle);
	//isp.Deinit(isp_handle);

	return 0;
}
