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

/******************************************************************************
*  Macro definitions
******************************************************************************/

/********************************************************************************
*                           Type Declaration                                    *
********************************************************************************/

enum afd_mode {
    AFD_MODE_OFF = 0,
    AFD_MODE_50HZ,
    AFD_MODE_60HZ,
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


enum allib_afd_set_parameter_type {
    ALAFD_SET_PARAM_TUNING_DATA = 0,
    ALAFD_SET_PARAM_STATS_SIZE,
    ALAFD_SET_PARAM_SENSOR_INFO,
    ALAFD_SET_PARAM_SETTING,
    ALAFD_SET_PARAM_DETECTION,
    ALAFD_SET_PARAM_EXPOSURE_INFO,
    ALAFD_SET_PARAM_POWER_LINE_FREQUENCY_CONTROL,   //UVC command

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
        struct afd_setting_t                    afd_setting;
        struct afd_parameter_detect_t           detection;
        struct afd_exposure_info_t              exposure_info;
        enum afd_power_line_frequency_control   power_line_frequency_control;
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
    BOOL                                        flag_enable;
    BOOL                                        flag_bending;
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
