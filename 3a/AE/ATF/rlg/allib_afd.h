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
@file allib_afd.h
@brief Definition of AFD lib. type declaration
@author Han Tseng
@date 2022/01/11
*/

#ifndef _AL_AFD_H_
#define _AL_AFD_H_


#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
* Include files
*******************************************************************************/
#include "allib_mtype.h"
#include "Mylog.h"

/******************************************************************************
*  Macro definitions
******************************************************************************/

/********************************************************************************
*                           Type Declaration                                    *
********************************************************************************/

enum afd_mode {
    AFD_MODE_OFF = 0,
    AFD_MODE_50HZ = 1,
    AFD_MODE_60HZ = 2,
    FS_MODE_90HZ = 3,
    FS_MODE_144HZ = 4,
    FS_MODE_240HZ = 5,
    FS_MODE_320HZ = 6,
    FS_MODE_360HZ = 7,
    AFD_MODE_MAX,
};

enum afd_level {
    AFD_LEVEL_STRONG = 0,
    AFD_LEVEL_MIDDLE,
    AFD_LEVEL_WEAK,
};

enum afd_power_line_frequency_control {
    AFD_POWER_LINE_FREQUENCY_CONTROL_DISABLE = 0,
    AFD_POWER_LINE_FREQUENCY_CONTROL_50HZ,
    AFD_POWER_LINE_FREQUENCY_CONTROL_60HZ,
    AFD_POWER_LINE_FREQUENCY_CONTROL_AUTO,
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct allib_afd_lib_version_t {
    UINT16                                      major_version;
    UINT16                                      minor_version;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

struct afd_setting_t {
    UINT8                                       enable;
    UINT8                                       fs_enable;
    UINT8                                       enable_close_after_detection;
    enum afd_mode                               mode;
};

struct afd_sensor_info_t {
    UINT32                                      line_time_ns;
    UINT32                                      sensor_width;
    UINT32                                      sensor_height;
    UINT32                                      a3engine_width;         // 0 if don't know
    UINT32                                      a3engine_height;        // 0 if don't know
    UINT32                                      sensor_woi_x;
    UINT32                                      sensor_woi_y;
    UINT32                                      sensor_woi_w;
    UINT32                                      sensor_woi_h;
};

struct afd_flicker_sensor_info_t {
    UINT32                                      sample_rate;
    UINT32                                      bin_num;
    float                                       freq_step;
};

struct afd_parameter_detect_t {
    enum afd_level                              afd_level_50hz;
    enum afd_level                              afd_level_60hz;
    enum afd_level                              afd_level_detect_th;
};

struct afd_exposure_info_t {
    UINT32                                      exposure_time;
    UINT32                                      gain;
    UINT32                                      average_value;
};

struct afd_tuning_t {
    //sensor info
    UINT16                                  line_time_ns;
    UINT16                                  sensor_height;
    UINT16                                  sensor_width;
    //Because of sample rate, 50Hz/60Hz flicker pos maybe different.
    UINT16                                  _50Hz_flicker_pos;
    UINT16                                  _60Hz_flicker_pos;
    //default param
    UINT16                                  default_hz;                     // 1 : 50hz; 2 : 60hz
    UINT16                                  level_50;                       // 0 ~ 2, strong to weak
    UINT16                                  level_60;                       // 0 ~ 2, strong to weak
    UINT16                                  level_stable_th;                // 0 ~ 2, strong to weak
    UINT16                                  base;                           // 8192
    UINT16                                  ratio_shift;                    // 10

    //stats down_sampling
    UINT16                                  jump_sample_num;                //suggest 16
    UINT16                                  jump_sample_do_avg_count;       //suggest 8

    // detection
    UINT16                                  enable_2nd_region;              // 1
    UINT16                                  ratio_max_range;                // base 1024
    UINT16                                  points_1st_region;              // dft points on 1st region; up to AFD_MAX_POINTS_REGION
    UINT16                                  points_2nd_region;              // dft points on 2nd region; up to AFD_MAX_POINTS_REGION
    UINT16                                  th_amp;                         // 60
    UINT16                                  th_amp_min;                     // 36
    UINT16                                  ratio_peak_dc;                  // base 1024
    UINT16                                  ratio_peak_2nd;                 // base 1024
    UINT16                                  index_range_50_0;               // base 10, 50
    UINT16                                  index_range_50_1;               // base 10, 35
    UINT16                                  index_range_50_2;               // base 10, 20
    UINT16                                  index_range_60_0;               // base 10, 50
    UINT16                                  index_range_60_1;               // base 10, 35
    UINT16                                  index_range_60_2;               // base 10, 20
    UINT16                                  th_symm_min;                    // 4
    UINT16                                  size_symmetric;                 // 3, max to 4
    UINT16                                  ratio_symmetric[4];             // base 1024, ratio * ratio * 1024
    UINT16                                  th_symm_50hz;                   // 24
    UINT16                                  th_symm_60hz;                   // 24
    UINT16                                  peak_decay_count;               // 2
    UINT16                                  ratio_low_amp;                  // base 1024
};

struct afd_fs_tuning_t {
    //flicker sensor result judgement
    UINT16                                  fs_th_dc_amp;
    UINT16                                  amp_2nd_peak_ratio;
    UINT16                                  freq_box[7];
    UINT16                                  vote_delta_th;
    UINT16                                  fs_th_stable;
    UINT16                                  fs_enable_close_after_change_mode;
};

typedef struct
{
    UINT32    freq;
    UINT32    amp;
} freq_t;

struct afd_fsresult_t
{
    freq_t    freq1;
    freq_t    freq2;
    freq_t    freq3;
};

#define DEFAULT_PARAM_NUM (11)
#define EFFECT_PARAM_NUM  (26)
#define ENABLE_PARAM_NUM  (3)
#define FS_CALC_NUM       (12)

struct parseParam
{
    UINT16  defaultParam[DEFAULT_PARAM_NUM][2];
    UINT16  setEffectParam[EFFECT_PARAM_NUM][2];
    UINT16  enableFlag[ENABLE_PARAM_NUM][2];
    UINT16  setFScalcParam[FS_CALC_NUM][2];
};

enum allib_afd_set_parameter_type {
    ALAFD_SET_PARAM_TUNING_DATA = 0,
    ALAFD_SET_PARAM_STATS_SIZE,
    ALAFD_SET_PARAM_SENSOR_INFO,
    ALAFD_SET_PARAM_FLICKER_SENSOR_INFO,
    ALAFD_SET_PARAM_SETTING,
    ALAFD_SET_PARAM_DETECTION,
    ALAFD_SET_PARAM_EXPOSURE_INFO,
    ALAFD_SET_PARAM_POWER_LINE_FREQUENCY_CONTROL,   //UVC command
    ALAFD_SET_PARAM_ENABLE, // algo enable
    ALAFD_SET_PARAM_TUNING, // detection param tuneable
    ALAFD_SET_PARAM_TUNING_FS, // detection param tuneable
    ALAFD_SET_PARAM_FSRESULT, // FS RESULT UPDATE

    ALAFD_SET_PARAM_MAX,
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct allib_afd_set_parameter_t {
    enum allib_afd_set_parameter_type           type;
    union {
        void                                    *tuning_data;
        UINT32                                  stats_size;
        struct afd_sensor_info_t                sensor_info;
        struct afd_flicker_sensor_info_t        flicker_sensor_info;
        struct afd_setting_t                    afd_setting;
        struct afd_parameter_detect_t           detection;
        struct afd_exposure_info_t              exposure_info;
        enum afd_power_line_frequency_control   power_line_frequency_control;

        struct afd_tuning_t                     tuning_setting;
        struct afd_fs_tuning_t                  fs_tuning_setting;
        struct afd_fsresult_t                   fs_result;
    }   para;
};
#pragma pack(pop)  /* restore old alignment setting from stack */


enum allib_afd_get_parameter_type {
    ALAFD_GET_PARAM_SENSOR_INFO,
    ALAFD_GET_PARAM_SETTING,
    ALAFD_GET_PARAM_DETECTION,
    ALAFD_GET_PARAM_POWER_LINE_FREQUENCY_CONTROL,   //UVC command
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct allib_afd_get_parameter_t {
    enum allib_afd_get_parameter_type           type;
    union {
        struct afd_sensor_info_t                sensor_info;
        struct afd_setting_t                    afd_setting;
        struct afd_parameter_detect_t           detection;
        enum afd_power_line_frequency_control   power_line_frequency_control;
    }   para;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct allib_afd_output_data_t {
    UINT8                                       flag_enable;
    UINT8                                       flag_bending;
    enum afd_mode                               afd_mode;
    enum afd_mode                               afd_mode_stable;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

typedef UINT32(*allib_afd_init_func)(void **afd_initial_buffer);
typedef UINT32(*allib_afd_deinit_func)(void *afd_obj);
typedef UINT32(*allib_afd_set_param_func)(struct allib_afd_set_parameter_t *param, void *afd_dat);
typedef UINT32(*allib_afd_get_param_func)(struct allib_afd_get_parameter_t *param, void *afd_dat);
typedef UINT32(*allib_afd_push_stats_func)(void *g_channel_rows_avg_data, void *afd_dat);
typedef UINT32(*allib_afd_kernel_process_func)(void *afd_dat, struct allib_afd_output_data_t *output);

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct allib_afd_runtime_obj_t {
    UINT32                                      identityid;  /*!< framework tag for current instance */
    void                                        *data;
    allib_afd_init_func                         initial;
    allib_afd_deinit_func                       deinit;
    allib_afd_set_param_func                    set_param;
    allib_afd_get_param_func                    get_param;
    allib_afd_push_stats_func                   push_stats;
    allib_afd_kernel_process_func               kernel_process;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
typedef struct al4ahd {
    struct allib_afd_lib_version_t      afd_ver;
    struct allib_afd_runtime_obj_t      afd_func_obj;
    struct allib_afd_set_parameter_t    afd_set_para;
    struct allib_afd_get_parameter_t    afd_get_para;
    struct allib_afd_output_data_t      afd_output;
    void* afd_obj;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

/********************************************************************************
*                           Public Function Prototype                          *
********************************************************************************/

UINT32      allib_afd_getlib_runtimebuf_size(void);
void        allib_afd_getlib_version(struct allib_afd_lib_version_t *lib_version);


/* Return: TRUE: loading with no error , FALSE: false loading function APIs address. */
UINT32      allib_afd_loadfunc(struct allib_afd_runtime_obj_t *afd_run_obj, UINT32 identityid);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _AL_AFD_H_ */
