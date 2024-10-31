#pragma once

#ifndef _DITHER_H_
#define _DITHER_H_

#include <stdint.h>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>

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

#ifndef dither_min
#define dither_min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef dither_max
#define dither_max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef clip
#define clip(x,a,b) (dither_min(dither_max(x,a),b))
#endif

#ifndef DATA_BITS
#define DATA_BITS 10
#endif

#ifndef MAX_DATA_VALUE
#define MAX_DATA_VALUE ((1<<DATA_BITS)-1)
#endif

#ifndef MAX_WIDTH
#define MAX_WIDTH 3840
#endif

typedef signed char schar;
typedef unsigned char uchar;
typedef unsigned short ushort;

static_assert(sizeof(uchar) == 1 && sizeof(short) == 2
	&& sizeof(int) == 4 && sizeof(float) == 4
	&& sizeof(double) == 8, "size required");

typedef struct
{
	int32_t RegEnable;// 1u
	int32_t RegEnableOutput8Bit;// 1u
	int32_t RegEnableDither;// 1u
	int32_t RegEnableOutputFormat420;// 1u
	int32_t RegEnableUVMean;// 1u
	int32_t RegWidth;// 12u
	int32_t RegHeight;// 12u
	int32_t RegInitStateY;// 22u
	int32_t RegInitStateU;// 22u
	int32_t RegInitStateV;// 22u
}_dither_param_t;

typedef struct
{
	uint32_t InputRowIndex;
	uint32_t InputColIndex;
	uint32_t CurrentStateY;
	uint32_t CurrentStateU;
	uint32_t CurrentStateV;
	uint32_t PrevRemainderY;
	uint32_t PrevRemainderU;
	uint32_t PrevRemainderV;
}_dither_internal_t;

class CDither
{
private:
	ushort* pLineBuffY, * pLineBuffUV;
	_dither_internal_t m_sInternalParam;
	void WriteOneLineToBuffer(ushort* Src, uint32_t RowIndex);
	void GetOneLineToOutput(ushort* Dst, uint32_t RowIndex);
	void UpdateState();
	void GetPhaseFromRemainder(uint32_t Phase[4], uint32_t Remainder);
	void ProcOne2x2Y(ushort* pLineBuff, ushort* Src, ushort* Dst, uint32_t RowIndex, uint32_t ColIndex, uint32_t CurrentRemainderY);
	void ProcOne2x2UV(ushort* pLineBuff, ushort* Src, ushort* Dst, uint32_t RowIndex, uint32_t ColIndex, uint32_t CurrentRemainderU, uint32_t CurrentRemainderV);

public:
	_dither_param_t m_sCtrlParam;
	CDither()
	{
		pLineBuffY = (ushort*)malloc(MAX_WIDTH * sizeof(ushort));
		pLineBuffUV = (ushort*)malloc(MAX_WIDTH * sizeof(ushort));
	}

	~CDither()
	{
		if (pLineBuffY)
			free(pLineBuffY);
		if (pLineBuffUV)
			free(pLineBuffUV);
	}

	void ProcessImage(ushort* Src, ushort* Dst);
};

#endif