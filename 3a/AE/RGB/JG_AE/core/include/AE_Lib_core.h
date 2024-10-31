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
*   \file       AE_Lib_core.h
*   \brief      Parameters of AE Library.
*   \author     Rachel Lin
*   \version    1.0
*   \date       2022/04/27
*/
#ifndef _AL_AE_CORE_H_
#define _AL_AE_CORE_H_

#ifndef _AL_MATH_
#include <math.h>
#endif

//#include "alwrapper.h"
#include "allib_mtype.h"
#include "allib_ae_errcode.h"
#include "allib_ae.h"

#ifdef __cplusplus
extern "C"
{
#endif

//#define _AL_MATH_

/****************************************************************************
*                               Include files                               *
*****************************************************************************/


/****************************************************************************
*                           Macro definitions                               *
*****************************************************************************/
#define _ALAE_MAX_MEAN_10BITS_                      (1023)
#define _ALAE_MIN_MEAN_8BITS_                       (1)
#define _ALAE_MIN_BV                                (-9000)     // an experience value

#define AE_METERINGPROCESS_BITDEPTH                 (8)
#define AE_MAX_STATS_BLOCK                          (36)
#define AE_MAX_STATS_BANK                           (24)
#define AE_HWSTATS_INFO_SIZE                        (128)   /* unit: byte */
#define AE_HWSTATS_FLYBY_MODE                       (1)
#define MAX_AE_YWIGHT_TABLE_LENGTH                  (256)   /* Y weighting table nodes, 256 for 8-bit */
#define MAX_IAE_PARA_SWITCH_CASE                    (10)
#define AL_MAX_HIST_NUM                             (256)   /* 256, 8 bits value array */
#define AE_WDR_SMOOTH_QUEUE_NUM                     (8)
#define SENSOR_CNT                                  (2)
#define MAX_IAE_BVTH_COUNT                          (18)
#define MAX_IAE_HISTRATIO_COUNT                     (6)

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
/********************************************************************************
*                           Global Type Declaration                             *
********************************************************************************/
#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(1)    /* new alignment setting */
typedef struct {
    UINT16 sat_th_r;     /*!< 10bit, 1024: disable */
    UINT16 sat_th_gr;    /*!< 10bit, 1024: disable */
    UINT16 sat_th_gb;    /*!< 10bit, 1024: disable */
    UINT16 sat_th_b;     /*!< 10bit, 1024: disable */
} ae_core_hwstats_info_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct rgb_gain_t
@brief r/g/b gain
*/
struct rgb_gain_t {
    UINT16 r;  /*!< r-gain */
    UINT16 g;  /*!< g-gain */
    UINT16 b;  /*!< b-gain */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/* calibration parameter */
typedef struct {
    UINT32 min_iso;
    UINT32 calib_r_gain;  /*!< scale x1000 */
    UINT32 calib_g_gain;  /*!< scale x1000 */
    UINT32 calib_b_gain;  /*!< scale x1000 */
} ae_core_calibration_data_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    UINT8 *metering_table;
    UINT32 udPcurveMaxExpTime;  /*!< unit:us */
    UINT32 udPcurveMinExpTime;  /*!< unit:us */
    UINT32 udPcurveMaxIso;
    UINT32 udAEMean_8bit;   /*!< scale100 */
    UINT16 uwbgAEMean_8bit; /*!< scale100, total average mean */
    INT32 dImageBV;
    INT32 dTargetAllcompBV;
    INT16 wDeltaBV;
	INT16 wiAEDeltaBV;
	UINT16 uwmidtones_gain;
    UINT32 udY_hist[AL_MAX_HIST_NUM];  /*!< 8bit Y hist */
    UINT16 uwiAE_shortTargetMean;   /*!< 8bit, scale100, e.g. Input 3825 means 38.25 */
	UINT8 uciAE_AEconverge_cnt;
    float uExpIndex;
    INT16  preBV;
    bool  isDynamic;
    INT32 convergeDiff;
    UINT32 frameid;
    UINT8 masterID;
    UINT8 slaveID;
    UINT16 max_isp_midtones_gain;
    float beforeWdrExpIndex;
    INT16 minExpBV;
    bool multiCamMasterFlag;
} ae_runtime_data_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    UINT16 uwCountingIdx;   /*!< each processing would be increased by 1 */
    UINT8  ucIsAEEnable;    /*!< 0: disable, 1: enable */
    UINT8  ucIsConverged;   /*!< normal AE convege st, 0:Not converged, 1:Converged */
    enum ae_converge_level_type_t   ae_progressive_level;

} ae_corelib_libstatus_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/* AE core lib inital value */
typedef struct {
    UINT32 exposure_time;   /*!< unit: us */
    INT32  BV;              /*!< scale 1000 */
    UINT16 adgain;          /*!< scale 100 */
    UINT16 iso;
	UINT16 midtones_gain; /*!< scale 100 */
}  ae_core_initial_param_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/*  HW environment setting */
typedef struct {
    UINT32 FN_x1000;    /*!< FN number, scale 1000 */

} ae_core_HWSetting_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    UINT8 average_table0[AL_MAX_AE_STATS_NUM];
    UINT8 user_def[AL_MAX_AE_STATS_NUM];
    UINT8 multiCameraleft[AL_MAX_AE_STATS_NUM];
    UINT8 multiCameraright[AL_MAX_AE_STATS_NUM];
} ae_core_metering_table;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    UINT8 hist_sat_low;                     /* 8bit target of highlight region */
    UINT8 hist_sat_high;                    /* 8bit target of highlight region */
    UINT8 hist_dark_low;                    /* 8bit target of dark region */
    UINT8 hist_dark_high;                   /* 8bit target of dark region */
    UINT8 hist_sat_pushup_range_start[3];   /* R,G,B pixel percentage threshold */
    UINT8 hist_sat_pushup_range_end[3];     /* R,G,B pixel percentage threshold */
    UINT8 hist_sat_pushdown_range_start[3]; /* R,G,B pixel percentage threshold */
    UINT8 hist_sat_pushdown_range_end[3];   /* R,G,B pixel percentage threshold */
    UINT8 hist_dark_range_start[3];         /* R,G,B pixel percentage threshold */
    UINT8 hist_dark_range_end[3];           /* R,G,B pixel percentage threshold */
    UINT16 max_adjust_ratio;                /* scale by 100, 100 means 1.0x */
    UINT16 min_adjust_ratio;                /* scale by 100, 100 means 1.0x */
    UINT16 max_midtones_gain;               /* scale by 100, 100 means 1.0x, limitaion of midtones gain */
    UINT16 max_shadows_gain;                /* scale by 100, 100 means 1.0x, limitaion of shadows gain */
    UINT16 max_isp_midtones_gain;
} ae_core_iae_tuning_param_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

typedef enum WDRMode {
    WDR_2MAXMIDTONESGAIN,
    WDR_USEMIDWITHOUTLTM,
    WDR_WDRTARGETTABLE_DEPENDUSEMID,
    WDR_MODEMAX
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
//typedef struct {
//    UINT8 ucIsEnableiAE;    /* 0: disable, 1: enable */
//    INT16 wlow_bv_th1;      /* scale by 1000 */
//    INT16 wlow_bv_th2;      /* scale by 1000 */
//    INT16 whigh_bv_th1;     /* scale by 1000 */
//    INT16 whigh_bv_th2;     /* scale by 1000 */
//    ae_core_iae_tuning_param_t iAE_tuning_para[MAX_IAE_PARA_SWITCH_CASE];
//} ae_core_iae_param_t;

typedef struct {
    UINT8 ucIsEnableiAE;    /* 0: disable, 1: enable */
    INT16 iaeluxIdxTh[MAX_IAE_BVTH_COUNT];
    INT16 iaeHistRatio[MAX_IAE_HISTRATIO_COUNT];
    enum WDRMode WDRMode; //0:2max midtonegain 1:wdr target depend iae  2:wdr target depend hist
    INT16 wdrTargetGainInTh;
    INT16 wdrTargetGainOutTh;
    ae_core_iae_tuning_param_t iAE_tuning_para[MAX_IAE_PARA_SWITCH_CASE];
} ae_core_iae_param_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    UINT32 udTargetmean;            /*!< scale 100 of 8 bits domain */
    UINT8  ucFlickerPriorityMode;   /*!< Set 1 means higher priority avoiding flicker when manual gain, otherwise set 0. */
	UINT8  ucAntiFlickerMode;		/*!< 0: off, 1: fix 50Hz, 2: fix 60Hz, 3: AUTO(AFD) */
    UINT8  ucStatsBlockNum;         /*!< Default value: 16, valid range [1, 36] */
    UINT8  ucStatsBankNum;          /*!< Default value: 16, valid range [1, 24] */
    float  ucWeightTable_user_weight;  /*!< gauss sigma, Default value: 1.3, valid range [0.5, 3] */
    ae_core_metering_table  weight_table;
    ae_core_iae_param_t     iAE_para;
    ae_core_hwstats_info_t  stats_para;
    UINT8 aucYWeightTable[MAX_AE_YWIGHT_TABLE_LENGTH];
    multiCamStruct multiCamera_para;
} ae_core_tuning_param_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    /* UI command flag */
    UINT8  commandFlg[AE_SET_PARAM_MAX];
    /* main obj status */
    struct ae_set_param_content_t  para;
} ae_set_param_data_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    float fbased_speed;         /*!< 0.00001 */
    float  fmax_step;           /*!< limit step max=0.6, except direct speed 100 */
    UINT16 slow_speed_level;    /*!<  5:  0.00001*5  = 0.0002/4 */
    UINT16 smooth_speed_level;  /*!< 10:  0.00001*10 = 0.0002/2 */
    UINT16 normal_speed_level;  /*!< 20:  0.00001*20 = 0.0002   */
    UINT16 fast_speed_level;    /*!< 60:  0.00001*60 = 0.0002*3 */
    UINT16 direct_speed_level;  /*!<100:  direct */
} ae_converge_speed_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct al3awrapper_stats_ae_t
@brief Identify hw3a AE stats data format
*/
struct al3awrapper_stats_ae_t {
    UINT32  ustructuresize;  /*!< here for confirmation */

    /* Common info */
    UINT16 uframeidx;       /*!< HW3a_frame_idx */

    /* AE info */
    UINT32 uaestatssize;    /*!< size of stats data + info */

    /* AE stats info */
    UINT32 udpixelsperblocks;
    UINT32 udbanksize;
    UINT8  ucvalidblocks;
    UINT8  ucvalidbanks;
    UINT8  ucstatsdepth;    /*!< 8: 8 bits, 10: 10 bits */

    /* framework time/frame idx info */
    UINT32 udsys_sof_idx;

    /* AE stats */
    UINT16 statsy[AL_MAX_AE_STATS_NUM];
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    /* calibration parameter */
    ae_core_calibration_data_t calibration_data;

    /* shadow buffer when calling set_param, only some parameter would be update directly to both shawdow & current parameter */
    ae_set_param_data_t   ae_shadow_set_param;

    /* current setting parameter, should be updated each process calling for most parameter */
    ae_set_param_data_t   ae_current_set_param;

    /* after processing, this data should be copy back to local buffer */
    struct ae_output_data_t  ae_current_output_dat;

    /* initial exposure parameters when power on */
    ae_core_initial_param_t  ae_initial_param;

    /* runtime data in process */
    ae_runtime_data_t  ae_runtime_data;

    /* status in process */
	ae_corelib_libstatus_t  lib_runtime_st;

	/* hardware setting */
    ae_core_HWSetting_t  curHWSetting;

    /* stats data after parsing */
    struct al3awrapper_stats_ae_t ae_dispatched_data;

    /* AE tuning parameters */
    ae_core_tuning_param_t  ae_tuning_para;

    /* parameters of converge speed */
    ae_converge_speed_t ae_speed_param;

    UINT8  ucCameraId;      /*!camera id for identify camera static memeroy*/

} ae_corelib_obj_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

/********************************************************************************
*                           Local Type Declaration                              *
********************************************************************************/
#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    UINT8   ucStatsDepth;        /* 8: 8 bits, 10: 10 bits, ... */
    UINT16 *pudstatsY;
    UINT8   ucWidthBlockNum;
    UINT8   ucHeightBlockNum;
    UINT8  *puwWeightTable;
    UINT8  *pucYWeightTable;   /* for Y weighting */

} ae_metering_core_input;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    UINT32    udYMean_8bit;   /* scale100 */

} ae_metering_core_output;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    UINT32 udexposure_time;
    UINT32 udISO;
} al_ae_exp_param_t;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    UINT32  udETMax;    /*!< max exposure time, unit : us */
    UINT32  udETMin;    /*!< min exposure time, unit : us */
    UINT32  udISOMax;   /*!< Iso Maximum */
    UINT32  udISOMin;   /*!< Iso Minimum */
} ae_expo_limit;
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct {
    INT32 targetBV;
    UINT32 variance;
    INT32  bg_bvresult;
    INT32  bvresult;
    UINT16 midtones_gain;
    UINT16 shadows_gain;
    UINT16 uwbgAEMean_8bit;
    UINT16 udAEMean_8bit;
    UINT16 isp_midtones_gain;
} ae_multiCamera_storeAE;
#pragma pack(pop)  /* restore old alignment setting from stack */


static int storeYstats[2][AE_MAX_STATS_BANK][AE_MAX_STATS_BLOCK];
static bool storeStatsValid[2];
static ae_multiCamera_storeAE storeExp[2]; //camera count 2

/********************************************************************************
*                           Private Function Prototype                          *
********************************************************************************/
UINT32 alAE_estimation(void *ae_runtime_dat);
UINT32 ManageInputParam(void *ae_runtime_dat);
UINT32 updateWeightTable(void *ae_runtime_dat);
INT32 alAELib_Abs(INT32 value);
INT32 alAELib_Log2_65536(INT32 value_256);
INT32 alAELib_Pow2_65536(INT32 value_256);
INT32 alAELib_CalculateBv(UINT32 uwAperture, UINT32 udExposureTime, UINT32 uwIsoSpeed);
INT32 alAELib_CalculateBv_midtones_comp(UINT32 uwAperture, UINT32 udExposureTime, UINT32 uwIsoSpeed, UINT16 midtones_gain100);
UINT32 alAELib_CalculateExposureTime(UINT32 uwAperture, INT32 dBv, UINT32 uwIsoSpeed);
UINT32 alAELib_CalculateIsoSpeed(UINT32 uwAperture, UINT32 udExposureTime, INT32 dBv);
INT32 alAELib_Interpolation(INT32 range_start, INT32 value_start, INT32 range_end, INT32 value_end, INT32 input);
INT32 alAELib_Lux2BV1000(UINT32 lux_value);
UINT32 alAELib_BV2Lux(INT32 bv1000);
UINT32 alAELib_GetExposureParameter(ae_corelib_obj_t *ptAEBuffer, al_ae_exp_param_t *ae_output, INT16 wBv1000);
UINT32 alAELib_ModExposureTimeADGainByAntiflikerMode(UINT32 *a_udExpTime, UINT32 *a_udISOspeed, enum ae_antiflicker_mode_t flickermode, ae_expo_limit tExpLimtParam, UINT32 *udConvergeStatus);
UINT32 LoadDefaultSetting(void *ae_obj);
void alAELib_Core_CheckLimit(UINT32 *udExpTime, UINT32 *udISOSpeed, UINT32 *udConvergeStatus, ae_expo_limit range);
INT32 alAELib_CalculateProgressiveAE(INT32 wNowBV1000, INT32 wTargetBV1000, UINT8 a_ucConverge_SPD, ae_corelib_obj_t *ptAEBuffer);

//UINT32 updateTarget(void* ae_runtime_dat);
//UINT32 matchExpoTable(void* ptAEBuffer, void* ae_output, INT16 wBv1000, int aemode);
//UINT32 matchExpoTableNotATF(void* ptAEBuffer, void* ae_output, INT16 wBv1000);

void alAELib_Core_mixStatsSingle(UINT16 index, UINT16 * pt_stats_y, UINT16 banks, UINT16 blocks, void* ae_runtimedat);
void alAELib_Core_mixStats(UINT16 index, UINT16 * pt_stats_y, UINT16 banks, UINT16 blocks, void* ae_runtimedat);
void alAELib_Core_mixExposure(ae_corelib_obj_t * ptAEBuffer, int* targetBV);
void alAELib_Core_calVariance(UINT16 index, UINT16 * pt_stats_y, void* ae_runtimedat);
void alAELib_Core_estimateSlave(ae_corelib_obj_t * ptAEBuffer);
void alAELib_Core_calMeteringRatio(ae_corelib_obj_t * ptAEBuffer, UINT8 * *metering_table);

/********************************************************************************
*                           Wrapper Function Prototype                          *
********************************************************************************/
/**
@brief This API used for patching HW3A stats from ISP for AE libs, after patching completed, AE ctrl should prepare patched stats to AE libs
@param ae_hw3a_stats[In]: hw3a stats from ISP
@param ae_proc_data[In]: Recruit AE data for process
@param alWrappered_AE_Dat[Out]: patched data, used for AE lib
@param ae_runtimedat[In]: AE lib runtime buffer after calling init, must passing correct addr to into this API
@return: error code
 */
UINT32 al3awrapper_dispatchhw3a_aestats(void *ae_hw3a_stats, struct alhw3a_ae_proc_data_t ae_proc_data, struct al3awrapper_stats_ae_t *alwrappered_ae_dat, void *ae_runtimedat);

/**
@brief This API used for patching HW3A stats from ISP of Y-histogram
@param yhis_hw3a_stats[In]: hw3a stats from ISP
@param ae_runtimedat[In]: AE lib runtime buffer after calling init, must passing correct addr to into this API
@return error code
 */
UINT32 al3awrapper_dispatchhw3a_yhiststats(void *yhis_hw3a_stats, void *ae_runtimedat);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _AL_AE_CORE_H_ */
