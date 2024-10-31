#pragma once

#include <stdint.h>
#include <stdlib.h>

# ifdef GXR_RAW2BGR_Build
# define GXR_RAW2BGR_Export __declspec(dllexport)
# else
# define GXR_RAW2BGR_Export __declspec(dllimport)
# endif

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
	char wben, expen;

	int aesat; // 12 位意义上

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
} gxr_raw2bgr;


GXR_RAW2BGR_ExternC GXR_RAW2BGR_Export int gxr_raw2bgr_initialize(gxr_raw2bgr* mr);

GXR_RAW2BGR_ExternC GXR_RAW2BGR_Export void gxr_raw2bgr_release(gxr_raw2bgr* mr);

GXR_RAW2BGR_ExternC GXR_RAW2BGR_Export void gxr_raw2bgr_run_all(gxr_raw2bgr* mr);

GXR_RAW2BGR_ExternC GXR_RAW2BGR_Export GXR_RAW2BGR_VERSION gxr_raw2bgr_version();