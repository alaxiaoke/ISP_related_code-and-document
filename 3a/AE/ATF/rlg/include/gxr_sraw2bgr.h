#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
#  define GXR_RAW2BGR_ExternC extern "C"
#else
#  define GXR_RAW2BGR_ExternC
#endif

enum GXR_RawPatternTypes
{
	GXR_Raw_RGGB,
	GXR_Raw_BGGR,
	GXR_Raw_GRBG,
	GXR_Raw_GBRG,
	GXR_Raw_Patt_Impl = 4,
};


typedef struct GXR_RAW2BGR_VERSION
{
	const char* Version;
} GXR_RAW2BGR_VERSION;


/* lsctable input output内存为内部状态
/* lsctable 每次调用gxr_raw2bgr_run_all返回前阻塞
/* input output 每两次调用gxr_raw2bgr_run_all返回前阻塞
 */
typedef struct
{
	void* impl;

	/* 位标志，调试用，打印信息	*/
	char verbose;

	/* Bayer Pattern */
	int rpat;

	/* 位深 */
	int depth;

	/* BPC */
	char bpc;

	/* 去噪 锐化 */
	float ynoise[5 * 8], cnoise[5 * 8];
	float sharp[5 * 8];
	float bdlarge[8];

	/* 凝视区 */
	int top, bottom, left, right;

	/* 白平衡和曝光参数模式
	 * 0 内部估计，1 外部传入有效
	 */
	char wben, expen, atfen;

	int awb_blknumx, awb_blknumy;
	int awbsat; // 8位意义上
	int ae_blknumx, ae_blknumy;
	int aesat; // 10位意义上
	int atf_roiheight, atf_roiwidth; // 中间的多少行多少列

	/* R Gr Gb B */
	float wbgain[4];
	int blacklevel[4];

	/* CCM，RGB 顺序 */
	float ccm[9];

	/* 曝光参数 */
	float sensorgain, ispgain, adrcgain;
	float shutter, luxindex;

	/* LSC 4 个通道 R Gr Gb B */
	int lsc_height, lsc_width;
	float* lsctable[4];

	/* Gamma表，长度为257 */
	unsigned short* gmtablex, *gmtabley;

	/* 输入输出 */
	int height, width;
	unsigned short* input;
	unsigned char* output;

	// 调试用
	int ntick;
	int64_t tickpre, ticksum;
	void* handle[8];
} gxr_sraw2bgr;


GXR_RAW2BGR_ExternC int gxr_sraw2bgr_initialize(gxr_sraw2bgr* mr);

GXR_RAW2BGR_ExternC void gxr_sraw2bgr_release(gxr_sraw2bgr* mr);

GXR_RAW2BGR_ExternC void gxr_sraw2bgr_run_all(gxr_sraw2bgr* mr); // 一张图用一个实例来调，如果要不同线程调，返回了再调

GXR_RAW2BGR_ExternC GXR_RAW2BGR_VERSION gxr_sraw2bgr_version();