#pragma once

#ifndef _GAMMA_H_
#define _GAMMA_H_

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

#ifndef gamma_min
#define gamma_min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef gamma_max
#define gamma_max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef clip
#define clip(x,a,b) (gamma_min(gamma_max(x,a),b))
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
	int32_t RegWidth;// 12u
	int32_t RegHeight;// 12u
	int32_t RegLut[64];// u8
	int32_t RegLutFinal; // u9

	int32_t RegBoundary[5];// u12
	int32_t RegIndex[6];// u6
	int32_t RegStepBit[6];// u4
}_gamma_param_t;

class Gamma
{
public:
	_gamma_param_t m_sCtrlParam;

	Gamma()
	{
	}

	~Gamma()
	{
	}

	void ProcessImage(ushort* Src, ushort* Dst);
};

#endif