#pragma once

#ifndef _LUT3D_009_H_
#define _LUT3D_009_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define  LUT_SIZE 33   //查找表取值空间是32*32*32，
#define  SHIFT 20	

//YUV转RGB系数：
//R = Y + c0 * (Cr - c4)
//G = Y + c1 * (Cb - c4) + c2 * (Cr - c4)
//B = Y + c3 (Cb - c4)
//RGB转YUV系数：
//Y = c0 * R + c1 * G + c2 * B
//U = c3 * R + c4 * G + c5 * B + c9
//V = c6 * R + c7 * G + c8 * B + c9
typedef struct {
	int32_t RegEnable;
	int32_t RegImgWidth;
	int32_t RegImgHeight;
	int32_t YUV2RGB_coef[5];
	int32_t RGB2YUV_coef[10];

}_isp_Lut3d_Param_t;


class CLut3D
{
private:

	int cubeIndex(int r, int g, int b);

public:
	_isp_Lut3d_Param_t lutParam;

	CLut3D()
	{
		memset(&lutParam, 0, sizeof(_isp_Lut3d_Param_t));
	}

	~CLut3D()
	{
	}
	bool SaveSettingFile(const char* filename)
	{
		lutParam.RegEnable = 1;//l bit
		lutParam.RegImgHeight = 3280;//l2 bit 
		lutParam.RegImgWidth = 2448;//l2 bit
		lutParam.YUV2RGB_coef[0] = 1.402 * 4096; //l4 bit
		lutParam.YUV2RGB_coef[1] = -0.344136 * 4096;
		lutParam.YUV2RGB_coef[2] = -0.714136 * 4096;
		lutParam.YUV2RGB_coef[3] = 1.772 * 4096;
		lutParam.YUV2RGB_coef[4] = 128;

		lutParam.RGB2YUV_coef[0] = 0.299 * 4096;
		lutParam.RGB2YUV_coef[1] = 0.587 * 4096;
		lutParam.RGB2YUV_coef[2] = 0.114 * 4096;
		lutParam.RGB2YUV_coef[3] = -0.1687 * 4096;
		lutParam.RGB2YUV_coef[4] = -0.3313 * 4096;
		lutParam.RGB2YUV_coef[5] = 0.5 * 4096;
		lutParam.RGB2YUV_coef[6] = 0.5 * 4096;
		lutParam.RGB2YUV_coef[7] = -0.4187 * 4096;
		lutParam.RGB2YUV_coef[8] = -0.0813 * 4096;
		lutParam.RGB2YUV_coef[9] = 128;


		FILE* pf;
		pf = fopen(filename, "wb+");
		if (pf)
		{
			fprintf(pf, "RegEnable=%d\n", lutParam.RegEnable);
			fprintf(pf, "RegImgHeight=%d\n", lutParam.RegImgHeight);
			fprintf(pf, "RegImgWidth=%d\n", lutParam.RegImgWidth);

			fprintf(pf, "YUV2RGB_coef=%d\n", lutParam.YUV2RGB_coef[0]);
			fprintf(pf, "YUV2RGB_coef=%d\n", lutParam.YUV2RGB_coef[1]);
			fprintf(pf, "YUV2RGB_coef=%d\n", lutParam.YUV2RGB_coef[2]);
			fprintf(pf, "YUV2RGB_coef=%d\n", lutParam.YUV2RGB_coef[3]);
			fprintf(pf, "YUV2RGB_coef=%d\n", lutParam.YUV2RGB_coef[4]);

			fprintf(pf, "RGB2YUV_coef=%d\n", lutParam.RGB2YUV_coef[0]);
			fprintf(pf, "RGB2YUV_coef=%d\n", lutParam.RGB2YUV_coef[1]);
			fprintf(pf, "RGB2YUV_coef=%d\n", lutParam.RGB2YUV_coef[2]);
			fprintf(pf, "RGB2YUV_coef=%d\n", lutParam.RGB2YUV_coef[3]);
			fprintf(pf, "RGB2YUV_coef=%d\n", lutParam.RGB2YUV_coef[4]);
			fprintf(pf, "RGB2YUV_coef=%d\n", lutParam.RGB2YUV_coef[5]);
			fprintf(pf, "RGB2YUV_coef=%d\n", lutParam.RGB2YUV_coef[6]);
			fprintf(pf, "RGB2YUV_coef=%d\n", lutParam.RGB2YUV_coef[7]);
			fprintf(pf, "RGB2YUV_coef=%d\n", lutParam.RGB2YUV_coef[8]);
			fprintf(pf, "RGB2YUV_coef=%d\n", lutParam.RGB2YUV_coef[9]);

			fclose(pf);
			pf = NULL;
		}
		else
		{
			printf("error : Save Stting File Fail\n");
			return false;
		}

		return true;
	}

	bool LoadSettingFile(const char* filename)
	{
		char Line[1024];
		int aaa;
		int bbb[18];
		int32_t i;
		FILE* pf;
		pf = fopen(filename, "rb");
		if (pf)
		{
			for (i = 0; i < 18; i++)
			{
				fgets(Line, 1024, pf);
				const char* equal_pos = strchr(Line, '=');
				if (equal_pos != nullptr) {
					++equal_pos; // 跳过等号
					int value;
					sscanf(equal_pos, "%d", &bbb[i]);
				}
			}


			fclose(pf);
			pf = NULL;

			lutParam.RegEnable = bbb[0];
			lutParam.RegImgHeight = bbb[1];
			lutParam.RegImgWidth = bbb[2];
			lutParam.YUV2RGB_coef[0] = bbb[3];
			lutParam.YUV2RGB_coef[1] = bbb[4];
			lutParam.YUV2RGB_coef[2] = bbb[5];
			lutParam.YUV2RGB_coef[3] = bbb[6];
			lutParam.YUV2RGB_coef[4] = bbb[7];

			lutParam.RGB2YUV_coef[0] = bbb[8];
			lutParam.RGB2YUV_coef[1] = bbb[9];
			lutParam.RGB2YUV_coef[2] = bbb[10];
			lutParam.RGB2YUV_coef[3] = bbb[11];
			lutParam.RGB2YUV_coef[4] = bbb[12];
			lutParam.RGB2YUV_coef[5] = bbb[13];
			lutParam.RGB2YUV_coef[6] = bbb[14];
			lutParam.RGB2YUV_coef[7] = bbb[15];
			lutParam.RGB2YUV_coef[8] = bbb[16];
			lutParam.RGB2YUV_coef[9] = bbb[17];
		}
		else
		{
			printf("error : load Stting File Fail\n");
			return false;
		}

		return true;
	}

	void RGBtoYUV420(const uint8_t* rgb, uint8_t* yuv, int width, int height);
	void YUV420toRGB(uint8_t* yuv, uint8_t* rgb, int width, int height);
	int  ProcessImage(uint8_t* yuv, uint8_t* LUTD3, uint8_t* yuvout);
	int  ProcessImagePCTest(uint8_t* rgb, uint8_t* LUTD3, uint8_t* rgbout);

protected:

};

#endif


