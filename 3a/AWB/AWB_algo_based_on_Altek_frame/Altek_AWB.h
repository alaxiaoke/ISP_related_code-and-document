/****************************************************************************
*                                                                           *
*       This software and proprietary is confidential and may be used       *
*        only as expressly authorized by a licensing agreement from         *
*                                                                           *
*                            Altek Semiconductor                            *
*                                                                           *
*                   (C) COPYRIGHT 2023 ALTEK SEMICONDUCTOR                  *
*                            ALL RIGHTS RESERVED                            *
*                                                                           *
*                 The entire notice above must be reproduced                *
*                  on all copies and should not be removed.                 *
*                                                                           *
*****************************************************************************/


#ifndef _ALTEK_AWB_
#define _ALTEK_AWB_

#include "IQ_AWB.h"
#include "AWB.h"
//#include "MyLog.h"


/* GravityXr Zou Zhuoliang add. */
//void AWB_ROI_PROCESS(unsigned short* colorRoi, unsigned int rg, unsigned int bg, unsigned int white_point_rg, unsigned int white_point_bg,
//    unsigned int white_point_rg_sum, unsigned int white_point_bg_sum, unsigned int count);

void rankingTop3(unsigned int* greyCountList, unsigned int* top3Region);
short stableCalculate(AWB_MODULE_PARA* module_para, unsigned short RG, unsigned short BG, unsigned short noShiftRG, unsigned short noShiftBG , unsigned int* greyCountList, unsigned short cameraID);
/* Initial AWB 	default setting. */
enum altek_awb_error_code   AWB_InitialSetting(IQAWB_INPUT_PARA *inputPara, IQAWB_OUTPUT_PARA *outputPara);

/* AWB algorithm setting. */
enum altek_awb_error_code   AWB_SetAlgorithmPara(IQAWB_INPUT_PARA *inputPara);

/* Caculate AWB patch data. */
enum altek_awb_error_code   AWB_ComputePatch(IQAWB_ALGORITHM_PARA *algorithm_Para, IQAWB_ALGORITHM_REPORT *algorithm_Report);

/* AWB algorithm related. */
void  AWB_MultiWin_WeightingResult(AWB_VOTING_REPORT *report);
void  AWB_MultiWin_SetGrayLightSourceArray(IQAWB_INPUT_PARA* inputPara, unsigned short light_num, void* lightsource_data, AWB_LIGHTSOURCE_ARRAY* ptLightSourceArray);
void  AWB_MultiVoting(IQAWB_ALGORITHM_PARA *algorithm_Para, IQAWB_ALGORITHM_REPORT *algorithm_Report, unsigned short cameraID);
void  AWB_SetStylishPara(AWB_STYLISH_PARA *stylish_Para);
void  AWB_Stylish_Shift(unsigned short *RG, unsigned short *BG, short comp_RG, short comp_BG);
void  AWB_Stylish_Gain(short BV, short ct, unsigned short *R_Gain, unsigned short *B_Gain, AWB_STYLISH_PARA *para, AWB_STYLISH_REPORT *data);

/* load AWB algorithm parameter. */
enum altek_awb_error_code   AWB_LoadTuningFileAddr(IQAWB_INPUT_PARA* inputPara);

/* Get awb private size to allocate mem before init. */
unsigned int	   ALTEK_Init_GetMemSize(void);

/* Initial api. */
enum altek_awb_error_code   		ALTEK_AWB_Init(IQAWB_INPUT_PARA *alInit_para, IQAWB_OUTPUT_PARA *outputPara);
enum altek_awb_error_code   	 	IQAWB_Init(IQAWB_INPUT_PARA *alInit_para, IQAWB_OUTPUT_PARA *outputPara);

/* Uninit api. */
enum altek_awb_error_code		    ALTEK_AWB_Deinit(void);
enum altek_awb_error_code		    IQAWB_Deinit(void);

/* Estimation api. */
//void   		ALTEK_AWB_Estimation1(IQAWB_INPUT_PARA *inputPara, IQAWB_OUTPUT_PARA *outputPara, void *debugbuf);
enum altek_awb_error_code   		ALTEK_AWB_Estimation(unsigned short cameraID,IQAWB_INPUT_PARA *inputPara, IQAWB_OUTPUT_PARA *outputPara, void *debugbuf);
enum altek_awb_error_code    		IQAWB_Estimation(unsigned short cameraID,IQAWB_INPUT_PARA *inputPara, IQAWB_OUTPUT_PARA *outputPara, void *debugbuf);

/* AWB STATS related. */
void    AWB_RawRGB_To_HW3A(unsigned short *output_Y, unsigned short *output_RG, unsigned short *output_BG, unsigned int RValue, unsigned int GValue, unsigned int BValue, short iso_r_128, short iso_b_128);
void    altek_AWB_ComputePatch_ISP1(unsigned short cameraID, IQAWB_INPUT_PARA *inputPara);

/* Get memory size. */
unsigned int     IQAWB_Init_GetMemSize();

/* Other API. */
void    AWB_Calculate_rgbGain(unsigned short rg, unsigned short bg, AWB_RGB* rgb);

 
struct Point
{
    short x;
    short y;
};
struct Point ADD(const struct Point a, const struct Point b);
struct Point MINUS(const struct Point a, const struct Point b);
short dot(const struct Point a, const struct Point b);
short cross(const struct Point a, const struct Point b);

#endif
