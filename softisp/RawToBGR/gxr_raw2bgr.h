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

	/* λ��־�������ã���ӡ��Ϣ	*/
	char verbose;

	/* Bayer Pattern */
	int rpat;

	/* λ�� */
	int depth;

	/* BPC */
	char bpc;

	/* ȥ�� �� */
	float ynoise[5 * 8], cnoise[5 * 8];
	float sharp[5 * 8];
	float bdlarge[8];

	/* ������ */
	int top, bottom, left, right;

	/* ��ƽ����ع����ģʽ
	 * 0 �ڲ����ƣ�1 �ⲿ������Ч
	 */
	char wben, expen;

	int aesat; // 12 λ������

	/* R Gr Gb B */
	float wbgain[4];
	int blacklevel[4];

	/* CCM��RGB ˳�� */
	float ccm[9];

	/* �ع���� */
	float sensorgain, ispgain, adrcgain;
	float shutter, luxindex;

	/* LSC 4 ��ͨ�� R Gr Gb B */
	int lsc_height, lsc_width;
	float* lsctable[4];

	/* Gamma������Ϊ257 */
	unsigned short* gmtablex, *gmtabley;

	/* ������� */
	int height, width;
	unsigned short* input;
	unsigned char* output;

	// ������
	int ntick;
	int64_t tickpre, ticksum;
	void* handle[8];
} gxr_raw2bgr;


GXR_RAW2BGR_ExternC GXR_RAW2BGR_Export int gxr_raw2bgr_initialize(gxr_raw2bgr* mr);

GXR_RAW2BGR_ExternC GXR_RAW2BGR_Export void gxr_raw2bgr_release(gxr_raw2bgr* mr);

GXR_RAW2BGR_ExternC GXR_RAW2BGR_Export void gxr_raw2bgr_run_all(gxr_raw2bgr* mr);

GXR_RAW2BGR_ExternC GXR_RAW2BGR_Export GXR_RAW2BGR_VERSION gxr_raw2bgr_version();