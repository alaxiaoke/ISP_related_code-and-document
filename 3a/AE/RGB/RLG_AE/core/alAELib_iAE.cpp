/****************************************************************************
*                                                                           *
*       This software and proprietary is confidential and may be used       *
*        only as expressly authorized by a licensing agreement from         *
*                                                                           *
*                            Altek Semiconductor                            *
*                                                                           *
*                   (C) COPYRIGHT 2022 ALTEK SEMICONDUCTOR                  *
*                            ALL RIGHTS RESERVED                            *
*                                                                           *
*                 The entire notice above must be reproduced                *
*                  on all copies and should not be removed.                 *
*                                                                           *
*****************************************************************************/
/**
@file alAELib_iAE.h
@brief Content of iAE Library.
@author Rachel Lin
@version 2022/04/27
*/

#include "./include/AE_Lib_core.h"
#include "./include/alAELib_iAE.h"
#include <memory>


void Calculate_DTC_GAIN(iAE_input* param, UINT16* fmidtones_gain, UINT16* fshadows_gain, UINT16* fshort_target, INT32 expindex, void* ae_runtime_dat);

UINT32 iAE_param_lookup(iAE_input* buf, ae_core_iae_tuning_param_t* iAE_tuning_para, INT16 expindex)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    *iAE_tuning_para = buf->iae_param->iAE_tuning_para[0];
    if (expindex <= buf->iae_param->iaeluxIdxTh[0])
    {
        *iAE_tuning_para = buf->iae_param->iAE_tuning_para[0];
        return errRet;
    }
    if (expindex > buf->iae_param->iaeluxIdxTh[MAX_IAE_BVTH_COUNT - 1])
    {
        *iAE_tuning_para = buf->iae_param->iAE_tuning_para[MAX_IAE_PARA_SWITCH_CASE - 1];
        return errRet;
    }

    for (int i = 1; i < MAX_IAE_BVTH_COUNT; i++)
    {
        if (i % 2 != 0 && expindex <= buf->iae_param->iaeluxIdxTh[i])
        {
            iAE_tuning_para->hist_dark_high = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].hist_dark_high, buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].hist_dark_high, expindex);
            iAE_tuning_para->hist_dark_low = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].hist_dark_low, buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].hist_dark_low, expindex);
            iAE_tuning_para->hist_sat_high = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].hist_sat_high, buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].hist_sat_high, expindex);
            iAE_tuning_para->hist_sat_low = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].hist_sat_low, buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].hist_sat_low, expindex);
            for (int j = 0; j < 3; j++)
            {
                iAE_tuning_para->hist_dark_range_end[j] = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].hist_dark_range_end[j], buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].hist_dark_range_end[j], expindex);
                iAE_tuning_para->hist_dark_range_start[j] = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].hist_dark_range_start[j], buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].hist_dark_range_start[j], expindex);
                iAE_tuning_para->hist_sat_pushdown_range_end[j] = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].hist_sat_pushdown_range_end[j], buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].hist_sat_pushdown_range_end[j], expindex);
                iAE_tuning_para->hist_sat_pushdown_range_start[j] = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].hist_sat_pushdown_range_start[j], buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].hist_sat_pushdown_range_start[j], expindex);
                iAE_tuning_para->hist_sat_pushup_range_end[j] = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].hist_sat_pushup_range_end[j], buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].hist_sat_pushup_range_end[j], expindex);
                iAE_tuning_para->hist_sat_pushup_range_start[j] = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].hist_sat_pushup_range_start[j], buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].hist_sat_pushup_range_start[j], expindex);
            }
            iAE_tuning_para->max_adjust_ratio = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].max_adjust_ratio, buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].max_adjust_ratio, expindex);
            iAE_tuning_para->min_adjust_ratio = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].min_adjust_ratio, buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].min_adjust_ratio, expindex);
            iAE_tuning_para->max_midtones_gain = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].max_midtones_gain, buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].max_midtones_gain, expindex);
            iAE_tuning_para->max_shadows_gain = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].max_shadows_gain, buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].max_shadows_gain, expindex);
            iAE_tuning_para->max_isp_midtones_gain = alAELib_Interpolation(buf->iae_param->iaeluxIdxTh[i - 1], buf->iae_param->iAE_tuning_para[(i - 1) / 2].max_isp_midtones_gain, buf->iae_param->iaeluxIdxTh[i], buf->iae_param->iAE_tuning_para[(i + 1) / 2].max_isp_midtones_gain, expindex);
            return errRet;
        }
        else if(i % 2 == 0 && expindex <= buf->iae_param->iaeluxIdxTh[i])
        {
            *iAE_tuning_para = buf->iae_param->iAE_tuning_para[i / 2];
            return errRet;
        }
    }
    return errRet;
}


UINT32 calc_histogram(UINT16 *stats_y, UINT32 *Y_hist, UINT16 total_num)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    int i = 0;
    //reset Y histogram
    memset(Y_hist, 0, sizeof(UINT32) * AL_MAX_HIST_NUM); /* 8bit histogram */

    for (i = 0; i < total_num; i++)
    {
        if ((stats_y[i] >> 2) <= 0)   /* >>2: 10bit to 8bit */
            Y_hist[0]++;
        else if ((stats_y[i] >> 2) >= 255)
            Y_hist[255]++;
        else
            Y_hist[stats_y[i] >> 2]++;
    }

    return errRet;
}

void iAE_process(iAE_input* buf, UINT16* midtones_gain, UINT16* shadows_gain, UINT16* short_target, INT32 expindex, void* ae_runtime_dat)
{
    //call main function
    ae_corelib_obj_t * ae_obj = (ae_corelib_obj_t*)ae_runtime_dat;
    Calculate_DTC_GAIN(buf, midtones_gain, shadows_gain, short_target, expindex, ae_obj);//output = drc_gain, drc_gain_dark
}

void Calculate_DTC_GAIN(iAE_input* param, UINT16* fmidtones_gain, UINT16* fshadows_gain, UINT16* fshort_target, INT32 expindex, void* ae_runtime_dat)
{
    int cameraId = param->ucCameraId;
    ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ae_runtime_dat;
    //UINT32* R_hist = param->aec_param.R_hist;
    UINT32 *G_hist = param->aec_param.G_hist;
    //UINT32* B_hist = param->aec_param.B_hist;
    UINT16 hist_length = param->aec_param.hist_length;
	ae_core_iae_tuning_param_t   iAE_tuning_para;
    UINT32 sum_q_mid = 0, sum_q_shadow = 0;
    static UINT32 iAE_q_mid[SENSOR_CNT][AE_WDR_SMOOTH_QUEUE_NUM], iAE_q_shadow[SENSOR_CNT][AE_WDR_SMOOTH_QUEUE_NUM];
    static UINT8 iAE_q_id[SENSOR_CNT] = { 0, 0};
    static BOOL iAE_first_flag[SENSOR_CNT] = { TRUE, TRUE};
	UINT8 q_cnt;
    bool isDynamic = FALSE;
    HIST_MEAN_RESULT result_Y;//result_R, result_G, result_B;

	//. iAE tuning param brightness zone lookup table
	iAE_param_lookup(param, &iAE_tuning_para, expindex);
    hist_mean_monitor(&iAE_tuning_para, G_hist, hist_length, &result_Y, 1);  //output 8b result
    ae_obj->ae_runtime_data.max_isp_midtones_gain = iAE_tuning_para.max_isp_midtones_gain;

    UINT32 lw_pushup = (result_Y.lw_pushup * param->aec_param.g_gain);
    UINT32 lw_pushdown = (result_Y.lw_pushdown * param->aec_param.g_gain);
    UINT32 lw_dark = (result_Y.lw_dark * param->aec_param.g_gain);
    UINT32 lw_avg = param->aec_param.cur_mean;  //8bits

    // divide by zero protection
    lw_pushup = lw_pushup <= 100 ? 100 : lw_pushup;
    lw_pushdown = lw_pushdown <= 100 ? 100 : lw_pushdown;
    lw_dark = lw_dark <= 100 ? 100 : lw_dark;

    //four target
    UINT32   Bright_low_target = 100 * (iAE_tuning_para.hist_sat_low * lw_avg) / lw_pushup;
    UINT32   Bright_high_target = 100 * (iAE_tuning_para.hist_sat_high * lw_avg) / lw_pushdown;
    UINT32   Dark_low_target = 100 * (iAE_tuning_para.hist_dark_low * lw_avg) / lw_dark;
    UINT32   Dark_high_target = 100 * (iAE_tuning_para.hist_dark_high * lw_avg) / lw_dark;

    UINT32 target[4] = { Bright_low_target ,Bright_high_target ,Dark_low_target ,Dark_high_target };
    CCSort_f(target, 4);
    UINT32 target_low = target[1];
    UINT32 target_high = target[2];

    LOG_DEBUG("[IAE Process] brightLow %u  brighthigh %u  darklow %u  darkhigh %u  lw_avg %u  pushup %u  pushdown %u  dark %u", Bright_low_target, Bright_high_target, Dark_low_target, Dark_high_target, lw_avg, lw_pushup, lw_pushdown, lw_dark);
    //safe target
    UINT32 safe_target = param->aec_param.ref_target;
    if (target_low > param->aec_param.ref_target)
        safe_target = target_low;
    else if (target_high < param->aec_param.ref_target)
        safe_target = target_high;
    else
        safe_target = param->aec_param.ref_target;

    safe_target = max(min(safe_target, param->aec_param.ref_target * iAE_tuning_para.max_adjust_ratio / 100), 
        param->aec_param.ref_target * iAE_tuning_para.min_adjust_ratio / 100);

    UINT32 short_target = safe_target;
    UINT32 temp = 0;
    UINT32 long_target = safe_target;

    //short target
    if (Bright_low_target <= Bright_high_target)
        short_target = min(Bright_high_target, safe_target);
    else if (Bright_low_target > Bright_high_target)
    {
        //swap
        temp = Bright_low_target;
        Bright_low_target = Bright_high_target;
        Bright_high_target = temp;

        short_target = min(Bright_low_target, safe_target);
    }
    short_target = max(100, short_target);
    UINT32 adrcgain1 = (100 * safe_target + (short_target >> 1)) / short_target;

    UINT32 adrcgain = min(adrcgain1, iAE_tuning_para.max_midtones_gain);
    short_target = 100 * safe_target / max(100, adrcgain);
    //long target
    UINT32 darkboost = 1;
    if (Dark_low_target > safe_target)
    {
        darkboost = min(iAE_tuning_para.max_shadows_gain, (100 * Dark_low_target + (safe_target >> 1)) / safe_target);
        long_target = darkboost * safe_target / 100;
    }

    //final check
    long_target = max(long_target, safe_target);
    short_target = min(short_target, safe_target);

    UINT32 midtones_gain = 100 * safe_target / short_target;
    UINT32 shadows_gain = 100 * long_target / safe_target;

    //printf("pmidtones_gain is %d \n", midtones_gain);
    //printf("short_target is %d \n", short_target);

    // smooth output results
	if (TRUE == iAE_first_flag[cameraId])
	{
		for (q_cnt = 0; q_cnt < AE_WDR_SMOOTH_QUEUE_NUM; q_cnt++)
		{
			iAE_q_mid[cameraId][q_cnt] = 100;
			iAE_q_shadow[cameraId][q_cnt] = 100;
		}
		iAE_first_flag[cameraId] = FALSE;
	}
	iAE_q_mid[cameraId][iAE_q_id[cameraId]] = midtones_gain;
	iAE_q_shadow[cameraId][iAE_q_id[cameraId]] = shadows_gain;
	iAE_q_id[cameraId]++;
	if (iAE_q_id[cameraId] == AE_WDR_SMOOTH_QUEUE_NUM)
		iAE_q_id[cameraId] = 0;

	for (q_cnt = 0; q_cnt < AE_WDR_SMOOTH_QUEUE_NUM; q_cnt++)
	{
		sum_q_mid += iAE_q_mid[cameraId][q_cnt];
		sum_q_shadow += iAE_q_shadow[cameraId][q_cnt];
	}
	sum_q_mid /= AE_WDR_SMOOTH_QUEUE_NUM;
	sum_q_shadow /= AE_WDR_SMOOTH_QUEUE_NUM;

    *fmidtones_gain = sum_q_mid;
    *fshadows_gain = sum_q_shadow;

    *fshort_target = short_target;
    LOG_DEBUG("[IAE Process] short target %u  safe target %u  long target %u ", short_target, safe_target, long_target);

}

void hist_mean_monitor(ae_core_iae_tuning_param_t *param, UINT32 *hist, UINT16 length, HIST_MEAN_RESULT *result, UINT8 color_idx)
{
    static UINT32 cdf[AL_MAX_HIST_NUM];

    //color_idx: 0,1,2 = R,G,B
    UINT32 n_hist = 0;
    UINT64 hist_sum = 0;
	UINT16 i = 0;
    for (i = 0; i < length; i++)
    {
        n_hist += hist[i];
        hist_sum += (UINT64)hist[i] * i;

        if (i == 0)
            cdf[i] = hist[i];
        else
            cdf[i] = hist[i] + cdf[i - 1];
    }

    UINT32 pushup_s = (100 * n_hist - n_hist * param->hist_sat_pushup_range_start[color_idx]) / 100;
    UINT32 pushup_e = (100 * n_hist - n_hist * param->hist_sat_pushup_range_end[color_idx]) / 100;
    UINT32 pushdown_s = (100 * n_hist - n_hist * param->hist_sat_pushdown_range_start[color_idx]) / 100;
    UINT32 pushdown_e = (100 * n_hist - n_hist * param->hist_sat_pushdown_range_end[color_idx]) / 100;
    UINT32 dark_s = (n_hist * param->hist_dark_range_start[color_idx]) / 100;
    UINT32 dark_e = (n_hist * param->hist_dark_range_end[color_idx]) / 100;

    UINT32 lw_pushup = cal_cdf_region_mean(cdf, length, pushup_e, pushup_s);
    UINT32 lw_pushdown = cal_cdf_region_mean(cdf, length, pushdown_e, pushdown_s);
    UINT32 lw_dark = cal_cdf_region_mean(cdf, length, dark_s, dark_e);
    UINT32 lw_avg_hist = (n_hist) ? ((double)hist_sum / n_hist) : 0.0f;

    /* 10bit to 8bit */
    if (1024 == length)
    {
        lw_pushup = lw_pushup >> 2;
        lw_pushdown = lw_pushdown >> 2;
        lw_dark = lw_dark >> 2;
        lw_avg_hist = lw_avg_hist == 0 ? 0 : lw_avg_hist >> 2;
    }

    result->lw_pushup = lw_pushup;
    result->lw_pushdown = lw_pushdown;
    result->lw_dark = lw_dark;
    result->lw_avg_hist = lw_avg_hist;

}


UINT32 cal_cdf_region_mean(UINT32 *cdf, UINT16 length, UINT32 min_cnt, UINT32 max_cnt)
{
    UINT32 s_idx = 0, e_idx = 0, remain_s = 0, remain_e = 0;
    UINT16 i = 0;
    UINT32 mean = 0;
    UINT64 sum = 0, weight = 0;

    for (i = 0; i < length - 1; i++)
    {

        if (min_cnt <= cdf[0])
        {
            s_idx = 0;
            remain_s = cdf[0] - min_cnt;
        }
        else if (min_cnt > cdf[i])
        {
            s_idx = i + 1;
            remain_s = cdf[i + 1] - min_cnt;
        }
    }

    for (i = 0; i < length - 1; i++)
    {
        if (max_cnt <= cdf[0])
        {
            e_idx = 0;
            remain_e = max_cnt - 0;
        }
        if (max_cnt > cdf[i])
        {
            e_idx = i + 1;
            remain_e = max_cnt - cdf[i];
        }
    }

    if (e_idx == s_idx)
        mean = e_idx = s_idx;
    else
    {
        sum = 0;
        weight = 0;
        for (i = s_idx + 1; i < e_idx; i++)
        {
            sum += (UINT64)cdf[i] * i;
            weight += cdf[i];
        }

        sum = sum + (UINT64)remain_s * s_idx + (UINT64)remain_e * e_idx;
        weight = weight + remain_s + remain_e;

        if (weight != 0)
            mean = (UINT32)(sum / weight);
        else
            mean = 0;
    }

    return mean;
}


void CCSort_f(UINT32 *a_pwWindow, UINT16 a_uwNum)
{
    int i, j;
    UINT32 temp;
    for (i = 0; i < a_uwNum; i++)
    {
        temp = a_pwWindow[i];
        for (j = i - 1; j >= 0 && temp < a_pwWindow[j]; j--)
        {
            a_pwWindow[j + 1] = a_pwWindow[j];
        }
        a_pwWindow[j + 1] = temp;
    }
}
