#include <cstring>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <queue>
#include <thread>
#include <future>
#include <iostream>
#include <fstream>
#include <windows.h>

#include "IspSoftAeApi.h"
#include "IspSoftAwbApi.h"
#include "IspSoftAtfApi.h"
#include "IspSoftIspApi.h"
#include "out.h"

using namespace std;

//static IspSoftIspAeStatusOut ae_stats;

enum owner_mode {
	ISP_SOFT_ISP = 0,
	ISP_SOFT_AE,
	ISP_SOFT_AE1,
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
queue<IspFrameData> AeOutEnquene1;
queue<IspFrameData> AwbOutEnquene;
queue<IspFrameData> AtfOutEnquene;

queue<IspFrameData> AeInEnquene;
queue<IspFrameData> AwbInEnquene;
queue<IspFrameData> AtfInEnquene;

IspSoftAeOut aeouttmp;
IspSoftAeOut aeouttmp1;

void* part;
void* part1;
owner_mode get_owner_mode(void* owner)
{
	if (owner == part)
	{
	    return ISP_SOFT_AE;
	}
	else if(owner == part1)
	{
		return ISP_SOFT_AE1;
	}

}

bool DispathIspMessage(uint32_t postType, void* data)
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
		IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
        //frameData.out_buffer = ready->ae_out;

		
		memcpy(&aeouttmp, (ready->ae_out).Data,sizeof(IspSoftAeOut));
		frameData.out_buffer.Data = (uint8_t*) & aeouttmp;
		frameData.out_buffer.Length = ready->ae_out.Length;

		AeOutEnquene.push(frameData);
		return true;
	}
	default:
		return false;
	}
}

bool DispathAeMessage1(uint32_t postType, void* data)
{
	switch (postType)
	{
	case ISP_SOFT_AE_POST_OUT_READY:
	{
		auto ready = reinterpret_cast<IspSoftAeOutReady*>(data);
		IspFrameData frameData = { 0 };
		frameData.frame_id = ready->frame_id;
		frameData.timestamp = ready->timestamp;
		//frameData.out_buffer = ready->ae_out;

		memcpy(&aeouttmp1, (ready->ae_out).Data, sizeof(IspSoftAeOut));
		frameData.out_buffer.Data = (uint8_t*)&aeouttmp1;
		frameData.out_buffer.Length = ready->ae_out.Length;

		AeOutEnquene1.push(frameData);
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
	case ISP_SOFT_AE1:
	{
		return DispathAeMessage1(postType, data);

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


static int height = 1736;
static int width = 1800;
static unsigned short* input_buffer;
static unsigned char* output_buffer;

int main(int argc, char** argv)
{
	int bigframeid = 0;
	while (bigframeid < 10000)
	{


		/* buffer*/
		//input_buffer = (unsigned short*)malloc(height * width * sizeof(unsigned short));
		//output_buffer = (unsigned char*)malloc(height * width * 3 * sizeof(unsigned char));


		//IspSoftModule isp = IspSoftIspLoad();
		//IspSoftModule awb = IspSoftAwbLoad();
		IspSoftModule ae = IspSoftAeLoad();
		//IspSoftModule atf = IspSoftAtfLoad();
		void* owner;

		uint8_t cameraId = 0;
		/* isp init param*/
		IspSoftIspInitParam isp_init;
		isp_init.camera_id = cameraId;

		//IspSoftHandle isp_handle = isp.Init(owner, post_message, &isp_init);

		/* ae init param*/
		//enum atf_mode {
		//	ATF_MODE_OFF = 0,
		//	ATF_MODE_50HZ = 1,
		//	ATF_MODE_60HZ = 2,
		//	ATF_MODE_90HZ = 3,
		//	ATF_MODE_144HZ = 4,
		//	ATF_MODE_240HZ = 5,
		//	ATF_MODE_320HZ = 6,
		//	ATF_MODE_360HZ = 7,
		//	ATF_MODE_MAX,
		//};

		//struct atfbin
		//{
		//	uint8_t                                     flag_enable;
		//	uint8_t                                     flag_bending;
		//	enum atf_mode                               atf_mode;
		//	enum atf_mode                               atf_mode_stable;
		//};
		//atfbin atfdatawrite;
		//atfdatawrite.flag_enable = 0;
		//atfdatawrite.flag_bending = 0;
		//atfdatawrite.atf_mode = (atf_mode)0;
		//atfdatawrite.atf_mode_stable = (atf_mode)0;
		//ofstream fpatf("atfout.bin", ios::out | ios::binary);
		//fpatf.write((char*)&atfdatawrite,sizeof(atfdatawrite));
		//fpatf.close();

		ifstream filetuning;
		filetuning.open("AEParam.bin", ios::in | ios::binary | ios::ate);
		int tuningsize = filetuning.tellg();
		char* tuningdata = new char[tuningsize];
		filetuning.seekg(0, ios::beg);
		filetuning.read(tuningdata, tuningsize);
		filetuning.close();

		ifstream fileotp;
		fileotp.open("otpbin.bin", ios::in | ios::binary | ios::ate);
		int otpsize = fileotp.tellg();
		char* otpdata = new char[otpsize];
		fileotp.seekg(0, ios::beg);
		fileotp.read(otpdata, otpsize);
		fileotp.close();


		IspSoftAeInitParam ae_init;
		ae_init.camera_id = 0;
		ae_init.initial_sensor_adgain = 100;
		ae_init.initial_sensor_shutter_us = 1000;
		ae_init.otp_bin.Data = (uint8_t*)otpdata;
		ae_init.otp_bin.Length = otpsize;
		ae_init.tuning_bin.Data = (uint8_t*)tuningdata;
		ae_init.tuning_bin.Length = tuningsize;
		//ae_init.tuning_bin.Data = NULL;
		//ae_init.tuning_bin.Length = 0;
		ae_init.config_path = "D:\\3aalgo\\3a-tmp\\3a\\3ASimuHulk\\lib\\release";
		owner = &ae_init;
		part = &ae_init;
		IspSoftHandle ae_handle = ae.Init(owner, post_message, &ae_init);


		IspSoftAeInitParam ae_init1;
		ae_init1.camera_id = 1;
		ae_init1.initial_sensor_adgain = 100;
		ae_init1.initial_sensor_shutter_us = 1000;
		ae_init1.otp_bin.Data = (uint8_t*)otpdata;
		ae_init1.otp_bin.Length = otpsize;
		ae_init1.tuning_bin.Data = (uint8_t*)tuningdata;
		ae_init1.tuning_bin.Length = tuningsize;
		ae_init1.config_path = "D:\\3aalgo\\3a-tmp\\3a\\3ASimuHulk\\lib\\release";
		owner = &ae_init1;
		part1 = &ae_init1;
		IspSoftHandle ae_handle1 = ae.Init(owner, post_message, &ae_init1);
		/* awb init param*/
		IspSoftAwbInitParam awb_init;
		awb_init.camera_id = cameraId;

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
		AeOutEnquene1.push(init);
		/* fake isp stats out*/


		ifstream fileaestats;
		fileaestats.open("ISPAeStatusOut.bin", ios::in | ios::binary | ios::ate);
		int aestatssize = fileaestats.tellg();
		char* aestatsdata = new char[aestatssize];
		fileaestats.seekg(0, ios::beg);
		fileaestats.read(aestatsdata, aestatssize);
		fileaestats.close();
		//memset(aestatsdata + 20, 0, aestatssize-20);
		//memset(aestatsdata, 0, aestatssize);

		IspFrameData isp_out_ae;
		isp_out_ae.frame_id = 1;
		isp_out_ae.timestamp = 1;
		isp_out_ae.out_buffer.Data = (uint8_t*)aestatsdata;
		isp_out_ae.out_buffer.Length = aestatssize;

		AeInEnquene.push(isp_out_ae);

		static float cam0time[1000000];
		static float cam1time[1000000];

		LARGE_INTEGER tc;
		QueryPerformanceFrequency(&tc);
		printf("\n qcount %lld\n", tc.QuadPart);

		ifstream fileatfout;
		fileatfout.open("atfout.bin", ios::in | ios::binary | ios::ate);
		int atfoutsize = fileatfout.tellg();
		char* atfoutdata = new char[atfoutsize];
		fileatfout.seekg(0, ios::beg);
		fileatfout.read(atfoutdata, atfoutsize);
		fileatfout.close();


		ifstream fileawbout;
		fileawbout.open("AwbOut.bin", ios::in | ios::binary | ios::ate);
		int awboutsize = fileawbout.tellg();
		char* awboutdata = new char[awboutsize];
		fileawbout.seekg(0, ios::beg);
		fileawbout.read(awboutdata, awboutsize);
		fileawbout.close();



		/* process*/
		while (frameId < 1000)
		{
			IspSoftBuffer atf_out = AtfOutEnquene.back().out_buffer;
			IspSoftBuffer awb_out = AwbOutEnquene.back().out_buffer;
			IspSoftBuffer ae_out = AeOutEnquene.back().out_buffer;
			IspSoftBuffer ae_out1 = AeOutEnquene1.back().out_buffer;
			if (frameId > 1)
			{
				printf("frameId %d  debug %u, %u, debug1 %u  %u\n", frameId, ((IspSoftAeOut*)ae_out.Data)->ae_adgain, ((IspSoftAeOut*)ae_out.Data)->ae_shutter,
					((IspSoftAeOut*)ae_out1.Data)->ae_adgain, ((IspSoftAeOut*)ae_out1.Data)->ae_shutter);

			}

			// pop 
			//AtfOutEnquene.pop();
			//AwbOutEnquene.pop();
			AeOutEnquene.pop();
			AeOutEnquene1.pop();
			/* ISP*/

			/* AWB*/


			/* AE*/
			IspSoftBuffer ae_stats = AeInEnquene.front().out_buffer;
			IspSoftAeInImage ae_in;
			ae_in.frame_id = frameId;
			ae_in.timestamp = 1000;
			ae_in.atf_out.Data = (uint8_t*)atfoutdata;
			ae_in.atf_out.Length = atfoutsize;
			ae_in.awb_out.Data = (uint8_t*)awboutdata;
			ae_in.awb_out.Length = awboutsize;

			//ae_in.atf_out.Data = NULL;
			//ae_in.atf_out.Length = 0;
			//ae_in.awb_out.Data = NULL;
			//ae_in.awb_out.Length = 0;

			ae_in.isp_out = ae_stats;

			//ae.Process(ae_handle, ISP_SOFT_AE_PROCESS_IN_TUNING, &ae_in);
			ae.Process(ae_handle, ISP_SOFT_AE_PROCESS_IN_DEBUG, &ae_in);
			LARGE_INTEGER t1, t2;
			QueryPerformanceCounter(&t1);
			//auto aeerr = async(ae.Process, ae_handle, ISP_SOFT_AE_PROCESS_IN_IMAGE, &ae_in);
			int aeerrcode = ae.Process(ae_handle, ISP_SOFT_AE_PROCESS_IN_IMAGE, &ae_in);
			QueryPerformanceCounter(&t2);
			//int aeerrcode = aeerr.get();



			IspSoftAeInImage ae_in1;
			ae_in1.frame_id = frameId;
			ae_in1.timestamp = 1000;
			ae_in1.atf_out.Data = (uint8_t*)atfoutdata;
			ae_in1.atf_out.Length = atfoutsize;
			ae_in1.awb_out.Data = (uint8_t*)awboutdata;
			ae_in1.awb_out.Length = awboutsize;

			//ae_in1.atf_out.Data = NULL;
			//ae_in1.atf_out.Length = 0;
			//ae_in1.awb_out.Data = NULL;
			//ae_in1.awb_out.Length = 0;

			ae_in1.isp_out = ae_stats;

			//ae.Process(ae_handle1, ISP_SOFT_AE_PROCESS_IN_TUNING, &ae_in);
			ae.Process(ae_handle1, ISP_SOFT_AE_PROCESS_IN_DEBUG, &ae_in1);
			LARGE_INTEGER t3, t4;
			QueryPerformanceCounter(&t3);
			//auto aeerr1 = async(ae.Process, ae_handle1, ISP_SOFT_AE_PROCESS_IN_IMAGE, &ae_in);
			int aeerrcode1 = ae.Process(ae_handle1, ISP_SOFT_AE_PROCESS_IN_IMAGE, &ae_in1);
			QueryPerformanceCounter(&t4);
			//int aeerrcode1 = 0;
			//aeerrcode1 = aeerr1.get();

			if (aeerrcode != 0 || aeerrcode1 != 0)
			{
				printf("ERROR!!AE  cam0 %d  cam1 %d", aeerrcode, aeerrcode1);
				return 0;
			}
			cam0time[frameId] = (t2.QuadPart - t1.QuadPart) / 10000.0f;
			cam1time[frameId] = (t4.QuadPart - t3.QuadPart) / 10000.0f;
			printf("cam0 time %f,  cam1time1 %f\n", cam0time[frameId], cam1time[frameId]);


			/* ATF*/

			/* callback func get atf out print*/
			//IspSoftAeOut* ae_out_print = (IspSoftAeOut*)AeOutEnquene.back().out_buffer.Data;

			frameId++;
		}

		//atf.Deinit(atf_handle);
		ae.Deinit(ae_handle);
		ae.Deinit(ae_handle1);
		//awb.Deinit(awb_handle);
		//isp.Deinit(isp_handle);

		//ofstream ohfile;
		//ohfile.open("cam0time0000.txt", ios::out);
		//for (int i = 0; i < 10000; i++)
		//{
		//	ohfile << cam0time[i] << endl;
		//}
		//ohfile.close();
		//ofstream ocfile;
		//ocfile.open("cam1time0000.txt", ios::out);
		//for (int i = 0; i < 10000; i++)
		//{
		//	ocfile << cam1time[i] << endl;
		//}
		//ocfile.close();
		bigframeid++;
		printf("\ndebug------------- %d\n", bigframeid);

		AeInEnquene.pop();
		delete[] tuningdata;
		tuningdata = NULL;
		delete[] otpdata;
		otpdata = NULL;
		delete[] aestatsdata;
		aestatsdata = NULL;
		delete[] atfoutdata;
		atfoutdata = NULL;
		delete[] awboutdata;
		awboutdata = NULL;

	}
	return 0;
}


