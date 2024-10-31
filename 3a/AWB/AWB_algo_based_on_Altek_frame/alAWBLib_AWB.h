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


#ifndef _ALTEK_AWB_LIB_
#define _ALTEK_AWB_LIB_

// #define MAKE_DLL_API extern "C" __declspec(dllexport)

#include "Altek_AWB.h"


/* Initial AWB parameters */
unsigned int  alAWBLib_init(void *obj, int statsSize);

/* 	Initial AWB parameters */
unsigned int  alAWBLib_deinit(void *obj);

/*  Algorithm parameters setting */
unsigned int  alAWBLib_set_param(struct allib_awb_set_parameter_t *param, void *awb_dat);

/*  Algorithm parameters get setting or output */
unsigned int  alAWBLib_get_param(struct allib_awb_get_parameter_t *param, void *awb_dat);

/*  AWB Algorithm Estimation */
unsigned int  alAWBLib_process(void *HW3a_stats_Data, void *awb_dat, struct allib_awb_output_data_t *awb_output_d, int H, int W, void *debugbuf, unsigned short cameraID, txt_params *params_from_txt);




#endif