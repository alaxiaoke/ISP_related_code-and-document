#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "../include/gxr_raw2bgr.h"

using namespace std;

static int height = 1744; // 输入图高
static int width = 2328; // 输入图宽
static int const lsc_height = 13;
static int const lsc_width = 17;
static float lsctable[lsc_height * lsc_width * 4];
static unsigned short gmtablex[257] = { 0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,256,260,264,268,272,276,280,284,288,292,296,300,304,308,312,316,320,324,328,332,336,340,344,348,352,356,360,364,368,372,376,380,384,388,392,396,400,404,408,412,416,420,424,428,432,436,440,444,448,452,456,460,464,468,472,476,480,484,488,492,496,500,504,508,512,516,520,524,528,532,536,540,544,548,552,556,560,564,568,572,576,580,584,588,592,596,600,604,608,612,616,620,624,628,632,636,640,644,648,652,656,660,664,668,672,676,680,684,688,692,696,700,704,708,712,716,720,724,728,732,736,740,744,748,752,756,760,764,768,772,776,780,784,788,792,796,800,804,808,812,816,820,824,828,832,836,840,844,848,852,856,860,864,868,872,876,880,884,888,892,896,900,904,908,912,916,920,924,928,932,936,940,944,948,952,956,960,964,968,972,976,980,984,988,992,996,1000,1004,1008,1012,1016,1020,1023 };
static unsigned short gmtabley[257] = { 0, 22, 43, 60, 74, 86, 96, 106, 114, 123, 130, 137, 144, 153, 167, 180, 192, 204, 214, 227, 240, 252, 264, 275, 286, 297, 307, 317, 327, 336, 345, 354, 363, 371, 380, 388, 396, 403, 411, 418, 426, 433, 440, 447, 454, 461, 467, 474, 480, 487, 493, 499, 505, 511, 517, 523, 529, 534, 540, 546, 551, 557, 562, 567, 573, 578, 583, 588, 593, 598, 603, 608, 613, 618, 623, 627, 632, 637, 642, 646, 651, 655, 660, 664, 669, 673, 677, 682, 686, 690, 695, 699, 703, 707, 711, 715, 719, 723, 727, 731, 735, 739, 743, 747, 751, 755, 759, 762, 766, 770, 774, 777, 781, 785, 788, 792, 796, 799, 803, 806, 809, 811, 814, 816, 819, 822, 824, 827, 829, 832, 834, 837, 839, 841, 844, 846, 849, 851, 854, 856, 858, 861, 863, 865, 868, 870, 872, 875, 877, 879, 881, 884, 886, 888, 890, 893, 895, 897, 899, 901, 904, 906, 908, 910, 912, 914, 917, 919, 921, 923, 925, 927, 929, 931, 933, 935, 938, 940, 942, 944, 946, 948, 950, 952, 954, 956, 958, 960, 962, 964, 966, 968, 970, 972, 973, 975, 977, 979, 981, 983, 985, 987, 989, 991, 993, 994, 994, 995, 996, 996, 997, 997, 998, 999, 999, 1000, 1000, 1001, 1001, 1002, 1003, 1003, 1004, 1004, 1005, 1006, 1006, 1007, 1007, 1008, 1008, 1009, 1010, 1010, 1011, 1011, 1012, 1012, 1013, 1014, 1014, 1015, 1015, 1016, 1016, 1017, 1018, 1018, 1019, 1019, 1020, 1020, 1021, 1021, 1022, 1023, 1023 };
static unsigned short* input_buffer; // 输入图 bayer unpack
static unsigned char* output_buffer; // 输出图 bgr pack

static bool ppmWrite(char const* name, unsigned char* src, int rows, int cols)
{
	char header[256];
	int szhead = snprintf(header, sizeof(header),
		"P%c\n# Generated by GXR Chen Jiayun\n%d %d\n255\n",
		static_cast<char>('5' + 1), cols, rows);
	FILE * fid = fopen(name, "wb");
	fwrite(header, 1, szhead, fid);
	fwrite(src, 1, rows * cols * 3, fid);
	fclose(fid);
	return true;
}

int main(int argc, char** argv)
{
	/* buffer */
	input_buffer = (unsigned short*)malloc(height * width * sizeof(unsigned short));
	output_buffer = (unsigned char*)malloc(height * width * 3 * sizeof(unsigned char));

	/* lens shading表 */
	for (int i = 0; i < sizeof(lsctable) / sizeof(float); i++)
		lsctable[i] = 1.f;

	/* 算法句柄初始化 */
	printf("gxr_raw2bgr version: %s\n", gxr_raw2bgr_version().Version);
	gxr_raw2bgr * raw2bgr_handle = new gxr_raw2bgr;
	gxr_raw2bgr_initialize(raw2bgr_handle);

	/* 跑一系列图 */
	int frame_num = 0;
	while (frame_num++ < 10)
	{
		FILE* fid = fopen("0.raw", "rb");
		fread(input_buffer, sizeof(unsigned short), height * width, fid);
		fclose(fid);

		char verbose = 7;
		int rpat = GXR_Raw_RGGB;
		int depth = 10;
		int bpc = 0; // 暂时没有
		char wben = 1, expen = 1; // 白平衡和曝光外部传入
		int awbsat = 255; // 8位意义上
		int awb_blknumx = 16;
		int awb_blknumy = 16;
		vector<int> awbstats(awb_blknumx * awb_blknumy * 5);
		int aesat = 1023; // 10位意义上
		int ae_blknumx = 16;
		int ae_blknumy = 16;
		vector<int> aestats(ae_blknumx * ae_blknumy * 8);
		vector<int> histgram(256);
		int top = 0, bottom = height, left = 0, right = width; // 凝视区域
		float wbgain[4] = { 1,1,1,1 }; // r gr gb b
		int blacklevel[4] = { 64, 64, 64, 64 }; // 按10bit
		float ccm[9] = { 1.568639,-0.358657,-0.209982,-0.191115,1.646191,-0.455076,0.070816,-0.684568,1.613751 }; // rgb
		float sensorgain = 1;
		float ispgain = 1;
		float adrcgain = 1;
		float shutter = 4000; // us
		float luxindex = 1;

		raw2bgr_handle->verbose = verbose;
		raw2bgr_handle->rpat = rpat;
		raw2bgr_handle->depth = depth;
		raw2bgr_handle->bpc = bpc;
		raw2bgr_handle->height = height;
		raw2bgr_handle->width = width;
		raw2bgr_handle->top = top;
		raw2bgr_handle->bottom = bottom;
		raw2bgr_handle->left = left;
		raw2bgr_handle->right = right;
		raw2bgr_handle->wben = wben;
		raw2bgr_handle->expen = expen;
		raw2bgr_handle->awbsat = awbsat;
		raw2bgr_handle->awb_blknumx = awb_blknumx;
		raw2bgr_handle->awb_blknumy = awb_blknumy;
		raw2bgr_handle->aesat = aesat;
		raw2bgr_handle->ae_blknumx = ae_blknumx;
		raw2bgr_handle->ae_blknumy = ae_blknumy;
		memcpy(raw2bgr_handle->blacklevel, blacklevel, sizeof(blacklevel));
		raw2bgr_handle->sensorgain = sensorgain;
		raw2bgr_handle->ispgain = ispgain;
		raw2bgr_handle->adrcgain = adrcgain;
		raw2bgr_handle->shutter = shutter;
		raw2bgr_handle->luxindex = luxindex;
		raw2bgr_handle->lsc_height = lsc_height;
		raw2bgr_handle->lsc_width = lsc_width;
		raw2bgr_handle->lsctable[0] = lsctable;
		raw2bgr_handle->lsctable[1] = lsctable + lsc_height * lsc_width;
		raw2bgr_handle->lsctable[2] = lsctable + lsc_height * lsc_width * 2;
		raw2bgr_handle->lsctable[3] = lsctable + lsc_height * lsc_width * 3;
		raw2bgr_handle->gmtablex = gmtablex;
		raw2bgr_handle->gmtabley = gmtabley;
		raw2bgr_handle->input = input_buffer;
		raw2bgr_handle->output = output_buffer;
		// 拿统计
		gxr_raw2bgr_run_all(raw2bgr_handle);
		memcpy(awbstats.data(), raw2bgr_handle->handle[2], awb_blknumx * awb_blknumy * 5 * sizeof(int));
		memcpy(aestats.data(), raw2bgr_handle->handle[3], ae_blknumx * ae_blknumy * 8 * sizeof(int));
		memcpy(histgram.data(), raw2bgr_handle->handle[4], 256 * sizeof(int));

		// 调用2A，wbgain、ccm、lsctable、ispgain更新传进来
		// callawb(awbstats.data(), wbgain, ccm, lsctable);
		// callae(aestats.data(), histgram.data(), &ipsgain);
		memcpy(raw2bgr_handle->wbgain, wbgain, sizeof(wbgain));
		memcpy(raw2bgr_handle->ccm, ccm, sizeof(ccm));
		raw2bgr_handle->ispgain = ispgain;
		raw2bgr_handle->lsctable[0] = lsctable;
		raw2bgr_handle->lsctable[1] = lsctable + lsc_height * lsc_width;
		raw2bgr_handle->lsctable[2] = lsctable + lsc_height * lsc_width * 2;
		raw2bgr_handle->lsctable[3] = lsctable + lsc_height * lsc_width * 3;
		gxr_raw2bgr_run_all(raw2bgr_handle);

		ppmWrite("0.ppm", output_buffer, height, width);
	}

	gxr_raw2bgr_release(raw2bgr_handle);
	delete raw2bgr_handle;
	free(input_buffer);
	free(output_buffer);
	return 0;
}