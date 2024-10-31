#pragma once

#ifndef _YUV_CROP_H_
#define _YUV_CROP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef int8_t
#define int8_t char
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef int16_t
#define int16_t short
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef int32_t
#define int32_t int
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef int64_t
#define int64_t long
#endif

#ifndef uint64_t
#define uint64_t unsigned long
#endif

#ifndef min 
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef clip
#define clip(x,a,b) (min(max(x,a),b))
#endif

#ifndef DATA_BITS
#define DATA_BITS 10
#endif

typedef struct {
	int32_t RegEnable;//l bit

	int32_t RegInputWidth;//l2 bit 
	int32_t RegInputHeight;//l2 bit

	int32_t RegStartH;//l2 bit 
	int32_t RegCropHeight;//l2 bit 

	int32_t RegStartW;//l2 bit 
	int32_t RegCropWidth;//l2 bit 

}_isp_Yuv_Crop_Param_t;


class CYuvCrop
{
private:

public:
	_isp_Yuv_Crop_Param_t m_sCtrlParam;

	CYuvCrop()
	{
	}

	~CYuvCrop()
	{
	}

	bool SaveSettingFile( char *filename)
	{
		m_sCtrlParam.RegEnable = 1;//l bit

		m_sCtrlParam.RegInputWidth = 640;//l2 bit 
		m_sCtrlParam.RegInputHeight = 480;//l2 bit

		m_sCtrlParam.RegStartH = 0;//l2 bit 
		m_sCtrlParam.RegCropHeight = 480;//l2 bit 

		m_sCtrlParam.RegStartW = 0;//l2 bit 
		m_sCtrlParam.RegCropWidth = 640;//l2 bit 

		FILE* pf;
		pf = fopen(filename, "wb+");
		if (pf)
		{
			fprintf(pf, "RegEnable = %d\n", m_sCtrlParam.RegEnable);
			fprintf(pf, "RegInputWidth = %d\n", m_sCtrlParam.RegInputWidth);
			fprintf(pf, "RegInputHeight = %d\n", m_sCtrlParam.RegInputHeight);
			fprintf(pf, "RegStartH = %d\n", m_sCtrlParam.RegStartH);
			fprintf(pf, "RegCropHeight = %d\n", m_sCtrlParam.RegCropHeight);
			fprintf(pf, "RegStartW = %d\n", m_sCtrlParam.RegStartW);
			fprintf(pf, "RegCropWidth = %d\n", m_sCtrlParam.RegCropWidth);
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

	bool LoadSettingFile(char* filename)
	{
		char Line[1024];
		int aaa;
		int bbb[7];
		int32_t i;
		FILE* pf;
		pf = fopen(filename, "rb");
		if (pf)
		{
			for (i = 0; i < 7; i++)
			{
				fgets(Line, 1024, pf);
				//sscanf("linEgG_ddddd      = 123", "%*[a-zA-Z= _]%d", &a);
				sscanf(Line, "%*[a-zA-Z= _]%d", &bbb[i]);
				//char *p = strchr(&Line[0], '=');
				//printf("%d\n", bbb[i]);
			}

			
			fclose(pf);
			pf = NULL;

			m_sCtrlParam.RegEnable = bbb[0];//l bit

			m_sCtrlParam.RegInputWidth = bbb[1];//l2 bit 
			m_sCtrlParam.RegInputHeight = bbb[2];//l2 bit

			m_sCtrlParam.RegStartH = bbb[3];//l2 bit 
			m_sCtrlParam.RegCropHeight = bbb[4];//l2 bit 

			m_sCtrlParam.RegStartW = bbb[5];//l2 bit 
			m_sCtrlParam.RegCropWidth = bbb[6];//l2 bit 
		}
		else
		{
			printf("error : Save Stting File Fail\n");
			return false;
		}

		return true;
	}
	bool ProcessImage(uint16_t*Dst, uint16_t* Src);
	//bool TestSingleFilter(int argc, char** argv, bool bSaveBMP = false, bool bLoadBinary = true);
	
protected:

};



#endif