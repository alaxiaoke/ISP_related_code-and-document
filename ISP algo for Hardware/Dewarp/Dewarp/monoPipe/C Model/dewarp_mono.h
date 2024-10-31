#pragma once

#ifndef _DEWARP_MONO_H_
#define _DEWARP_MONO_H_

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
#define LINE_BUFFER_HEIGHT 129
#endif

#ifndef LINE_BUFFER_WIDTH
#define LINE_BUFFER_WIDTH 1280
#endif

#ifndef TABLE_BUFFER_WIDTH
#define TABLE_BUFFER_WIDTH 502
#endif
#ifndef DEBUGMODE
#define DEBUGMODE 0
#endif
typedef struct {
	int32_t RegEnable;//l bit

	int32_t RegFullWidth;//l2 bit 
	int32_t RegFullHeight;//l2 bit

	int32_t RegStartH;//l2 bit 
	int32_t RegInputHeight;//l2 bit 

	int32_t RegStartW;//l2 bit 
	int32_t RegInputWidth;//l2 bit 

	int32_t RegMapWidth;//l2 bit
	int32_t RegMapHeight;//l2 bit

	int32_t RegBlockWidth; // 12 bit
	int32_t RegInitLinebuffHeight;// 12 bit

}_isp_Dewarp_mono_Param_t;


typedef struct {

	uint16_t* pLineBuff[LINE_BUFFER_HEIGHT];
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

	int32_t pYH[2];
	int32_t pYW[2];
	int32_t pYGain[2];//0-gainx,1-gainy
	
	int32_t pPixelY[2][2];
	
}_dewarp_mono_internal_t;

typedef struct {

	int32_t pYHD[2][2];
	int32_t pYWD[2][2];
	int32_t pYHRangeMax;
	int32_t pYWRangeMax;
	int32_t* pInputNum;
	int32_t* pRollingNum;
}_Dewarp_Mono_Debug_t;


class CDewarp_mono
{
private:
	_dewarp_mono_internal_t m_sInternalParam;
	_Dewarp_Mono_Debug_t DebugInformation;
	uint16_t* pLineBuff1[LINE_BUFFER_HEIGHT];
	
	int32_t StartFrame(uint16_t* Src, uint8_t* DewarpTable);
	int32_t StartLine(int32_t RowIndex);
	int32_t EndLine();
	int32_t UpdateCoor(int32_t ColumnIndex);
	int32_t GetFinalCoor(int32_t ColumnIndex);
	int32_t GetRowColIndexGain();
	int32_t ProcessOnePixelY();
	int32_t GetInputROllingNum(int32_t RowIndex);
	int32_t GetDataToLineBuffer(uint16_t* Src);
	int32_t RollingDataOutLineBuffer();
	int32_t GetCoorToTableBuffer(uint8_t* DewarpTable);
	int32_t RollingDataOutTableBuffer();
	int32_t ProcOneLine(uint16_t* DstY, int32_t RowIndex);
public:
	_isp_Dewarp_mono_Param_t m_sCtrlParam;
	CDewarp_mono()
	{
		memset(&DebugInformation, 0, sizeof(_Dewarp_Mono_Debug_t));
		DebugInformation.pYHRangeMax = 0;
		DebugInformation.pYWRangeMax = 0;
		DebugInformation.pInputNum = new int32_t[8000];
		DebugInformation.pRollingNum = new int32_t[8000];


		memset(&m_sInternalParam, 0, sizeof(_dewarp_mono_internal_t));
		m_sInternalParam.pLineBuff[0] = new uint16_t[LINE_BUFFER_WIDTH * LINE_BUFFER_HEIGHT];
		m_sInternalParam.pTableLineBuff[0] = new uint8_t[TABLE_BUFFER_WIDTH * 3];
		int32_t i;
		if (m_sInternalParam.pLineBuff[0] && m_sInternalParam.pTableLineBuff[0])
		{
			for (i = 1; i < LINE_BUFFER_HEIGHT; i++)
			{
				m_sInternalParam.pLineBuff[i] = m_sInternalParam.pLineBuff[i - 1] + LINE_BUFFER_WIDTH;
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
	~CDewarp_mono()
	{
		if (m_sInternalParam.pLineBuff[0])
			delete m_sInternalParam.pLineBuff[0];
		if (m_sInternalParam.pTableLineBuff[0])
			delete m_sInternalParam.pTableLineBuff[0];

		if (DebugInformation.pInputNum)
			delete DebugInformation.pInputNum;
		if (DebugInformation.pRollingNum)
			delete DebugInformation.pRollingNum;
	}

	bool ProcessImage(uint16_t*Dst, uint16_t* Src, uint8_t* DewarpTable);
	//bool TestSingleFilter(int argc, char** argv, bool bSaveBMP = false, bool bLoadBinary = true);
	
protected:

};



#endif