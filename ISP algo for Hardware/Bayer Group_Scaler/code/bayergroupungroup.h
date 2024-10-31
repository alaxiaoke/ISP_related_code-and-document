#pragma once
#ifndef _BAYER_GROUPUNGROUP_H_
#define _BAYER_GROUPUNGROUP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#ifndef int8_t
#define int8_t char
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef int16_t
#define int16_t short
#endif

// #ifndef uint16_t
// #define uint16_t unsigned short 
// #endif

#ifndef uint16_t
#define uint16_t short 
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
#define min(a,b) ((a < b) ? a : b)
#endif

#ifndef max
#define max(a,b) ((a > b) ? a : b)
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
#define DATA_BITS 12
#endif

#ifndef MAX_DATA_VALUE
#define MAX_DATA_VALUE ((1<<DATA_BITS)-1)
#endif 

#ifndef LINE_BUFFER_HEIGHT
#define LINE_BUFFER_HEIGHT 16
#endif

#ifndef LINE_BUFFER_HEIGHT_UN
#define LINE_BUFFER_HEIGHT_UN 8
#endif

#ifndef FILTER_WIDTH
#define FILTER_WIDTH 16         // odd number
#endif

#ifndef FILTER_HEIGHT
#define FILTER_HEIGHT 16         // odd number
#endif

#ifndef LINE_BUFFER_WIDTH
#define LINE_BUFFER_WIDTH  2328
#endif

#ifndef SWINVPRECISION
#define SWINVPRECISION 8
#endif

#ifndef SWPRECISION
#define SWPRECISION 12
#endif

#ifndef WPRECISION
#define WPRECISION 15
#endif

#ifndef OVERLAPPRECISION
#define OVERLAPPRECISION 8
#endif

//log
#ifndef TAP
#define TAP 5                           
#endif

typedef struct {
	int32_t RegEnableGroup;
	int32_t RegEnableUngroup;

	int32_t RegInputWidth;
	int32_t RegInputHeight;
	int32_t RegOutputWidth;
	int32_t RegOutputHeight;

	int32_t Regwt[2];
	int32_t Reght[2];
	int32_t Regwto[2];
	int32_t Reghto[2];
	int32_t Regwst[3];
	int32_t Reghst[3];

	int32_t* Reglutw;                      //12.12
	int32_t Reglutwlenleft;
	int32_t Reglutwlenall;
	int32_t Regwo1padlen;                 //new
	int32_t Reglutwlogstep;               //new
	int32_t* Regluth;                      //12.12
	int32_t Regluthlenleft;
	int32_t Regluthlenall;
	int32_t Regho1padlen;                 //new
	int32_t Regluthlogstep;               //new

	int32_t Regbayerscalerw;
	int32_t Regbayerscalerh;
	// int32_t Regbayerscalerwun;
	// int32_t Regbayerscalerhun;

	// int32_t tap;                        //log(tap)
	int32_t Regtableygroup[(1<<TAP)*2];
	int32_t Regtableyungroup[(1<<TAP)*2];

	int32_t Regoverlapsize;
	int32_t Regoverlaplogstep;                      //log(step)
	int32_t Regoverlaptablen;
	int32_t Regoverlaptable[16];

}_isp_Group_Param_t;

typedef struct {
	int32_t Weight[LINE_BUFFER_HEIGHT];
	int32_t Coor[LINE_BUFFER_HEIGHT];
	int32_t arrayWfloor;
	int32_t distWs;
	int32_t trans;
	int32_t swinv;
	int32_t rown;

	int32_t WeightUn[LINE_BUFFER_HEIGHT_UN];
	int32_t CoorUn[LINE_BUFFER_HEIGHT_UN];
	int32_t rightlut;
	int32_t leftlut;
	
}_isp_Group_internal_t;

class GROUP
{
private:
	_isp_Group_internal_t n_sInternalParam;
	uint16_t* pLineBuff[LINE_BUFFER_HEIGHT];
	uint16_t* pLineBuffUngroup[LINE_BUFFER_HEIGHT_UN];
	
	int32_t HorProcOneLine(uint16_t* Dst, uint16_t* Src);
	int32_t HorProc(uint16_t* Dst_Data, uint16_t* Src_Data);
	int32_t GetCoorPhase();
	int32_t CalWeightLineIdx();
	int32_t GetLinebuff(uint16_t* Src);
	int32_t VerProcOneLine(uint16_t* Dst);
	int32_t VerProc(uint16_t* Dst_Data, uint16_t* Src_Data);


	int32_t GetCoorPhaseUngroup(int32_t* wt,int32_t* wto,int32_t* wst,int32_t widthg,int32_t bayerscalerw,int32_t lutwlenall,
								int32_t lutwlenleft,int32_t ho1padlen,int32_t lutwlogstep,int32_t* lutw);
	int32_t CalWeightLineIdxUngroup(int32_t rown, int32_t maxcoor);
	int32_t GetLinebuffUngroup(uint16_t* Src);
	int32_t VerProcOneLineUngroup(uint16_t* Dst);
	int32_t VerProcUngroup(uint16_t* Dst_Data, uint16_t* Src_Data);
	int32_t HorProcOneLineUngroup(uint16_t* Dst, uint16_t* Src);
	int32_t HorProcUngroup(uint16_t* Dst_Data, uint16_t* Src_Data);
	

public:
	_isp_Group_Param_t n_sCtrlParam;	
	GROUP()
	{
		// n_sInternalParam.pLineBuff[0] = (uint16_t*)malloc(sizeof(uint16_t) * LINE_BUFFER_WIDTH * LINE_BUFFER_HEIGHT);
		// if (!n_sInternalParam.pLineBuff[0])
		// {
		// 	printf("GROUP::pLineBuff malloc fail\n");
		// }
		// else {
		// 	printf("GROUP::pLineBuff malloc suncess\n");
		// }
		// for (int32_t i = 1; i < LINE_BUFFER_HEIGHT; i++) {
		// 	n_sInternalParam.pLineBuff[i] = n_sInternalParam.pLineBuff[i - 1] + LINE_BUFFER_WIDTH;
		// }
	}
	~GROUP()
	{
		// if (n_sInternalParam.pLineBuff[0]){
		// 	free(n_sInternalParam.pLineBuff[0]);
		// 	printf("pLineBuff is delete");
		// }

		//if (pLineBuff[0]){
		//	free(pLineBuff[0]);
		//	printf("Horout is delete");
		//}


	}
	int32_t Group(uint16_t* Dst_Data, uint16_t* Src_Data);
	int32_t Ungroup(uint16_t* Dst_Data, uint16_t* Src_Data);
};

#endif