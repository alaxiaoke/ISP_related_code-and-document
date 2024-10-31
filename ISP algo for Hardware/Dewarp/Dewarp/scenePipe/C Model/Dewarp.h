#pragma once

#ifndef _DEWARP_H_
#define _DEWARP_H_

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

#ifndef FLOATPRECISION
#define FLOATPRECISION 12
#endif

#ifndef PRECISIONMAX
#define PRECISIONMAX ((1<<FLOATPRECISION)-1)
#endif

#ifndef PRECISIONHALF
#define PRECISIONHALF (1<<(FLOATPRECISION-1))
#endif

#ifndef DATA_BITS
#define DATA_BITS 10
#endif

#ifndef MAX_DATA_VALUE
#define MAX_DATA_VALUE ((1<<DATA_BITS)-1)
#endif

#ifndef LINE_BUFFER_HEIGHT
#define LINE_BUFFER_HEIGHT 219
#endif

#ifndef LINE_BUFFER_WIDTH
#define LINE_BUFFER_WIDTH 2816
#endif

#ifndef TABLE_BUFFER_WIDTH
#define TABLE_BUFFER_WIDTH 560
#endif

typedef struct {
	int32_t RegEnable;//l bit

	int32_t RegFullWidth;//l2 bit 
	int32_t RegFullHeight;//l2 bit

	int32_t RegStartH;//l2 bit 
	int32_t RegInputHeight;//l2 bit 

	int32_t RegStartW;//l2 bit 
	int32_t RegInputWidth;//l2 bit 

	int32_t RegMapWidth;//9 bit
	int32_t RegMapHeight;//9 bit

	int32_t RegBlockWidth; // 4 bit
	int32_t RegInitLinebuffHeight;// 8 bit

}_isp_Dewarp_Param_t;


typedef struct {

	uint16_t* pLineBuffY[LINE_BUFFER_HEIGHT];
	uint16_t* pLineBuffUV[LINE_BUFFER_HEIGHT];
	uint8_t* pTableLineBuff[3];
	int32_t pDataStartLine;
	int32_t pDataValidLine;
	int32_t pMapStartLine;
	int32_t pRealMapWidth;
	int32_t pBlockWidth;
	int32_t pInputNum;
	int32_t pRollingNum;
	int32_t pCoorGainH;
	int32_t pOutputLine;
	int32_t pOutputColumn;
	int32_t pCoorX[2];//0-left,1-right
	int32_t pCoorY[2];//0-left,1-right
	int32_t pCoorFinalY[2];//0-x,1-y
	int32_t pCoorFinalUV[2];//0-x,1-y

	int32_t pYH[2];
	int32_t pYW[2];
	int32_t pYGain[2];//0-gainx,1-gainy
	
	int32_t pUVH[2];
	int32_t pUVW[2];
	int32_t pUVGain[2];//0-gainx,1-gainy

	int32_t pPixelY[2][2];
	int32_t pPixelUV[2][2];
}_dewarp_internal_t;

class CDewarp
{
private:
	_dewarp_internal_t m_sInternalParam;
	int32_t StartFrame(uint16_t* Src, uint8_t* DewarpTable);
	int32_t StartLine(int32_t RowIndex);
	int32_t EndLine();
	int32_t UpdateCoor(int32_t ColumnIndex);
	int32_t GetFinalCoor(int32_t ColumnIndex);
	int32_t GetRowColIndexGain();
	int32_t ProcessOnePixelY();
	int32_t ProcessOnePixelUV(int32_t Offset);
	int32_t GetInputROllingNum(int32_t RowIndex);
	int32_t GetDataToLineBuffer(uint16_t* Src);
	int32_t RollingDataOutLineBuffer();
	int32_t GetCoorToTableBuffer(uint8_t* DewarpTable);
	int32_t RollingDataOutTableBuffer();
	int32_t ProcOneLine(uint16_t* DstY, uint16_t* DstUV, int32_t RowIndex);
public:
	_isp_Dewarp_Param_t m_sCtrlParam;

	CDewarp()
	{
		memset(&m_sInternalParam, 0, sizeof(_dewarp_internal_t));
		m_sInternalParam.pLineBuffY[0] = new uint16_t[LINE_BUFFER_WIDTH * LINE_BUFFER_HEIGHT];
		m_sInternalParam.pLineBuffUV[0] = new uint16_t[LINE_BUFFER_WIDTH * LINE_BUFFER_HEIGHT];
		m_sInternalParam.pTableLineBuff[0] = new uint8_t[TABLE_BUFFER_WIDTH * 3];
		int32_t i;
		if (m_sInternalParam.pLineBuffY[0] && m_sInternalParam.pLineBuffUV[0] && m_sInternalParam.pTableLineBuff[0])
		{
			for (i = 1; i < LINE_BUFFER_HEIGHT; i++)
			{
				m_sInternalParam.pLineBuffY[i] = m_sInternalParam.pLineBuffY[i - 1] + LINE_BUFFER_WIDTH;
				m_sInternalParam.pLineBuffUV[i] = m_sInternalParam.pLineBuffUV[i - 1] + LINE_BUFFER_WIDTH;
			}
			for (i = 1; i < 3; i++)
			{
				m_sInternalParam.pTableLineBuff[i] = m_sInternalParam.pTableLineBuff[i - 1] + TABLE_BUFFER_WIDTH;
			}
			printf("the internal Buffer malloc Success\n");
		}
		else
		{
			printf("error: the internal Buffer malloc Fail\n");
		}
	}

	~CDewarp()
	{
		if (m_sInternalParam.pLineBuffY[0])
			delete m_sInternalParam.pLineBuffY[0];
		if (m_sInternalParam.pLineBuffUV[0])
			delete m_sInternalParam.pLineBuffUV[0];
		if (m_sInternalParam.pTableLineBuff[0])
			delete m_sInternalParam.pTableLineBuff[0];
	}

	bool SaveSettingFile( char *filename)
	{
		m_sCtrlParam.RegEnable = 1;//l bit

		m_sCtrlParam.RegFullWidth = 640;//l2 bit 
		m_sCtrlParam.RegFullHeight = 480;//l2 bit

		m_sCtrlParam.RegStartH = 0;//l2 bit 
		m_sCtrlParam.RegInputHeight = 480;//l2 bit 

		m_sCtrlParam.RegStartW = 0;//l2 bit 
		m_sCtrlParam.RegInputWidth = 640;//l2 bit 

		m_sCtrlParam.RegMapWidth = 44;//9 bit
		m_sCtrlParam.RegMapHeight = 31;//9 bit

		m_sCtrlParam.RegBlockWidth = 4; // 4 bit
		m_sCtrlParam.RegInitLinebuffHeight = 100;// 8 bit

		FILE* pf;
		pf = fopen(filename, "wb+");
		if (pf)
		{
			fprintf(pf, "RegEnable = %d\n", m_sCtrlParam.RegEnable);
			fprintf(pf, "RegFullWidth = %d\n", m_sCtrlParam.RegFullWidth);
			fprintf(pf, "RegFullHeight = %d\n", m_sCtrlParam.RegFullHeight);
			fprintf(pf, "RegStartH = %d\n", m_sCtrlParam.RegStartH);
			fprintf(pf, "RegInputHeight = %d\n", m_sCtrlParam.RegInputHeight);
			fprintf(pf, "RegStartW = %d\n", m_sCtrlParam.RegStartW);
			fprintf(pf, "RegInputWidth = %d\n", m_sCtrlParam.RegInputWidth);
			fprintf(pf, "RegMapWidth = %d\n", m_sCtrlParam.RegMapWidth);
			fprintf(pf, "RegMapHeight = %d\n", m_sCtrlParam.RegMapHeight);
			fprintf(pf, "RegBlockWidth = %d\n", m_sCtrlParam.RegBlockWidth);
			fprintf(pf, "RegInitLinebuffHeight = %d\n", m_sCtrlParam.RegInitLinebuffHeight);

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
		int bbb[11];
		int32_t i;
		FILE* pf;
		pf = fopen(filename, "rb");
		if (pf)
		{
			for (i = 0; i < 11; i++)
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

			m_sCtrlParam.RegFullWidth = bbb[1];//l2 bit 
			m_sCtrlParam.RegFullHeight = bbb[2];//l2 bit

			m_sCtrlParam.RegStartH = bbb[3];//l2 bit 
			m_sCtrlParam.RegInputHeight = bbb[4];//l2 bit 

			m_sCtrlParam.RegStartW = bbb[5];//l2 bit 
			m_sCtrlParam.RegInputWidth = bbb[6];//l2 bit 

			m_sCtrlParam.RegMapWidth = bbb[7];//9 bit
			m_sCtrlParam.RegMapHeight = bbb[8];//9 bit

			m_sCtrlParam.RegBlockWidth = bbb[9]; // 4 bit
			m_sCtrlParam.RegInitLinebuffHeight = bbb[10];// 8 bit
		}
		else
		{
			printf("error : Save Stting File Fail\n");
			return false;
		}

		return true;
	}
	bool ProcessImage(uint16_t*Dst, uint16_t* Src, uint8_t* DewarpTable);
	//bool TestSingleFilter(int argc, char** argv, bool bSaveBMP = false, bool bLoadBinary = true);
	
protected:

};



#endif