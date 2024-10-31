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

#ifndef _ALTEK_AWB_LIB_API_
#define _ALTEK_AWB_LIB_API_

#include "alAWBLib_AWB_Common.h"


//. ISO
enum altek_awb_error_code       alAWBLib_Derive_ISO128(short *iso_r_128, short *iso_b_128, struct awb_calibration_data_t *in, struct awb_calibration_data_t *out);

//. HW3A Config
enum altek_awb_error_code		alAWBLib_Dispatch_HW3A( void *HW3a_stats_Data, alAWBLib_runtime_data_t *awb_runtime_data, struct al3awrapper_stats_awb_t *al3aWrapper_stat_awb);

#endif

