/****************************************************************************
*                                                                           *
*       This software and proprietary is confidential and may be used       *
*        only as expressly authorized by a licensing agreement from         *
*                                                                           *
*                             Semiconductor                            *
*                                                                           *
*                   (C) COPYRIGHT 2022  SEMICONDUCTOR                  *
*                            ALL RIGHTS RESERVED                            *
*                                                                           *
*                 The entire notice above must be reproduced                *
*                  on all copies and should not be removed.                 *
*                                                                           *
*****************************************************************************/
/**
@file alAELib_iAE.h
@brief Parameters of iAE Library.
@author Rachel Lin
@version 2022/04/27
*/

#ifndef __IAE_H__
#define __IAE_H__

#include "AE_Lib_core.h"

typedef struct {
    UINT32 lw_pushup;
    UINT32 lw_pushdown;
    UINT32 lw_dark;
    UINT32 lw_avg_hist;

} HIST_MEAN_RESULT;

typedef struct {
    UINT16 ref_target;      /* 8bits target, scale 100 */
    UINT16 cur_mean;        /* 8bits current mean(metering), scale 100 */
    UINT32 *R_hist;         /* 10bits length */
    UINT32 *G_hist;         /* 10bits length */
    UINT32 *B_hist;         /* 10bits length */
    UINT16 hist_length;     /* sync with yhis bit width */
    UINT16 r_gain;          /* AWB gain */
    UINT16 g_gain;          /* AWB gain */
    UINT16 b_gain;          /* AWB gain */
} AEC_INFO;

typedef struct {
	ae_core_iae_param_t *iae_param;
	AEC_INFO aec_param;
    UINT8   ucCameraId;
} iAE_input;

void hist_mean_monitor(ae_core_iae_tuning_param_t *param, UINT32 *hist, UINT16 length, HIST_MEAN_RESULT *result, UINT8 color_idx);
UINT32 cal_cdf_region_mean(UINT32 *cdf, UINT16 length, UINT32 min_cnt, UINT32 max_cnt);
void CCSort_f(UINT32 *a_pwWindow, UINT16 a_uwNum);
void iAE_process(iAE_input *buf, UINT16 *midtones_gain, UINT16 *shadows_gain, UINT16 *short_target, INT32 expindex, void* ae_runtime_dat);
/* Use AE stats data to calculate histogram when hw yhis is invalid */
UINT32 calc_histogram(UINT16 *stats_y, UINT32 *Y_hist, UINT16 total_num);
#endif
