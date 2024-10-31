#include <cstring>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <queue>
#include <thread>
#include <future>
#include <iostream>
#include <corecrt_io.h>
#include <fstream>
#include <windows.h>
#include <malloc.h>
#include "CTimes.h"
#include <string>

#include "IspSoftAeApi.h"
#include "IspSoftAwbApi.h"
#include "IspSoftAtfApi.h"
#include "IspSoftIspApi.h"
#include "out.h"

using namespace std;

#define FRAME_COUNT         8

static CTimes times;
static int64_t AEtimestamps[FRAME_COUNT][3];
static int64_t AE2timestamps[FRAME_COUNT][3];
static int64_t AWBtimestamps[FRAME_COUNT][3];
static int64_t AWB2timestamps[FRAME_COUNT][3];
static int64_t ATFtimestamps[FRAME_COUNT][3];

//static IspSoftIspAeStatusOut ae_stats;

queue<vector<float>>    expousureList;
queue<vector<float>>    expousureraw;
IspSoftAeOut aeoutbuffer;
IspSoftAwbOut awboutbuffer;
IspSoftAtfOut atfoutbuffer;
enum owner_mode {
	ISP_SOFT_ISP = 0,
	ISP_SOFT_AE,
	ISP_SOFT_AE2,
	ISP_SOFT_ATF,
	ISP_SOFT_AWB,
	ISP_SOFT_AWB2,

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

void* awbprt, * awbprt2, * aeprt, * aeprt1, * atfprt, * ispprt;

owner_mode get_owner_mode(void* owner)
{
	if (owner == aeprt)
	{
		return ISP_SOFT_AE;
	}
	else if (owner == aeprt1)
	{
		return ISP_SOFT_AE2;
	}
	else if (owner == awbprt)
	{
		return ISP_SOFT_AWB;
	}
	else if (owner == awbprt2)
	{
		return ISP_SOFT_AWB2;
	}
	else if (owner == atfprt)
	{
		return ISP_SOFT_ATF;
	}
	else if (owner == ispprt)
	{
		return ISP_SOFT_ISP;
	}
}

int32_t OnPostMessage(void* owner, uint32_t postType, void* data)
{
	switch (postType)
	{
	case ISP_SOFT_ISP_POST_OUT_STATS_READY:
	{
		auto ready = reinterpret_cast<IspSoftIspStatusReady*>(data);
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
		AEtimestamps[ready->frame_id][1] = times.GetTimestampUs();
		//printf("AE1 Done Time:%d\n", AEtimestamps[ready->frame_id][1]);
		IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
		//frameData.out_buffer = ready->ae_out;
		
		memcpy(&aeoutbuffer, ready->ae_out.Data, ready->ae_out.Length);
		frameData.out_buffer.Data = (uint8_t*) & aeoutbuffer;
		frameData.out_buffer.Length = ready->ae_out.Length;

		AeOutEnquene.push(frameData);
		printf("AE Done:%d\n", ready->frame_id);

		IspSoftAeOut* readybuffer = (IspSoftAeOut*)(ready->ae_out.Data);
		FILE* fp;
		fp = fopen("outdata\\AeOut.txt", "a+");
		if (fp)
		{
			fprintf(fp, "ae_shutter=%d\n", readybuffer->ae_shutter);
			fprintf(fp, "ae_adgain=%d\n", readybuffer->ae_adgain);
			fprintf(fp, "ae_ispgain=%d\n", readybuffer->ae_ispgain);
			fprintf(fp, "ae_lux_result=%d\n", readybuffer->ae_lux_result);
			fprintf(fp, "Ymean=%d\n", readybuffer->Ymean);
			fprintf(fp, "ae_converged=%d\n", readybuffer->ae_converged);
			fprintf(fp, "nextbv=%d\n", readybuffer->nextbv);
			fprintf(fp, "bgbv=%d\n", readybuffer->bgbv);
			fprintf(fp, "iso=%d\n", readybuffer->iso);
			fprintf(fp, "midtones_gain=%d\n", readybuffer->midtones_gain);
			fprintf(fp, "shadows_gain=%d\n", readybuffer->shadows_gain);
			fprintf(fp, "\n\r");
			fprintf(fp, "\n\r");
			fprintf(fp, "\n\r");
			fclose(fp);
		}


		fp = NULL;


		return true;
	}
	default:
		return false;
	}
}

bool DispathAeMessage2(uint32_t postType, void* data)
{
	switch (postType)
	{
	case ISP_SOFT_AE_POST_OUT_READY:
	{
		auto ready = reinterpret_cast<IspSoftAeOutReady*>(data);
		AE2timestamps[ready->frame_id][1] = times.GetTimestampUs();
		/*IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
		frameData.out_buffer = ready->ae_out;

		AeOutEnquene.push(frameData);*/
		printf("AE2 Done:%d\n", ready->frame_id);
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
		ATFtimestamps[ready->frame_id][1] = times.GetTimestampUs();
		IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
		//frameData.out_buffer = ready->atf_out;

		memcpy(&atfoutbuffer, ready->atf_out.Data, ready->atf_out.Length);
		frameData.out_buffer.Data = (uint8_t*)&atfoutbuffer;
		frameData.out_buffer.Length = ready->atf_out.Length;

		AtfOutEnquene.push(frameData);
		printf("ATF Done:%d\n", ready->frame_id);
		IspSoftAtfOut* readybuffer = (IspSoftAtfOut*)(ready->atf_out.Data);
		FILE* fp;
		fp = fopen("outdata\\AtfOut.txt", "a+");
		if (fp)
		{
			fprintf(fp, "flag_enable=%d\n", readybuffer->flag_enable);
			fprintf(fp, "flag_bending=%d\n", readybuffer->flag_bending);
			fprintf(fp, "atf_mode=%d\n", readybuffer->atf_mode);
			fprintf(fp, "atf_mode_stable=%d\n", readybuffer->atf_mode_stable);
			fprintf(fp, "\n\r");
			fprintf(fp, "\n\r");
			fprintf(fp, "\n\r");
			fclose(fp);
		}
		fp = NULL;
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
		AWBtimestamps[ready->frame_id][1] = times.GetTimestampUs();
		IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
		frameData.out_buffer = ready->awb_out;

		memcpy(&awboutbuffer, ready->awb_out.Data, ready->awb_out.Length);
		frameData.out_buffer.Data = (uint8_t*)&awboutbuffer;
		frameData.out_buffer.Length = ready->awb_out.Length;

		AwbOutEnquene.push(frameData);
		printf("AWB Done:%d\n", ready->frame_id);
		IspSoftAwbOut* readybuffer = (IspSoftAwbOut*)(ready->awb_out.Data);
		FILE* fp;
		fp = fopen("outdata\\AwbOut.txt", "a+");
		if (fp)
		{
			fprintf(fp, "awb_update=%d\n", readybuffer->awb_update);
			fprintf(fp, "rg_allbalance=%d\n", readybuffer->rg_allbalance);
			fprintf(fp, "bg_allbalance=%d\n", readybuffer->bg_allbalance);
			fprintf(fp, "r_gain=%d\n", readybuffer->r_gain);
			fprintf(fp, "g_gain=%d\n", readybuffer->g_gain);
			fprintf(fp, "b_gain=%d\n", readybuffer->b_gain);
			fprintf(fp, "color_temp=%d\n", readybuffer->color_temp);
			for (int32_t i = 0; i < 9; i++)
			{
				fprintf(fp, "final_CCM=%f\n", readybuffer->final_CCM[i]);
			}
			for (int32_t j = 0; j < 13 * 17 * 4; j++)
			{
				fprintf(fp, "final_lsc=%f\n", readybuffer->final_lsc[j]);
			}
			fprintf(fp, "\n\r");
			fprintf(fp, "\n\r");
			fprintf(fp, "\n\r");
			fclose(fp);
		}

		fp = NULL;
		return true;
	}
	default:
		return false;
	}
}

bool DispathAwbMessage2(uint32_t postType, void* data)
{
	switch (postType)
	{
	case ISP_SOFT_AWB_POST_OUT_READY:
	{
		auto ready = reinterpret_cast<IspSoftAwbOutReady*>(data);
		AWB2timestamps[ready->frame_id][1] = times.GetTimestampUs();
		//printf("AWB2 Done Time:%d\n", AWB2timestamps[ready->frame_id][1]);
		/*IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
		frameData.out_buffer = ready->awb_out;


		AwbOutEnquene.push(frameData);*/
		printf("AWB2 Done:%d\n", ready->frame_id);
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
	case ISP_SOFT_AE:
	{
		return DispathAeMessage(postType, data);
	}
	case ISP_SOFT_AE2:
	{
		return DispathAeMessage2(postType, data);
	}
	case ISP_SOFT_ATF:
	{
		return DispathAtfMessage(postType, data);
	}
	case ISP_SOFT_AWB:
	{
		return DispathAwbMessage(postType, data);
	}
	case ISP_SOFT_AWB2:
	{
		return DispathAwbMessage2(postType, data);
	}
	default:
		break;
	}

}

static void OnNotifyGPUCompute(void* owner)
{
	// 可打 LOG

	return;
}

static void packraw(uint16_t* src, uint8_t* dst, int rows, int cols, int alignsz = 1)
{
	int line = cols * 10 / 8;
	int step = (line + alignsz - 1) & -alignsz;
	int count = 0;
	for (int h = 0; h < rows; ++h)
	{
		uint16_t const* S = src + h * cols;
		uint8_t* P = dst + h * step;
		for (int w = 0; w < cols; w += 4)
		{
			P[0] = static_cast<uint8_t>((S[w + 0]));
			P[1] = static_cast<uint8_t>((S[w + 0] >> 8) | (S[w + 1] << 2));
			P[2] = static_cast<uint8_t>((S[w + 1] >> 6) | (S[w + 2] << 4));
			P[3] = static_cast<uint8_t>((S[w + 2] >> 4) | (S[w + 3] << 6));
			P[4] = static_cast<uint8_t>((S[w + 3] >> 2));
			P += 5;
			count += 5;
		}
	}
}

int rawModify(uint16_t* p, string filename, int length, UINT32 shutter, UINT32 sensorgain)
{
	float rawsensorgain = 1.0f;
	float rawexptime = 1.0f;
	float rawispgain = 1.0f;

	rawsensorgain = stof(filename.substr(filename.find("SenG[") + 5, 5));
	string timetemp = filename.substr(filename.find("Shut[") + 5, 5);


	if (timetemp.find("[") != string::npos)
	{
		timetemp = timetemp.substr(0, timetemp.find("["));
	}

	rawexptime = stof(timetemp);


	//vector<float> expousureSetRaw;
	//if (expousureraw.empty())
	//{
	//	expousureSetRaw[0] = rawexptime;
	//	expousureSetRaw[1] = rawsensorgain;
	//}
	//else
	//{
	//	expousureSetRaw = expousureraw.front();
	//	expousureraw.pop();
	//}
	//

	//vector<float> expousureReadRaw;
	//expousureReadRaw.push_back(rawexptime); //init exptime
	//expousureReadRaw.push_back(rawsensorgain);  //init adgain
	//expousureReadRaw.push_back(rawispgain);  //init ispgain
	//expousureReadRaw.push_back(0);    //sofid
	//expousureraw.push(expousureReadRaw);
	//printf("debug readraw sensorgain %f  exptime %f\n", rawsensorgain, rawexptime);
	//rawexptime = expousureSetRaw[0];
	//rawsensorgain = expousureSetRaw[1];
	//printf("debug setraw sensorgain %f  exptime %f\n", rawsensorgain, rawexptime);


	float missGain = sensorgain * shutter / (rawsensorgain * rawexptime * 100);
	printf("gain %f  %u %u %u %u, %d %d\n", missGain, *(p + 0), *(p + 10), *(p + 500), *(p + 1000), sizeof(*(p + 0)), sizeof(unsigned short));
	for (int i = 0; i < length; i++)
	{
		*(p + i) = (*(p + i) - 64) > 0 ? ((*(p + i) - 64) / 959.0f * 1023 * missGain + 64) : 64;
		*(p + i) = (*(p + i) > 1023) ? 1023 : *(p + i);

	}
	printf("gain %f  %u %u %u %u, %d %d\n", missGain, *(p + 0), *(p + 10), *(p + 500), *(p + 1000), sizeof(*(p + 0)), sizeof(unsigned short));
	return 0;
}

void saveRaw(const uint16_t* p, string savePath, int length) {
	if (p == NULL)
	{
		std::cout << "raw path is error" << endl;
		return;
	}
	FILE* fp;
	fp = fopen(savePath.c_str(), "wb");
	if (fp == NULL)
	{
		std::cout << "file point create fail" << endl;
		return;
	}
	fwrite(p, 2, length, fp);
	printf("\nsaveRaw1 %u %u %u %u, %d %d", *(p + 0), *(p + 10), *(p + 500), *(p + 1000), sizeof(*(p + 0)), sizeof(unsigned short));
	fclose(fp);
	return;
}

void saveRaw(const uint8_t* p, string savePath, int length) {
	if (p == NULL)
	{
		std::cout << "raw path is error" << endl;
		return;
	}
	FILE* fp;
	fp = fopen(savePath.c_str(), "wb");
	if (fp == NULL)
	{
		std::cout << "file point create fail" << endl;
		return;
	}
	fwrite(p, 1, length, fp);
	printf("\nsaveRaw1 %u %u %u %u, %d %d", *(p + 0), *(p + 10), *(p + 500), *(p + 1000), sizeof(*(p + 0)), sizeof(unsigned short));
	fclose(fp);
	return;
}

static int AEheight = 2920;
static int AEwidth = 3000;
static unsigned short* ae_input_buffer;
static unsigned char* ae_output_buffer;

static int ATFheight = 2460;
static int ATFwidth = 3280;
static unsigned short* atf_input_buffer;
static unsigned char* atf_output_buffer;

int64_t ProjectStart = times.GetTimestampUs();
int64_t ProcessStart;
int64_t ProjectFinal;

int main(int argc, char** argv)
{
	//_CrtSetBreakAlloc(227);
	/* buffer*/
	ae_input_buffer = (unsigned short*)malloc(AEheight * AEwidth * sizeof(unsigned short));
	ae_output_buffer = (unsigned char*)malloc(AEheight * AEwidth * 3 * sizeof(unsigned char));

	atf_input_buffer = (unsigned short*)malloc(ATFheight * ATFwidth * sizeof(unsigned short));
	atf_output_buffer = (unsigned char*)malloc(ATFheight * ATFwidth * 3 * sizeof(unsigned char));

	IspSoftModule isp = IspSoftIspLoad();
	IspSoftModule awb = IspSoftAwbLoad();
	IspSoftModule ae = IspSoftAeLoad();
	IspSoftModule atf = IspSoftAtfLoad();
	void* owner;
	string fileName;
	FILE* fp;
	fpos_t startpos, endpos;
	long file1en = 0;

	uint8_t cameraId = 0;
	/* isp init param*/
	IspSoftIspInitParam InitParam;
	InitParam.camera_id = cameraId;
	InitParam.rotation = 0;
	InitParam.in_image_info.Format = ISP_SOFT_IMAGE_RAW10_PACK;
	InitParam.in_image_info.Size.Height = AEheight;
	InitParam.in_image_info.Size.Width = AEwidth;
	InitParam.out_yuv_info = InitParam.in_image_info;
	InitParam.out_yuv_info.Format = ISP_SOFT_IMAGE_YUV420;
	InitParam.out_rgb_info.Format = ISP_SOFT_IMAGE_RGB888;
	InitParam.out_rgb_info.Size.Height = 3840;
	InitParam.out_rgb_info.Size.Width = 3552;
	InitParam.OnNotifyGPUCompute = OnNotifyGPUCompute;

	fp = fopen("lib\\debug\\ISPconfig\\tuningbin", "rb");
	if (fp == NULL)
	{
		std::cout << "open file failed！" << endl;
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &endpos);
	fseek(fp, 0, SEEK_SET);
	fgetpos(fp, &startpos);
	file1en = (long)(endpos - startpos);

	uint8_t* isptuning = (uint8_t*)malloc(file1en);
	fread(isptuning, 1, file1en, fp);

	InitParam.tuning_bin.Data = isptuning;
	InitParam.tuning_bin.Length = (int32_t)file1en;

	////unpack to pack
	//fp = fopen("lib\\debug\\test64bcase\ISPinunpackraw\\10hulkUnpack.raw", "rb");
	//if (fp == NULL)
	//{
	//	std::cout << "open file failed！" << endl;
	//	return 1;
	//}
	//fseek(fp, 0, SEEK_END);
	//fgetpos(fp, &endpos);
	//fseek(fp, 0, SEEK_SET);
	//fgetpos(fp, &startpos);
	//file1en = (long)(endpos - startpos);

	//uint16_t* uninput = (uint16_t*)malloc(file1en);
	//fread(uninput, 1, file1en, fp);
	//IspSoftIspInImage ispSoftIspInImage = { 0 };
	//uint8_t* packinput = (uint8_t*)malloc(AEheight * AEwidth  * 10 / 8);
	//packraw(uninput, packinput, AEwidth, AEheight);
	//fp = fopen("lib\\debug\\test64bcase\ISPinpackraw\\10hulkPack.raw","wb");
	//fwrite(packinput,1,10950000, fp);
	//fclose(fp);


	owner = &InitParam;
	owner = ispprt;

	IspSoftHandle isp_handle = isp.Init(owner, OnPostMessage, &InitParam);


	/* ae init param*/

	ifstream filetuning;
	filetuning.open("lib\\debug\\AEconfig\\AEParam.bin", ios::in | ios::binary | ios::ate);
	int tuningsize = filetuning.tellg();
	char* tuningdata = new char[tuningsize];
	filetuning.seekg(0, ios::beg);
	filetuning.read(tuningdata, tuningsize);
	filetuning.close();

	ifstream fileotp;
	fileotp.open("lib\\debug\\AEdata\\otpbin.bin", ios::in | ios::binary | ios::ate);
	int otpsize = fileotp.tellg();
	char* otpdata = new char[otpsize];
	fileotp.seekg(0, ios::beg);
	fileotp.read(otpdata, otpsize);
	fileotp.close();


	IspSoftAeInitParam ae_init;
	ae_init.camera_id = 0;
	ae_init.initial_sensor_adgain = 100;
	ae_init.initial_sensor_shutter_us = 2288;
	ae_init.otp_bin.Data = (uint8_t*)otpdata;
	ae_init.otp_bin.Length = otpsize;
	ae_init.tuning_bin.Data = (uint8_t*)tuningdata;
	ae_init.tuning_bin.Length = tuningsize;
	owner = &ae_init;
	aeprt = &ae_init;

	IspSoftHandle ae_handle = ae.Init(owner, post_message, &ae_init);


	/* awb init param*/

	fp = fopen("lib\\debug\\AWBconfig\\AWBParam.bin", "rt");
	if (fp == NULL)
	{
		std::cout << "open file failed！" << endl;
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &endpos);
	fseek(fp, 0, SEEK_SET);
	fgetpos(fp, &startpos);
	file1en = (long)(endpos - startpos + 1000);

	uint8_t* tuning = (uint8_t*)malloc(file1en);
	fread(tuning, 1, file1en, fp);

	IspSoftAwbInitParam awb_init;
	awb_init.tuning_bin.Data = tuning;
	awb_init.tuning_bin.Length = file1en;
	awb_init.camera_id = 0;
	owner = &awb_init;
	awbprt = &awb_init;

	IspSoftHandle awb_handle = awb.Init(owner, post_message, &awb_init);


	/* atf init param*/
	string pathName;
	IspSoftAtfInitParam atf_init;
	atf_init.line_time_ns = 3500;
	atf_init.sensor_height = 3496;
	atf_init.sensor_width = 4656;
	//als
	atf_init.als_config.bin_num = 0;
	atf_init.als_config.freq_step = 0;
	atf_init.als_config.sample_rate = 0;

	// tuning bin
	///* tuning bin read in*/
	fileName = "lib\\debug\\ATFconfig\\ATFTuningParam.bin";
	fp = fopen(fileName.c_str(), "rt");
	if (fp == NULL)
	{
		std::cout << "open file failed！" << endl;
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &endpos);
	fseek(fp, 0, SEEK_SET);
	fgetpos(fp, &startpos);
	file1en = (long)(endpos - startpos);

	uint8_t* tuning1 = (uint8_t*)malloc(file1en);
	fread(tuning1, 1, file1en, fp);
	fclose(fp);
	fp = NULL;

	atf_init.tuning_bin.Data = tuning1;
	atf_init.tuning_bin.Length = file1en;

	owner = &atf_init;
	atfprt = &atf_init;
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

	int32_t len = 0;

	intptr_t hFile1 = 0;
	struct _finddata_t fileInfo1;
	string path1 = "lib\\debug\\test64bcase\\ISPinunpackraw";
	string rawpath = path1;
	hFile1 = _findfirst(pathName.assign(path1).append("\\*.raw").c_str(), &fileInfo1);
	if (hFile1 == -1) {
		std::cout << "there is no flie to read" << endl;
		return 1;
	}


	IspSoftIspInImage ispSoftIspInImage = { 0 };
	//unpack!!!
	uint16_t* input;


	/* process*/
	while (frameId < FRAME_COUNT && !_findnext(hFile1, &fileInfo1))
	{
		fileName = path1 + "\\" + fileInfo1.name;
		if (!AtfOutEnquene.empty() && !AwbOutEnquene.empty() && !AeOutEnquene.empty())
		{
			IspSoftBuffer atf_out = AtfOutEnquene.back().out_buffer;
			IspSoftBuffer awb_out = AwbOutEnquene.back().out_buffer;
			IspSoftBuffer ae_out = AeOutEnquene.back().out_buffer;

			// pop 
			AtfOutEnquene.pop();
			AwbOutEnquene.pop();
			AeOutEnquene.pop();

			/* ISP*/

			//cal unpack
			fp = fopen(fileName.c_str(), "rb");
			if (fp == NULL)
			{
				std::cout << "open file failed！" << endl;
				return 1;
			}
			fseek(fp, 0, SEEK_END);
			fgetpos(fp, &endpos);
			fseek(fp, 0, SEEK_SET);
			fgetpos(fp, &startpos);
			file1en = (long)(endpos - startpos);
			  //unpack!!!
			input = (uint16_t*)malloc(file1en);
			fread(input, 1, file1en, fp);
            fclose(fp);

			if (!ae_out.Length)
			{
				ispSoftIspInImage.sensor_shutter_us = 2288;
				ispSoftIspInImage.sensor_adgain = 100;
			}
			else
			{
				ispSoftIspInImage.ae_out.Data = ae_out.Data;
				ispSoftIspInImage.ae_out.Length = ae_out.Length;
				ispSoftIspInImage.awb_out.Data = awb_out.Data;
				ispSoftIspInImage.awb_out.Length = awb_out.Length;
				ispSoftIspInImage.sensor_shutter_us = ((IspSoftAeOut*)ae_out.Data)->ae_shutter;
				ispSoftIspInImage.sensor_adgain = ((IspSoftAeOut*)ae_out.Data)->ae_adgain;
			}
            rawModify(input, fileName, AEheight* AEwidth, ispSoftIspInImage.sensor_shutter_us, ispSoftIspInImage.sensor_adgain);
			char newfilename[500];
			if ((IspSoftAeOut*)ae_out.Data != NULL)
			{
			    sprintf(newfilename,"%u_%f_%u_%f_%f_%f_%u_.raw",frameId, ispSoftIspInImage.sensor_adgain/100.0f, ispSoftIspInImage.sensor_shutter_us, ((IspSoftAeOut*)ae_out.Data)->ae_ispgain/100.0f,((IspSoftAwbOut*)awb_out.Data)->r_gain/100.0f,
				    ((IspSoftAwbOut*)awb_out.Data)->b_gain / 100.0f, ((IspSoftAwbOut*)awb_out.Data)->color_temp);
				printf("debug %u %u  %u  %d", ((IspSoftAeOut*)ae_out.Data)->ae_adgain, ((IspSoftAeOut*)ae_out.Data)->ae_shutter,((IspSoftAtfOut*)atf_out.Data)->atf_mode,sizeof(IspSoftAeOut));
			}
			else
			{
				sprintf(newfilename, "%u_%f_%u_1.000_1.600_1.600_5500_.raw", frameId, ispSoftIspInImage.sensor_adgain/100.0f, ispSoftIspInImage.sensor_shutter_us);
			}
			string outraw = path1 + "\\AEOutRaw\\" + newfilename;
			saveRaw(input, outraw, AEheight* AEwidth);
			//saveccm
			FILE* fpccm;
			string outccm = path1 + "\\AEOutRaw\\saveccm.txt";
			
			//unpack to pack to isp
			file1en = AEheight * AEwidth * 10 / 8;
	        uint8_t* packinput = (uint8_t*)malloc(file1en);
	        packraw(input, packinput, AEwidth, AEheight);
			//string outrawpack = path1 + "\\AEOutRawpack\\newpack_" + fileInfo1.name;
			//saveRaw(packinput, outrawpack, file1en);

			ispSoftIspInImage.rawdata.Data = packinput;
			ispSoftIspInImage.rawdata.Length = (int32_t)file1en;


            isp.Process(isp_handle, ISP_SOFT_ISP_PROCESS_IN_IMAGE, &ispSoftIspInImage);

			printf("debug %d", ispSoftIspInImage.ae_out.Length);
			bool aeinenque = AeInEnquene.empty();

			/* AWB*/
			while (true)
			{
				/*printf("%d", 1);*/
				if (!AtfInEnquene.empty())
				{
					IspSoftBuffer awb_stats = AwbInEnquene.back().out_buffer;
					IspSoftIspAwbStatusOut* ready1 = (IspSoftIspAwbStatusOut*)(awb_stats.Data);
					printf("%d\n", ready1->stats_woi_height);
					AwbInEnquene.pop();
					IspSoftAwbInImage awb_in;
					awb_in.frame_id = frameId;
					awb_in.timestamp = 1000;
					awb_in.ae_out = ae_out;
					awb_in.isp_out = awb_stats;
					fp = fopen("outdata\\IspAwb.txt", "a+");
					if (fp)
					{
						fprintf(fp, "stats_woi_height=%d\n", ready1->stats_woi_height);
						fprintf(fp, "stats_woi_width=%d\n", ready1->stats_woi_width);
						fprintf(fp, "awb_stats_blockY=%d\n", ready1->awb_stats_blockY);
						fprintf(fp, "awb_stats_blockX=%d\n", ready1->awb_stats_blockX);
						for (int32_t i = 0; i < 5 * 64 * 48; i++)
						{
							fprintf(fp, "awb_stats=%d\n", ready1->awb_stats[i]);
						}
						fprintf(fp, "\n\r");
						fprintf(fp, "\n\r");
						fprintf(fp, "\n\r");
						fclose(fp);
					}
					fp = NULL;
					/*ifstream fileatfout;
					fileatfout.open("lib\\debug\\AEdata\\atfout.bin", ios::in | ios::binary | ios::ate);
					int atfoutsize = fileatfout.tellg();
					char* atfoutdata = new char[atfoutsize];
					fileatfout.seekg(0, ios::beg);
					fileatfout.read(atfoutdata, atfoutsize);
					fileatfout.close();

					ifstream fileawbout;
					fileawbout.open("lib\\debug\\AEdata\\AwbOut.bin", ios::in | ios::binary | ios::ate);
					int awboutsize = fileawbout.tellg();
					char* awboutdata = new char[awboutsize];
					fileawbout.seekg(0, ios::beg);
					fileawbout.read(awboutdata, awboutsize);
					fileawbout.close();

					ifstream fileaestats;
					fileaestats.open("lib\\debug\\AEdata\\ISPAeStatusOut.bin", ios::in | ios::binary | ios::ate);
					int aestatssize = fileaestats.tellg();
					char* aestatsdata = new char[aestatssize];
					fileaestats.seekg(0, ios::beg);
					fileaestats.read(aestatsdata, aestatssize);
					fileaestats.close();

					IspFrameData isp_out_ae;
					isp_out_ae.frame_id = 1;
					isp_out_ae.timestamp = 1;
					isp_out_ae.out_buffer.Data = (uint8_t*)aestatsdata;
					isp_out_ae.out_buffer.Length = aestatssize;*/
					/* AE*/
					IspSoftBuffer ae_stats = AeInEnquene.back().out_buffer;
					IspSoftIspAeStatusOut* ready2 = (IspSoftIspAeStatusOut*)(ae_stats.Data);
					printf("%d\n", ready2->sensor_shutter_us);
					AeInEnquene.pop();
					IspSoftAeInImage ae_in;
					ae_in.frame_id = frameId;
					ae_in.timestamp = 1000;
					/*ae_in.atf_out.Data = (uint8_t*)atfoutdata;
					ae_in.atf_out.Length = atfoutsize;
					ae_in.awb_out.Data = (uint8_t*)awboutdata;
					ae_in.awb_out.Length = awboutsize;*/
					//ae_in.isp_out = isp_out_ae.out_buffer;
					ae_in.atf_out = atf_out;
					ae_in.awb_out = awb_out;
					ae_in.isp_out = ae_stats;
					fp = fopen("outdata\\IspAe.txt", "a+");
					if (fp)
					{
						fprintf(fp, "sensor_shutter_us=%d\n", ready2->sensor_shutter_us);
						fprintf(fp, "sensor_adgain=%d\n", ready2->sensor_adgain);
						fprintf(fp, "stats_woi_height=%d\n", ready2->stats_woi_height);
						fprintf(fp, "stats_woi_width=%d\n", ready2->stats_woi_width);
						fprintf(fp, "ae_stats_blockY=%d\n", ready2->ae_stats_blockY);
						fprintf(fp, "ae_stats_blockX=%d\n", ready2->ae_stats_blockX);
						for (int32_t i = 0; i < 8 * 36 * 24; i++)
						{
							fprintf(fp, "ae_stats=%d\n", ready2->ae_stats[i]);
						}
						for (int32_t j = 0; j < 256; j++)
						{
							fprintf(fp, "ae_hist=%d\n", ready2->ae_hist[j]);
						}
						fprintf(fp, "\n\r");
						fprintf(fp, "\n\r");
						fprintf(fp, "\n\r");
						fclose(fp);
					}
					fp = NULL;

					/* ATF*/

					FS_Result fs_result;
					memset(&fs_result, 0, sizeof(FS_Result));

					IspSoftBuffer atf_stats = AtfInEnquene.back().out_buffer;
					IspSoftIspAtfStatusOut* ready3 = (IspSoftIspAtfStatusOut*)(atf_stats.Data);
					printf("%d\n", ready3->sensor_shutter_us);
					AtfInEnquene.pop();
					IspSoftAtfInImage atf_in;
					atf_in.frame_id = frameId;
					atf_in.timestamp = 1000;
					atf_in.ae_out = ae_out;
					atf_in.isp_out = atf_stats;
					atf_in.fs_out = fs_result;
					fp = fopen("outdata\\IspATF.txt", "a+");
					len = ceil((float)(atf_stats.Length) / sizeof(uint16_t) / 8);
					if (fp)
					{
						fprintf(fp, "sensor_shutter_us=%d\n", ready3->sensor_shutter_us);
						fprintf(fp, "sensor_adgain=%d\n", ready3->sensor_adgain);
						fprintf(fp, "stats_woi_height=%d\n", ready3->stats_woi_height);
						fprintf(fp, "stats_woi_width=%d\n", ready3->stats_woi_width);
						fprintf(fp, "atf_stats_line=%d\n", ready3->atf_stats_line);
						for (int32_t i = 0; i < 3496; i++)
						{
							fprintf(fp, "group_line=%d\n", ready3->group_line[i]);
						}
						for (int32_t j = 0; j < 2460; j++)
						{
							fprintf(fp, "atf_stats=%d\n", ready3->atf_stats[j]);
						}
						fprintf(fp, "\n\r");
						fprintf(fp, "\n\r");
						fprintf(fp, "\n\r");
						fclose(fp);
					}
					fp = NULL;


					AWBtimestamps[frameId][0] = times.GetTimestampUs();
					async(launch::async, awb.Process, awb_handle, ISP_SOFT_AWB_PROCESS_IN_IMAGE, &awb_in);

					//ae.Process(ae_handle, ISP_SOFT_AE_PROCESS_IN_TUNING, &ae_in);
                    ae.Process(ae_handle, ISP_SOFT_AE_PROCESS_IN_DEBUG, &ae_in);
					AEtimestamps[frameId][0] = times.GetTimestampUs();
					//async(launch::async, ae.Process, ae_handle, ISP_SOFT_AE_PROCESS_IN_IMAGE, &ae_in);
					auto errorcode = ae.Process(ae_handle, ISP_SOFT_AE_PROCESS_IN_IMAGE, &ae_in);
					if (errorcode != 0)
					{
						printf("%d\n", errorcode);
					}
					ATFtimestamps[frameId][0] = times.GetTimestampUs();
					//async(launch::async, atf.Process, atf_handle, ISP_SOFT_ATF_PROCESS_IN_IMAGE, &atf_in);
					atf.Process(atf_handle, ISP_SOFT_ATF_PROCESS_IN_IMAGE, &atf_in);
					break;
				}
			}
			frameId++;
		}



	}
	_findclose(hFile1);

	atf.Deinit(atf_handle);
	ae.Deinit(ae_handle);
	awb.Deinit(awb_handle);


	ProjectFinal = times.GetTimestampUs();

	float ProjectUse = (float)(ProjectFinal - ProjectStart);
	printf("Project Use Time:%fms\n", ProjectUse / 1000);
	//int64_t AWB1UseTime = 0, AWB2UseTime = 0, AE1UseTime = 0, AE2UseTime = 0, ATFUseTime = 0;
	//int64_t AWB1min = 99999999, AWB1max = 0;
	//int64_t AWB2min = 99999999, AWB2max = 0;
	//int64_t AE1min = 999999999, AE1max = 0;
	//int64_t AE2min = 99999999, AE2max = 0;
	//int64_t ATFmin = 99999999, ATFmax = 0;
	//int32_t* AWB1TimeOutFrame = new int32_t[FRAME_COUNT]();
	//int32_t* AWB2TimeOutFrame = new int32_t[FRAME_COUNT]();
	//int32_t* AE1TimeOutFrame = new int32_t[FRAME_COUNT]();
	//int32_t* AE2TimeOutFrame = new int32_t[FRAME_COUNT]();
	//int32_t* ATFTimeOutFrame = new int32_t[FRAME_COUNT]();
	//for (int32_t i = 0; i < FRAME_COUNT; i++)
	//{
	//	AWB1UseTime += AWBtimestamps[i][1] - AWBtimestamps[i][0];
	//	AWB2UseTime += AWB2timestamps[i][1] - AWB2timestamps[i][0];
	//	AE1UseTime += AEtimestamps[i][1] - AEtimestamps[i][0];
	//	AE2UseTime += AE2timestamps[i][1] - AE2timestamps[i][0];
	//	ATFUseTime += ATFtimestamps[i][1] - ATFtimestamps[i][0];

	//	if (AWBtimestamps[i][1] - AWBtimestamps[i][0] < AWB1min)
	//	{
	//		AWB1min = AWBtimestamps[i][1] - AWBtimestamps[i][0];
	//	}
	//	if (AWBtimestamps[i][1] - AWBtimestamps[i][0] >= AWB1max)
	//	{
	//		AWB1max = AWBtimestamps[i][1] - AWBtimestamps[i][0];
	//	}
	//	if (AWB2timestamps[i][1] - AWB2timestamps[i][0] < AWB2min)
	//	{
	//		AWB2min = AWB2timestamps[i][1] - AWB2timestamps[i][0];
	//	}
	//	if (AWB2timestamps[i][1] - AWB2timestamps[i][0] >= AWB2max)
	//	{
	//		AWB2max = AWB2timestamps[i][1] - AWB2timestamps[i][0];
	//	}
	//	if (AEtimestamps[i][1] - AEtimestamps[i][0] < AE1min)
	//	{
	//		AE1min = AEtimestamps[i][1] - AEtimestamps[i][0];
	//	}
	//	if (AEtimestamps[i][1] - AEtimestamps[i][0] >= AE1max)
	//	{
	//		AE1max = AEtimestamps[i][1] - AEtimestamps[i][0];
	//	}
	//	if (AE2timestamps[i][1] - AE2timestamps[i][0] < AE2min)
	//	{
	//		AE2min = AE2timestamps[i][1] - AE2timestamps[i][0];
	//	}
	//	if (AE2timestamps[i][1] - AE2timestamps[i][0] >= AE2max)
	//	{
	//		AE2max = AE2timestamps[i][1] - AE2timestamps[i][0];
	//	}
	//	if (ATFtimestamps[i][1] - ATFtimestamps[i][0] < ATFmin)
	//	{
	//		ATFmin = ATFtimestamps[i][1] - ATFtimestamps[i][0];
	//	}
	//	if (ATFtimestamps[i][1] - ATFtimestamps[i][0] >= ATFmax)
	//	{
	//		ATFmax = ATFtimestamps[i][1] - ATFtimestamps[i][0];
	//	}

	//	if (AWBtimestamps[i][1] - AWBtimestamps[i][0] > 5000)   // 符号“>”右数字表示自定义超时阈值，每1000为1ms
	//	{
	//		AWB1TimeOutFrame[i] = 1;
	//	}
	//	if (AWB2timestamps[i][1] - AWB2timestamps[i][0] > 5000)     // 符号“>”右数字表示自定义超时阈值，每1000为1ms
	//	{
	//		AWB2TimeOutFrame[i] = 1;
	//	}
	//	if (AEtimestamps[i][1] - AEtimestamps[i][0] > 5000)		// 符号“>”右数字表示自定义超时阈值，每1000为1ms
	//	{
	//		AWB2TimeOutFrame[i] = 1;
	//	}
	//	if (AE2timestamps[i][1] - AE2timestamps[i][0] > 5000)		// 符号“>”右数字表示自定义超时阈值，每1000为1ms
	//	{
	//		AWB2TimeOutFrame[i] = 1;
	//	}
	//	if (ATFtimestamps[i][1] - ATFtimestamps[i][0] > 5000)		// 符号“>”右数字表示自定义超时阈值，每1000为1ms
	//	{
	//		AWB2TimeOutFrame[i] = 1;
	//	}
	//}



	//printf("AWB1 Use Time Total:%fms, Mean:%fms, Min:%fms, Max:%fms\n", (float)(AWB1UseTime) / 1000, (float)(AWB1UseTime) / 1000 / FRAME_COUNT, (float)(AWB1min) / 1000, (float)(AWB1max) / 1000);
	//printf("AWB2 Use Time Total :%fms, Mean:%fms, Min:%fms, Max:%fms\n", (float)(AWB2UseTime) / 1000, (float)(AWB2UseTime) / 1000 / FRAME_COUNT, (float)(AWB2min) / 1000, (float)(AWB2max) / 1000);
	//printf("AE1 Use Time Total:%fms, Mean:%fms, Min:%fms, Max:%fms\n", (float)(AE1UseTime) / 1000, (float)(AE1UseTime) / 1000 / FRAME_COUNT, (float)(AE1min) / 1000, (float)(AE1max) / 1000);
	//printf("AE2 Use Time Total:%fms, Mean:%fms, Min:%fms, Max:%fms\n", (float)(AE2UseTime) / 1000, (float)(AE2UseTime) / 1000 / FRAME_COUNT, (float)(AE2min) / 1000, (float)(AE2max) / 1000);
	//printf("ATF Use Time Total:%fms, Mean:%fms, Min:%fms, Max:%fms\n", (float)(ATFUseTime) / 1000, (float)(ATFUseTime) / 1000 / FRAME_COUNT, (float)(ATFmin) / 1000, (float)(ATFmax) / 1000);

	//printf("AWB1超过5ms的帧ID为：");
	//for (int32_t i = 0; i < FRAME_COUNT; i++)
	//{
	//	if (AWB1TimeOutFrame[i])
	//	{
	//		printf("%d ", i);
	//	}
	//}
	//printf("\n");

	//printf("AWB2超过5ms的帧ID为：");
	//for (int32_t i = 0; i < FRAME_COUNT; i++)
	//{
	//	if (AWB2TimeOutFrame[i])
	//	{
	//		printf("%d ", i);
	//	}
	//}
	//printf("\n");

	//printf("AE1超过5ms的帧ID为：");
	//for (int32_t i = 0; i < FRAME_COUNT; i++)
	//{
	//	if (AE1TimeOutFrame[i])
	//	{
	//		printf("%d ", i);
	//	}
	//}
	//printf("\n");

	//printf("AE2超过5ms的帧ID为：");
	//for (int32_t i = 0; i < FRAME_COUNT; i++)
	//{
	//	if (AE2TimeOutFrame[i])
	//	{
	//		printf("%d ", i);
	//	}
	//}
	//printf("\n");

	//printf("ATF超过5ms的帧ID为：");
	//for (int32_t i = 0; i < FRAME_COUNT; i++)
	//{
	//	if (ATFTimeOutFrame[i])
	//	{
	//		printf("%d ", i);
	//	}
	//}
	//printf("\n");


	//_CrtDumpMemoryLeaks();
	return 0;
}


