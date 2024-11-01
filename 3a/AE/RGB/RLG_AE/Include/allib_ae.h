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
@file allib_ae.h
@brief Definition of AE lib. type declaration
@author Rachel Lin
@date 2022/04/27
*/

#ifndef _AL_AE_H_
#define _AL_AE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
* Include files
*******************************************************************************/

#include "allib_mtype.h"
#include "../core/include/MyLog.h"

/******************************************************************************
*  Macro definitions
******************************************************************************/
/**
@def AL_MAX_AE_STATS_NUM
@brief Constant to identify the total blocks of AE stats
*/
#define AL_MAX_AE_STATS_NUM                     (864)  /*!< 36 x 24 total blocks */

/**
@def HW3A_AE_STATS_BUFFER_SIZE
@brief Constant to identify the hw3a AE stats buffer size
*/
#define HW3A_AE_STATS_BUFFER_SIZE               (27*1024 + AE_HWSTATS_INFO_SIZE)

/**
@def AL_MAX_ROI_NUM
@brief The maximum support number of ROI
*/
#define AL_MAX_ROI_NUM                          (10)

#define maxMeteringParamINTCount 9
#define maxIaeParamSwitchCase 10
#define maxIaeParamCount 27
#define maxIaeBVThCount 18
#define maxIaeHistRatioCount 9
#define arbParamIdxStart 98
#define sceneCnt 5
#define aeTargetRow 8
#define aeTargetCol 3
#define aeTable_Cnt 4
#define aeExpoRow 8
#define aeExpoCol 4

/********************************************************************************
*                           Type Declaration                                    *
********************************************************************************/
/* hw3a_stats */
#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct alhw3a_statisticsdldregion_t
@brief Identify the woi information
*/
struct alhw3a_statisticsdldregion_t {
    UINT16 uwborder_ratiox;
    UINT16 uwborder_ratioy;
    UINT16 uwblk_numx;
    UINT16 uwblk_numy;
    UINT16 uwoffset_ratiox;
    UINT16 uwoffset_ratioy;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct alhw3a_ae_cfginfo_t
@brief HW3A configuration for AE
*/
struct alhw3a_ae_cfginfo_t {
    UINT16 token_id;
    struct alhw3a_statisticsdldregion_t ae_region;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

/**
@enum alhw3a_ae_engine_id
@brief Specify the hw3a engine ID, e.g. 3A_a or 3A_b
@arg ALHW3A_A Indicate 3A_a for hw3a engine
@arg ALHW3A_B Indicate 3A_b for hw3a engine
@arg ALHW3A_MAX_INDEX The maximum number of engine ID
*/
enum alhw3a_ae_engine_id {
    ALHW2A_A,
    ALHW3A_B,
    ALHW3A_MAX_ENGINE_ID
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct alhw3a_ae_proc_data_t
@brief Recruit AE data for process
*/
struct alhw3a_ae_proc_data_t {
    UINT32  udsys_sof_idx;  /*!< System SOF index, counting by framework */
    enum    alhw3a_ae_engine_id hw_engine_id;  /*!< hw engine ID, to specify where ae stats comes from */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

/**
@enum ae_metering_mode_type_t
@brief The mode of metering table
@arg AE_METERING_AVERAGE Corresponds to average metering table
@arg AE_METERING_USERDEF_WT Corresponds to user-defined metering table
@arg AE_METERING_MAX_MODE The maximum number of metering mode
*/
enum ae_metering_mode_type_t {
    AE_METERING_AVERAGE,    /*!< If AE_METERING_AVERAGE is set, alAELib will be applied with average metering table.*/
    AE_METERING_USERDEF_WT, /*!< If AE_METERING_USERDEF_WT is set, alAELib will be applied with user-defined metering table which is generated by tuning tool.*/
    AE_METERING_MAX_MODE
};

/**
@enum ae_target_mode_type_t
@brief The mode of target
@arg AE_TARGET_FIX Corresponds to fix target
@arg AE_TARGET_NORMAL Corresponds to normal mode target table
@arg AE_TARGET_USERDEF Corresponds to night mode target table
@arg AE_TARGET_MAX_MODE The maximum number of target mode
*/
enum ae_target_mode_type_t {
    AE_TARGET_FIX,      /*!< If AE_TARGET_FIX is set, alAELib will be applied with fix target which is generated by developer and user modify.*/
    AE_TARGET_NORMAL,   /*!< If AE_TARGET_NORMAL is set, alAELib will be applied with normal scene target table which is generated by developer measure.*/
    AE_TARGET_NIGHT,    /*!< If AE_TARGET_NIGHT is set, alAELib will be applied with night scene target table which is generated by developer measure.*/
    AE_TARGET_USERDEF,  /*!< If AE_METERING_USERDEF is set, alAELib will be applied with user-defined target table which is generated by self define.*/
    AE_TARGET_MAX_MODE
};

/**
@enum ae_expo_mode_type_t
@brief The mode of expo table mode
@arg AE_EXPO_ISO_PRIOR Corresponds to time fix table
@arg AE_EXPO_TIME_PRIOR Corresponds to iso fix table
@arg AE_EXPO_NORMAL Corresponds to normal expo table
@arg AE_EXPO_NIGHT Corresponds to night expo table
@arg AE_EXPO_USERDEF Corresponds to userdefine expo table
@arg AE_EXPO_MAX_MODE The maximum number of expo mode
*/
enum ae_expo_mode_type_t {
    AE_EXPO_ISO_PRIOR,    /*!< If AE_EXPO_ISO_PRIOR is set, alAELib will be applied with ISO modify prior.*/
    AE_EXPO_TIME_PRIOR,    /*!< If AE_EXPO_TIME_PRIOR is set, alAELib will be applied with time modify prior.*/
    AE_EXPO_NORMAL,   /*!< If AE_EXPO_NORMAL is set, alAELib will be applied with normal scene EXPO table which is generated by developer measure.*/
    AE_EXPO_NIGHT,    /*!< If AE_EXPO_NIGHT is set, alAELib will be applied with night scene EXPO table which is generated by developer measure.*/
    AE_EXPO_USERDEF, /*!< If AE_EXPO_USERDEF is set, alAELib will be applied with user-defined EXPO table which is generated by self define.*/
    AE_EXPO_MAX_MODE
};

/**
@enum ae_iso_mode_t
@brief The choices of ISO value
@arg AE_ISO_AUTO ISO value will be calculated automatically
@arg AE_ISO_MAX The maximum number of ISO value
*/
enum ae_iso_mode_t {
    AE_ISO_AUTO = 0,
    AE_ISO_100,
    AE_ISO_200,
    AE_ISO_400,
    AE_ISO_800,
    AE_ISO_1600,
    AE_ISO_3200,
    AE_ISO_6400,
    AE_ISO_12800,
    AE_ISO_MAX
};

/**
@typedef ae_converge_level_type_t
@brief AE converge speed level
*/
enum ae_converge_level_type_t {
    AE_CONVERGE_NORMAL, /* normal speed */
    AE_CONVERGE_DIRECT, /* fastest speed */
    AE_CONVERGE_FAST,   /* fast speed */
    AE_CONVERGE_SMOOTH, /* slower than normal, recommand for video or roi_ae  */
    AE_CONVERGE_SLOW,   /* slower than smooth, recommand for video or roi_ae  */
    AE_CONVERGE_SPEED_LVMAX
};

/**
@enum ae_antiflicker_mode_t
@brief The mode of anti-flicker
@arg ANTIFLICKER_OFF No need to fit in with anti-flicker
@arg ANTIFLICKER_50HZ Current anti-flicker mode is 50Hz
@arg ANTIFLICKER_60HZ Current anti-flicker mode is 60Hz
@arg ANTIFLICKER_MAX The maximum number of anti-flicker mode
*/
enum ae_antiflicker_mode_t {
    ANTIFLICKER_OFF = 0,
    ANTIFLICKER_50HZ,
    ANTIFLICKER_60HZ,
    ANTIFLICKER_MAX,
};


/**
@enum ae_debug_log
@brief Definition of debug log
*/
enum ae_debug_log {
    USERSET_AE_DEBUG_LOG_DISABLE = 0,   /*!< Disable debug log */
    USERSET_AE_DEBUG_LOG_NORMAL,        /*!< Enable debug log for normal flow */
    USERSET_AE_DEBUG_LOG_AE_STATS,      /*!< Enable debug log for AE bayer stats */
    USERSET_AE_DEBUG_LOG_BLOCK_Y_MEAN_8BIT,  /*!< Enable debug log for Block Y mean, e.g. 16*16 Block Y mean */
    USERSET_AE_DEBUG_LOG_MAX_INDEX,
};

/**
@typedef ae_frame_type_t
@brief AE frame type with IR status
*/
enum ae_frame_type_t {
    AE_FRAME_IR_OFF = 0,
    AE_FRAME_IR_ON,
    AE_FRAME_DUMMY
};

/**
@typedef ae_estframe_type_t
@brief AE process on each frame or frame pair
@arg AE_EST_EACH_FRAME: process all frame
@arg AE_EST_FRAME_PAIR: support IR on & IR off pair
*/
enum ae_estframe_type_t {
    AE_EST_EACH_FRAME = 0,
    AE_EST_FRAME_PAIR
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_calibration_data_t
@brief Calibration data
*/
struct ae_calibration_data_t {
    UINT32 min_iso;         /*!< minimum ISO */
    UINT32 calib_r_gain;    /*!< scale 1000 */
    UINT32 calib_g_gain;    /*!< scale 1000 */
    UINT32 calib_b_gain;    /*!< scale 1000 */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_init_exposure_param_t
@brief Exposure parameter of initial value (first time to open camera)
*/
struct ae_init_exposure_param_t {
    UINT32  exp_time;   /*!< unit:us, for initial, exposure time is only for reference, exposure line would be more precise */
    INT32   bv_val;     /*!< scale 1000, bv_val = 1000 means BV = 1 */
    UINT16  ad_gain;    /*!< scale 100, if 1.0x, set 100 */
    UINT16  iso;
	UINT16 midtones_gain; /*!< scale 100, if 1.0x, set 100 */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct rect_roi_t {
    uint32 w;
    uint32 h;
    uint32 left;
    uint32 top;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_sensor_info_t
@brief Sensor information
*/
struct ae_sensor_info_t {
    UINT32 min_fps;  /*!< scale 100, if 16.0 FPS, input 1600 as min FPS, suggest value: 1600 */
    UINT32 max_fps;  /*!< scale 100, if 30.0 FPS, input 3000 as max FPS, suggest value: 3000 */

    UINT32 min_line_cnt;    /*!< sensor minimun exposure line, suggest value 1~ 4 (follow sensor FAE suggestion) */
    UINT32 max_line_cnt;    /*!< sensor maximun exposure line, suggest value 65535 (follow sensor FAE suggestion), which corresponding to min FPS setting
    if min FPS = 16, means max exposure time = 1/16 = 62.5 ms
    if 1 line = 20 us, max line count = 62.5ms / 20us = 3125 lines
    */
    UINT32 exposuretime_per_exp_line_ns;    /*!< used for more precise control.
    if set 0 , would use min FPS vs max line count to calculate automatically,
    note: use ns as units to keep precision, which would have difference for long exposure
    ex: min_fps = 15.99, max line = 3000
    1 line = 1s * 100/1599 / 3000 = 1000000000 (ns) *100 / 1599/3000
    = 20.846 (us)           = 20846 (ns)
    */

    UINT32 sw_min_exptime;   /*!< minimum exposure time for SW limitation, unit:us */
    UINT32 sw_max_exptime;   /*!< maximum exposure time for SW limitation, unit:us */

    UINT32 min_gain;  /*!< scale 100, if 1.0x, set 100, suggest value: 100 (1.0x is most setting for every sensor) */
    UINT32 max_gain;  /*!< scale 100, if 128.0x, set 12800 */

	UINT16 min_midtones_gain;

    UINT32 f_number_x1000;  /*!< Fn number, should refer to lens spec, if aperture = 2.0, here input 2000 */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) //push current alignment setting to stack
#pragma pack(4)    //new alignment setting
/**
@struct ae_ir_param_t
@brief IR information
*/
struct ae_ir_param_t {
    UINT8  ir_level;            /*!< IR level, including clamp level, range:0-255 */
    UINT8  ir_turnon_flag;      /*!< IR turn on flag, FALSE: Turn off, TRUE: Turn on */
};
#pragma pack(pop)  //restore old alignment setting from stack

#pragma pack(push) //push current alignment setting to stack
#pragma pack(4)    //new alignment setting
/**
@struct ae_ir_ctrl_cfg_t
@brief Dynamic IR configuration
*/
struct ae_ir_ctrl_cfg_t {
    UINT8   ir_ctrl_enable;     /*!< FALSE: disable, TRUE: enable */
    UINT8   max_ir_level;       /*!< Maximum IR level, including ir_clamp_level, range:ir_clamp_level < max_ir_level <=255 */
    UINT8   ir_clamp_level;     /*!< IR clamp level, range:0-255 */
    UINT8   allow_level_ctrl;   /*!< FALSE: no level control, only one ir level; TRUE: allow level control */
};
#pragma pack(pop)  //restore old alignment setting from stack

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_sof_notify_param_t
@brief SOF information
*/
struct ae_sof_notify_param_t {
    enum ae_frame_type_t    nIRFrameType;   /*!< 0:frame of IR LED off, 1:frame of IR LED on, 2: frame of dummy */
    struct ae_ir_param_t    current_ir_para;/*!< Current IR ctrl parameters(Effective setting) */
    UINT32 sys_sof_index;   /*!< current system SOF index, counting by framework */
    UINT32 exp_time;        /*!< unit:us, 1.0s should set 1000000 */
    UINT32 exp_adgain;      /*!< scale 100, if 1.0x, set 100 */
	UINT16  midtones_gain; /*!< current midtones_gain , scale 100, if 1.0, set 100 */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

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
} ae_core_iae_tuning_param_t_cp;

/**
@struct ae_sof_notify_param_t
@brief SOF information
*/
#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct ae_core_iae_tuning_param {
    UINT8 ucIsEnableiAE;    /* 0: disable, 1: enable */
    INT16 iaeluxIdxTh[maxIaeBVThCount];
    INT16 iaeHistRatio[maxIaeHistRatioCount];
    ae_core_iae_tuning_param_t_cp iaeTuningCoreParam[maxIaeParamSwitchCase];

};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_awb_info_t
@brief AWB information for AE lib.
*/
struct ae_awb_info_t {
    UINT16  gain_r;  /*!< 256 base */
    UINT16  gain_g;  /*!< 256 base */
    UINT16  gain_b;  /*!< 256 base */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct rect_roi_wt_t {
    struct rect_roi_t roi;
    uint32 weight;      /* Reserved para, NOT USED in this project ; unit: 1000, set 1000 to be default */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct rect_roi_config_t {
    struct rect_roi_wt_t roi[AL_MAX_ROI_NUM];
    uint16  roi_count;          /* total valid ROI region numbers */
    uint16  ref_frame_width;    /* image width of object ROI */
    uint16  ref_frame_height;   /* image height of object ROI */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_target_table_t
@brief target table of AE
*/
struct ae_target_table_t {
    int   aeTargetEn;       /*!< AE metering sim param enable */
    int                             table[aeTargetRow][aeTargetCol];            /*!< target table of cur mode*/
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_expo_table_t
@brief Expo table of AE
*/
struct ae_expo_table_t {
    int aeExpoEn;       /*!< AE expo sim param enable */
    int                             table[aeExpoRow][aeExpoCol];            /*!< expo table of cur mode*/
};
#pragma pack(pop)  /* restore old alignment setting from stack */

enum multiCamModeType {
    AE_MULTICAM_MODE_BYSELF,
    AE_MULTICAM_MODE_ONLYMASTER,
    AE_MULTICAM_MODE_WEIGHTEXP_BYMASTER,
    AE_MULTICAM_MODE_WEIGHTSTATS_SLAVEBYSELF,
    AE_MULTICAM_MODE_WEIGHTSTATS_MASTERTWOINONE,
    AE_MULTICAM_MODE_MAX
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct cameraMixROIParam {
    int leftstart;
    int rightend;
    int topstart;
    int bottom;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct multiCamStruct {
    multiCamModeType multiCamMode;
    int BVStart;
    int BVEnd;
    float slaveWeightStart;
    float slaveWeightEnd;
    int varianceStableTh;
    int ymeanStableTh;
    int stableFrame;
    cameraMixROIParam cameraMixROI[2];
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct configInitInfo {
    int aeEnable;
    int pipedelay;
    int fNumberx1000;
    int maxfps;
    int minfps;
    int maxGain;
    int minGain;
    int maxLineCnt;
    int minLineCnt;
    int swMaxExptime;
    int swMinExptime;
    int exptimePerLineNs;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_set_parameter_init_t
@brief AE initial setting
*/
struct ae_set_parameter_init_t {
    /* tuning parameter setting */
    void   *ae_setting_data;    /*!< the pointer of ae setting file(tuning bin) */

    /* AE Debug info ptr */
    void   *ae_debug_info_ptr;  /*!< the pointer of ae debug info, please input a buffer that will store AE debug information */

    enum ae_antiflicker_mode_t      afd_flicker_mode;       /*!< anti-flicker mode */

    enum ae_metering_mode_type_t    ae_metering_mode;       /*!< AE metering mode */

    /*  basic control param */
    UINT8   ae_enable;            /*!< for initial, should be true */
    UINT16  ae_stats_size;       /*!< at least stats buffer size, unit: byte, ex. 16x12 = 16*12*32 + 40(info) = 6184 (byte) */

    /* sensor info setting, RAW size, fps , etc. */
    struct ae_sensor_info_t         preview_sensor_info;  /*!< Normal preview mode setting */

    /* initial AWB info */
    struct ae_awb_info_t            wb_data;    /*!< gain of rgb transfer to y (use when there's no otp), scale 256, set 256 means 1x gain */

    /* OTP data */
    struct ae_calibration_data_t    ae_calib_wb_gain;   /*!< Please get from Qmerge shading bin*/
    
    /* dynamic ir control cfg */
    struct ae_ir_ctrl_cfg_t         dynamic_ir_cfg;

    UINT32 external_lux_value;    /*!< The ambient lux value for exposure paramters of initial frame */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_set_param_content_t
@brief Parameter content of set-param
*/
struct ae_set_param_content_t {
	void   *ae_setting_data;									/*!< the pointer of ae setting file (tuning bin) */
    /* sensor info setting, fps , etc. */
    struct ae_sensor_info_t         preview_sensor_info;        /*!< Normal preview mode setting */
    /* frame based update command */
    struct ae_sof_notify_param_t    sof_notify_param;           /*!< Current sof notify param, should be updated for each SOF received */
    struct ae_set_parameter_init_t  ae_initial_setting;         /*!< Initial setting, basic setting related */
    struct rect_roi_config_t        ae_set_object_roi_setting;  /*!< should base on HW3A cropped domain */
    
    enum ae_metering_mode_type_t    ae_metering_mode;           /*!< AE metering mode */
    enum ae_target_mode_type_t      ae_target_mode;             /*!< AE target mode */
    enum ae_expo_mode_type_t        ae_expo_mode;               /*!< AE expo mode */
    struct ae_target_table_t        ae_target_table[sceneCnt];            /*!<AE target table of diff scene>*/
    struct ae_expo_table_t          ae_expo_table[aeTable_Cnt];              /*!<AE expo table of diff scene>*/
    enum ae_converge_level_type_t   converge_speedlv;           /*!< AE converge speed */
    enum ae_iso_mode_t              manual_isolevel;            /*!< Manual ISO level */
    enum ae_estframe_type_t         ae_estframe_type;           /*!< AE process on each frame or frame pair (IR on & IR off pair) */
    enum ae_antiflicker_mode_t      afd_flicker_mode;           /*!< anti-flicker mode from AFD output results or UI setting */
    UINT32                          manual_exptime;             /*!< Manual exposure time, unit:us, set 1000000 means 1.0s */
    UINT32                          manual_adgain;              /*!< Manual adgain, scale 100, set 150 means 1.5x gain */
    INT16                           manual_ev_comp;             /*!< Manual EV compensation, range -2EV ~ +2EV, scale1000, e.g. Set 1500 means +1.5EV, Set -2000 means -2EV */

    UINT8                           ae_enable;                  /*!< The switch of AE lib. If set false, skip ae estimation and the output will NOT be updated */
    UINT16                          manual_ae_target;           /*!< Manual AE target, 8bit, scale100, e.g. Input 3825 means 38.25 */
    UINT8                           wdr_enable;                 /*!< The switch of WDR lib. If set false, skip wdr estimation and the output will NOT be updated */
    UINT8                           backlight_enable;           /*!< The switch of backlight compensation, 0: disable, 1: enable */
    UINT8                           dark_mode_enable;           /*!< The switch of dark mode, 0: disable, 1: enable */
    UINT8                           privacy_enable;             /*!< The switch of privacy mode control, 0: disable, 1: enable */
    UINT8                           gaze_enable;                /*!< The switch of gaze AE. If set false, skip gaze AE estimation and the output will NOT be modify */

    struct ae_core_iae_tuning_param     iAE_param;                /*!< iAE tuning param, should be updated while hdr ae effect tuning */
    UINT8                           statsBankNum;
    UINT8                           statsBlockNum;
    UINT8                           cameraId;                 /*!camera id for identify camera static memeroy*/
    UINT8                           pipeDelay;

};
#pragma pack(pop)  /* restore old alignment setting from stack */

/**
@enum ae_set_param_type_t
@brief Set param command ID
*/
enum ae_set_param_type_t {
    AE_SET_PARAM_INVALID_ENTRY, /* invalid entry start */
    AE_SET_PARAM_INIT_SETTING,
    AE_SET_PARAM_ENABLE,        /* enable or disable ae estimation */
    AE_SET_PARAM_SOF_NOTIFY,    /* inform AE lib current SOF changed */
    AE_SET_PARAM_METERING_MODE,
    AE_SET_PARAM_CONVERGE_SPD,  /* control AE converge speed */
    AE_SET_PARAM_OBJECT_ROI,
    AE_SET_PARAM_PROCESS_FRAME_TYPE,    /* control AE process only on IR-on or all frame */
    AE_SET_PARAM_ANTIFLICKER_MODE,
    AE_SET_PARAM_INIT_EXPO,     /* modify init expo setting for simu*/

    /* Manual control command */
    AE_SET_PARAM_MANUAL_ISO_LEVEL,
    AE_SET_PARAM_MANUAL_EXPTIME,
    AE_SET_PARAM_MANUAL_ADGAIN,
    AE_SET_PARAM_MANUAL_EV_COMPENSATION,

    AE_SET_PARAM_MANUAL_AE_TARGET,
    AE_SET_PARAM_WDR_ENABLE,
	AE_SET_PARAM_UPDATE_SETTING_FILE,
    AE_SET_PARAM_BACKLIGHT_ENABLE,
    AE_SET_PARAM_FACTORY_DARK_MODE,
    AE_SET_PARAM_ENABLE_UVC_PRIVACY,

    /*  default parameter tuning  */
    AE_SET_PARAM_iAE_PARAM_BV_TH,   //iae low / high threshold tuning param
    AE_SET_PARAM_iAE_TUNING_PARAM,  // iAE tuning param to modify hdr effect
    AE_SET_PARAM_iAE_PARAM_HIST_RATIO,

    /*  sensor parameter update  */
    AE_SET_PARAM_SENSOR_FN_UPDATE,
    AE_SET_PARAM_SENSOR_GAIN_UPDATE,
    AE_SET_PARAM_SENSOR_FPS_UPDATE,             // FPS Range update, to astrict hw expo time range
    AE_SET_PARAM_SENSOR_HW_EXPO_UPDATE,         // update preline time and max/min line, to astrict hw expo range  
    AE_SET_PARAM_SENSOR_SW_EXPO_UPDATE,         // sw support expo range --> astrict real fps

    /* weight Parameter sync with stats info */
    AE_SET_PARAM_WEIGHT_TABLE_PARAM,

    /* expo table mode and target control */
    AE_SET_PARAM_EXPO_MODE,
    AE_SET_PARAM_TARGET_MODE,
    AE_SET_PARAM_TARGET_TABLE,
    AE_SET_PARAM_EXPO_TABLE,

    /* GAZE AE enable */
    AE_SET_PARAM_GAZE_ENABLE,

    AE_SET_PARAM_MAX
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_set_param_t
@brief Parameter body of set-param
*/
struct ae_set_param_t {
    enum  ae_set_param_type_t  ae_set_param_type;   /*!< Set-param-type specifies that which parameters users want to set */
    struct ae_set_param_content_t    set_param;     /*!< The content of parameters */
};
#pragma pack(pop)  /* restore old alignment setting from stack */



enum simINTType {
    ucAntiFlickerMode,
    manual_isolevel,
    manual_exptime,
    manual_adgain,
    manual_ae_target,
    ae_metering_mode,
    converge_speedlv,
    ae_target_mode,
    ae_expo_mode,
    simEnINTMAX
};

struct targetTableStruct {
    int aeTargetEn;
    int aetargetTable[aeTargetRow][aeTargetCol];
};

struct expousureTableStruct {
    int aeExpoEn;
    int aeExpoTable[aeExpoRow][aeExpoCol];
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct paramVersion {
    float minorVer;
    int   majorVer;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

struct parseParam {
    struct paramVersion paramVer;
    int paramSingleInt[maxMeteringParamINTCount][2];
    int iaeHistEn;
    int paramIAEHist[maxIaeParamCount][maxIaeParamSwitchCase];
    int iaeBVThEn;
    int wdrEnable;
    int paramIAEBVTh[maxIaeBVThCount];
    int paramIAEHistRatio[maxIaeHistRatioCount];
    struct targetTableStruct targetTable[sceneCnt];
    struct expousureTableStruct expousureTable[aeTable_Cnt];
    struct multiCamStruct multiCamParam;
    struct configInitInfo configInfo;
};
/**
@enum ae_get_param_type_t
@brief Get-param command ID
*/
enum ae_get_param_type_t {
    AE_GET_PARAM_INVALID_ENTRY,
    AE_GET_INIT_EXPOSURE_PARAM,
    AE_GET_ALHW3A_CONFIG,
    AE_GET_CURRENT_CALIB_DATA,
    AE_GET_CURRENT_WB,
    AE_GET_DEBUG_INFO,
    AE_GET_PARAM_OBJECT_ROI,
    AE_GET_PARAM_ANTIFLICKER_MODE,
    AE_GET_PARAM_MANUAL_EXPTIME,
    AE_GET_PARAM_MANUAL_ADGAIN,
    AE_GET_PARAM_EXPOSURE_LIMIT_RANGE,
    AE_GET_MANUAL_EXPTIME_GAIN_RESOLUTION,
    AE_GET_MANUAL_EV_COMPENSATION,
    AE_GET_INIT_PIPEDELAY,
    AE_GET_PARAM_MAX
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_deubg_info_t
@brief The size of debug information
 */
struct ae_deubg_info_t {
    UINT32  size;
    void *ptr;  /* Require user to assign the buffer address through "ae_debug_info_ptr" if needed */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct al_ae_exp_limit {
    uint32 max_exptime;     /*!< unit:us, 1000000 means 1.0s */
    uint32 min_exptime;     /*!< unit:us, 1000 means 1.0ms */
    uint16 max_adgain;      /*!< sensor adgain limit, scale 100, 150 means 1.5x */
    uint16 min_adgain;      /*!< sensor adgain limit, scale 100, 150 means 1.5x */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct al_ae_exp_resolution {
    uint16 exptime_gapvalue;      /*!< manual exposure time resolution for UVC command, unit:us */
    uint16 adgain_gapvalue;       /*!< manual adgain resolution for UVC command, scale 100 */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct gxr_ae_exif_info_t {
    //*current expo param
    UINT32  cur_exp_time;   /*!< unit:us, for initial, exposure time is only for reference, exposure line would be more precise */
    UINT16  cur_ad_gain;    /*!< scale 100, if 1.0x, set 100 */
    
    //*process exif
    UINT16  target;
    INT32   bvY_8bit;       /*!< Luma value for current brightness (not update when AE disabled)*/
    INT32   bgYmean_8bit;    /*!< Luma value from average mean (not update when AE disabled), for awb ae_non_comp_bv_val */
    UINT32  lux_result;     /*!< Lux value from average mean (not update when AE disabled)*/
    INT32   next_bv;

    //*out
    UINT32  next_exp_time;  /*!< unit:us, for initial, exposure time is only for reference, exposure line would be more precise */
    UINT16  next_ad_gain;   /*!< scale 100, if 1.0x, set 100 */
    UINT16  midtones_gain;  /*!< scale 100, if 1.0x, set 100 */
    UINT16  shadows_gain;  /*!< scale 100, if 1.0x, set 100 */
    INT16   ae_converged;   /*!< 0:Not converged, 1:Converged */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_get_param_t
@brief Parameter body of get-param
*/
struct ae_get_param_t {
    enum ae_get_param_type_t                ae_get_param_type;      /*!< Get-param-type specifies that which parameters users want to get */
    union {
    struct ae_init_exposure_param_t ae_get_init_expo_param;     /*!< initial exposure setup */
    struct alhw3a_ae_cfginfo_t      alhw3a_aeconfig;            /*!< default hw3a config for reference */
    struct ae_calibration_data_t    calib_data;                 /*!< calibration data from initial setting, scale 1000 */
    struct ae_awb_info_t            wb_data;                    /*!< wb data from initial setting, scale 256 */
    struct ae_deubg_info_t          debug_info;                 /*!< debug info (buffer size) */
    struct rect_roi_config_t        object_roi_setting;         /*!< should base on HW3A cropped domain */
    struct al_ae_exp_limit          ae_exp_limit_range;         /*!< intersection of HW and SW exptime/adgain limitations */
    struct al_ae_exp_resolution     ae_manual_resolution;       /*!< manual reolution for UVC command */
    enum ae_antiflicker_mode_t      afd_flicker_mode;           /*!< anti-flicker mode from AFD output results or UI setting */
    struct gxr_ae_exif_info_t       exif_info;                  /*!<*/
    UINT32                          manual_exptime;             /*!< Get current manual exposure time, unit:us, 1000000 means 1.0s */
    UINT16                          manual_adgain;              /*!< Get current manual adgain, scale 100, 150 means 1.5x gain */
	INT16                           manual_ev_comp;             /*!< Get current manual EV compensation, range -2EV ~ +2EV, scale1000, -1500 means -1.5EV*/
    UINT8                           pipeDelay;
    } para;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct ae_output_data_t
@brief Parameter body of AE output result
*/
struct ae_output_data_t {
    UINT32  iso;                        /*!< ISO for AE estimation result */
    UINT32  udexposure_time;            /*!< Exposure time for AE estimation result */
    UINT32  udsensor_ad_gain;           /*!< Sensor ADGain for AE estimation result, scale100, e.g. 100 means 1.0x adgain */
    INT32   bvresult;                   /*!< BV value for current brightness (not update when AE disabled)*/
    INT32   bg_bvresult;                /*!< BV value from average mean (not update when AE disabled), for awb ae_non_comp_bv_val */
    UINT32  lux_result;                 /*!< Lux value from average mean (not update when AE disabled)*/
    INT32   uwBLcomp;                   /*!< high dynamic scene BV compensation*/
    INT16   ae_converged;               /*!< 0:Not converged, 1:Converged */
    UINT16  ymean100_8bit;              /*!< Average Y (8-bit), scale 100 */
    UINT16  midtones_gain;              /*!< Gain for middle tone control */
    UINT16  shadows_gain;               /*!< Gain for shadows tone control */
    UINT16  ae_debug_info_valid_size;   /*!< The data size of debug info */
    UINT8   next_ir_level;              /*!< Next IR level, including clamp level, range:0-255 */
    UINT8   next_ir_turnon_flag;        /*!< Next IR turn on flag, FALSE: Turn off, TRUE: Turn on */
    UINT8   is_privacy_mode;            /*!< shutter close or not, FALSE: shutter open, TRUE: shutter close */
    UINT8   isface;                     /*!< face detect result, 1 : has a face, 0: no face */
    void    *pae_debug_info_ptr;        /*!< The data pointer of debug info */
    UINT16  isp_midtones_gain;
};
#pragma pack(pop)  /* restore old alignment setting from stack */


/********************************************************************************/
/*                           Public Function Prototype                          */
/********************************************************************************/
/**
@fn unsigned int (* allib_ae_initial)
@brief AE lib. initial API, to load default setting
@param ae_init_buffer [Out], Buffer address of run-time buffer
@return Error code
*/
typedef unsigned int (*allib_ae_initial)(void **ae_init_buffer);

/**
@fn unsigned int (* allib_ae_deinit )
@brief AE lib. De-initial API, to release buffer
@param ae_obj [In], Address of run-time buffer
@return Error code
*/
typedef unsigned int (*allib_ae_deinit)(void *ae_obj);

/**
@fn unsigned int (* allib_ae_set_param )
@brief AE set parameter event for AE lib
@param param [In], structure of ae_set_param_t
@param ae_output [Out], Not support currently, please set to NULL
@param ae_runtime_dat [In], Address of run-time buffer
@return Error code
*/
typedef unsigned int (*allib_ae_set_param)(struct ae_set_param_t *param, struct ae_output_data_t *ae_output, void *ae_runtime_dat);

/**
@fn unsigned int (* allib_ae_get_param )
@brief AE get parameter event for AE lib
@param param [Out], structure of ae_set_param_t
@param ae_runtime_dat [In], Address of run-time buffer
@return Error code
*/
typedef unsigned int (*allib_ae_get_param)(struct ae_get_param_t *param, void *ae_runtime_dat);

/**
@fn unsigned int (* allib_ae_process )
@brief Do AE process
@param hw3a_ae_stats_data  [In], Address of hw3a AE stats
@param hw3a_yhis_stats_data[In], Address of hw3a YHis stats
@param ae_proc_data     [In], Recruit AE data for process
@param ae_runtime_dat   [In], Address of run-time buffer
@param ae_output        [Out], Address of AE lib. output data
@return Error code
*/
typedef unsigned int (*allib_ae_process)(void *hw3a_ae_stats_data, void *hw3a_yhis_stats_data, struct alhw3a_ae_proc_data_t ae_proc_data, void *ae_runtime_dat, struct ae_output_data_t *ae_output);

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct alaeruntimeobj
@brief AE lib function object (APIs address)
*/
struct alaeruntimeobj {
UINT32  identityid;  /*!< framework tag for current instance */
void                 *ae;       /*!< runtime working buffer */
allib_ae_initial     initial;   /*!< Initial API for AE lib. */
allib_ae_deinit      deinit;    /*!< De-initial API for AE lib. */
allib_ae_set_param   set_param; /*!< Set-param API for AE lib. */
allib_ae_get_param   get_param; /*!< Get-param API for AE lib. */
allib_ae_process     process;   /*!< Process API for AE lib. */
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
/**
@struct alaelib_version
@brief AE lib version
@arg major_version Main version of AE lib.
@arg minor_version Minor version of AE lib.
*/
struct alaelib_version {
    UINT16  major_version;
    float   minor_version;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

/**
@brief Get AE lib API address
@param aec_run_obj [Out], object of AE lib APIs address
@param identityID [In], framework tag for current instance
@return FALSE: loading with no error , TRUE: false loading function APIs address
*/
unsigned char allib_ae_loadfunc(struct alaeruntimeobj *aec_run_obj, UINT32 identityID);

/**
@brief Get AE lib version number
@param AE_LibVersion [Out], AE lib version number
*/
void allib_ae_getlib_version(struct alaelib_version *AE_LibVersion);

/**
@brief Control debug log flag
@param ae_debug_log_flag [In], The flag of debug log, see enum ae_debug_log
@return Error code
*/
//unsigned int allib_ae_set_debug_log_flag(enum ae_debug_log ae_debug_log_flag);
unsigned int allib_ae_set_debug_log_flag(enum LogLevel_em ae_debug_log_flag, BOOL file_tag);

/**
@brief Get AE lib working buffer size
@param None
@return Working buffer size (unit:byte)
*/
unsigned short allib_ae_getlib_runtimebuf_size(void);

typedef struct al4ahd {

    struct alaelib_version              ae_ver;
    struct alaeruntimeobj               ae_func_obj;
    struct ae_set_param_t               ae_set_para;
    struct ae_get_param_t               ae_get_para;
    struct ae_output_data_t             ae_output;
    struct alhw3a_ae_proc_data_t        ae_proc_data;
    void* ae_obj;

}al4ahd;

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _AL_AE_H_ */