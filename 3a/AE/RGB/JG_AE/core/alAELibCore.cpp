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
@file alAELibCore.c
@brief AE library Core
@author Rachel Lin
@version 2022/07/28
*/

#include <fstream>
#include <string>
#include <windows.h>
#include <memory>

#ifdef _AL_AE_C_MODEL_
#include "stdafx.h"
#include <stdarg.h>
#include <stdio.h>
#endif

#include "./include/AE_Lib_core.h"
#include "./include/alAELib_defaultParam.h"
#include "./include/alAEVersion.h"
#include "./include/table.h"
#include "./include/alAELib_iAE.h"

/********************************************************************************
*                           Macro definitions                                   *
********************************************************************************/
#define _MATH_SCALE256_                 (256)

/********************************************************************************
*                           Private Function Prototype                          *
********************************************************************************/
UINT32 alAE_estimation(void *ae_runtime_dat);
UINT32 gazeAE(void *ae_runtime_dat);
UINT32 alAELib_generate_metering_table(ae_corelib_obj_t *ae_obj, UINT8 *metering_table);
UINT32 alAELib_Core_AEMetering(void *input, void *output);
UINT32 ParsingLibScenarioStatus(ae_set_param_data_t *set_param, ae_corelib_libstatus_t *output_st, void *ae_runtime_dat);
void   alAE_Simple_AE_Metering(ae_corelib_obj_t *ae_obj, struct ae_output_data_t *ae_output, struct al3awrapper_stats_ae_t *ae_stats_data);
UINT32 TranslateISOLevel(enum ae_iso_mode_t isolevel, UINT32 *udISOSpeed);

/********************************************************************************
*                           Public Global Variable                              *
********************************************************************************/
const int CAMERANUM = 8;
static ae_corelib_obj_t ae_corelib_array_multi[CAMERANUM];

ae_corelib_obj_t* ae_corelib_array = NULL;
static ae_corelib_obj_t gaze_ae_runtime_dat;

int convBVCnt;
INT32 prevYmeanBV;
bool preWdrFlag;

static int estimateSlaveCount = 6;
static UINT8 lastMasterID = 0;
static UINT8 curALLMasterID = 0;

static queue<ae_sensorExpoList> expousureList[2];

/********************************************************************************
*                           Public Function Prototype                          *
********************************************************************************/

unsigned int alAE_initial(void **ae_init_buffer);
unsigned int alAE_deinit(void *ae_obj);
unsigned int alAE_set_param(struct ae_set_param_t *param, struct ae_output_data_t *ae_output, void *ae_runtime_dat);
unsigned int alAE_get_param(struct ae_get_param_t *param, void *ae_runtime_dat );
unsigned int alAE_process(void *hw3a_ae_stats_data, void *hw3a_yhis_stats_data, struct alhw3a_ae_proc_data_t ae_proc_data, void *ae_runtime_dat, struct ae_output_data_t *ae_output);

/********************************************************************************
*                           Public Function                                     *
********************************************************************************/

unsigned int allib_ae_set_debug_log_flag(int ae_debug_log_flag, BOOL file_tag, BOOL terminal_tag)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    if (ae_debug_log_flag < LogLevel_OFF || ae_debug_log_flag >= LogLevel_end)
    {
        LOG_ERROR("please input right log level");
        errRet = _AL_AELIB_INVALID_DEBUGLEVEL;
        return errRet;
    }
    //w_emCurLogLevel = ae_debug_log_flag;
    //g_log_to_file = file_tag;
    set_log_level(ae_debug_log_flag);
    //log_shutdown_terminal();

    if (file_tag)
    {
        log_to_file();
    }
    else
    {
        log_shutdown_file();
    }

    if (terminal_tag)
    {
        log_terminal();
    }
    else
    {
        log_shutdown_terminal();
    }
    LOG_DEBUG("debug level/to_file/to_terminal: %d/%d/%d", w_emCurLogLevel, g_log_to_file, g_log_terminal);
    return errRet;
}

void allib_ae_getlib_version(struct alaelib_version *ae_libversion)
{
    ae_libversion->major_version = AE_LIB_MAJOR_VER;
    ae_libversion->minor_version = AE_LIB_MINOR_VER;
}

unsigned short allib_ae_getlib_runtimebuf_size(void)
{
    unsigned short uwRunTimeBufSize = 0;
    uwRunTimeBufSize = sizeof(ae_corelib_obj_t);

    return uwRunTimeBufSize;
}

/**
@brief Transfer lux value to BV
@param lux_value [In], valid range: 1 ~ 20971520
@return bv_result (scale 1000)
*/
int allib_ae_lux2bv(unsigned int lux_value);
int allib_ae_lux2bv(unsigned int lux_value)
{
    return alAELib_Lux2BV1000(lux_value);
}

/**
@brief Transfer BV to lux value
@param  bv1000 [In], BV scale up 1000
@return lux_result, valid range: 0 ~ 81919
*/
unsigned int allib_ae_bv2lux(int bv1000);
unsigned int allib_ae_bv2lux(int bv1000)
{
    return alAELib_BV2Lux(bv1000);
}

unsigned char allib_ae_loadfunc(struct alaeruntimeobj *aec_run_obj, UINT32 identityid)
{
    unsigned char result = FALSE;

    if ( aec_run_obj == NULL )
    {
        result = TRUE;
        return result;
    }

    // hook related API
    aec_run_obj->initial = alAE_initial;
    aec_run_obj->deinit  = alAE_deinit;
    aec_run_obj->set_param = alAE_set_param;
    aec_run_obj->get_param = alAE_get_param;
    aec_run_obj->process = alAE_process;

    // loading identidyID
    aec_run_obj->identityid  = identityid;
    ae_corelib_array = &ae_corelib_array_multi[identityid];

    LOG_INFO("camera ID: %d", identityid);
    return result;
}

unsigned int alAE_initial(void **ae_init_buffer)
{
    UINT32 ret = _AL_3ALIB_SUCCESS;
    ae_corelib_obj_t *aec_obj = ae_corelib_array;

    if (aec_obj == NULL)
    {
        ret = _AL_AELIB_INIT_BUFFER_FAIL;
        return ret;
    }
    else
        *ae_init_buffer = aec_obj;

    //create local buffer
    memset(aec_obj, 0, allib_ae_getlib_runtimebuf_size());
    //loading parameters
    ret = LoadDefaultSetting( aec_obj );
    if (ret != _AL_3ALIB_SUCCESS)
    {
        return ret;
    }

    return ret;
}

unsigned int alAE_deinit(void *ae_obj)
{
    UINT32 ret = _AL_3ALIB_SUCCESS;

    // release related resources, such as aec_obj;
    if ( ae_obj == NULL )
        return _AL_AELIB_INVALID_ADDR;

    // Reset AE buffer
    memset(ae_obj, 0, allib_ae_getlib_runtimebuf_size());

    return ret;
}

/* AE set parameter event for AE lib */
unsigned int alAE_set_param(struct ae_set_param_t *param, struct ae_output_data_t *ae_output, void *ae_runtime_dat)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    ae_corelib_obj_t  *ae_obj;

    if ( param == NULL ||  ae_runtime_dat == NULL  )
    {
        errRet = _AL_AELIB_INVALID_ADDR;
        return errRet;
    }

    // Check parameter validity, if not valid, return error
    if ( param->ae_set_param_type >= AE_SET_PARAM_MAX || param->ae_set_param_type < AE_SET_PARAM_INVALID_ENTRY)
    {
        errRet = _AL_AELIB_INVALID_PARAM;
        return errRet;
    }

    ae_obj = (ae_corelib_obj_t  *)ae_runtime_dat;    // casting to runtime buffer
    ae_obj->ae_shadow_set_param.commandFlg[param->ae_set_param_type] = 1;
    switch(param->ae_set_param_type)
    {
        case AE_SET_PARAM_INIT_SETTING:
            ae_obj->ae_shadow_set_param.commandFlg[param->ae_set_param_type] = 0;   // need reset after processed

            // update to OTP WB data if data is valid
            if ( param->set_param.ae_initial_setting.ae_calib_wb_gain.calib_b_gain != 0 && param->set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain != 0 && param->set_param.ae_initial_setting.ae_calib_wb_gain.calib_r_gain != 0 )
            {
                if((FLOAT32)100000.0/param->set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain > param->set_param.ae_initial_setting.ae_calib_wb_gain.min_iso)
                    ae_obj->calibration_data.min_iso = (UINT32)((FLOAT32)100000.0/param->set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain + 0.5);
                else
                    ae_obj->calibration_data.min_iso = param->set_param.ae_initial_setting.ae_calib_wb_gain.min_iso;

                ae_obj->calibration_data.calib_b_gain =  param->set_param.ae_initial_setting.ae_calib_wb_gain.calib_b_gain;
                ae_obj->calibration_data.calib_g_gain =  param->set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain;
                ae_obj->calibration_data.calib_r_gain =  param->set_param.ae_initial_setting.ae_calib_wb_gain.calib_r_gain;
            }
            else if (ae_obj->calibration_data.calib_b_gain != 0 && ae_obj->calibration_data.calib_g_gain != 0 && ae_obj->calibration_data.calib_r_gain != 0)
            {

            }
            else
            {
                errRet =  _AL_AELIB_INVALID_PARAM;
            }
                

            // assign ae enable
            ae_obj->ae_shadow_set_param.para.ae_enable = param->set_param.ae_initial_setting.ae_enable;
            ae_obj->ae_current_set_param.para.ae_enable = ae_obj->ae_shadow_set_param.para.ae_enable;

            //camerId
            ae_obj->ucCameraId = param->set_param.cameraId;
            // update flicker mode
            ae_obj->ae_shadow_set_param.para.ae_initial_setting.afd_flicker_mode = param->set_param.ae_initial_setting.afd_flicker_mode;
            break;

        case AE_SET_PARAM_INIT_EXPO:
            ae_obj->ae_initial_param.exposure_time = param->set_param.sof_notify_param.exp_time;
            ae_obj->ae_initial_param.adgain = param->set_param.sof_notify_param.exp_adgain;
            ae_obj->ae_initial_param.midtones_gain = param->set_param.sof_notify_param.midtones_gain;
            ae_obj->ae_initial_param.BV = alAELib_CalculateBv_midtones_comp(ae_obj->curHWSetting.FN_x1000, ae_obj->ae_initial_param.exposure_time, ae_obj->ae_initial_param.adgain, ae_obj->ae_initial_param.midtones_gain);
            LOG_DEBUG("INITEXP gain %u, shutter %u, midtonegain %u, bv %d", ae_obj->ae_initial_param.adgain, ae_obj->ae_initial_param.exposure_time, ae_obj->ae_initial_param.midtones_gain, ae_obj->ae_initial_param.BV);
            // copy initial setting to preview report
            ae_obj->ae_current_output_dat.udexposure_time = ae_obj->ae_initial_param.exposure_time;
            ae_obj->ae_current_output_dat.udsensor_ad_gain = ae_obj->ae_initial_param.adgain;
            ae_obj->ae_current_output_dat.iso = ((UINT64)ae_obj->ae_initial_param.adgain) * 1000 / (ae_obj->calibration_data.calib_g_gain);
            ae_obj->ae_current_output_dat.bvresult = ae_obj->ae_initial_param.BV;
            ae_obj->ae_runtime_data.preBV = ae_obj->ae_initial_param.BV;
            ae_obj->ae_runtime_data.uExpIndex = log10(1.0f / pow(2, (ae_obj->ae_runtime_data.preBV - ae_obj->ae_runtime_data.minExpBV) / 1000.0f)) / log10(1.03);
            prevYmeanBV = ae_obj->ae_initial_param.BV;
            break;

        case AE_SET_PARAM_PIPEDELAY:
            if (param->set_param.pipeDelay < 1)
            {
                errRet = _AL_AELIB_INVALID_PIPEDELAY;
            }
            else
            {
                ae_obj->ae_shadow_set_param.para.pipeDelay = param->set_param.pipeDelay;
            }         
            break;
        case AE_SET_PARAM_SOF_NOTIFY:
            if (param->set_param.sof_notify_param.exp_time < ae_obj->ae_current_set_param.para.preview_sensor_info.sw_min_exptime || param->set_param.sof_notify_param.exp_time > ae_obj->ae_current_set_param.para.preview_sensor_info.sw_max_exptime)
            {
                errRet = _AL_AELIB_INVALID_EXP_TIME;
            }
                
            else if(param->set_param.sof_notify_param.exp_adgain < ae_obj->ae_current_set_param.para.preview_sensor_info.min_gain || param->set_param.sof_notify_param.exp_adgain > ae_obj->ae_current_set_param.para.preview_sensor_info.max_gain)
                errRet = _AL_AELIB_INVALID_GAIN;
			else if(param->set_param.sof_notify_param.midtones_gain < ae_obj->ae_current_set_param.para.preview_sensor_info.min_midtones_gain)
                errRet = _AL_AELIB_INVALID_MIDTONES;
            else
                memcpy(&ae_obj->ae_shadow_set_param.para.sof_notify_param, &param->set_param.sof_notify_param, sizeof( struct ae_sof_notify_param_t));
            break;
        case AE_SET_PARAM_MANUAL_ISO_LEVEL:
            if (param->set_param.manual_isolevel >= AE_ISO_MAX)
            {
                errRet = _AL_AELIB_INVALID_ISOLEVEL;
            }
            ae_obj->ae_shadow_set_param.para.manual_isolevel = param->set_param.manual_isolevel;
            break;
        case AE_SET_PARAM_MANUAL_EXPTIME:
            if(param->set_param.manual_exptime < ae_obj->ae_current_set_param.para.preview_sensor_info.sw_min_exptime && param->set_param.manual_exptime != 0)
                errRet = _AL_AELIB_INVALID_EXP_TIME;
            else
                ae_obj->ae_shadow_set_param.para.manual_exptime = param->set_param.manual_exptime;
            break;
        case AE_SET_PARAM_MANUAL_ADGAIN:
            if(param->set_param.manual_adgain < ae_obj->ae_current_set_param.para.preview_sensor_info.min_gain && param->set_param.manual_adgain != 0)
                errRet = _AL_AELIB_INVALID_GAIN;
            else
                ae_obj->ae_shadow_set_param.para.manual_adgain = param->set_param.manual_adgain;
            break;
        case AE_SET_PARAM_ENABLE:
            ae_obj->ae_shadow_set_param.para.ae_enable = param->set_param.ae_enable;
            break;
        case AE_SET_PARAM_METERING_MODE:
            if (param->set_param.ae_metering_mode >= AE_METERING_MAX_MODE)
            {
                errRet = _AL_AELIB_INVALID_METERINGMODE;
            }
            else
            {
                ae_obj->ae_shadow_set_param.para.ae_metering_mode = param->set_param.ae_metering_mode;
            }
            break;
        case AE_SET_PARAM_ANTIFLICKER_MODE:
            if(param->set_param.afd_flicker_mode >= ANTIFLICKER_MAX)
            {
                errRet = _AL_AELIB_INVALID_FLICKER_MODE;
            }
            else
                ae_obj->ae_shadow_set_param.para.afd_flicker_mode = param->set_param.afd_flicker_mode;
            LOG_DEBUG("ATFmode %d", param->set_param.afd_flicker_mode);
            break;
        case AE_SET_PARAM_MANUAL_EV_COMPENSATION:
            if((param->set_param.manual_ev_comp < DEFAULT_Min_MANUAL_EV_COMP) || (param->set_param.manual_ev_comp > DEFAULT_MAX_MANUAL_EV_COMP))
                errRet = _AL_AELIB_INVALID_EV_COMPENSATION;
            else
                ae_obj->ae_shadow_set_param.para.manual_ev_comp = param->set_param.manual_ev_comp;
            break;
        case AE_SET_PARAM_CONVERGE_SPD:
            if(param->set_param.converge_speedlv >= AE_CONVERGE_SPEED_LVMAX)
                errRet = _AL_AELIB_INVALID_CONVERGE_SPEED_LEVEL;
            else
                ae_obj->ae_shadow_set_param.para.converge_speedlv = param->set_param.converge_speedlv;
            break;
        case AE_SET_PARAM_OBJECT_ROI:
  
            memcpy(&ae_obj->ae_shadow_set_param.para.ae_set_object_roi_setting, &param->set_param.ae_set_object_roi_setting, sizeof(struct rect_roi_config_t));
            //// gaze area
            //ae_obj->ae_shadow_set_param.para.ae_set_object_roi_setting.ref_frame_height = 1744;
            //ae_obj->ae_shadow_set_param.para.ae_set_object_roi_setting.ref_frame_width = 2328;
            //ae_obj->ae_shadow_set_param.para.ae_set_object_roi_setting.roi_count = 1;
            //ae_obj->ae_shadow_set_param.para.ae_set_object_roi_setting.roi[0].roi.h = 872;
            //ae_obj->ae_shadow_set_param.para.ae_set_object_roi_setting.roi[0].roi.w = 1164;
            //ae_obj->ae_shadow_set_param.para.ae_set_object_roi_setting.roi[0].roi.left = 0;
            //ae_obj->ae_shadow_set_param.para.ae_set_object_roi_setting.roi[0].roi.top = 0;
            //ae_obj->ae_shadow_set_param.para.ae_set_object_roi_setting.roi[0].weight = 300;     //scale 100
            break;
        case AE_SET_PARAM_PROCESS_FRAME_TYPE:
            break;
        case AE_SET_PARAM_MANUAL_AE_TARGET:
            if(((1 * 100) > param->set_param.manual_ae_target) || ((255 * 100) < param->set_param.manual_ae_target))
                ae_obj->ae_shadow_set_param.para.manual_ae_target = 0;//3825;
            else
                ae_obj->ae_shadow_set_param.para.manual_ae_target = param->set_param.manual_ae_target;
            break;
        case AE_SET_PARAM_WDR_ENABLE:
            ae_obj->ae_tuning_para.iAE_para.ucIsEnableiAE = param->set_param.wdr_enable;
            break;
		case AE_SET_PARAM_UPDATE_SETTING_FILE:

			break;
        case AE_SET_PARAM_FACTORY_DARK_MODE:
            break;
        case AE_SET_PARAM_ENABLE_UVC_PRIVACY:
            break;
        case AE_SET_PARAM_ANTIFLICKERMODE:
            if (param->set_param.ucAntiFlickerMode < 0)
            {
                errRet = _AL_AELIB_INVALID_FLICKER_PRIORITY_MODE;
            }
            else
            {
                ae_obj->ae_tuning_para.ucAntiFlickerMode = param->set_param.ucAntiFlickerMode; //manual antiflickermode    
            }
            
            break;
        case AE_SET_PARAM_iAE_PARAM_BV_TH:
            if (sizeof(ae_obj->ae_tuning_para.iAE_para.iaeluxIdxTh) != sizeof(param->set_param.iAE_param.iaeluxIdxTh))
            {
                errRet = _AL_AELIB_INVALID_IAE_PARA;
            }
            else
            {
                memcpy(ae_obj->ae_tuning_para.iAE_para.iaeluxIdxTh, param->set_param.iAE_param.iaeluxIdxTh, sizeof(ae_obj->ae_tuning_para.iAE_para.iaeluxIdxTh));
            }
            break;

        case AE_SET_PARAM_iAE_PARAM_HIST_RATIO:
            if ((sizeof(ae_obj->ae_tuning_para.iAE_para.iaeHistRatio) + 3 * sizeof(INT16)) != sizeof(param->set_param.iAE_param.iaeHistRatio))
            {
                errRet = _AL_AELIB_INVALID_IAE_PARA;
            }
            else
            {
                memcpy(ae_obj->ae_tuning_para.iAE_para.iaeHistRatio, param->set_param.iAE_param.iaeHistRatio, sizeof(ae_obj->ae_tuning_para.iAE_para.iaeHistRatio));
                ae_obj->ae_tuning_para.iAE_para.WDRMode = (WDRMode)param->set_param.iAE_param.iaeHistRatio[6];
                ae_obj->ae_tuning_para.iAE_para.wdrTargetGainInTh = param->set_param.iAE_param.iaeHistRatio[7];
                ae_obj->ae_tuning_para.iAE_para.wdrTargetGainOutTh = param->set_param.iAE_param.iaeHistRatio[8];
            }

            break;

        case AE_SET_PARAM_iAE_TUNING_PARAM:
            if (sizeof(ae_obj->ae_tuning_para.iAE_para.iAE_tuning_para) != sizeof(param->set_param.iAE_param.iaeTuningCoreParam))
            {
                errRet = _AL_AELIB_INVALID_IAE_PARA;
            }
            else
            {
                memcpy(&ae_obj->ae_tuning_para.iAE_para.iAE_tuning_para, &param->set_param.iAE_param.iaeTuningCoreParam, sizeof(ae_obj->ae_tuning_para.iAE_para.iAE_tuning_para));
            }
            
            break;

        case AE_SET_PARAM_SENSOR_FN_UPDATE:
            if (param->set_param.preview_sensor_info.f_number_x1000 < 1)
            {
                errRet = _AL_AELIB_INVALID_FNUMBER;
            }
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.f_number_x1000 = param->set_param.preview_sensor_info.f_number_x1000;
            ae_obj->curHWSetting.FN_x1000 = ae_obj->ae_shadow_set_param.para.preview_sensor_info.f_number_x1000;

            break;

        case AE_SET_PARAM_SENSOR_GAIN_UPDATE:
            if (param->set_param.preview_sensor_info.min_gain < 100 || param->set_param.preview_sensor_info.max_gain < 100 || param->set_param.preview_sensor_info.max_SensorGain < 100)
            {
                errRet = _AL_AELIB_INVALID_GAIN;
            }
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.max_gain = param->set_param.preview_sensor_info.max_gain;
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.min_gain = param->set_param.preview_sensor_info.min_gain;
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.max_SensorGain = param->set_param.preview_sensor_info.max_SensorGain;
            ae_obj->ae_current_set_param.para.preview_sensor_info.max_gain = ae_obj->ae_shadow_set_param.para.preview_sensor_info.max_gain;
            ae_obj->ae_current_set_param.para.preview_sensor_info.min_gain = ae_obj->ae_shadow_set_param.para.preview_sensor_info.min_gain;
            ae_obj->ae_current_set_param.para.preview_sensor_info.max_SensorGain = ae_obj->ae_shadow_set_param.para.preview_sensor_info.max_SensorGain;
            break;

        case AE_SET_PARAM_SENSOR_FPS_UPDATE:
            if (param->set_param.preview_sensor_info.max_fps < 1 || param->set_param.preview_sensor_info.min_fps < 1)
            {
                errRet = _AL_AELIB_INVALID_FPS;
            }
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.max_fps = param->set_param.preview_sensor_info.max_fps;
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.min_fps = param->set_param.preview_sensor_info.min_fps;

            ae_obj->ae_current_set_param.para.preview_sensor_info.max_fps = ae_obj->ae_shadow_set_param.para.preview_sensor_info.max_fps;
            ae_obj->ae_current_set_param.para.preview_sensor_info.min_fps = ae_obj->ae_shadow_set_param.para.preview_sensor_info.min_fps;
            break;
        case AE_SET_PARAM_SENSOR_HW_EXPO_UPDATE:
            if (param->set_param.preview_sensor_info.max_line_cnt > 65535 || param->set_param.preview_sensor_info.min_line_cnt < 1 || param->set_param.preview_sensor_info.exposuretime_per_exp_line_ns < 1)
            {
                errRet = _AL_AELIB_INVALID_LINE;
            }
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.exposuretime_per_exp_line_ns = param->set_param.preview_sensor_info.exposuretime_per_exp_line_ns;
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.max_line_cnt = param->set_param.preview_sensor_info.max_line_cnt;
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.min_line_cnt = param->set_param.preview_sensor_info.min_line_cnt;

            ae_obj->ae_current_set_param.para.preview_sensor_info.exposuretime_per_exp_line_ns = ae_obj->ae_shadow_set_param.para.preview_sensor_info.exposuretime_per_exp_line_ns;
            ae_obj->ae_current_set_param.para.preview_sensor_info.max_line_cnt = ae_obj->ae_shadow_set_param.para.preview_sensor_info.max_line_cnt;
            ae_obj->ae_current_set_param.para.preview_sensor_info.min_line_cnt = ae_obj->ae_shadow_set_param.para.preview_sensor_info.min_line_cnt;
            ae_obj->ae_runtime_data.minExpBV = alAELib_CalculateBv_midtones_comp(ae_obj->curHWSetting.FN_x1000, ae_obj->ae_current_set_param.para.preview_sensor_info.exposuretime_per_exp_line_ns / 1000, ae_obj->ae_current_set_param.para.preview_sensor_info.min_gain, 100);
            break;
        case AE_SET_PARAM_SENSOR_SW_EXPO_UPDATE:
            if (param->set_param.preview_sensor_info.sw_max_exptime < param->set_param.preview_sensor_info.sw_min_exptime || param->set_param.preview_sensor_info.sw_min_exptime < 1)
            {
                errRet = _AL_AELIB_INVALID_EXP_TIME;
            }
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.sw_max_exptime = param->set_param.preview_sensor_info.sw_max_exptime;
            ae_obj->ae_shadow_set_param.para.preview_sensor_info.sw_min_exptime = param->set_param.preview_sensor_info.sw_min_exptime;

            ae_obj->ae_current_set_param.para.preview_sensor_info.sw_max_exptime = ae_obj->ae_shadow_set_param.para.preview_sensor_info.sw_max_exptime;
            ae_obj->ae_current_set_param.para.preview_sensor_info.sw_min_exptime = ae_obj->ae_shadow_set_param.para.preview_sensor_info.sw_min_exptime;
            break;

        case AE_SET_PARAM_WEIGHT_TABLE_PARAM:
            if (param->set_param.statsBankNum > AE_MAX_STATS_BANK || param->set_param.statsBlockNum > AE_MAX_STATS_BLOCK 
                || param->set_param.statsBankNum < 1 || param->set_param.statsBlockNum < 1)
            {
                errRet = _AL_WRP_AE_INVALID_BLOCKS;
            }
            else if (ae_obj->ae_tuning_para.ucStatsBankNum != param->set_param.statsBankNum || ae_obj->ae_tuning_para.ucStatsBlockNum != param->set_param.statsBlockNum)
            {
                ae_obj->ae_tuning_para.ucStatsBankNum = param->set_param.statsBankNum;
                ae_obj->ae_tuning_para.ucStatsBlockNum = param->set_param.statsBlockNum;
                updateWeightTable(ae_obj);
            }

            break;

        case AE_SET_PARAM_TARGET_MODE:
            if (param->set_param.ae_target_mode >= AE_TARGET_MAX_MODE || param->set_param.ae_target_mode < 0)
            {
                errRet = _AL_AELIB_INVALID_TARGET_MODE;
            }
            else
            {
                ae_obj->ae_shadow_set_param.para.ae_target_mode = param->set_param.ae_target_mode;
            }
            break;

        case AE_SET_PARAM_EXPO_MODE:
            if (param->set_param.ae_expo_mode >= AE_EXPO_MAX_MODE || param->set_param.ae_expo_mode < 0)
            {
                errRet = _AL_AELIB_INVALID_EXPO_MODE;
            }
            else
            {
                ae_obj->ae_shadow_set_param.para.ae_expo_mode = param->set_param.ae_expo_mode;

            }
            break;

        case AE_SET_PARAM_TARGET_TABLE:
            if (sizeof(ae_obj->ae_shadow_set_param.para.ae_target_table) != sizeof(param->set_param.ae_target_table))
            {
                errRet = _AL_AELIB_INVALID_TABLE_PARA;
            }
            else
            {
                for (int i = 0; i < SCENECNT; i++)
                {
                    memcpy(ae_obj->ae_shadow_set_param.para.ae_target_table[i].table, param->set_param.ae_target_table[i].table, sizeof(int) * aeTargetRow * aeTargetCol);
                }
            }

            break;

        case AE_SET_PARAM_EXPO_TABLE:
            if (sizeof(ae_obj->ae_shadow_set_param.para.ae_expo_table) != sizeof(param->set_param.ae_expo_table))
            {
                errRet = _AL_AELIB_INVALID_TABLE_PARA;
            }
            else
            {
                for (int i = 0; i < AETABLE_CNT; i++)
                {
                    memcpy(ae_obj->ae_shadow_set_param.para.ae_expo_table[i].table, param->set_param.ae_expo_table[i].table, sizeof(int) * aeExpoRow * aeExpoCol);
                }
            }


            break;

        case AE_SET_PARAM_GAZE_ENABLE:
            ae_obj->ae_shadow_set_param.para.gaze_enable = param->set_param.gaze_enable;
            break;

        case AE_SET_PARAM_INITEXPOLIST:
        {
            uint8 pipeDelay = ae_obj->ae_shadow_set_param.para.pipeDelay;
            ae_sensorExpoList expoInit;
            expoInit.frameID = 0.0f;
            expoInit.shutter = ae_obj->ae_initial_param.exposure_time * 1.0f;
            expoInit.sensorGain = ae_obj->ae_initial_param.adgain * 1.0f;
            expoInit.ispGain = 1.0f;
            expoInit.midtoneGain = ae_obj->ae_initial_param.midtones_gain * 1.0f;
            expoInit.ispmidtoneGain = expoInit.midtoneGain;
            for (int i = 0; i < pipeDelay; i++)
            {
                expoInit.frameID = i * 1.0f;
                expousureList[ae_obj->ucCameraId].push(expoInit);
            }
        }
             break;
        case AE_SET_PARAM_FRAMEID:
            ae_obj->ae_runtime_data.frameid = param->set_param.frameid;
            break;
        case AE_SET_PARAM_EXPOSOFOUT:
        {
            ae_sensorExpoList expoSOFOut;
            memcpy(&expoSOFOut, &param->set_param.expoSOFOut,sizeof(ae_sensorExpoList));
            expousureList[ae_obj->ucCameraId].push(expoSOFOut);
        }
            break;
        case AE_SET_PARAM_AWBGAIN:
            ae_obj->ae_current_set_param.para.awbInfo.rGain = param->set_param.awbInfo.rGain;
            ae_obj->ae_current_set_param.para.awbInfo.gGain = param->set_param.awbInfo.gGain;
            ae_obj->ae_current_set_param.para.awbInfo.bGain = param->set_param.awbInfo.bGain;
            break;
        case AE_SET_PARAM_MULTICAM_PARAM:
            memcpy(&ae_obj->ae_tuning_para.multiCamera_para, &param->set_param.multiCamSetParam, sizeof(ae_obj->ae_tuning_para.multiCamera_para));
            break;
        case AE_SET_PARAM_MAX:
        default:  //no valid case, return with unrecognized parameters error
            errRet = _AL_AELIB_INVALID_PARAM;
    }

    return errRet;
}

/* AE get parameter event for AE lib */
unsigned int alAE_get_param(struct ae_get_param_t *param, void *ae_runtime_dat)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    ae_corelib_obj_t  *ae_obj;

    if ( param == NULL ||  ae_runtime_dat == NULL  )
    {
        errRet = _AL_AELIB_INVALID_ADDR;
        return errRet;
    }

    /* Check parameter validity, if not valid, return error */
    if ( param->ae_get_param_type > AE_GET_PARAM_MAX )
    {
        errRet = _AL_AELIB_INVALID_PARAM;
        return errRet;
    }

    ae_obj = (ae_corelib_obj_t  *)ae_runtime_dat;

    switch(param->ae_get_param_type)
    {
        case AE_GET_INIT_EXPOSURE_PARAM:
            param->para.ae_get_init_expo_param.ad_gain = ae_obj->ae_initial_param.adgain;
            param->para.ae_get_init_expo_param.iso = ae_obj->ae_initial_param.iso;
            param->para.ae_get_init_expo_param.exp_time = ae_obj->ae_initial_param.exposure_time;
            param->para.ae_get_init_expo_param.bv_val = ae_obj->ae_initial_param.BV;
			param->para.ae_get_init_expo_param.midtones_gain = ae_obj->ae_initial_param.midtones_gain;
            break;
        case AE_GET_ALHW3A_CONFIG:
            break;
        case AE_GET_CURRENT_CALIB_DATA:
            param->para.calib_data.min_iso = ae_obj->calibration_data.min_iso;
            param->para.calib_data.calib_b_gain = ae_obj->calibration_data.calib_b_gain;
            param->para.calib_data.calib_g_gain = ae_obj->calibration_data.calib_g_gain;
            param->para.calib_data.calib_r_gain = ae_obj->calibration_data.calib_r_gain;
            break;
        case AE_GET_CURRENT_WB:
            break;
        case AE_GET_DEBUG_INFO:
            param->para.exif_info.cur_ad_gain = ae_obj->ae_current_set_param.para.sof_notify_param.exp_adgain;
            param->para.exif_info.cur_exp_time = ae_obj->ae_current_set_param.para.sof_notify_param.exp_time;

            param->para.exif_info.bvY_8bit = ae_obj->ae_runtime_data.udAEMean_8bit;
            param->para.exif_info.bgYmean_8bit = ae_obj->ae_runtime_data.uwbgAEMean_8bit;
            param->para.exif_info.target = ae_obj->ae_tuning_para.udTargetmean;
            param->para.exif_info.next_bv = ae_obj->ae_current_output_dat.bvresult;
            param->para.exif_info.lux_result = ae_obj->ae_current_output_dat.lux_result;

            param->para.exif_info.ae_converged = ae_obj->ae_current_output_dat.ae_converged;
            param->para.exif_info.next_ad_gain = ae_obj->ae_current_output_dat.udsensor_ad_gain;
            param->para.exif_info.next_exp_time = ae_obj->ae_current_output_dat.udexposure_time;
            param->para.exif_info.midtones_gain = ae_obj->ae_current_output_dat.midtones_gain;
            param->para.exif_info.shadows_gain = ae_obj->ae_current_output_dat.shadows_gain;

            break;
        case AE_GET_PARAM_ANTIFLICKER_MODE:
            param->para.afd_flicker_mode = ae_obj->ae_shadow_set_param.para.afd_flicker_mode;
            break;
        case AE_GET_PARAM_OBJECT_ROI:
            break;
        case AE_GET_PARAM_MANUAL_EXPTIME:
            param->para.manual_exptime = ae_obj->ae_shadow_set_param.para.manual_exptime;
            break;
        case AE_GET_PARAM_MANUAL_ADGAIN:
            param->para.manual_adgain = ae_obj->ae_shadow_set_param.para.manual_adgain;
            break;
        case AE_GET_PARAM_EXPOSURE_LIMIT_RANGE:
            if (ae_obj->ae_runtime_data.udPcurveMaxExpTime != 0)
                param->para.ae_exp_limit_range.max_exptime = ae_obj->ae_runtime_data.udPcurveMaxExpTime;
            else
            {
                UINT32 udHWmaxtime = 0;
                if (0 != ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.exposuretime_per_exp_line_ns)
                    udHWmaxtime = (UINT32)((double)ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.exposuretime_per_exp_line_ns * (double)ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.max_line_cnt / 1000.0);
                else
                    udHWmaxtime = (UINT32)(1000000.0 / (float)ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.min_fps) * 100;
                param->para.ae_exp_limit_range.max_exptime = min(udHWmaxtime, ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.sw_max_exptime);

            }

            if (ae_obj->ae_runtime_data.udPcurveMinExpTime != 0)
                param->para.ae_exp_limit_range.min_exptime = ae_obj->ae_runtime_data.udPcurveMinExpTime;
            else
            {
                UINT32 udHWmintime = 0;
                if (0 != ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.exposuretime_per_exp_line_ns)
                    udHWmintime = (UINT32)((double)ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.exposuretime_per_exp_line_ns * (double)ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.min_line_cnt / 1000.0);
                else
                    udHWmintime = (UINT32)(1000000.0 / (double)ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.max_fps) * 100;

                param->para.ae_exp_limit_range.min_exptime = max(udHWmintime, ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.sw_min_exptime);

            }
            param->para.ae_exp_limit_range.max_adgain = ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.max_gain;
            param->para.ae_exp_limit_range.min_adgain = ae_obj->ae_shadow_set_param.para.ae_initial_setting.preview_sensor_info.min_gain;
            
            break;
        case AE_GET_MANUAL_EXPTIME_GAIN_RESOLUTION:
            break;
        case AE_GET_MANUAL_EV_COMPENSATION:
			param->para.manual_ev_comp = ae_obj->ae_shadow_set_param.para.manual_ev_comp;
            break;
        case AE_GET_CUREXPOLIST:
        {
            ae_sensorExpoList expoSOF;
            ae_sensorExpoList expoSOFTemp;
            UINT8 pipeDelay = ae_obj->ae_shadow_set_param.para.pipeDelay;
            //expoSOF = ae_obj->ae_runtime_data.expousureList.front();
            int queueSize = expousureList[ae_obj->ucCameraId].size();
            if (queueSize < pipeDelay)
            {
                float frameid = 0;
                if (queueSize > 0)
                {
                    expoSOFTemp.frameID = expousureList[ae_obj->ucCameraId].back().frameID + 1;
                    frameid = expoSOFTemp.frameID;
                    expoSOFTemp.ispGain = expousureList[ae_obj->ucCameraId].back().ispGain;
                    expoSOFTemp.ispmidtoneGain = expousureList[ae_obj->ucCameraId].back().ispmidtoneGain;
                    expoSOFTemp.midtoneGain = expousureList[ae_obj->ucCameraId].back().midtoneGain;
                    expoSOFTemp.sensorGain = expousureList[ae_obj->ucCameraId].back().sensorGain;
                    expoSOFTemp.shutter = expousureList[ae_obj->ucCameraId].back().shutter;
                }
                else
                {
                    expoSOFTemp.frameID = ae_obj->ae_runtime_data.frameid - pipeDelay;
                    frameid = expoSOFTemp.frameID;
                    expoSOFTemp.ispGain = 1.0f;
                    expoSOFTemp.ispmidtoneGain = ae_obj->ae_initial_param.midtones_gain * 1.0f;
                    expoSOFTemp.midtoneGain = ae_obj->ae_initial_param.midtones_gain * 1.0f;
                    expoSOFTemp.sensorGain = ae_obj->ae_initial_param.adgain * 1.0f;
                    expoSOFTemp.shutter = ae_obj->ae_initial_param.exposure_time * 1.0f;
                }

                //return _AL_WRP_AE_EMPTY_EXPOSOFLIST;

                for (int i = 0; i < pipeDelay - queueSize; i++)
                {
                    expoSOFTemp.frameID = frameid + i;
                    expousureList[ae_obj->ucCameraId].push(expoSOFTemp);
                }

                LOG_DEBUG("ERROR expousureList size < pipeDelay");
            }
            expoSOF = expousureList[ae_obj->ucCameraId].front();
            LOG_DEBUG("curframeid %u, expoSOF1 %f, %f, %f, %f, %f\n", ae_obj->ae_runtime_data.frameid, expoSOF.frameID, expoSOF.shutter, expoSOF.sensorGain, expoSOF.ispGain, expoSOF.midtoneGain);
            
            if (ae_obj->ae_runtime_data.frameid - expoSOF.frameID >= pipeDelay + 1)
            {
                int interval = min(ae_obj->ae_runtime_data.frameid - expoSOF.frameID, pipeDelay + pipeDelay - 1);
                for (int i = 0; i < interval - pipeDelay; i++)
                {
                    expousureList[ae_obj->ucCameraId].pop();
                }
                expoSOF = expousureList[ae_obj->ucCameraId].front();
                LOG_DEBUG("DEBUG exposof %f %f %f", expoSOF.frameID, expoSOF.sensorGain, expoSOF.shutter);
                ae_sensorExpoList expoSOFBack;
                expoSOFBack = expousureList[ae_obj->ucCameraId].back();
                for (int i = interval - pipeDelay; i > 0; i--)
                {
                    expoSOFBack.frameID = ae_obj->ae_runtime_data.frameid - i;
                    expousureList[ae_obj->ucCameraId].push(expoSOFBack);
                }
                expousureList[ae_obj->ucCameraId].pop();
            }
            else
            {
                expousureList[ae_obj->ucCameraId].pop();
            }
            LOG_DEBUG("pipeDelay %u, expoSOF2 %f, %f, %f, %f, %f\n", pipeDelay, expousureList[ae_obj->ucCameraId].front().frameID,
                expousureList[ae_obj->ucCameraId].front().shutter, expousureList[ae_obj->ucCameraId].front().sensorGain,
                expousureList[ae_obj->ucCameraId].front().ispGain, expousureList[ae_obj->ucCameraId].front().midtoneGain);
            memcpy(&param->para.expoSOF, &expoSOF, sizeof(ae_sensorExpoList));
        }
            break;
        case AE_GET_CUREXPOLISTFRONT:
        {
            ae_sensorExpoList expSOFFront;
            //expSOFFront = ae_obj->ae_runtime_data.expousureList.front();
            expSOFFront = expousureList[ae_obj->ucCameraId].front();
            LOG_DEBUG("DEBUG expsof front %f %f %f back %f %f %f", expSOFFront.sensorGain, expSOFFront.shutter, expSOFFront.ispGain,
                expousureList[ae_obj->ucCameraId].back().sensorGain, expousureList[ae_obj->ucCameraId].back().shutter, expousureList[ae_obj->ucCameraId].back().ispGain);
            memcpy(&param->para.expoSOF, &expSOFFront, sizeof(ae_sensorExpoList));
        }
            break;
        case AE_GET_MAXSENSORGAIN:
        {
            param->para.maxSensorGain = ae_obj->ae_current_set_param.para.preview_sensor_info.max_SensorGain;
        }
            break;
        case AE_GET_PARAM_MAX:
        default:
            return _AL_AELIB_INVALID_PARAM;
    }

    return errRet;
}

unsigned int alAE_process(void *hw3a_ae_stats_data, void *hw3a_yhis_stats_data, struct alhw3a_ae_proc_data_t ae_proc_data, void *ae_runtime_dat, struct ae_output_data_t *ae_output)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    ae_corelib_obj_t *ae_obj = (ae_corelib_obj_t *)ae_runtime_dat;
    struct ae_output_data_t  *output;
    INT32 ISO_G_gain = 0;
    struct al3awrapper_stats_ae_t * ae_stats_data;
    struct al3awrapper_stats_ae_t *ae_dispatched_data;
    UINT32 stats_SOF_idx = 0, sys_sof_index = 0;
    UINT32 out_adgain = 100;
    UINT16 midtones_gain = 100, isp_midtones_gain = 100, shadows_gain = 100, short_target;
    iAE_input iAE_buffer;

    // Basic checking for parameter validity
    if (hw3a_ae_stats_data == NULL || ae_obj == NULL || ae_output == NULL)
    {
        errRet = _AL_AELIB_INVALID_ADDR;
        return errRet;
    }
    output = ae_output;
    //memcpy(ae_output, &ae_obj->ae_current_output_dat, sizeof(struct ae_output_data_t));

    //Check is AE Enabled?
    if (FALSE == ae_obj->ae_shadow_set_param.para.ae_enable)
    {
        ae_obj->lib_runtime_st.ucIsAEEnable = 0;
        //ae break, keep last result out 
        memcpy(ae_output, &ae_obj->ae_current_output_dat, sizeof(struct ae_output_data_t));
        //backup output report to runtime report
        //memcpy(&ae_obj->ae_current_output_dat, output, sizeof(struct ae_output_data_t));
        return errRet;
    }
    else
        ae_obj->lib_runtime_st.ucIsAEEnable = 1;

    // AE wrapper start: dispatch hw data
    ae_dispatched_data = &ae_obj->ae_dispatched_data;
    errRet = al3awrapper_dispatchhw3a_aestats(hw3a_ae_stats_data, ae_proc_data, ae_dispatched_data, ae_obj);
    if (errRet != _AL_3ALIB_SUCCESS)
    {
        return errRet;
    }

    //only master exposure
    if ((ae_obj->ae_tuning_para.multiCamera_para.multiCamMode == AE_MULTICAM_MODE_ONLYMASTER || ae_obj->ae_tuning_para.multiCamera_para.multiCamMode == AE_MULTICAM_MODE_WEIGHTSTATS_MASTERTWOINONE ) 
        && ae_obj->ae_runtime_data.slaveID == ae_obj->ucCameraId) //todo:is turn on both camera?
    {
        al_ae_exp_param_t exp_output;
        memset(&exp_output, 0, sizeof(al_ae_exp_param_t));

        int targetBV;
        targetBV = storeExp[ae_obj->ae_runtime_data.masterID].targetBV;

        alAELib_GetExposureParameter(ae_obj, &exp_output, targetBV);
        ae_obj->ae_runtime_data.preBV = targetBV;

        ISO_G_gain = ae_obj->calibration_data.calib_g_gain;
        if (ISO_G_gain == 0)
        {
            ISO_G_gain = 1000; // use default value
            errRet = _AL_AELIB_INVALID_CALIB_GAIN;
        }

        out_adgain = exp_output.udISO * ISO_G_gain / 1000;
        ae_obj->ae_current_output_dat.iso = exp_output.udISO;
        ae_obj->ae_current_output_dat.udsensor_ad_gain = out_adgain <= ae_obj->ae_current_set_param.para.preview_sensor_info.min_gain ? ae_obj->ae_current_set_param.para.preview_sensor_info.min_gain : out_adgain;
        ae_obj->ae_current_output_dat.udexposure_time = exp_output.udexposure_time;


        ae_obj->ae_current_output_dat.ae_converged = 1;

         //update output data in runtime buffer
        ae_obj->ae_current_output_dat.bg_bvresult = storeExp[ae_obj->ae_runtime_data.masterID].bg_bvresult;
        ae_obj->ae_current_output_dat.lux_result = alAELib_BV2Lux(output->bg_bvresult);
        ae_obj->ae_current_output_dat.midtones_gain = storeExp[ae_obj->ae_runtime_data.masterID].midtones_gain;
        ae_obj->ae_current_output_dat.shadows_gain = storeExp[ae_obj->ae_runtime_data.masterID].shadows_gain;
        ae_obj->ae_current_output_dat.isp_midtones_gain = storeExp[ae_obj->ae_runtime_data.masterID].isp_midtones_gain;
        //Output AE Data
        LOG_DEBUG("slave ucIsConverged/uciAE_AEconverge_cnt/wiAEDeltaBV: %d/%d/%d", ae_obj->lib_runtime_st.ucIsConverged, ae_obj->ae_runtime_data.uciAE_AEconverge_cnt, ae_obj->ae_runtime_data.wiAEDeltaBV);
        //if ((ae_obj->lib_runtime_st.ucIsConverged == 0) || (ae_obj->ae_runtime_data.uciAE_AEconverge_cnt < 2) || ae_obj->ae_runtime_data.wiAEDeltaBV != 0) //converged or hdr done, output fresh off

        output->iso = ae_obj->ae_current_output_dat.iso;
        output->udsensor_ad_gain = ae_obj->ae_current_output_dat.udsensor_ad_gain;
        output->udexposure_time = ae_obj->ae_current_output_dat.udexposure_time;
        output->ae_converged = ae_obj->ae_current_output_dat.ae_converged;
        output->midtones_gain = ae_obj->ae_current_output_dat.midtones_gain;
        output->shadows_gain = ae_obj->ae_current_output_dat.shadows_gain;
        output->isp_midtones_gain = ae_obj->ae_current_output_dat.isp_midtones_gain;

        output->bvresult = storeExp[ae_obj->ae_runtime_data.masterID].bvresult;
        output->lux_result = ae_obj->ae_current_output_dat.lux_result;
        output->ymean100_8bit = storeExp[ae_obj->ae_runtime_data.masterID].uwbgAEMean_8bit;

        LOG_DEBUG("[OUT]iso/ad_gain/expoTime: %u/%u/%u", output->iso, output->udsensor_ad_gain, output->udexposure_time);
        LOG_DEBUG("[OUT]midtones_gain/shadows_gain: %u/%u", output->midtones_gain, output->shadows_gain);
        LOG_DEBUG("[OUT]bg_bv/bv/lux/ymean_8bit: %d/%d/%u/%u\n", output->bg_bvresult, output->bvresult, output->lux_result, output->ymean100_8bit);

        LOG_INFO("lib_runtime_st.uwCountingIdx: %u\n", ae_obj->lib_runtime_st.uwCountingIdx);

        LOG_WARN("[WhiteBox]lux %d", output->lux_result);
        LOG_WARN("[WhiteBox]ucIsConverged %d", ae_obj->lib_runtime_st.ucIsConverged);
        LOG_WARN("[WhiteBox]ymean_8bit %d", output->ymean100_8bit);
        LOG_WARN("[WhiteBox]iso/ad_gain/expoTime: %d/%d/%d", output->iso, output->udsensor_ad_gain, output->udexposure_time);
        return errRet;
    }

    // assign patched data buffer pointer
    ae_stats_data = ae_dispatched_data;

    // FrameIndex = ae_stats_data->uframeidx;
    stats_SOF_idx = ae_stats_data->udsys_sof_idx;
    if (sizeof( struct al3awrapper_stats_ae_t ) != ae_stats_data->ustructuresize)
    {
        errRet = _AL_AELIB_MISMATCH_STATS_SIZE;
        return errRet;
    }

    // verify SOF validity, if not same frame, skip all processing
    sys_sof_index = ae_obj->ae_shadow_set_param.para.sof_notify_param.sys_sof_index;
    LOG_INFO("sof_index/stats_sof_idx: %ld / %ld", sys_sof_index, stats_SOF_idx);
    LOG_DEBUG("sof_index/stats_sof_idx: %ld / %ld", sys_sof_index, stats_SOF_idx);
    if ( sys_sof_index != stats_SOF_idx )
    {
        errRet = _AL_AELIB_SOF_FRAME_INDEX_MISMATCH;   // mark error code here, do error handling
    }

    // dispatch yhis hw data
    if (NULL != hw3a_yhis_stats_data)
    {
        errRet = al3awrapper_dispatchhw3a_yhiststats(hw3a_yhis_stats_data, ae_obj);
        if (errRet != _AL_3ALIB_SUCCESS)
        {
            return errRet;
        }
    }
    else
    {
        /* calculate histogram from ae stats data if there's no hw yhis */
        UINT16 block_num = ae_stats_data->ucvalidblocks * ae_stats_data->ucvalidbanks;
        calc_histogram(ae_stats_data->statsy, ae_obj->ae_runtime_data.udY_hist, block_num);
    }

    // manage parameter loading
    errRet = ManageInputParam(ae_obj);

    ISO_G_gain = ae_obj->calibration_data.calib_g_gain;
    if ( ISO_G_gain == 0 )
    {
        ISO_G_gain = 1000; // use default value
        errRet = _AL_AELIB_INVALID_CALIB_GAIN;
    }

    // loading from SOF notify info
    UINT32 valid_ISO = (ae_obj->ae_current_set_param.para.sof_notify_param.exp_adgain * 1000 + (ISO_G_gain>>1)) / ISO_G_gain;
    UINT32 udFn = ae_obj->curHWSetting.FN_x1000;
    ae_obj->ae_runtime_data.dImageBV = alAELib_CalculateBv_midtones_comp( udFn, ae_obj->ae_current_set_param.para.sof_notify_param.exp_time, valid_ISO, ae_obj->ae_current_set_param.para.sof_notify_param.midtones_gain);//exp_adgain,scale 100

    LOG_DEBUG("now imageBv is %d", ae_obj->ae_runtime_data.dImageBV);
    
    //modify target by image bv
    LOG_DEBUG("now target mode is %02d", ae_obj->ae_current_set_param.para.ae_target_mode);
    int aemode = g_AE_NormalTarget.eSceneMode;
    switch (ae_obj->ae_current_set_param.para.ae_target_mode)  //modify taget table or input mode
    {
    case AE_TARGET_FIX:
        ae_obj->ae_tuning_para.udTargetmean = ae_obj->ae_current_set_param.para.manual_ae_target;
        break;
    case AE_TARGET_NORMAL:
        updateTarget(ae_obj, aemode);
        break;
    case AE_TARGET_NIGHT:
        break;
    case AE_TARGET_USERDEF:
        break;
    default:  //fix

        break;
    }
    LOG_DEBUG("now target is %.2d", ae_obj->ae_tuning_para.udTargetmean);

    LOG_INFO("stats weight blk  bnk: %d * %d", ae_stats_data->ucvalidblocks, ae_stats_data->ucvalidbanks);
    // update AE Metering
    alAE_Simple_AE_Metering(ae_obj, output, ae_stats_data);

    // loading metering table by metering mode
    errRet = alAELib_generate_metering_table(ae_obj, ae_obj->ae_runtime_data.metering_table);

    if (errRet != _AL_3ALIB_SUCCESS)
    {
        return errRet;
    }

    //Prepare metering parameters
    ae_metering_core_input ae_metering_input;
    ae_metering_core_output ae_metering_output;
    ae_metering_input.ucStatsDepth = ae_stats_data->ucstatsdepth;
    ae_metering_input.pudstatsY = ae_stats_data->statsy;
    ae_metering_input.ucWidthBlockNum = ae_stats_data->ucvalidblocks;
    ae_metering_input.ucHeightBlockNum = ae_stats_data->ucvalidbanks;
    ae_metering_input.puwWeightTable = ae_obj->ae_runtime_data.metering_table;
    ae_metering_input.pucYWeightTable = ae_obj->ae_tuning_para.aucYWeightTable;
    errRet = alAELib_Core_AEMetering(&ae_metering_input, &ae_metering_output);
    if (errRet != _AL_3ALIB_SUCCESS)
    {
        return errRet;
    }

    ae_obj->ae_runtime_data.udAEMean_8bit = ae_metering_output.udYMean_8bit;
    storeExp[ae_obj->ucCameraId].udAEMean_8bit = ae_obj->ae_runtime_data.udAEMean_8bit;
    storeExp[ae_obj->ucCameraId].uwbgAEMean_8bit = ae_obj->ae_runtime_data.uwbgAEMean_8bit;
    LOG_DEBUG("udYmean_weight: %d", ae_metering_output.udYMean_8bit);

    if ((TRUE != ae_obj->ae_tuning_para.iAE_para.ucIsEnableiAE) || 
        (0 != ae_obj->ae_current_set_param.para.manual_ae_target))
    {
        ae_obj->ae_runtime_data.uwiAE_shortTargetMean = 0;
        midtones_gain = shadows_gain = 100;
        ae_obj->ae_runtime_data.max_isp_midtones_gain = 100;
        isp_midtones_gain = 100;
    }
    else if ((0 != ae_obj->ae_current_set_param.para.manual_ev_comp) ||
        (0 != ae_obj->ae_current_set_param.para.manual_exptime) ||
        (0 != ae_obj->ae_current_set_param.para.manual_adgain) ||
        (0 != ae_obj->ae_current_set_param.para.manual_isolevel))
    {
        // keep last result when manual ae
        midtones_gain = ae_obj->ae_current_output_dat.midtones_gain;
        shadows_gain = ae_obj->ae_current_output_dat.shadows_gain;
        ae_obj->ae_runtime_data.uwmidtones_gain = midtones_gain;
        ae_obj->ae_runtime_data.max_isp_midtones_gain = ae_obj->ae_tuning_para.iAE_para.iAE_tuning_para[0].max_isp_midtones_gain;
        isp_midtones_gain = ae_obj->ae_current_output_dat.isp_midtones_gain;
    }
    else
    {
        //update iAE while AE converged

        if (((ae_obj->lib_runtime_st.ucIsConverged == 0) || (ae_obj->ae_runtime_data.uciAE_AEconverge_cnt < 5)) && !preWdrFlag)
        {
            if (ae_obj->lib_runtime_st.ucIsConverged == 1)
                ae_obj->ae_runtime_data.uciAE_AEconverge_cnt++;

            midtones_gain = ae_obj->ae_current_output_dat.midtones_gain;
            shadows_gain = ae_obj->ae_current_output_dat.shadows_gain;
            short_target = ae_obj->ae_runtime_data.uwiAE_shortTargetMean;
            ae_obj->ae_runtime_data.uwiAE_shortTargetMean = short_target;
            ae_obj->ae_runtime_data.uwmidtones_gain = midtones_gain;
            isp_midtones_gain = ae_obj->ae_current_output_dat.isp_midtones_gain;

        }
        else
        {
            iAE_buffer.iae_param = &ae_obj->ae_tuning_para.iAE_para;
            iAE_buffer.aec_param.cur_mean = ae_obj->ae_runtime_data.udAEMean_8bit;
            iAE_buffer.aec_param.hist_length = AL_MAX_HIST_NUM;
            iAE_buffer.aec_param.ref_target = ae_obj->ae_tuning_para.udTargetmean;

            // reverse stats Ymean
			iAE_buffer.aec_param.cur_mean = (iAE_buffer.aec_param.cur_mean * 100) / ae_obj->ae_current_set_param.para.sof_notify_param.midtones_gain;
            iAE_buffer.aec_param.R_hist = iAE_buffer.aec_param.G_hist = iAE_buffer.aec_param.B_hist = (UINT32*)ae_obj->ae_runtime_data.udY_hist;
            iAE_buffer.aec_param.r_gain = iAE_buffer.aec_param.g_gain = iAE_buffer.aec_param.b_gain = 100;

            iAE_buffer.ucCameraId = ae_obj->ucCameraId;

            iAE_process(&iAE_buffer, &midtones_gain, &shadows_gain, &short_target, (INT32)ae_obj->ae_runtime_data.beforeWdrExpIndex, ae_obj);

            LOG_DEBUG("before wdr expindex %f, effect midtones_gain is %d", ae_obj->ae_runtime_data.beforeWdrExpIndex, midtones_gain);
            //LOG_DEBUG("effect shadow_gain is %d", shadows_gain);
            //LOG_DEBUG("effect short_target is %d", short_target);

            ae_obj->ae_runtime_data.uwiAE_shortTargetMean = short_target;
            ae_obj->ae_runtime_data.uwmidtones_gain = midtones_gain;

            if (ae_obj->ae_tuning_para.iAE_para.WDRMode == WDR_2MAXMIDTONESGAIN)
            {
                isp_midtones_gain = min(midtones_gain, ae_obj->ae_runtime_data.max_isp_midtones_gain);
            }
            else
            {
                isp_midtones_gain = midtones_gain;
            }


            if (midtones_gain != 100)
            {
                preWdrFlag = true;
            }
            else
            {
                preWdrFlag = false;
            }
       }
    }

    //gaze AE
    //LOG_DEBUG("gaze enable: %d", ae_obj->ae_current_set_param.para.gaze_enable);
    if (ae_obj->ae_current_set_param.para.gaze_enable)
    {
        errRet = gazeAE(ae_obj);

        if (errRet != _AL_3ALIB_SUCCESS)
        {
            return errRet;
        }
    }

    // BV estimation
    errRet = alAE_estimation(ae_obj);

    ae_obj->ae_runtime_data.beforeWdrExpIndex = log10(1.0f / pow(2, (ae_obj->ae_current_output_dat.bvresult - ae_obj->ae_runtime_data.minExpBV) / 1000.0f)) / log10(1.03);


    if (errRet != _AL_3ALIB_SUCCESS)
    {
        return errRet;
    }

    // calculate exposure time, exposure line, ad gain
    al_ae_exp_param_t exp_output;
    memset(&exp_output, 0, sizeof(al_ae_exp_param_t));

    int targetBV = ae_obj->ae_current_output_dat.bvresult + ae_obj->ae_runtime_data.wiAEDeltaBV;

    storeExp[ae_obj->ucCameraId].targetBV = targetBV;
    //mix master and slave
    if (ae_obj->ae_tuning_para.multiCamera_para.multiCamMode == AE_MULTICAM_MODE_WEIGHTEXP_BYMASTER)
    {
         alAELib_Core_mixExposure(ae_obj, &targetBV);
    }

    alAELib_GetExposureParameter(ae_obj, &exp_output, targetBV);
    ae_obj->ae_runtime_data.preBV = targetBV;


    out_adgain = exp_output.udISO * ISO_G_gain / 1000;
    ae_obj->ae_current_output_dat.iso = exp_output.udISO;
    ae_obj->ae_current_output_dat.udsensor_ad_gain = out_adgain <= ae_obj->ae_current_set_param.para.preview_sensor_info.min_gain ? ae_obj->ae_current_set_param.para.preview_sensor_info.min_gain : out_adgain;
    ae_obj->ae_current_output_dat.udexposure_time = exp_output.udexposure_time;

    //recal bv after check sensor step 
    int32 rebvresult = alAELib_CalculateBv_midtones_comp(udFn, ae_obj->ae_current_output_dat.udexposure_time, ae_obj->ae_current_output_dat.iso, midtones_gain);//exp_adgain,scale 100
    ae_obj->ae_current_output_dat.bvresult = rebvresult;


    ae_obj->ae_current_output_dat.ae_converged = ae_obj->lib_runtime_st.ucIsConverged;
    
    ae_obj->ae_current_output_dat.midtones_gain = midtones_gain;

    // update output data in runtime buffer
    ae_obj->ae_current_output_dat.bg_bvresult = output->bg_bvresult;
    ae_obj->ae_current_output_dat.lux_result = alAELib_BV2Lux(output->bg_bvresult);
    ae_obj->ae_current_output_dat.midtones_gain = midtones_gain;
    ae_obj->ae_current_output_dat.shadows_gain = shadows_gain;
    ae_obj->ae_current_output_dat.isp_midtones_gain = isp_midtones_gain;

    //Output AE Data
    LOG_DEBUG("ucIsConverged/uciAE_AEconverge_cnt/wiAEDeltaBV: %d/%d/%d", ae_obj->lib_runtime_st.ucIsConverged, ae_obj->ae_runtime_data.uciAE_AEconverge_cnt, ae_obj->ae_runtime_data.wiAEDeltaBV);
    //if ((ae_obj->lib_runtime_st.ucIsConverged == 0) || (ae_obj->ae_runtime_data.uciAE_AEconverge_cnt < 2) || ae_obj->ae_runtime_data.wiAEDeltaBV != 0) //converged or hdr done, output fresh off
    //bool gainFlag = alAELib_Abs(ae_obj->ae_current_set_param.para.sof_notify_param.exp_adgain - ae_obj->ae_current_output_dat.udsensor_ad_gain) < 6 ? true : false;
    //bool shutterFlag = alAELib_Abs(ae_obj->ae_current_set_param.para.sof_notify_param.exp_time - ae_obj->ae_current_output_dat.udexposure_time) < 30 ? true : false;
    //if ((gainFlag == false) || (shutterFlag == false))
    //{
        output->iso = ae_obj->ae_current_output_dat.iso;
        output->isp_midtones_gain = ae_obj->ae_current_output_dat.isp_midtones_gain;
        output->udsensor_ad_gain = ae_obj->ae_current_output_dat.udsensor_ad_gain;
        output->udexposure_time = ae_obj->ae_current_output_dat.udexposure_time;
        output->ae_converged = ae_obj->ae_current_output_dat.ae_converged;
        output->midtones_gain = ae_obj->ae_current_output_dat.midtones_gain;
        output->shadows_gain = ae_obj->ae_current_output_dat.shadows_gain;
    //}


    output->bvresult = ae_obj->ae_current_output_dat.bvresult;
    output->lux_result = ae_obj->ae_current_output_dat.lux_result;
    output->ymean100_8bit = ae_obj->ae_runtime_data.uwbgAEMean_8bit;

    //only master exposure
    if ((ae_obj->ae_tuning_para.multiCamera_para.multiCamMode == AE_MULTICAM_MODE_ONLYMASTER || ae_obj->ae_tuning_para.multiCamera_para.multiCamMode == AE_MULTICAM_MODE_WEIGHTSTATS_MASTERTWOINONE) 
        && ae_obj->ae_runtime_data.masterID == ae_obj->ucCameraId)
    {
        storeExp[ae_obj->ucCameraId].targetBV = targetBV;
        storeExp[ae_obj->ucCameraId].bg_bvresult = ae_obj->ae_current_output_dat.bg_bvresult;
        storeExp[ae_obj->ucCameraId].bvresult = ae_obj->ae_current_output_dat.bvresult;
        storeExp[ae_obj->ucCameraId].midtones_gain = ae_obj->ae_current_output_dat.midtones_gain;
        storeExp[ae_obj->ucCameraId].shadows_gain = ae_obj->ae_current_output_dat.shadows_gain;
        storeExp[ae_obj->ucCameraId].uwbgAEMean_8bit = ae_obj->ae_runtime_data.uwbgAEMean_8bit;
        storeExp[ae_obj->ucCameraId].udAEMean_8bit = ae_obj->ae_runtime_data.udAEMean_8bit;
        storeExp[ae_obj->ucCameraId].isp_midtones_gain = ae_obj->ae_current_output_dat.isp_midtones_gain;
    }

    LOG_DEBUG("[OUT]iso/ad_gain/expoTime: %u/%u/%u, ver: %f", output->iso, output->udsensor_ad_gain, output->udexposure_time, AE_LIB_MINOR_VER);
    LOG_DEBUG("[OUT]midtones_gain/shadows_gain: %u/%u", output->midtones_gain, output->shadows_gain);
    LOG_DEBUG("[OUT]bg_bv/bv/lux/ymean_8bit: %d/%d/%u/%u\n", output->bg_bvresult, output->bvresult, output->lux_result, output->ymean100_8bit);

    LOG_INFO("lib_runtime_st.uwCountingIdx: %u\n", ae_obj->lib_runtime_st.uwCountingIdx);

    LOG_WARN("[WhiteBox]lux bvresult %d", output->lux_result);
    LOG_WARN("[WhiteBox]ucIsConverged %d", ae_obj->lib_runtime_st.ucIsConverged);
    LOG_WARN("[WhiteBox]ymean_8bit %d", output->ymean100_8bit);
    LOG_WARN("[WhiteBox]iso/ad_gain/expoTime: %d/%d/%d", output->iso, output->udsensor_ad_gain, output->udexposure_time);

    //LOG_TRACE("[WhiteBox]iso/ad_gain/expoTime: %d/%d/%d, ymean_8bit %d, ucIsConverged %d",output->iso, output->udsensor_ad_gain, output->udexposure_time, output->ymean100_8bit, ae_obj->lib_runtime_st.ucIsConverged);
    if ( ae_obj->lib_runtime_st.uwCountingIdx < 9999 )
        ae_obj->lib_runtime_st.uwCountingIdx++;
    else
        ae_obj->lib_runtime_st.uwCountingIdx = 0;

    return errRet;
}

/********************************************************************************
*                           Private Function                                    *
********************************************************************************/
/* alAELibCore.c */
UINT32 alAE_estimation(void *ae_runtime_dat)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    ae_corelib_obj_t *ae_obj = (ae_corelib_obj_t *)ae_runtime_dat;
    UINT32 ae_mean_8bit, udAE_Targetmean;
    INT32 wYmean2BV, ae_targetBV, ae_maxBV, ae_targetAllcompBV, ae_deltaBV, ae_nextBV, ae_imageBV, ae_preBV, ae_iaedeltaBV;
    INT16 wManualEvComp = 0;
    UINT8 ucConverge_SPD = AE_CONVERGE_SPEED_LVMAX;
	UINT32 udmidgain = (UINT32)ae_obj->ae_runtime_data.uwmidtones_gain;
    
    if (ae_obj == NULL)
        return _AL_AELIB_INVALID_ADDR;

    ae_mean_8bit = ae_obj->ae_runtime_data.udAEMean_8bit;
    ae_imageBV = ae_obj->ae_runtime_data.dImageBV;
    wManualEvComp = ae_obj->ae_current_set_param.para.manual_ev_comp;
    ucConverge_SPD = ae_obj->ae_current_set_param.para.converge_speedlv;
    ae_preBV = ae_obj->ae_current_output_dat.bvresult;
    LOG_DEBUG("[smoothAE] prevBv is %d", ae_preBV);
    if ( ae_mean_8bit < _ALAE_MIN_MEAN_8BITS_ )
        ae_mean_8bit = (UINT32)_ALAE_MIN_MEAN_8BITS_;


    udAE_Targetmean = ae_obj->ae_tuning_para.udTargetmean;

    if (0 != ae_obj->ae_current_set_param.para.manual_ae_target)
        udAE_Targetmean = ae_obj->ae_current_set_param.para.manual_ae_target;

	// calculate BV
    wYmean2BV =(INT32)(1000 *  alAELib_Log2_65536((ae_mean_8bit << 8) / 100)) >> 16; // /100:scale100

    // calculate target BV  ( assume target 40 * 4 )
    ae_targetBV =(INT32)(1000 * alAELib_Log2_65536((udAE_Targetmean << 8) / 100)) >> 16;  // << 8: scale 256, /100:scale100
	
    ae_maxBV =(INT32)(1000 * alAELib_Log2_65536(_ALAE_MAX_MEAN_10BITS_ << 6)) >> 16;    // <d< 6: scale 256 and 10bit to 8 bit

    ae_targetAllcompBV =  ae_targetBV + wManualEvComp;
    if ( ae_targetAllcompBV > ae_maxBV )
        ae_targetAllcompBV = ae_maxBV;
	
    ae_deltaBV = wYmean2BV - ae_targetAllcompBV;

    //LOG_DEBUG("wYmean2BV / ae_targetAllcompBV: %d(ev)/%d(ev)", wYmean2BV, ae_targetAllcompBV);
    //printf("ae_targetBV is %d \n", ae_targetBV);

    if ((TRUE == ae_obj->ae_tuning_para.iAE_para.ucIsEnableiAE) && (0 != ae_obj->ae_runtime_data.uwiAE_shortTargetMean))
    {
        if (ae_obj->ae_tuning_para.iAE_para.WDRMode == WDR_WDRTARGETTABLE_DEPENDUSEMID)
        {
            UINT32 tempTarget = udAE_Targetmean;
            isDynamicScene(ae_runtime_dat);
            if (ae_obj->ae_runtime_data.isDynamic)
            {
                int aemode = PREVIEW;
                updateTarget(ae_obj, aemode);
            }
            udmidgain = tempTarget * 100 / ae_obj->ae_tuning_para.udTargetmean;
            ae_obj->ae_tuning_para.udTargetmean = tempTarget;
            ae_obj->ae_runtime_data.uwmidtones_gain = udmidgain;
            LOG_DEBUG("[WDR AE]: not use iaemidgain, use wdrtarget midgain %d", udmidgain);
        }
		ae_iaedeltaBV = (INT32)(1000 * alAELib_Log2_65536((udmidgain << 8) / 100)) >> 16;
        //udAE_Targetmean /= (udmidgain /100.0f);
        LOG_DEBUG("[WDR AE]: short target : %d   udmidgain : %d", ae_obj->ae_runtime_data.uwiAE_shortTargetMean, udmidgain);
        //LOG_DEBUG("[WDR AE]: udmidgain : %d", udmidgain);
        //LOG_DEBUG("[WDR AE]: modify target : %d", udAE_Targetmean);
    }
    else
    {
        ae_iaedeltaBV = 0;
        LOG_DEBUG("[WDR AE]: notuse midtonegain");
    }
		


    ae_nextBV =  ae_imageBV + ae_deltaBV;
    if ( ae_nextBV < _ALAE_MIN_BV)
        ae_nextBV = _ALAE_MIN_BV;

    LOG_DEBUG("\n before smooth ae_nextBV is %d \n", ae_nextBV);

    ae_nextBV = alAELib_CalculateProgressiveAE(ae_preBV, ae_nextBV, ucConverge_SPD, ae_obj);

    LOG_DEBUG("\n after smooth ae_nextBV is %d, %d %d %d\n", ae_nextBV, convBVCnt, prevYmeanBV, wYmean2BV);


    if (alAELib_Abs(ae_imageBV - prevYmeanBV) == 0 && convBVCnt < 5)
    {
        convBVCnt++;
    }
    else if (alAELib_Abs(ae_imageBV - prevYmeanBV) != 0)
    {
        convBVCnt = 0;
    }

    if ((alAELib_Abs(udAE_Targetmean - ae_mean_8bit) < 300 ||  convBVCnt >= 5))
    {
        ae_obj->lib_runtime_st.ucIsConverged = 1;
    }
    //else
    else if (ae_obj->lib_runtime_st.ucIsConverged == 1 && alAELib_Abs(udAE_Targetmean - ae_mean_8bit) > 400)
    {
        convBVCnt = 0;
        ae_obj->lib_runtime_st.ucIsConverged = 0;
        ae_obj->ae_runtime_data.uciAE_AEconverge_cnt = 0;
    }
    prevYmeanBV = ae_imageBV;
    ae_obj->ae_current_output_dat.bvresult = ae_nextBV;
    ae_obj->ae_runtime_data.dTargetAllcompBV = ae_targetAllcompBV;
    ae_obj->ae_runtime_data.wDeltaBV = ae_deltaBV;
	ae_obj->ae_runtime_data.wiAEDeltaBV = ae_iaedeltaBV;
    return errRet;
}

UINT32 gazeAE(void* ae_runtime_dat)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ae_runtime_dat;
    UINT16 roi_num = ae_obj->ae_current_set_param.para.ae_set_object_roi_setting.roi_count;
    UINT16 win1_weight = 4;
    UINT32 win2_weight[AL_MAX_ROI_NUM] = { 0 };
    UINT8 st_block[AL_MAX_ROI_NUM] = { 0 };
    UINT8 st_bank[AL_MAX_ROI_NUM] = { 0 };
    UINT8 end_block[AL_MAX_ROI_NUM] = { 0 };
    UINT8 end_bank[AL_MAX_ROI_NUM] = { 0 };
    for (int i = 0; i < roi_num; i++)
    {
        st_block[i] = floor(ae_obj->ae_current_set_param.para.ae_set_object_roi_setting.roi[i].roi.left / (float)ae_obj->ae_current_set_param.para.ae_set_object_roi_setting.ref_frame_width * ae_obj->ae_tuning_para.ucStatsBlockNum);
        st_bank[i] = floor(ae_obj->ae_current_set_param.para.ae_set_object_roi_setting.roi[i].roi.top / (float)ae_obj->ae_current_set_param.para.ae_set_object_roi_setting.ref_frame_height * ae_obj->ae_tuning_para.ucStatsBankNum);       
        end_block[i] = floor(ae_obj->ae_current_set_param.para.ae_set_object_roi_setting.roi[i].roi.w / (float)ae_obj->ae_current_set_param.para.ae_set_object_roi_setting.ref_frame_width * ae_obj->ae_tuning_para.ucStatsBlockNum);
        end_bank[i] = floor(ae_obj->ae_current_set_param.para.ae_set_object_roi_setting.roi[i].roi.h / (float)ae_obj->ae_current_set_param.para.ae_set_object_roi_setting.ref_frame_height * ae_obj->ae_tuning_para.ucStatsBankNum);
        win2_weight[i] = ae_obj->ae_current_set_param.para.ae_set_object_roi_setting.roi[i].weight;
    }
    ae_metering_core_input gaze_ae_metering_input[AL_MAX_ROI_NUM];
    ae_metering_core_output gaze_ae_metering_output[AL_MAX_ROI_NUM];

    struct al3awrapper_stats_ae_t* gaze_ae_stats_data = &gaze_ae_runtime_dat.ae_dispatched_data;


    for (int i = 0; i < roi_num; i++)
    {
        // weight table
        gaze_ae_runtime_dat.ae_tuning_para.ucStatsBlockNum = end_block[i];
        gaze_ae_runtime_dat.ae_tuning_para.ucStatsBankNum = end_bank[i];
        gaze_ae_runtime_dat.ae_tuning_para.ucWeightTable_user_weight = 100.0f / win2_weight[i] * 3.3f;
        updateWeightTable(&gaze_ae_runtime_dat);

        //cal Y of gaze roi
        gaze_ae_stats_data->ucstatsdepth = 10;
        gaze_ae_stats_data->ucvalidblocks = end_block[i];
        gaze_ae_stats_data->ucvalidbanks = end_bank[i];
        for (int h = 0; h < end_bank[i] * end_block[i]; h++)
        {
            UINT8 ic = h / end_block[i];
            UINT8 ir = h % end_block[i];
            gaze_ae_stats_data->statsy[h] = ae_obj->ae_dispatched_data.statsy[st_block[i] + ir + (ic + st_bank[i]) * ae_obj->ae_tuning_para.ucStatsBlockNum];
        }

        gaze_ae_metering_input[i].ucStatsDepth = gaze_ae_stats_data->ucstatsdepth;
        gaze_ae_metering_input[i].pudstatsY = gaze_ae_stats_data->statsy;
        gaze_ae_metering_input[i].ucWidthBlockNum = gaze_ae_stats_data->ucvalidblocks;
        gaze_ae_metering_input[i].ucHeightBlockNum = gaze_ae_stats_data->ucvalidbanks;
        gaze_ae_metering_input[i].puwWeightTable = gaze_ae_runtime_dat.ae_tuning_para.weight_table.user_def;
        gaze_ae_metering_input[i].pucYWeightTable = ae_obj->ae_tuning_para.aucYWeightTable;

        errRet = alAELib_Core_AEMetering(&gaze_ae_metering_input[i], &gaze_ae_metering_output[i]);
        printf("roi Ymean: %d \n",gaze_ae_metering_output->udYMean_8bit);
        gaze_ae_runtime_dat.ae_runtime_data.udAEMean_8bit = gaze_ae_metering_output->udYMean_8bit;
    }
    //ae_obj->ae_runtime_data.udAEMean_8bit = gaze_ae_runtime_dat.ae_runtime_data.udAEMean_8bit;

    return errRet;
}

void alAE_Simple_AE_Metering(ae_corelib_obj_t *ae_obj, struct ae_output_data_t *ae_output, struct al3awrapper_stats_ae_t *ae_stats_data)
{
    UINT16 *ae_stats_Y = ae_stats_data->statsy;
    UINT16 i = 0;
    UINT32 NonWYmean = 0, udAE_Targetmean;
    UINT8 ucvalidBlock = ae_stats_data->ucvalidblocks;
    UINT8 ucvalidBank = ae_stats_data->ucvalidbanks;
    UINT16 uwTotalBlock = ucvalidBlock * ucvalidBank;
    INT32 ae_bv = 0, ae_targetBV = 0, ae_deltaBV = 0, ae_nextBV = 0;
    INT32 ae_imageBV = ae_obj->ae_runtime_data.dImageBV;

    if ( uwTotalBlock <= 0 )  // if value is not valid, refuse to update normal result
        ae_output->bg_bvresult = 0;
    else
    {
        for( i=0; i<uwTotalBlock; i++ )
            NonWYmean +=ae_stats_Y[i];

        if ( uwTotalBlock == 0 )
            NonWYmean = 0;  /* avoid div0  */
        else
            NonWYmean = (UINT32)(NonWYmean * 100 / uwTotalBlock) >> 2;  // *100:scale 100, >>2:10bit to 8bit

        ae_bv =(INT32)(1000 * alAELib_Log2_65536((NonWYmean << 8) / 100)) >> 16;  // <<8: scale 256, /100:scale100

        //ae_targetBV =(INT32)(1000 * alAELib_Log2_65536(9792)) >> 16;   // 8 bits target: 38.25 * 256, fixed value to prevent exposure influence
        
        // using modify target by imageBv
        udAE_Targetmean = ae_obj->ae_tuning_para.udTargetmean;

        if (0 != ae_obj->ae_current_set_param.para.manual_ae_target)
            udAE_Targetmean = ae_obj->ae_current_set_param.para.manual_ae_target;

        ae_targetBV = (INT32)(1000 * alAELib_Log2_65536((udAE_Targetmean << 8) / 100)) >> 16;   // 8 bits target: target * 256, fixed value to prevent exposure influence

        ae_deltaBV = ae_bv - ae_targetBV;
        //LOG_INFO("AE deltaBV is %d", ae_deltaBV);

        ae_nextBV = ae_imageBV + ae_deltaBV;
        if (ae_nextBV < _ALAE_MIN_BV)
            ae_nextBV = _ALAE_MIN_BV;

        // update value to report
        ae_output->bg_bvresult = ae_nextBV;
        ae_obj->ae_runtime_data.uwbgAEMean_8bit = (UINT16)NonWYmean;
        LOG_DEBUG("uwbgAEMean_8bit: %d", NonWYmean);
    }
}

UINT32 alAELib_generate_metering_table(ae_corelib_obj_t *ae_obj, UINT8 *metering_table)
{
    UINT32 err = _AL_3ALIB_SUCCESS;

    if (ae_obj == NULL || metering_table == NULL)
        return _AL_AELIB_INVALID_ADDR;

    switch (ae_obj->ae_current_set_param.para.ae_metering_mode)
    {
        case AE_METERING_AVERAGE:
            metering_table = ae_obj->ae_tuning_para.weight_table.average_table0;
            if (ae_obj->ae_tuning_para.multiCamera_para.multiCamMode == AE_MULTICAM_MODE_WEIGHTSTATS_SLAVEBYSELF)
            {
                alAELib_Core_calMeteringRatio(ae_obj, &metering_table);
            }
            break;
        case AE_METERING_USERDEF_WT:
            metering_table = ae_obj->ae_tuning_para.weight_table.user_def;
            break;
        default:
            return _AL_AELIB_INVALID_PARAM;
    }

    // update final address to runtime buffer
    ae_obj->ae_runtime_data.metering_table = metering_table;

    return err;
}

UINT32 alAELib_Core_AEMetering(void *input, void *output)
{
    UINT32 ret = _AL_3ALIB_SUCCESS;
    UINT16 ic = 0;
    UINT32 udYmean_8bit = 0, udTabCount = 0;
    UINT64 ulSum = 0;

    // checking parameter validity
    if ( input == NULL || output == NULL)
        return _AL_AELIB_INVALID_ADDR;

    ae_metering_core_input *localBuf = (ae_metering_core_input *)input;
    ae_metering_core_output *metering_output = (ae_metering_core_output *)output;
    UINT8 ucWidth = localBuf->ucWidthBlockNum;
    UINT8 ucHeight = localBuf->ucHeightBlockNum;
    UINT16 uwTotalBlock = ucWidth * ucHeight;
    UINT16 *puwBlockY = localBuf->pudstatsY;
    UINT8 *puwWeightTable = localBuf->puwWeightTable;
    UINT8 *pucYWeightTable = localBuf->pucYWeightTable;
    SINT8 cShiftBit = (SINT8)localBuf->ucStatsDepth - AE_METERINGPROCESS_BITDEPTH;
    UINT16 clamp_value = (1 << localBuf->ucStatsDepth) - 1;
    UINT16 blockY_8bit = 0; /* for lookup value in Y weighting table */

    if ( uwTotalBlock == 0 || ucWidth == 0 || ucHeight == 0 )
        return _AL_AELIB_INVALID_PARAM;

    if (cShiftBit >= 0)
    {
        for (ic = 0; ic < uwTotalBlock; ic++)
        {
            blockY_8bit = (puwBlockY[ic] >= clamp_value ? clamp_value : puwBlockY[ic]) >> cShiftBit;
            ulSum += ((UINT64)puwBlockY[ic] * (UINT64)puwWeightTable[ic] * (UINT64)pucYWeightTable[blockY_8bit]);
            udTabCount += ((UINT32)puwWeightTable[ic] * (UINT32)pucYWeightTable[blockY_8bit]);
        }
    } else
    {
        for (ic = 0; ic < uwTotalBlock; ic++)
        {
            blockY_8bit = (puwBlockY[ic] >= clamp_value ? clamp_value : puwBlockY[ic]) << alAELib_Abs(cShiftBit);
            ulSum += ((UINT64)puwBlockY[ic] * (UINT64)puwWeightTable[ic] * (UINT64)pucYWeightTable[blockY_8bit]);
            udTabCount += ((UINT32)puwWeightTable[ic] * (UINT32)pucYWeightTable[blockY_8bit]);
        }
    }
    
    if( udTabCount == 0)
        udYmean_8bit = 0;
    else
    {
        if(cShiftBit >= 0)
            udYmean_8bit = (UINT32)((ulSum * 100 / udTabCount)) >> cShiftBit;  // *100:scale 100
        else
            udYmean_8bit = (UINT32)((ulSum * 100 / udTabCount)) << alAELib_Abs(cShiftBit);  // *100:scale 100
    }

    //Update output
    metering_output->udYMean_8bit = udYmean_8bit;

    return ret;
}

// AE algorithm - Generate exposure parameter
UINT32 alAELib_GetExposureParameter(ae_corelib_obj_t *ptAEBuffer, al_ae_exp_param_t *ae_output, INT16 wBv1000)
{
    UINT32 err = _AL_3ALIB_SUCCESS;
    UINT32 udManualExpTime, udManualISO = 0, udManualADGain, udISOFactor, udset_min_fps;
    UINT32 udHWMaxExpTime, udHWMinExpTime, udHWMaxADgain, udHWMinADgain, udHWMaxISO, udHWMinISO, udMaxExpoTime_FromMinFPS;
    UINT32 udSWMaxExpTime, udSWMinExpTime;
    UINT32  udISOSpeed = 0, udExpTime = 0, udModExpTime = 0, udModISO = 0;
    ae_set_param_data_t* buffer = &ptAEBuffer->ae_current_set_param;
    enum ae_antiflicker_mode_t antibanding_mode;
    UINT32 udFn = 0;
    ae_expo_limit  intersection_range;
    UINT16 udMaxExpLine = ptAEBuffer->ae_shadow_set_param.para.preview_sensor_info.max_line_cnt;
    UINT16 udMinExpLine = ptAEBuffer->ae_shadow_set_param.para.preview_sensor_info.min_line_cnt;
    UINT32 udExpTimePerLine = ptAEBuffer->ae_shadow_set_param.para.preview_sensor_info.exposuretime_per_exp_line_ns;    //unit: ns
    float fTimeusPerLine = 0;
    
    if ( udMinExpLine <= 1 )    udMinExpLine = 1;
    if ( udMaxExpLine <= 1 )    udMaxExpLine = 1;
    udManualExpTime = buffer->para.manual_exptime;
    udManualADGain = buffer->para.manual_adgain;
    err = TranslateISOLevel( buffer->para.manual_isolevel, &udManualISO);
	if(ptAEBuffer->ae_tuning_para.ucAntiFlickerMode < ANTIFLICKER_MAX)	// manual
		antibanding_mode = (enum ae_antiflicker_mode_t)ptAEBuffer->ae_tuning_para.ucAntiFlickerMode;
	else
	    antibanding_mode = buffer->para.afd_flicker_mode;    // auto or set from FW

    udHWMaxADgain = buffer->para.preview_sensor_info.max_gain;
    udHWMinADgain = buffer->para.preview_sensor_info.min_gain;
    udset_min_fps = buffer->para.preview_sensor_info.min_fps;

    if ( udHWMinADgain <= 100 )
        udHWMinADgain = DEFAULT_MIN_GAIN;
    if ( udHWMaxADgain <= 100 )
        udHWMaxADgain = DEFAULT_MAX_GAIN;

    /* loading sensor/HW limitation setting */
    // first load Fn numbers
    udFn = ptAEBuffer->curHWSetting.FN_x1000;

    /* calculate FPS information */
    if ( udset_min_fps == 0 )
        udMaxExpoTime_FromMinFPS = 100000; //default 100 ms, 10 fps
    else
        udMaxExpoTime_FromMinFPS = 100000000 / udset_min_fps;  //100000000 = 1000000 * 100, *100:fps scale100
    udHWMaxExpTime = udMaxExpoTime_FromMinFPS;
    
    if (udExpTimePerLine == 0)
    {
        if ( udset_min_fps != 0 && udMaxExpLine != 0 )
            fTimeusPerLine = (float)(1000000000) / udset_min_fps / udMaxExpLine*100 / 1000;
        else
            fTimeusPerLine = _DEFAULT_TIME_PER_LINE;
    }
    else
        fTimeusPerLine = (float)udExpTimePerLine/1000.0f;    // convert ns to us unit
    
    if (fTimeusPerLine < 0.001)
        fTimeusPerLine = _DEFAULT_TIME_PER_LINE;

    udHWMinExpTime = (UINT32)(fTimeusPerLine * udMinExpLine);
    udHWMaxExpTime = min(udHWMaxExpTime, (UINT32)(fTimeusPerLine * udMaxExpLine));

    if ( ptAEBuffer->calibration_data.calib_g_gain != 0 )
        udISOFactor = ptAEBuffer->calibration_data.calib_g_gain;
    else
        udISOFactor = 1000;  // use 1000 as calibration G gain

    udHWMaxISO = ( udHWMaxADgain*1000 + (udISOFactor>>1) ) / udISOFactor;
    udHWMinISO = ( udHWMinADgain*1000 + (udISOFactor>>1) ) / udISOFactor;
    // check if min iso from calibration data is valid
    if (ptAEBuffer->calibration_data.min_iso >= udHWMinISO)
        udHWMinISO = ptAEBuffer->calibration_data.min_iso;

    udSWMaxExpTime = buffer->para.preview_sensor_info.sw_max_exptime;
	udSWMinExpTime = buffer->para.preview_sensor_info.sw_min_exptime;

    //Check Max Exp-time range
    if(udHWMaxExpTime >= udSWMaxExpTime)
        intersection_range.udETMax = udSWMaxExpTime;
    else if(udHWMaxExpTime < udSWMaxExpTime)
        intersection_range.udETMax = udHWMaxExpTime;
    //Check Min Exp-time range
    if(udHWMinExpTime <= udSWMinExpTime)
        intersection_range.udETMin = udSWMinExpTime;
    else if(udHWMinExpTime > udSWMinExpTime)
        intersection_range.udETMin = udHWMinExpTime;
    //Check ISO limitation
    intersection_range.udISOMax = udHWMaxISO;
    intersection_range.udISOMin = udHWMinISO;

    //Update Pcurve maximum setting
    ptAEBuffer->ae_runtime_data.udPcurveMaxExpTime = intersection_range.udETMax;
    ptAEBuffer->ae_runtime_data.udPcurveMinExpTime = intersection_range.udETMin;
    ptAEBuffer->ae_runtime_data.udPcurveMaxIso = intersection_range.udISOMax;

    LOG_DEBUG("configinfo ExpTimePerLine %u  exprange iso [%u, %u]  time [%u, %u]", udExpTimePerLine, intersection_range.udISOMin, intersection_range.udISOMax, intersection_range.udETMin, intersection_range.udETMax);

    if( udManualADGain != 0 )   /* manual gain has priority over manual iso */
        udManualISO = (udManualADGain*1000 + (udISOFactor>>1) ) /udISOFactor;

    if ( udManualExpTime != 0 && udManualISO != 0 )
    {
        if(udManualExpTime > udHWMaxExpTime)
            udManualExpTime = udHWMaxExpTime;
        if(udManualExpTime < udHWMinExpTime)
            udManualExpTime = udHWMinExpTime;
        if (udManualISO > udHWMaxISO)
            udManualISO = udHWMaxISO;
        if (udManualISO < udHWMinISO)
            udManualISO = udHWMinISO;

        udExpTime = udManualExpTime;
        udISOSpeed = udManualISO;
    }
    else if ( udManualExpTime != 0 )
    {
        if(udManualExpTime > udHWMaxExpTime)
            udManualExpTime = udHWMaxExpTime;
        if(udManualExpTime < udHWMinExpTime)
            udManualExpTime = udHWMinExpTime;

        udExpTime = udManualExpTime;
        udISOSpeed = alAELib_CalculateIsoSpeed(udFn, udExpTime, wBv1000);

        if (udISOSpeed > udHWMaxISO)
            udISOSpeed = udHWMaxISO;
        if (udISOSpeed < udHWMinISO)
            udISOSpeed = udHWMinISO;
    }
    else if ( udManualISO != 0 )
    {
        if (udManualISO > udHWMaxISO)
            udManualISO = udHWMaxISO;
        if (udManualISO < udHWMinISO)
            udManualISO = udHWMinISO;

        udISOSpeed = udManualISO;
        udExpTime = alAELib_CalculateExposureTime(udFn, wBv1000, udISOSpeed);  //us

        if (udExpTime > udHWMaxExpTime)
            udExpTime = udHWMaxExpTime;
        if (udExpTime < udHWMinExpTime)
            udExpTime = udHWMinExpTime;
    }
    else
    {
        LOG_DEBUG("ae_expo_mode is %02d", ptAEBuffer->ae_current_set_param.para.ae_expo_mode);
        int aemode = AETABLE_CAPTURE_AUTO;

        //table
        switch (ptAEBuffer->ae_current_set_param.para.ae_expo_mode)  //modify expo table or input mode
        {

        case AE_EXPO_ISO_PRIOR:
            udExpTime = intersection_range.udETMin + (intersection_range.udETMax - intersection_range.udETMin) / 2;  // mean or a fixed value
            udISOSpeed = alAELib_CalculateIsoSpeed(udFn, udExpTime, wBv1000);
            break;

        case AE_EXPO_TIME_PRIOR:
            udISOSpeed = intersection_range.udISOMin;
            udExpTime = alAELib_CalculateExposureTime(udFn, wBv1000, udISOSpeed);
            break;

        case AE_EXPO_NORMAL:
            aemode = AETABLE_CAPTURE_AUTO;
            //todo:auto switch aemode
            matchExpoTable(ptAEBuffer, ae_output, wBv1000, aemode);
            udISOSpeed = ae_output->udISO;
            udExpTime = ae_output->udexposure_time;
            break;

        case AE_EXPO_NIGHT:
            aemode = AETABLE_CAPTURE_NIGHT;
            matchExpoTable(ptAEBuffer, ae_output, wBv1000, aemode);
            udISOSpeed = ae_output->udISO;
            udExpTime = ae_output->udexposure_time;
            break;

        case AE_EXPO_USERDEF:

        default:  //time prior
            udISOSpeed = intersection_range.udISOMin;
            udExpTime = alAELib_CalculateExposureTime(udFn, wBv1000, udISOSpeed);
            break;
        }
        //check limit
        alAELib_Core_CheckLimit(&udExpTime, &udISOSpeed, NULL, intersection_range);

    }
    // check long exposure with antiflicker mode design
    udModExpTime = udExpTime;
    udModISO = udISOSpeed;
    // antiflicker mod time, if manual not ISO speed, manual exposure time
    if (antibanding_mode != ANTIFLICKER_OFF)
    {
        if (ptAEBuffer->ae_tuning_para.ucFlickerPriorityMode != 0)
        {
            if ( udManualExpTime == 0 )
                alAELib_ModExposureTimeADGainByAntiflikerMode( &udModExpTime, &udModISO, antibanding_mode, intersection_range, NULL );
        }
        else
        {
            if ( udManualExpTime ==  0 && udManualISO == 0)
                alAELib_ModExposureTimeADGainByAntiflikerMode( &udModExpTime, &udModISO, antibanding_mode, intersection_range, NULL );
        }
    }
    // fill exposure parameter
    ae_output->udexposure_time = udModExpTime;
    ae_output->udISO = udModISO;

    return err;
}

UINT32 ManageInputParam(void *ae_runtime_dat)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    ae_corelib_obj_t *ae_obj = (ae_corelib_obj_t *)ae_runtime_dat;

    // copy buffer for test phase, here should parsing data seperatedly
    memcpy(&ae_obj->ae_current_set_param,  &ae_obj->ae_shadow_set_param, sizeof(ae_set_param_data_t));
    // reset command flag in shadow parameter
    memset(&ae_obj->ae_shadow_set_param.commandFlg[0], 0, sizeof ( UINT8) * AE_SET_PARAM_MAX);

    // parsing status according to command
	errRet = ParsingLibScenarioStatus(&ae_obj->ae_current_set_param, &ae_obj->lib_runtime_st, ae_runtime_dat);

    return errRet;
}

UINT32 updateWeightTable(void* ae_runtime_dat)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ae_runtime_dat;

    UINT16 blockNum_W = ae_obj->ae_tuning_para.ucStatsBlockNum;
    UINT16 blockNum_H = ae_obj->ae_tuning_para.ucStatsBankNum;

    double guass[AE_MAX_STATS_BANK][AE_MAX_STATS_BLOCK] = { 0.0 };
    UINT8 weight[AE_MAX_STATS_BANK * AE_MAX_STATS_BLOCK] = { 0 };
    double sum = 0;
    double x2 = 0;
    double y2 = 0;
    double center_x = (blockNum_W - 1) / 2.0;
    double center_y = (blockNum_H - 1) / 2.0;
    double sigma = ae_obj->ae_tuning_para.ucWeightTable_user_weight;

    for (int i = 0; i < blockNum_W; i++)
    {//使用x2，y2降低了运算速度，提高了程序的效率
        x2 = pow((i - center_x), 2);
        for (int j = 0; j < blockNum_H; j++)
        {
            y2 = pow((j - center_y), 2);
            sum += guass[j][i] = exp(-(float)sqrt(x2 + y2) / (2 * sigma * sigma));
            //weight[j * blockNum_W + i] = guass[j][i];
        }
    }

    if (sum != 0)
    {
        //归一化  
        for (int i = 0; i < blockNum_W; i++)
        {
            for (int j = 0; j < blockNum_H; j++)
            {
                weight[j * blockNum_W + i]  = (UINT8) (guass[j][i] / guass[0][0]);
            }
        }
    }

    for (int i = 0; i < blockNum_W; i++)
    {
        for (int j = 0; j < blockNum_H; j++)
        {
            if (i <= (int)(blockNum_W * 3.0f / 16 + 0.5f))
            {
                ae_obj->ae_tuning_para.weight_table.multiCameraleft[j * blockNum_H + i] = 0;
            }
            else
            {
                ae_obj->ae_tuning_para.weight_table.multiCameraleft[j * blockNum_H + i] = 1;
            }

            if (i >= (blockNum_W - (int)(blockNum_W * 3.0f / 16 + 0.5f)))
            {
                ae_obj->ae_tuning_para.weight_table.multiCameraright[j * blockNum_H + i] = 0;
            }
            else
            {
                ae_obj->ae_tuning_para.weight_table.multiCameraright[j * blockNum_H + i] = 1;
            }
        }
    }

    memset(ae_obj->ae_tuning_para.weight_table.average_table0, 1, blockNum_W * blockNum_H);    // default 16x16 table, sync with statistics block&bank number
    memcpy(ae_obj->ae_tuning_para.weight_table.user_def, weight, AE_MAX_STATS_BANK * AE_MAX_STATS_BLOCK);    // default 16x16 table, sync with statistics block&bank number

    return errRet;
}

UINT32 ParsingLibScenarioStatus(ae_set_param_data_t *set_param, ae_corelib_libstatus_t *output_st, void *ae_runtime_dat)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;
    UINT16 i = 0;
    ae_corelib_obj_t *ae_obj = (ae_corelib_obj_t *)ae_runtime_dat;

    for (i = 0; i < AE_SET_PARAM_MAX; i++)
    {
        if (set_param->commandFlg[i] != 0)
        {
            switch ( i )
            {
                case AE_SET_PARAM_ENABLE:
                    output_st->ucIsAEEnable = set_param->para.ae_enable;
                    ae_obj->lib_runtime_st.ucIsAEEnable = set_param->para.ae_enable;
                    break;
                case AE_SET_PARAM_CONVERGE_SPD:
                    output_st->ae_progressive_level = set_param->para.converge_speedlv;
                    break;
                default:
                    break;
            }
        }
    }
    return errRet;
}

UINT32 TranslateISOLevel(enum ae_iso_mode_t isolevel, UINT32 *udISOSpeed)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;

    if ( isolevel >=  AE_ISO_MAX )
        return _AL_AELIB_INVALID_ISOLEVEL;

    switch ( isolevel )
    {
        case AE_ISO_AUTO:
            *udISOSpeed = 0;
            break;
        case AE_ISO_100:
            *udISOSpeed = 100;
            break;
        case AE_ISO_200:
            *udISOSpeed = 200;
            break;
        case AE_ISO_400:
            *udISOSpeed = 400;
            break;
        case AE_ISO_800:
            *udISOSpeed = 800;
            break;
        case AE_ISO_1600:
            *udISOSpeed = 1600;
            break;
        case AE_ISO_3200:
            *udISOSpeed = 3200;
            break;
        case AE_ISO_6400:
            *udISOSpeed = 6400;
            break;
        case AE_ISO_12800:
            *udISOSpeed = 12800;
            break;
        default:
            *udISOSpeed = 0;
            break;
    }
    return errRet;
}
UINT32 LoadDefaultAESetting(void *ae_obj);
UINT32 LoadDefaultSetting(void *ae_obj)
{
    UINT32 err = _AL_3ALIB_SUCCESS;

    if ( ae_obj == NULL )
        return _AL_AELIB_INVALID_ADDR;

    if ( LoadDefaultAESetting( ae_obj ) != _AL_3ALIB_SUCCESS )
        return _AL_AELIB_FAIL_INIT_AE_SET;

    return err;
}



// This would create default buffer setting when calling inital API
UINT32 LoadDefaultAESetting(void *ae_obj)
{
    UINT32 err = _AL_3ALIB_SUCCESS;
    ae_corelib_obj_t *localBuf;
    UINT8 index = 0;

    if ( ae_obj == NULL )
        return _AL_AELIB_INVALID_ADDR;

    localBuf = (ae_corelib_obj_t *)ae_obj;

    UINT8 local_weight_table_avg[256] = AE_METERING_TABLE_AVERAGE_16x16;
    UINT8 local_weight_table_user[256] = AE_METERING_TABLE_USER_DEFINE_16x16;
    UINT8 local_multicamera_left_weight_table[256] = AE_METERING_TABLE_MULTICAMERA_LEFT_16x16;
    INT8 local_multicamera_right_weight_table[256] = AE_METERING_TABLE_MULTICAMERA_RIGHT_16x16;
    UINT8 YweightTable[MAX_AE_YWIGHT_TABLE_LENGTH] = AE_Y_WEIGHT_TABLE;
    UINT8 iae_hist_highlight_target[MAX_IAE_PARA_SWITCH_CASE] = AE_WDR_HIST_HIGHLIGHT_TARGET;
    UINT8 iae_hist_shadows_target[MAX_IAE_PARA_SWITCH_CASE] = AE_WDR_HIST_SHADOWS_TARGET;
    UINT8 iae_hist_highlight_percent_range[MAX_IAE_PARA_SWITCH_CASE] = AE_WDR_HIST_HIGHLIGHT_PERCENT_RANGE;
    UINT8 iae_hist_shadows_percent_range[MAX_IAE_PARA_SWITCH_CASE] = AE_WDR_HIST_SHADOWS_PERCENT_RANGE;
    UINT16 iae_max_midtones_gain[MAX_IAE_PARA_SWITCH_CASE] = AE_WDR_MAX_MIDTONES_GAIN;
    UINT16 iae_max_shadows_gain[MAX_IAE_PARA_SWITCH_CASE] = AE_WDR_MAX_SHADOWS_GAIN;
    UINT16 iae_bv_th[MAX_IAE_BVTH_COUNT] = AE_WDR_EXPINDEX_TH;
    UINT16 iae_hist_ratio[MAX_IAE_HISTRATIO_COUNT] = AE_WDR_HIST_RATIO;

    preWdrFlag = false;
    // Calibration default setting
    localBuf->calibration_data.min_iso = _DEFAULT_AE_CALIB_MIN_ISO;
    localBuf->calibration_data.calib_r_gain = _DEFAULT_AE_CALIB_R_GAIN;
    localBuf->calibration_data.calib_g_gain = _DEFAULT_AE_CALIB_G_GAIN;    // this would impact ISO <--> AD gain
    localBuf->calibration_data.calib_b_gain = _DEFAULT_AE_CALIB_B_GAIN;

    // Initial setting
    localBuf->ae_initial_param.iso = INIT_AE_ISO;
    localBuf->ae_initial_param.adgain  = INIT_AE_ADGAIN;
    localBuf->ae_initial_param.BV = INIT_AE_BV;
    localBuf->ae_initial_param.exposure_time = INIT_AE_EXPOSURE;
    localBuf->ae_initial_param.midtones_gain = INIT_AE_MIDTONES_GAIN;
    prevYmeanBV = INIT_AE_BV;
    // copy initial setting to preview report
    localBuf->ae_current_output_dat.udexposure_time = localBuf->ae_initial_param.exposure_time;
    localBuf->ae_current_output_dat.udsensor_ad_gain = localBuf->ae_initial_param.adgain;
    localBuf->ae_current_output_dat.iso = ((UINT64)localBuf->ae_initial_param.adgain)*1000 /( localBuf->calibration_data.calib_g_gain);
    localBuf->ae_current_output_dat.bvresult = localBuf->ae_initial_param.BV;
    localBuf->ae_current_output_dat.bg_bvresult = 0;
    localBuf->ae_current_output_dat.ae_converged = 0;
    localBuf->ae_current_output_dat.midtones_gain = 100;
    localBuf->ae_current_output_dat.shadows_gain = 100;
    localBuf->ae_current_output_dat.isp_midtones_gain = 100;

    // lib status initial
    localBuf->lib_runtime_st.uwCountingIdx = 0;
    localBuf->lib_runtime_st.ucIsConverged = 0;
    localBuf->lib_runtime_st.ucIsAEEnable = TRUE;
    localBuf->lib_runtime_st.ae_progressive_level = AE_SPEED_MODE;
    
    localBuf->ae_speed_param.fbased_speed = AE_SPEED_FBASED_SPEED;
    localBuf->ae_speed_param.fmax_step = AE_SPEED_FMAX_STEP;
    localBuf->ae_speed_param.normal_speed_level = AE_SPEED_NORMAL;
    localBuf->ae_speed_param.direct_speed_level = AE_SPEED_DIRECT;
    localBuf->ae_speed_param.slow_speed_level = AE_SPEED_SLOW;
    localBuf->ae_speed_param.smooth_speed_level = AE_SPEED_SMOOTH;
    localBuf->ae_speed_param.fast_speed_level = AE_SPEED_FAST;
    
    // HW setting
    localBuf->curHWSetting.FN_x1000 = DEFAULT_FN;

    // User tuning parameters
    localBuf->ae_tuning_para.udTargetmean = AE_TARGET_MEAN;
    localBuf->ae_tuning_para.ucFlickerPriorityMode = AE_FLICKER_PRIORITY;
    localBuf->ae_tuning_para.ucStatsBlockNum = 16;
    localBuf->ae_tuning_para.ucStatsBankNum = 16;
    localBuf->ae_tuning_para.ucWeightTable_user_weight = 1.3f;

    // assign 16x16 table
    memset(&localBuf->ae_tuning_para.weight_table, 0, sizeof(ae_core_metering_table));
    memcpy(localBuf->ae_tuning_para.weight_table.average_table0, local_weight_table_avg, 256);    // default 16x16 table, sync with statistics block&bank number
    memcpy(localBuf->ae_tuning_para.weight_table.user_def, local_weight_table_user, 256);    // default 16x16 table, sync with statistics block&bank number
    memcpy(localBuf->ae_tuning_para.weight_table.multiCameraleft, local_multicamera_left_weight_table, 256);
    memcpy(localBuf->ae_tuning_para.weight_table.multiCameraright, local_multicamera_right_weight_table, 256);
    memcpy(localBuf->ae_tuning_para.aucYWeightTable, YweightTable, MAX_AE_YWIGHT_TABLE_LENGTH);
    localBuf->ae_runtime_data.metering_table = localBuf->ae_tuning_para.weight_table.average_table0;

    localBuf->ae_tuning_para.iAE_para.ucIsEnableiAE = AE_WDR_ENABLE;
	localBuf->ae_runtime_data.uciAE_AEconverge_cnt = 0;

    memcpy(localBuf->ae_tuning_para.iAE_para.iaeluxIdxTh, iae_bv_th, sizeof(iae_bv_th));
    memcpy(localBuf->ae_tuning_para.iAE_para.iaeHistRatio, iae_hist_ratio, sizeof(iae_hist_ratio));
    for (index = 0; index < MAX_IAE_PARA_SWITCH_CASE; index++)
    {
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_dark_high = iae_hist_shadows_target[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_dark_low = iae_hist_shadows_target[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_high = iae_hist_highlight_target[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_low = iae_hist_highlight_target[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_dark_range_end[0] = iae_hist_shadows_percent_range[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_dark_range_end[1] = iae_hist_shadows_percent_range[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_dark_range_end[2] = iae_hist_shadows_percent_range[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_dark_range_start[0] = 0;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_dark_range_start[1] = 0;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_dark_range_start[2] = 0;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushdown_range_end[0] = iae_hist_highlight_percent_range[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushdown_range_end[1] = iae_hist_highlight_percent_range[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushdown_range_end[2] = iae_hist_highlight_percent_range[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushdown_range_start[0] = 0;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushdown_range_start[1] = 0;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushdown_range_start[2] = 0;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushup_range_end[0] = iae_hist_highlight_percent_range[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushup_range_end[1] = iae_hist_highlight_percent_range[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushup_range_end[2] = iae_hist_highlight_percent_range[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushup_range_start[0] = 0;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushup_range_start[1] = 0;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].hist_sat_pushup_range_start[2] = 0;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].max_adjust_ratio = 100;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].min_adjust_ratio = 100;
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].max_midtones_gain = iae_max_midtones_gain[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].max_shadows_gain = iae_max_shadows_gain[index];
        localBuf->ae_tuning_para.iAE_para.iAE_tuning_para[index].max_isp_midtones_gain = iae_max_midtones_gain[index];
    }
    localBuf->ae_tuning_para.iAE_para.WDRMode = (WDRMode)0;
    localBuf->ae_tuning_para.iAE_para.wdrTargetGainInTh = 400;
    localBuf->ae_tuning_para.iAE_para.wdrTargetGainOutTh = 110;

    localBuf->ae_tuning_para.ucAntiFlickerMode = ANTIFLICKER_MAX;
    localBuf->ae_tuning_para.multiCamera_para.multiCamMode = AE_MULTICAM_MODE_BYSELF;
    localBuf->ae_tuning_para.multiCamera_para.BVStart = -3000;
    localBuf->ae_tuning_para.multiCamera_para.BVEnd = 2000;
    localBuf->ae_tuning_para.multiCamera_para.slaveWeightStart = 1.0f;
    localBuf->ae_tuning_para.multiCamera_para.slaveWeightEnd = 0.0f;
    localBuf->ae_tuning_para.multiCamera_para.varianceStableTh = 5;
    localBuf->ae_tuning_para.multiCamera_para.ymeanStableTh = 200;
    localBuf->ae_tuning_para.multiCamera_para.stableFrame = 5;
    localBuf->ae_tuning_para.multiCamera_para.cameraMixROI[0].leftstart = 3;
    localBuf->ae_tuning_para.multiCamera_para.cameraMixROI[0].rightend = 16;
    localBuf->ae_tuning_para.multiCamera_para.cameraMixROI[0].topstart = 0;
    localBuf->ae_tuning_para.multiCamera_para.cameraMixROI[0].bottom = 16;
    localBuf->ae_tuning_para.multiCamera_para.cameraMixROI[1].leftstart = 0;
    localBuf->ae_tuning_para.multiCamera_para.cameraMixROI[1].rightend = 13;
    localBuf->ae_tuning_para.multiCamera_para.cameraMixROI[1].topstart = 0;
    localBuf->ae_tuning_para.multiCamera_para.cameraMixROI[1].bottom = 16;

    localBuf->ae_shadow_set_param.para.pipeDelay = 4;
    localBuf->ae_shadow_set_param.para.preview_sensor_info.f_number_x1000 = DEFAULT_FN;
    localBuf->ae_shadow_set_param.para.preview_sensor_info.max_fps = DEFAULT_MAX_FPS;
    localBuf->ae_shadow_set_param.para.preview_sensor_info.min_fps = DEFAULT_MIN_FPS;
    localBuf->ae_shadow_set_param.para.preview_sensor_info.max_gain = DEFAULT_MAX_GAIN;
    localBuf->ae_shadow_set_param.para.preview_sensor_info.min_gain = DEFAULT_MIN_GAIN;
    localBuf->ae_shadow_set_param.para.preview_sensor_info.max_line_cnt = DEFAULT_MAX_LINE;
    localBuf->ae_shadow_set_param.para.preview_sensor_info.min_line_cnt = DEFAULT_MIN_LINE;
	localBuf->ae_shadow_set_param.para.preview_sensor_info.min_midtones_gain = 100;
    localBuf->ae_shadow_set_param.para.preview_sensor_info.sw_max_exptime = DEFAULT_MAX_EXP_TIME;   // unit: us
    localBuf->ae_shadow_set_param.para.preview_sensor_info.sw_min_exptime = DEFAULT_MIN_EXP_TIME;   // unit: us
    localBuf->ae_shadow_set_param.para.preview_sensor_info.exposuretime_per_exp_line_ns = 5633;        // unit: ns
    localBuf->ae_shadow_set_param.para.preview_sensor_info.max_SensorGain = 1600;
    localBuf->ae_shadow_set_param.para.sof_notify_param.sys_sof_index = 0;
    localBuf->ae_shadow_set_param.para.sof_notify_param.exp_time = INIT_AE_EXPOSURE;
    localBuf->ae_shadow_set_param.para.sof_notify_param.exp_adgain = INIT_AE_ADGAIN;
	localBuf->ae_shadow_set_param.para.sof_notify_param.midtones_gain = INIT_AE_MIDTONES_GAIN;
    localBuf->ae_shadow_set_param.para.sof_notify_param.nIRFrameType = AE_FRAME_IR_OFF;
    localBuf->ae_shadow_set_param.para.sof_notify_param.current_ir_para.ir_level = 0;
    localBuf->ae_shadow_set_param.para.sof_notify_param.current_ir_para.ir_turnon_flag = FALSE;
    localBuf->ae_shadow_set_param.para.ae_initial_setting.afd_flicker_mode = ANTIFLICKER_OFF;

    localBuf->ae_shadow_set_param.para.ae_metering_mode = AE_METERING_MODE;
    localBuf->ae_shadow_set_param.para.converge_speedlv = AE_SPEED_MODE;
    localBuf->ae_shadow_set_param.para.manual_isolevel = AE_ISO_AUTO;
    localBuf->ae_shadow_set_param.para.manual_exptime = 0;
    localBuf->ae_shadow_set_param.para.manual_adgain = 0;

    
    localBuf->ae_shadow_set_param.para.manual_ev_comp = 0;
    localBuf->ae_shadow_set_param.para.ae_enable = TRUE;
    localBuf->ae_shadow_set_param.para.manual_ae_target = 0;  // scale 100 of 8 bits domain
    localBuf->ae_shadow_set_param.para.afd_flicker_mode = localBuf->ae_shadow_set_param.para.ae_initial_setting.afd_flicker_mode;
    localBuf->ae_shadow_set_param.para.ae_initial_setting.ae_debug_info_ptr = NULL;
    //localBuf->ae_shadow_set_param.para.ae_initial_setting.ae_stats_size = 8192 + AE_HWSTATS_INFO_SIZE;
    localBuf->ae_shadow_set_param.para.ae_initial_setting.ae_stats_size = AE_MAX_STATS_BLOCK * AE_MAX_STATS_BANK * 32 + AE_HWSTATS_INFO_SIZE;
    localBuf->ae_shadow_set_param.para.ae_initial_setting.external_lux_value = 0;
    localBuf->ae_shadow_set_param.para.ae_target_mode = AE_TARGET_MODE;
    localBuf->ae_shadow_set_param.para.ae_expo_mode = AE_EXPO_MODE;

    localBuf->ae_shadow_set_param.para.gaze_enable = FALSE;

    for (int i = 0; i < SCENECNT; i++)
    {
        memcpy(localBuf->ae_shadow_set_param.para.ae_target_table[i].table, G_AE_TARGET[i], sizeof(G_AE_TARGET[i]));
    }
    for (int i = 0; i < AETABLE_CNT; i++)
    {
        memcpy(localBuf->ae_shadow_set_param.para.ae_expo_table[i].table, G_EVTABLE[i], sizeof(G_EVTABLE[i]));
    }

    memcpy(&localBuf->ae_current_set_param,&localBuf->ae_shadow_set_param,sizeof(ae_set_param_data_t));

    localBuf->ae_runtime_data.uExpIndex = INIT_AE_EXPINDEX;
    localBuf->ae_runtime_data.preBV= INIT_AE_BV;
    localBuf->ae_runtime_data.isDynamic = false;
    localBuf->ae_runtime_data.convergeDiff = 0;
    localBuf->ae_runtime_data.masterID = 0;
    localBuf->ae_runtime_data.slaveID = 1;
    localBuf->ae_runtime_data.beforeWdrExpIndex = INIT_AE_EXPINDEX;
    localBuf->ae_runtime_data.minExpBV = alAELib_CalculateBv_midtones_comp(localBuf->curHWSetting.FN_x1000, localBuf->ae_shadow_set_param.para.preview_sensor_info.exposuretime_per_exp_line_ns / 1000, localBuf->ae_shadow_set_param.para.preview_sensor_info.min_gain, 100);
    localBuf->ae_runtime_data.multiCamMasterFlag = false;
    return err;
}

void alAELib_Core_CheckLimit(UINT32 *udExpTime, UINT32 *udISOSpeed, UINT32 *udConvergeStatus, ae_expo_limit range)
{
    UINT64 bufexpNum, bufminExp, bufMaxExp;
    
    //check limit is valid?
    if(range.udETMax == 0 || range.udETMin == 0 || range.udISOMin == 0 || range.udISOMax == 0)
        return;

    bufexpNum = (UINT64)(*udExpTime) * (*udISOSpeed);
    bufminExp = (UINT64)(range.udETMin) * range.udISOMin;
    bufMaxExp = (UINT64)(range.udETMax) * range.udISOMax;

    //corner case
    if (bufexpNum >= bufMaxExp)
    {
        *udExpTime = range.udETMax;
        *udISOSpeed = range.udISOMax;

        return;
    }

    if (bufexpNum <= bufminExp)
    {
        *udExpTime = range.udETMin;
        *udISOSpeed = range.udISOMin;

        return;
    }

    while (*udExpTime > range.udETMax || *udExpTime < range.udETMin || *udISOSpeed > range.udISOMax || *udISOSpeed < range.udISOMin)
    {
        if( *udExpTime > range.udETMax )
        {
            *udISOSpeed = bufexpNum / range.udETMax;
            *udExpTime = range.udETMax;
        }
        else if (*udExpTime < range.udETMin)
        {
            *udISOSpeed = bufexpNum / range.udETMin;
            *udExpTime = range.udETMin;
        }
    
        if( *udISOSpeed > range.udISOMax )
        {
            *udExpTime = bufexpNum / range.udISOMax;
            *udISOSpeed = range.udISOMax;
        }
        else if (*udISOSpeed < range.udISOMin)
        {
            *udExpTime = bufexpNum / range.udISOMin;
            *udISOSpeed = range.udISOMin;
        }
    }

    // after modification, finally check if out of range
    if( *udISOSpeed > range.udISOMax )
    {
        *udISOSpeed = range.udISOMax;
        return;
    }
    if( *udISOSpeed < range.udISOMin )
    {
        *udISOSpeed = range.udISOMin;
        return;
    }
    if( *udExpTime > range.udETMax )
    {
        *udExpTime = range.udETMax;
        return;
    }
    if( *udExpTime < range.udETMin)
    {
        *udExpTime = range.udETMin;
        return;
    }
}

UINT32 alAELib_ModExposureTimeADGainByAntiflikerMode(UINT32 *a_udExpTime, UINT32 *a_udISOspeed, enum ae_antiflicker_mode_t flickermode, ae_expo_limit tExpLimtParam, UINT32 *udConvergeStatus)
{
    UINT32 err = _AL_3ALIB_SUCCESS;
    UINT32 udBaseExpTime =0, udFlickerMaxTime;
    UINT32 udModTime =0, udModISO =0;
    UINT64 ulCurTime = *a_udExpTime, ulCurISO = *a_udISOspeed;
    UINT32 udMaxExpTime = tExpLimtParam.udETMax;
    UINT32 udMaxISO = tExpLimtParam.udISOMax, udMinISO = tExpLimtParam.udISOMin;

    if (flickermode == ANTIFLICKER_50HZ)
        udBaseExpTime = _DEFAULT_AE_50HZ_TIME_US;
    else if (flickermode == ANTIFLICKER_60HZ)
        udBaseExpTime = _DEFAULT_AE_60HZ_TIME_US;
    else if (flickermode == ANTIFLICKER_90HZ)
        udBaseExpTime = _DEFAULT_AE_90HZ_TIME_US;
    else if (flickermode == ANTIFLICKER_144HZ)
        udBaseExpTime = _DEFAULT_AE_144HZ_TIME_US;
    else if (flickermode == ANTIFLICKER_240HZ)
        udBaseExpTime = _DEFAULT_AE_240HZ_TIME_US;
    else if (flickermode == ANTIFLICKER_320HZ)
        udBaseExpTime = _DEFAULT_AE_320HZ_TIME_US;
    else if (flickermode == ANTIFLICKER_360HZ)
        udBaseExpTime = _DEFAULT_AE_360HZ_TIME_US;
    else
        return err;    // without any modification with unknown flicker hz

    // calculating exposure time based on anti-flicker
    if ( udMaxExpTime != 0 && udMaxExpTime >= udBaseExpTime )
        udFlickerMaxTime = (UINT32)(udMaxExpTime / udBaseExpTime) * udBaseExpTime;
    else
        return err;    // without any modification with invalid max exposure time

    if ( ulCurTime > udBaseExpTime )
    {
        // if exp time > 10ms(@50 Hz) or 8.33ms(@60 Hz), make exposure time fit flicker band
        udModTime = (UINT32)((ulCurTime + (udBaseExpTime>>1))/udBaseExpTime * udBaseExpTime);
        udModISO =  (ulCurTime * ulCurISO + (udModTime>>1))/udModTime;

        if (udModISO < udMinISO)
        {
            udModTime = (UINT32)((ulCurTime + (udBaseExpTime>>1))/udBaseExpTime - 1) * udBaseExpTime;
            udModISO = (ulCurTime * ulCurISO + (udModTime>>1))/udModTime;;
        }
  
        if ( udModTime > udFlickerMaxTime )
        {
            udModTime = udFlickerMaxTime;
            udModISO = (ulCurTime * ulCurISO + (udModTime>>1)) / udModTime;
        }

        while ( udModISO > udMaxISO )
        {
            udModTime = (UINT32)(udModTime / udBaseExpTime + 1) * udBaseExpTime;

            //check exceed max exposure time (after flicker mod)
            if ( udModTime > udFlickerMaxTime )
            {
                udModTime = udFlickerMaxTime;
                udModISO = (ulCurTime * ulCurISO + (udModTime>>1)) / udModTime;

                // check max ISO range
                if ( udModISO > udMaxISO )
                {
                    udModISO = udMaxISO;
                }
                break;
            }
            udModISO = (ulCurTime * ulCurISO + (udModTime>>1)) / udModTime;
        }
        *a_udExpTime = udModTime;
        *a_udISOspeed = udModISO;
    }
    else if ( ulCurTime < udBaseExpTime )
    {
        // if exp time < basetime , reduce ISO to make exposure time fit flicker band
        if (ulCurTime * ulCurISO >= (UINT64)udMinISO * udBaseExpTime)
        {
            udModTime = udBaseExpTime;
            udModISO = (ulCurTime * ulCurISO) / udModTime;
            *a_udExpTime = udModTime;
            *a_udISOspeed = udModISO;
        }
        else if (ulCurISO > udMinISO)
        {
            udModISO = ulCurISO;
            udModTime = (ulCurTime * ulCurISO) / udModISO;
            *a_udExpTime = udModTime;
            *a_udISOspeed = udModISO;
        }
    }
    return err;
}

unsigned short AELib_Log2_Table[] = {
    0,   369,   736,  1102,  1466,  1829,  2190,  2551,  2909,  3267,  3623,  3978,  4331,  4683,  5034,  5384,
    5732,  6079,  6425,  6769,  7112,  7454,  7795,  8134,  8473,  8810,  9146,  9480,  9814, 10146, 10477, 10807,
    11136, 11464, 11791, 12116, 12440, 12764, 13086, 13407, 13727, 14046, 14363, 14680, 14996, 15310, 15624, 15937,
    16248, 16559, 16868, 17177, 17484, 17791, 18096, 18401, 18704, 19007, 19308, 19609, 19909, 20207, 20505, 20802,
    21098, 21393, 21687, 21980, 22272, 22564, 22854, 23144, 23433, 23720, 24007, 24293, 24579, 24863, 25146, 25429,
    25711, 25992, 26272, 26551, 26830, 27108, 27384, 27660, 27936, 28210, 28484, 28757, 29029, 29300, 29571, 29840,
    30109, 30378, 30645, 30912, 31178, 31443, 31707, 31971, 32234, 32496, 32758, 33019, 33279, 33538, 33797, 34055,
    34312, 34569, 34825, 35080, 35334, 35588, 35841, 36094, 36346, 36597, 36847, 37097, 37346, 37595, 37842, 38090,
    38336, 38582, 38827, 39072, 39316, 39559, 39802, 40044, 40286, 40527, 40767, 41006, 41246, 41484, 41722, 41959,
    42196, 42432, 42667, 42902, 43137, 43370, 43603, 43836, 44068, 44300, 44530, 44761, 44990, 45220, 45448, 45676,
    45904, 46131, 46357, 46583, 46809, 47034, 47258, 47482, 47705, 47928, 48150, 48372, 48593, 48813, 49034, 49253,
    49472, 49691, 49909, 50127, 50344, 50560, 50776, 50992, 51207, 51422, 51636, 51850, 52063, 52276, 52488, 52700,
    52911, 53122, 53332, 53542, 53751, 53960, 54169, 54377, 54584, 54791, 54998, 55204, 55410, 55615, 55820, 56025,
    56229, 56432, 56635, 56838, 57040, 57242, 57443, 57644, 57845, 58045, 58245, 58444, 58643, 58841, 59039, 59237,
    59434, 59631, 59827, 60023, 60219, 60414, 60609, 60803, 60997, 61190, 61384, 61576, 61769, 61961, 62152, 62343,
    62534, 62725, 62915, 63104, 63294, 63483, 63671, 63859, 64047, 64234, 64421, 64608, 64794, 64980, 65166, 65351
};

static unsigned short Pow2_Table[] =
  {
      0,   178,   356,   535,   714,   893,  1073,  1254,  1435,  1617,  1799,  1981,  2164,  2348,  2532,  2716,
   2902,  3087,  3273,  3460,  3647,  3834,  4022,  4211,  4400,  4590,  4780,  4971,  5162,  5353,  5546,  5738,
   5932,  6125,  6320,  6514,  6710,  6906,  7102,  7299,  7496,  7694,  7893,  8092,  8292,  8492,  8693,  8894,
   9096,  9298,  9501,  9704,  9908, 10113, 10318, 10524, 10730, 10937, 11144, 11352, 11560, 11769, 11979, 12189,
  12400, 12611, 12823, 13036, 13249, 13462, 13676, 13891, 14106, 14322, 14539, 14756, 14974, 15192, 15411, 15630,
  15850, 16071, 16292, 16514, 16737, 16960, 17183, 17408, 17633, 17858, 18084, 18311, 18538, 18766, 18995, 19224,
  19454, 19684, 19915, 20147, 20379, 20612, 20846, 21080, 21315, 21550, 21786, 22023, 22260, 22498, 22737, 22977,
  23216, 23457, 23698, 23940, 24183, 24426, 24670, 24915, 25160, 25406, 25652, 25900, 26148, 26396, 26645, 26895,
  27146, 27397, 27649, 27902, 28155, 28409, 28664, 28919, 29175, 29432, 29690, 29948, 30207, 30466, 30727, 30988,
  31249, 31512, 31775, 32039, 32303, 32568, 32834, 33101, 33369, 33637, 33906, 34175, 34446, 34717, 34988, 35261,
  35534, 35808, 36083, 36359, 36635, 36912, 37190, 37468, 37747, 38028, 38308, 38590, 38872, 39155, 39439, 39724,
  40009, 40295, 40582, 40870, 41158, 41448, 41738, 42029, 42320, 42613, 42906, 43200, 43495, 43790, 44087, 44384,
  44682, 44981, 45280, 45581, 45882, 46184, 46487, 46791, 47095, 47401, 47707, 48014, 48322, 48631, 48940, 49251,
  49562, 49874, 50187, 50500, 50815, 51131, 51447, 51764, 52082, 52401, 52721, 53041, 53363, 53685, 54008, 54333,
  54658, 54983, 55310, 55638, 55966, 56296, 56626, 56957, 57289, 57622, 57956, 58291, 58627, 58964, 59301, 59640,
  59979, 60319, 60661, 61003, 61346, 61690, 62035, 62381, 62727, 63075, 63424, 63774, 64124, 64476, 64828, 65182
  };

/**
@param value_256[In]: input value, scale by 256
@return: log2(value) scale by 65536
 */
INT32 alAELib_Log2_65536(INT32 value_256)
{
    long offset_65536 = 0;

    if (value_256 <= 0)
        return -2097152;//0x80000000;   //> Define Log(0) = (0x80000000/1024) = -2097152  <Sinder>
    //> This value only protect INT32 calculation to overflow,
    else if (value_256 < 256) {
        do {
            value_256    <<= 1;
            offset_65536 -=  65536;
        } while (value_256 < 256);
    } else if (value_256 >= 512) {
        do {
            value_256    >>= 1;
            offset_65536 +=  65536;
        } while (value_256 >= 512);
    }

    return AELib_Log2_Table[value_256 & 0xff] + offset_65536;
}

/**
@param value_256[In]: input value, scale by 256
@return: pow2(value) scale by 65536
 */
INT32 alAELib_Pow2_65536(INT32 value_256)
{
    long base_value, scale;

    if (value_256 > 3839)
        return 0x7fffffff;
    else if (value_256 < -4095)
        return 0;

    base_value = Pow2_Table[value_256 & 0xff] + 65536;
    scale      = value_256 >> 8;

    return (scale >= 0) ? (base_value << scale) : (base_value >> (-scale));
}

INT32 alAELib_Abs(INT32 value)
{
    return (value >= 0) ? value : (-value);
}

/**
@param uwAperture[In]: F-number, scale by 1000
@param udExposureTime[In]: exposure time, uint: us
@param uwIsoSpeed[In]: ISO speed
@return: brightness value, scale by 1000
 */
INT32 alAELib_CalculateBv(UINT32 uwAperture, UINT32 udExposureTime, UINT32 uwIsoSpeed)
{
    /* basic range check */
    if ( uwIsoSpeed <= 1)
        uwIsoSpeed = 1;

    if ( udExposureTime <= 1 )
        udExposureTime = 1;

    INT32 temp = (1000*alAELib_Log2_65536((UINT64)800 * uwAperture * uwAperture / udExposureTime / uwIsoSpeed)) >> 16;  //800: 256/0.32

    return temp;
}

/**
@param uwAperture[In]: F-number, scale by 1000
@param udExposureTime[In]: exposure time, uint: us
@param uwIsoSpeed[In]: ISO speed
@param midtones_gain100[In]: midtones gain, scale by 100
@return: brightness value, scale by 1000
 */
INT32 alAELib_CalculateBv_midtones_comp(UINT32 uwAperture, UINT32 udExposureTime, UINT32 uwIsoSpeed, UINT16 midtones_gain100)
{
	/* basic range check */
    if ( uwIsoSpeed <= 1)
        uwIsoSpeed = 1;

    if ( udExposureTime <= 1 )
        udExposureTime = 1;

    INT32 temp = (1000*alAELib_Log2_65536((UINT64)800 * uwAperture * uwAperture / udExposureTime / (uwIsoSpeed * midtones_gain100 / 100))) >> 16;  //800: 256/0.32

    return temp;
}

/**
@param uwAperture[In]: F-number, scale by 1000
@param dBv[In]: brightness value, scale by 1000
@param uwIsoSpeed[In]: ISO speed
@return: exposure time (uint: us)
 */
UINT32 alAELib_CalculateExposureTime(UINT32 uwAperture, INT32 dBv, UINT32 uwIsoSpeed)
{
    /* basic range check */
    if ( uwIsoSpeed <= 1)
        uwIsoSpeed = 1;

    if ( uwAperture <= 1 )
        uwAperture = 1;

    INT64 dTemp = 204800 * (INT64)uwAperture * (INT64)uwAperture;
    return  (UINT32)(dTemp / ((INT64)alAELib_Pow2_65536((INT32)dBv*_MATH_SCALE256_/1000) * uwIsoSpeed));

}

/**
@param uwAperture[In]: F-number, scale by 1000
@param udExposureTime[In]: exposure time, uint: us
@param dBv[In]: brightness value, scale by 1000
@return: ISO speed
 */
UINT32 alAELib_CalculateIsoSpeed(UINT32 uwAperture, UINT32 udExposureTime, INT32 dBv)
{
    /* basic range check */
    if ( uwAperture <= 1)
        uwAperture = 1;

    if ( udExposureTime <= 1 )
        udExposureTime = 1;
    INT64 dTemp = 204800 * (INT64)uwAperture * (INT64)uwAperture;
    return  (UINT32)(dTemp / ((INT64)alAELib_Pow2_65536((INT32)dBv*_MATH_SCALE256_/1000) * udExposureTime));
}

INT32 alAELib_Interpolation(INT32 range_start, INT32 value_start, INT32 range_end, INT32 value_end, INT32 input)
{
    INT32 tempx = range_start, tempy = value_start;
    INT32 result = 0;
    if(range_start > range_end) {
        range_start = range_end;
        range_end = tempx;
        value_start = value_end;
        value_end = tempy;
    } else if(range_start == range_end) {
        return (value_start + value_end) / 2;
    }

    if(input <= range_start)
        result = value_start;
    else if(input >= range_end)
        result = value_end;
    else
        result = value_start + ((value_end - value_start) * (input - range_start) + ((range_end - range_start) >> 1)) * 1.0f / (range_end - range_start);

    return result;
}

INT32 alAELib_Lux2BV1000(UINT32 lux_value)
{
    INT32 dLux2BV;
    
    if (lux_value < 1)
        lux_value = 1;
    
    if (lux_value < 20971520) /* avoid overflow at alAELib_Log2_65536() */
    {
        dLux2BV = (1000*alAELib_Log2_65536(((UINT64)lux_value<<9) / 5) >> 16) - 5000;
    } else
        dLux2BV = 18000;
    
    return dLux2BV;
}

UINT32 alAELib_BV2Lux(INT32 bv1000)
{
    // output lux: 2.5*2^(LV), valid lux range: 0~81919 (LV -2~15)
    UINT32 temp = ((UINT64)5 * alAELib_Pow2_65536((((bv1000 + 5000)<<8) + 500)/1000)) >> 17;    // +500: for rounding

    return temp;
}

/* for AE ctrl layer */
/**
@brief This API used for patching HW3A stats from ISP for AE libs, after patching completed, AE ctrl should prepare patched stats to AE libs
@param ae_hw3a_stats[In]: hw3a stats from ISP
@param ae_proc_data[In]: Recruit AE data for process
@param alWrappered_AE_Dat[Out]: patched data, used for AE lib
@param ae_runtimedat[In]: AE lib runtime buffer after calling init, must passing correct addr to into this API
@return: error code
 */
UINT32 al3awrapper_dispatchhw3a_aestats(void *ae_hw3a_stats, struct alhw3a_ae_proc_data_t ae_proc_data, struct al3awrapper_stats_ae_t *alwrappered_ae_dat, void *ae_runtimedat)
{
    UINT32 ret = _AL_3ALIB_SUCCESS;
    UINT8 *stats, *hw_stats_info;
    struct al3awrapper_stats_ae_t *ppatched_aedat;
    struct rgb_gain_t wb_gain;
    UINT32 udtotalblocks;
    UINT32 udbanksize;
    UINT32 udoffset;
    UINT16 i,j,blocks, banks, index, blocks_perbank = AE_MAX_STATS_BLOCK; // Fix max blocks only in AON
    UINT16 *pt_stats_y;
    UINT32 stats_r, stats_gr,  stats_gb, stats_g, stats_b;
    UINT16 HwStatsInfo_frame_idx = 0, HwStatsInfo_b = 0,HwStatsInfo_e = 0;
    UINT8 HwStatsInfo_blk_num = 0, HwStatsInfo_bnk_num = 0, HwStatsInfo_Size = AE_HWSTATS_INFO_SIZE; // 128: size of stats info, unit: byte
    UINT32 HwStatsInfo_stats_size = 0, HwStatsInfo_pixelsperblocks = 0, HwStats_size = 0;
    UINT8 isFlybyMode = AE_HWSTATS_FLYBY_MODE;
	UINT8 nonSatCntTH = 0;
	UINT16 midtones_comp;

    // check input parameter validity
    if ( ae_hw3a_stats == NULL )
        return _AL_WRP_AE_EMPTY_METADATA;
    if ( alwrappered_ae_dat == NULL || ae_runtimedat == NULL )
        return _AL_WRP_AE_INVALID_INPUT_PARAM;

    ppatched_aedat = (struct al3awrapper_stats_ae_t *)alwrappered_ae_dat;
    ae_corelib_obj_t *ae_obj = (ae_corelib_obj_t *)ae_runtimedat;
    //if (ae_obj->ae_tuning_para.iAE_para.WDRMode == WDR_USEMIDWITHOUTLTM)
    //{
    //    midtones_comp = 100;
    //    LOG_DEBUG("without LTM, don't use midgain in stats");
    //}
    //else
    //{
	    midtones_comp = ae_obj->ae_shadow_set_param.para.sof_notify_param.midtones_gain;
        LOG_DEBUG("midtones_comp %u", midtones_comp);
    //}

    // Since stats info is placed after stats data in flyby mode, the block/bank number are parsed from tuning bin
    blocks_perbank = ae_obj->ae_tuning_para.ucStatsBlockNum;
    HwStatsInfo_stats_size = blocks_perbank * ae_obj->ae_tuning_para.ucStatsBankNum * 32;
    
    // check stats size validity
    HwStats_size = ae_obj->ae_current_set_param.para.ae_initial_setting.ae_stats_size;
    if (HwStats_size < (HwStatsInfo_stats_size + HwStatsInfo_Size))
    {
        return _AL_WRP_AE_STATS_SIZE_EXCEED;
    }

    // check if stats ptr valid?
    if ( ae_hw3a_stats == NULL )
        return _AL_WRP_AE_INVALID_STATS_ADDR;
    else {
        if (isFlybyMode == FALSE) { /* stats info before stats */
            hw_stats_info = (UINT8 *)ae_hw3a_stats;
            stats = (UINT8 *)ae_hw3a_stats + HwStatsInfo_Size;
        } else { /* stats info after stats */
            hw_stats_info = (UINT8 *)ae_hw3a_stats + HwStatsInfo_stats_size;
            stats = (UINT8 *)ae_hw3a_stats;
        }
    }

    // Parse stats data
    //HwStatsInfo_b = (hw_stats_info[4] + (hw_stats_info[5] << 8)) & 0xFFF;
    //HwStatsInfo_blk_num = hw_stats_info[14] & 0xFF;
    //HwStatsInfo_bnk_num = hw_stats_info[15] & 0x7F;
    //HwStatsInfo_frame_idx = (hw_stats_info[24] + (hw_stats_info[25] << 8)) & 0xFFFF;
    //HwStatsInfo_e = (hw_stats_info[28] + (hw_stats_info[29] << 8)) & 0xFFF;

    HwStatsInfo_b = 100;
    HwStatsInfo_blk_num = ae_obj->ae_tuning_para.ucStatsBlockNum;
    HwStatsInfo_bnk_num = ae_obj->ae_tuning_para.ucStatsBankNum;
    HwStatsInfo_frame_idx = 0;
    HwStatsInfo_e = 100;

    // check stats block/bank number is the same as tuning bin
    //if (HwStatsInfo_blk_num != ae_obj->ae_tuning_para.ucStatsBlockNum || HwStatsInfo_bnk_num != ae_obj->ae_tuning_para.ucStatsBankNum)
    //{
    //    ret = _AL_WRP_AE_INVALID_BLOCKS;
    //    return ret;
    //}
    
    if (HwStatsInfo_blk_num > AE_MAX_STATS_BLOCK)
        HwStatsInfo_blk_num = AE_MAX_STATS_BLOCK;
    else if (HwStatsInfo_blk_num < 1)
        HwStatsInfo_blk_num = 1;
        
    if (HwStatsInfo_bnk_num > AE_MAX_STATS_BANK)
        HwStatsInfo_bnk_num = AE_MAX_STATS_BANK;
    else if (HwStatsInfo_bnk_num < 1)
        HwStatsInfo_bnk_num = 1;

    HwStatsInfo_pixelsperblocks = HwStatsInfo_b * HwStatsInfo_e / 4;  // 4:divided by r/gr/gb/b

    // Parse stats proc data
    UINT32 udSysSOFIdx = ae_proc_data.udsys_sof_idx;

    // Check stats size validity?
    if ( HwStatsInfo_stats_size > HW3A_AE_STATS_BUFFER_SIZE )
        return _AL_WRP_AE_STATS_SIZE_EXCEED;

    if (0 == HwStatsInfo_pixelsperblocks)
        return _AL_WRP_AE_INVALID_PIXEL_PER_BLOCKS;

    // update sturcture size, this would be double checked in AE libs
    ppatched_aedat->ustructuresize = sizeof( struct al3awrapper_stats_ae_t );

    udtotalblocks = HwStatsInfo_blk_num * HwStatsInfo_bnk_num;
    blocks = HwStatsInfo_blk_num;
    banks = HwStatsInfo_bnk_num;

    // check data AE blocks validity
    if ( udtotalblocks > AL_MAX_AE_STATS_NUM )
        return _AL_WRP_AE_INVALID_BLOCKS;

    // check WB validity, calibration WB is high priority
    //if ( ae_obj->calibration_data.calib_r_gain == 0 || ae_obj->calibration_data.calib_g_gain == 0 || ae_obj->calibration_data.calib_b_gain == 0 )
    //{
    //    return _AL_WRP_AE_INVALID_INPUT_WB;
    //}
    //else
    //{
    //    wb_gain.r = (UINT32)(ae_obj->calibration_data.calib_r_gain *256/ae_obj->calibration_data.calib_g_gain);
    //    wb_gain.g = (UINT32)(ae_obj->calibration_data.calib_g_gain *256/ae_obj->calibration_data.calib_g_gain);
    //    wb_gain.b = (UINT32)(ae_obj->calibration_data.calib_b_gain *256/ae_obj->calibration_data.calib_g_gain);
    //}

    LOG_DEBUG("WBGAIN %u, %u, %u", ae_obj->ae_current_set_param.para.awbInfo.rGain, ae_obj->ae_current_set_param.para.awbInfo.gGain, ae_obj->ae_current_set_param.para.awbInfo.bGain);
    if (ae_obj->ae_current_set_param.para.awbInfo.rGain == 0 || ae_obj->ae_current_set_param.para.awbInfo.gGain == 0 || ae_obj->ae_current_set_param.para.awbInfo.bGain == 0)
    {
        return _AL_WRP_AE_INVALID_INPUT_WB;
    }
    else
    {
        wb_gain.r = ae_obj->ae_current_set_param.para.awbInfo.rGain * 256/ ae_obj->ae_current_set_param.para.awbInfo.gGain;
        wb_gain.g = ae_obj->ae_current_set_param.para.awbInfo.gGain;
        wb_gain.b = ae_obj->ae_current_set_param.para.awbInfo.bGain * 256 / ae_obj->ae_current_set_param.para.awbInfo.gGain;
    }

    wb_gain.r = wb_gain.r * 76;   //  76.544: 0.299 * 256
    wb_gain.g = wb_gain.g * 150;  // 150.272: 0.587 * 256, Combine Gr Gb factor
    wb_gain.b = wb_gain.b * 29;   //  29.184: 0.114 * 256

    udoffset =0;
    index = 0;
    ppatched_aedat->ucstatsdepth = 10;

    // store patched data/common info/ae info from Wrapper
    ppatched_aedat->uframeidx           = HwStatsInfo_frame_idx;
    ppatched_aedat->uaestatssize        = HwStatsInfo_stats_size;
    ppatched_aedat->udpixelsperblocks   = HwStatsInfo_pixelsperblocks;
    ppatched_aedat->ucvalidblocks       = blocks;
    ppatched_aedat->ucvalidbanks        = banks;
    
    // store frame & timestamp
    ppatched_aedat->udsys_sof_idx       = udSysSOFIdx;

    pt_stats_y = ppatched_aedat->statsy;

    // 4:r/gr/gb/b(4 channels), 4:Each channel's 4 bytes (i.e. sizeof(UINT32)), 2:Multiply by 2 is for r/gr/gb/r non-sat-count
    // stats big-endian store
    udbanksize = 4 * 4 * 2 * blocks_perbank;
    for ( j =0; j < banks; j++ )
    {
        udoffset = udbanksize*j;
        for ( i = 0; i < blocks; i++ )
        {
            UINT32 non_sat_sumR_cnt = ( stats[udoffset+16] + (stats[udoffset+17]<<8) + (stats[udoffset+18]<<16) + (stats[udoffset+19]<<24) );
            UINT32 non_sat_sumGR_cnt = ( stats[udoffset+20] + (stats[udoffset+21]<<8) + (stats[udoffset+22]<<16) + (stats[udoffset+23]<<24) );
            UINT32 non_sat_sumGB_cnt = ( stats[udoffset+24] + (stats[udoffset+25]<<8) + (stats[udoffset+26]<<16) + (stats[udoffset+27]<<24) );
            UINT32 non_sat_sumB_cnt = ( stats[udoffset+28] + (stats[udoffset+29]<<8) + (stats[udoffset+30]<<16) + (stats[udoffset+31]<<24) );

			// due to data from HW, use direct address instead of casting
            if(non_sat_sumR_cnt <= nonSatCntTH)
            {
            	stats_r = 1023;
            }
			else
			{
				stats_r = (UINT32)(( stats[udoffset] + (stats[udoffset+1]<<8) + (stats[udoffset+2]<<16) + (stats[udoffset+3]<<24)) / non_sat_sumR_cnt);  // 10 bits
				stats_r = stats_r * midtones_comp / 100;
				stats_r = stats_r > 1023 ? 1023 : stats_r;
			}

            if(non_sat_sumGR_cnt <= nonSatCntTH)
            {
            	stats_gr = 1023;
            }
			else
			{
				stats_gr = (UINT32) ( ( stats[udoffset+4] + (stats[udoffset+5]<<8) + (stats[udoffset+6]<<16) + (stats[udoffset+7]<<24)) / non_sat_sumGR_cnt); // 10 bits
				stats_gr = stats_gr * midtones_comp / 100;
                stats_gr = stats_gr > 1023 ? 1023 : stats_gr;
			}

            if(non_sat_sumGB_cnt <= nonSatCntTH)
            {
            	stats_gb = 1023;
            }
			else
			{
				stats_gb = (UINT32) ( ( stats[udoffset+8] + (stats[udoffset+9]<<8) + (stats[udoffset+10]<<16) + (stats[udoffset+11]<<24)) / non_sat_sumGB_cnt); // 10 bits
				stats_gb = stats_gb * midtones_comp / 100;
				stats_gb = stats_gb > 1023 ? 1023 : stats_gb;
			}
			stats_g = (stats_gr + stats_gb) >> 1;
			
            
            if(non_sat_sumB_cnt <= nonSatCntTH)
            {
            	stats_b = 1023;
            }
			else
			{
                stats_b = (UINT32)(( stats[udoffset+12] + (stats[udoffset+13]<<8) + (stats[udoffset+14]<<16) + (stats[udoffset+15]<<24) ) / non_sat_sumB_cnt);  // 10 bits
				stats_b = stats_b * midtones_comp / 100;
				stats_b = stats_b > 1023 ? 1023 : stats_b;
			}
            

            // calculate Y
            pt_stats_y[index] = (UINT16)((UINT32)( stats_r* wb_gain.r + stats_g * wb_gain.g + stats_b * wb_gain.b) >> 16);  // 10 bits

            index++;
            udoffset += 32;
        }
    }

    LOG_DEBUG("multicamera mode %d, cameraid %u", ae_obj->ae_tuning_para.multiCamera_para.multiCamMode, ae_obj->ucCameraId);
    switch (ae_obj->ae_tuning_para.multiCamera_para.multiCamMode)
    {
        case AE_MULTICAM_MODE_BYSELF:
            break;
        case AE_MULTICAM_MODE_WEIGHTEXP_BYMASTER:
            alAELib_Core_calVariance(index, pt_stats_y, ae_obj);
            alAELib_Core_estimateSlave(ae_obj);
            break;
        case AE_MULTICAM_MODE_WEIGHTSTATS_SLAVEBYSELF:
            alAELib_Core_calVariance(index, pt_stats_y, ae_obj);
            alAELib_Core_estimateSlave(ae_obj);
            alAELib_Core_mixStats(index, pt_stats_y, banks, blocks, ae_obj);
            break;
        case AE_MULTICAM_MODE_WEIGHTSTATS_MASTERTWOINONE:
            alAELib_Core_calVariance(index, pt_stats_y, ae_obj);
            alAELib_Core_estimateSlave(ae_obj);
            alAELib_Core_mixStatsSingle(index, pt_stats_y, banks, blocks, ae_obj);
            break;
        default:
            break;
    }

    return ret;
}

/**
@brief This API used for patching HW3A stats from ISP of Y-histogram
@param yhis_hw3a_stats[In]: hw3a stats from ISP
@param ae_runtimedat[In]: AE lib runtime buffer after calling init, must passing correct addr to into this API
@return error code
 */
UINT32 al3awrapper_dispatchhw3a_yhiststats(void *yhis_hw3a_stats, void *ae_runtimedat)
{
	UINT32 ret = _AL_3ALIB_SUCCESS;
	UINT32 *stats;
	UINT32 udValidStatsSize;

	/* check input parameter validity */
	if ( yhis_hw3a_stats == NULL || ae_runtimedat == NULL)
		return _AL_WRP_AE_INVALID_STATS_ADDR;

	ae_corelib_obj_t *ae_obj = (ae_corelib_obj_t *)ae_runtimedat;

	stats = (UINT32 *)yhis_hw3a_stats;

	udValidStatsSize = AL_MAX_HIST_NUM * sizeof(UINT32); /* 4 byte for each bin */

	/* switched by stats addr or fix array */
	memcpy(ae_obj->ae_runtime_data.udY_hist, stats, udValidStatsSize);

    //big
    //LOG_DEBUG("take out quaddark hist, dark %u %u %u %u %u %u", ae_obj->ae_runtime_data.udY_hist[0], ae_obj->ae_runtime_data.udY_hist[1], ae_obj->ae_runtime_data.udY_hist[2], ae_obj->ae_runtime_data.udY_hist[3],
    //    ae_obj->ae_runtime_data.udY_hist[4], ae_obj->ae_runtime_data.udY_hist[5]);
    ////take out quaddark hist, 222033 = 2328*1744*(28*2/(16*16))/4, 28*2:metering table weight0 count
    //if (ae_obj->ae_runtime_data.udY_hist[0] + ae_obj->ae_runtime_data.udY_hist[1] + ae_obj->ae_runtime_data.udY_hist[2] > 222033)
    //{
    //    if (ae_obj->ae_runtime_data.udY_hist[0] >= 222033)
    //    {
    //        ae_obj->ae_runtime_data.udY_hist[0] = ae_obj->ae_runtime_data.udY_hist[0] - 222033;
    //    }
    //    else if (ae_obj->ae_runtime_data.udY_hist[0] + ae_obj->ae_runtime_data.udY_hist[1] >= 222033)
    //    {
    //        ae_obj->ae_runtime_data.udY_hist[1] = ae_obj->ae_runtime_data.udY_hist[1] - (222033 - ae_obj->ae_runtime_data.udY_hist[0]);
    //        ae_obj->ae_runtime_data.udY_hist[0] = 0;
    //    }
    //    else
    //    {
    //        ae_obj->ae_runtime_data.udY_hist[2] = ae_obj->ae_runtime_data.udY_hist[2] - (222033 - ae_obj->ae_runtime_data.udY_hist[1] - ae_obj->ae_runtime_data.udY_hist[0]);
    //        ae_obj->ae_runtime_data.udY_hist[0] = 0;
    //        ae_obj->ae_runtime_data.udY_hist[1] = 0;
    //    }
    //}
    //else
    //{
    //    ae_obj->ae_runtime_data.udY_hist[0] = 0;
    //    ae_obj->ae_runtime_data.udY_hist[1] = 0;
    //    ae_obj->ae_runtime_data.udY_hist[2] = 0;
    //}
	return ret;
}

INT32 alAELib_CalculateProgressiveAE(INT32 wNowBV1000, INT32 wTargetBV1000, UINT8 a_ucConverge_SPD, ae_corelib_obj_t *ptAEBuffer)
{
    INT32 wBv;
    //INT32 wDiff = wTargetBV1000 - wNowBV1000;
    UINT16 uwlevel = 40;
    float P_Gain = 0, ratio = ptAEBuffer->ae_speed_param.fbased_speed;
    float beta = 0.4f;
    float h1 = 0.3f, h2 = 0.0003f, h3 = 0.00000021f;

    switch (a_ucConverge_SPD){
        case AE_CONVERGE_DIRECT:
            beta = 1.0f;
            uwlevel = ptAEBuffer->ae_speed_param.direct_speed_level;
            break;
        case AE_CONVERGE_FAST:
            h1 = 0.2f;
            h2 = 0.00035f;
            h3 = 0.00000025f;
            uwlevel = ptAEBuffer->ae_speed_param.fast_speed_level;
            break;
        case AE_CONVERGE_NORMAL:
            h1 = 0.3f;
            h2 = 0.0003f;
            h3 = 0.00000021f;
            uwlevel = ptAEBuffer->ae_speed_param.normal_speed_level;
            break;
        case AE_CONVERGE_SMOOTH:
            h1 = 0.2f;
            h2 = 0.0003f;
            h3 = 0.00000025f;
            uwlevel = ptAEBuffer->ae_speed_param.smooth_speed_level;
            break;
        case AE_CONVERGE_SLOW:
            h1 = 0.2f;
            h2 = 0.0003f;
            h3 = 0.00000015f;
            uwlevel = ptAEBuffer->ae_speed_param.slow_speed_level;
            break;
        default:
            break;
    }

    INT32 prebt = ptAEBuffer->ae_runtime_data.convergeDiff;
    INT32 bt = beta * (wTargetBV1000 - wNowBV1000) + (1 - beta) * prebt;

    float P1 = sqrt(h1 / h3);
    float P2 = h2 / h3;
    if (P1 <= P2)
    {
        if (abs(bt) >= P2)
        {
            P_Gain = h2 * abs(bt);
        }
        else if (abs(bt) < P2 && abs(bt) >= P1)
        {
            P_Gain = h3 * abs(bt) * abs(bt);
        }
        else
        {
            P_Gain = h1;
        }
    }
    else
    {
        P_Gain = h2 * abs(bt);
        LOG_DEBUG("ERROR P1 > P2, set default P_Gain");
    }


    wBv = wNowBV1000 + P_Gain * bt;

    if ((wNowBV1000 <= wTargetBV1000 && wBv >= wTargetBV1000) || (wNowBV1000 >= wTargetBV1000 && wBv <= wTargetBV1000))
    {
        wBv = wTargetBV1000;
    }

    LOG_DEBUG("[Smooth]  SPD %u, prediff %d  targetbv %d  prebv %d  curbv %d  P_Gain %f", a_ucConverge_SPD, ptAEBuffer->ae_runtime_data.convergeDiff, wTargetBV1000, wNowBV1000, wBv, P_Gain);
    ptAEBuffer->ae_runtime_data.convergeDiff = wBv - wNowBV1000;
    
    return wBv;
}

void alAELib_Core_calVariance(UINT16 index, UINT16* pt_stats_y, void* ae_runtimedat)
{
    ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ae_runtimedat;
    //calculate variance
    int varSum = 0;
    for (int i = 0; i < index; i++)
    {
        varSum += pt_stats_y[i];
    }
    int meanY = varSum / index;
    int variance = 0;
    for (int i = 0; i < index; i++)
    {
        int temp = pt_stats_y[i] - meanY;
        variance += temp * temp;
    } 
    
    variance = sqrt(variance / index);
    storeExp[ae_obj->ucCameraId].variance = variance;
   
}

void alAELib_Core_mixStatsSingle(UINT16 index, UINT16* pt_stats_y, UINT16 banks, UINT16 blocks, void* ae_runtimedat)
{
    ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ae_runtimedat;

    //store before mix stats
    for (int i = 0; i < banks; i++)
    {
        for (int j = 0; j < blocks; j++)
        {
            storeYstats[ae_obj->ucCameraId][i][j] = pt_stats_y[i * blocks + j];
        }
    }
    storeStatsValid[ae_obj->ucCameraId] = true;
    //mixstats, left 0, right 1
    int leftID = 0, rightID = 1;
    int mixStatsLeft = 0, mixStatsRight = 0;
    int anotherID = ae_obj->ucCameraId == 0 ? 1 : 0;

    //test2 mixstats simple
    if (ae_obj->ucCameraId == ae_obj->ae_runtime_data.slaveID)//slave
    {
        return;
    }

    float slaveRatio = 1.0f;
    float slaveWeightStart = ae_obj->ae_tuning_para.multiCamera_para.slaveWeightStart; //1
    float slaveWeightEnd = ae_obj->ae_tuning_para.multiCamera_para.slaveWeightEnd; //0
    int BVStart = ae_obj->ae_tuning_para.multiCamera_para.BVStart; //-3000
    int BVEnd = ae_obj->ae_tuning_para.multiCamera_para.BVEnd; //2000
    slaveRatio = slaveWeightStart - (storeExp[ae_obj->ae_runtime_data.slaveID].targetBV - BVStart) * (slaveWeightStart - slaveWeightEnd) / (BVEnd - BVStart);
    slaveRatio = (slaveRatio > slaveWeightStart ? slaveWeightStart : (slaveRatio < slaveWeightEnd ? slaveWeightEnd : slaveRatio));

    if (ae_obj->ucCameraId == leftID)
    {
        mixStatsLeft = ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[ae_obj->ucCameraId].leftstart / 2;
        mixStatsRight = ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[ae_obj->ucCameraId].leftstart - mixStatsLeft;
    }
    else if (ae_obj->ucCameraId == rightID)
    {
        mixStatsLeft = (blocks - ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[ae_obj->ucCameraId].rightend) / 2;
        mixStatsRight = blocks - ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[ae_obj->ucCameraId].rightend - mixStatsLeft;
    }

    //cameraMixROI[cameraid]: vector<int> ROIList = {leftstart,rightend,topstart,bottom}
    //warning: 0 <= leftstart < rightend < blocks; 0 <= topstart < bottom < banks
    if (storeStatsValid[anotherID])
    {
        //mix
        for (int i = 0; i < banks; i++)
        {
            int count = ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[leftID].leftstart * 1.0f / mixStatsLeft + 0.5f;
            for (int j = 0, m = 0; j < mixStatsLeft && m < ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[leftID].leftstart; j++, m = m + count)
            {
                int sum = 0, sumCount = 0;
                for (int n = m; n < m + count && n < ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[leftID].leftstart; n++)
                {
                    sum += storeYstats[leftID][i][n];
                    sumCount++;
                }
                pt_stats_y[i * blocks + j] = sum / sumCount;
            }

            count = (blocks - ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[rightID].rightend) * 1.0f / mixStatsRight + 0.5f;
            for (int j = blocks - mixStatsRight, m = ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[rightID].rightend; j < blocks && m < blocks; j++, m = m + count)
            {
                int sum = 0, sumCount = 0;
                for (int n = m; n < m + count && n < blocks && n >= 0; n++)
                {
                    sum += storeYstats[rightID][i][n];
                    sumCount++;
                }
                pt_stats_y[i * blocks + j] = sum / sumCount;
            }

            for (int j = mixStatsLeft; j < blocks - mixStatsRight; j++)
            {
                int leftj = j - mixStatsLeft + ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[ae_obj->ucCameraId].leftstart;
                int rightj = j - mixStatsLeft;
                //pt_stats_y[i * blocks + j] = storeYstats[leftID][i][leftj] * 1.0f * storeYstats[leftID][i][leftj] / (storeYstats[leftID][i][leftj] + storeYstats[rightID][i][rightj])
                 //   + storeYstats[rightID][i][rightj] * 1.0f * storeYstats[rightID][i][rightj] / (storeYstats[leftID][i][leftj] + storeYstats[rightID][i][rightj]);

                if (ae_obj->ucCameraId == leftID) //master is left
                {
                    pt_stats_y[i * blocks + j] = storeYstats[leftID][i][leftj] * (1 - slaveRatio) + storeYstats[rightID][i][rightj] * slaveRatio;
                }
                else
                {
                    pt_stats_y[i * blocks + j] = storeYstats[leftID][i][leftj] * slaveRatio + storeYstats[rightID][i][rightj] * (1 - slaveRatio);
                }
            }
        }

    }
}


void alAELib_Core_mixStats(UINT16 index, UINT16* pt_stats_y, UINT16 banks, UINT16 blocks, void* ae_runtimedat)
{
    ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ae_runtimedat;

    float slaveRatio = 1.0f;
    float slaveWeightStart = ae_obj->ae_tuning_para.multiCamera_para.slaveWeightStart; //1
    float slaveWeightEnd = ae_obj->ae_tuning_para.multiCamera_para.slaveWeightEnd; //0
    int BVStart = ae_obj->ae_tuning_para.multiCamera_para.BVStart; //-3000
    int BVEnd = ae_obj->ae_tuning_para.multiCamera_para.BVEnd; //2000

    //store before mix stats
    for (int i = 0; i < banks; i++)
    {
        for (int j = 0; j < blocks; j++)
        {
            storeYstats[ae_obj->ucCameraId][i][j] = pt_stats_y[i * blocks + j];
        }
    }
    storeStatsValid[ae_obj->ucCameraId] = true;
    //mixstats, left 0, right 1
    int leftID = 0, rightID = 1;
    int anotherID = ae_obj->ucCameraId == 0 ? 1 : 0;


    if (ae_obj->ucCameraId == ae_obj->ae_runtime_data.slaveID)
    {
        slaveRatio = slaveWeightStart - (storeExp[ae_obj->ae_runtime_data.slaveID].targetBV - BVStart) * (slaveWeightStart - slaveWeightEnd) / (BVEnd - BVStart);
        slaveRatio = (slaveRatio > slaveWeightStart ? slaveWeightStart : (slaveRatio < slaveWeightEnd ? slaveWeightEnd : slaveRatio));
        if (ae_obj->ucCameraId == leftID)
        {
            for (int i = 0; i < banks; i++)
            {
                for (int j = ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[ae_obj->ucCameraId].leftstart; j < blocks && j >= 0; j++)
                {
                    int leftj = j;
                    int rightj = j - ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[rightID].leftstart;
                    //pt_stats_y[i * blocks + j] = storeYstats[leftID][i][leftj] * 1.0f * storeYstats[leftID][i][leftj] / (storeYstats[leftID][i][leftj] + storeYstats[rightID][i][rightj])
                    //    + storeYstats[rightID][i][rightj] * 1.0f * storeYstats[rightID][i][rightj] / (storeYstats[leftID][i][leftj] + storeYstats[rightID][i][rightj]);

                    pt_stats_y[i * blocks + j] = storeYstats[leftID][i][leftj] * slaveRatio
                        + storeYstats[rightID][i][rightj] *(1 - slaveRatio);
                }
            }
        
        }
        else if (ae_obj->ucCameraId == rightID)
        {
            for (int i = 0; i < banks; i++)
            {
                for (int j = ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[ae_obj->ucCameraId].leftstart; j < ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[ae_obj->ucCameraId].rightend; j++)
                {
                    int leftj = j + ae_obj->ae_tuning_para.multiCamera_para.cameraMixROI[leftID].leftstart;
                    int rightj = j;
                    //pt_stats_y[i * blocks + j] = storeYstats[leftID][i][leftj] * 1.0f * storeYstats[leftID][i][leftj] / (storeYstats[leftID][i][leftj] + storeYstats[rightID][i][rightj])
                    //    + storeYstats[rightID][i][rightj] * 1.0f * storeYstats[rightID][i][rightj] / (storeYstats[leftID][i][leftj] + storeYstats[rightID][i][rightj]);
                    pt_stats_y[i * blocks + j] = storeYstats[leftID][i][leftj] * (1 - slaveRatio)
                        + storeYstats[rightID][i][rightj] * slaveRatio;
                }
            }
        
        }
    }

}


void alAELib_Core_estimateSlave(ae_corelib_obj_t* ptAEBuffer)
{
    int anotherID = 0;
    uint8 curMasterID = 0;
    uint8 curSlaveID = 1;
    int varianceStableTh = ptAEBuffer->ae_tuning_para.multiCamera_para.varianceStableTh; //5
    int ymeanStableTh = ptAEBuffer->ae_tuning_para.multiCamera_para.ymeanStableTh; //200
    int stableFrame = ptAEBuffer->ae_tuning_para.multiCamera_para.stableFrame; //5

    if (ptAEBuffer->ucCameraId == 0) anotherID = 1;
    else if (ptAEBuffer->ucCameraId == 1) anotherID = 0;

    ptAEBuffer->ae_runtime_data.masterID = lastMasterID;
    ptAEBuffer->ae_runtime_data.slaveID = (lastMasterID == 0) ? 1 : 0;

    if (storeExp[ptAEBuffer->ucCameraId].variance > storeExp[anotherID].variance && (storeExp[ptAEBuffer->ucCameraId].variance - storeExp[anotherID].variance) > varianceStableTh)
    {
        curMasterID = ptAEBuffer->ucCameraId;
        curSlaveID = anotherID;
    }
    else if (storeExp[ptAEBuffer->ucCameraId].variance >= storeExp[anotherID].variance && (storeExp[ptAEBuffer->ucCameraId].variance - storeExp[anotherID].variance) <= varianceStableTh)
    {
        curMasterID = lastMasterID;
        curSlaveID = (curMasterID == 0) ? 1 : 0;
    }
    else if (storeExp[ptAEBuffer->ucCameraId].variance < storeExp[anotherID].variance && (storeExp[anotherID].variance - storeExp[ptAEBuffer->ucCameraId].variance) > varianceStableTh)
    {
        curMasterID = anotherID;
        curSlaveID = ptAEBuffer->ucCameraId;
    }
    else
    {
        curMasterID = lastMasterID;
        curSlaveID = (curMasterID == 0) ? 1 : 0;
    }

    if (storeExp[ptAEBuffer->ucCameraId].uwbgAEMean_8bit > storeExp[anotherID].uwbgAEMean_8bit && (storeExp[ptAEBuffer->ucCameraId].uwbgAEMean_8bit - storeExp[anotherID].uwbgAEMean_8bit) > ymeanStableTh)
    {
        curMasterID = ptAEBuffer->ucCameraId;
        curSlaveID = anotherID;
    }
    else if (storeExp[ptAEBuffer->ucCameraId].uwbgAEMean_8bit < storeExp[anotherID].uwbgAEMean_8bit && (storeExp[anotherID].uwbgAEMean_8bit - storeExp[ptAEBuffer->ucCameraId].uwbgAEMean_8bit) > ymeanStableTh)
    {
        curMasterID = anotherID;
        curSlaveID = ptAEBuffer->ucCameraId;
    }

    if (curMasterID == lastMasterID)
    {
        estimateSlaveCount++;
    }
    else
    {
        estimateSlaveCount = 0;
    }
    LOG_DEBUG("cur master %d  slave %d, variance %u %u, count %d", curMasterID, curSlaveID, storeExp[curMasterID].variance, storeExp[curSlaveID].variance, estimateSlaveCount);
    if (estimateSlaveCount > stableFrame)
    {
        curALLMasterID = curMasterID;
        estimateSlaveCount = 0;
    }
    lastMasterID = curMasterID;
    ptAEBuffer->ae_runtime_data.masterID = curALLMasterID;
    ptAEBuffer->ae_runtime_data.slaveID = (curALLMasterID == 0) ? 1 : 0;

    LOG_DEBUG("master %u  slave %u, variance %u %u", ptAEBuffer->ae_runtime_data.masterID, ptAEBuffer->ae_runtime_data.slaveID, storeExp[ptAEBuffer->ucCameraId].variance, storeExp[anotherID].variance);
}

void alAELib_Core_mixExposure(ae_corelib_obj_t* ptAEBuffer, int* targetBV)
{
    float slaveRatio = 1.0f;
    float slaveWeightStart = ptAEBuffer->ae_tuning_para.multiCamera_para.slaveWeightStart; //1
    float slaveWeightEnd = ptAEBuffer->ae_tuning_para.multiCamera_para.slaveWeightEnd; //0
    int BVStart = ptAEBuffer->ae_tuning_para.multiCamera_para.BVStart; //-3000
    int BVEnd = ptAEBuffer->ae_tuning_para.multiCamera_para.BVEnd; //2000
    if (ptAEBuffer->ucCameraId == ptAEBuffer->ae_runtime_data.slaveID && ptAEBuffer->ae_runtime_data.multiCamMasterFlag)
    {
        slaveRatio = slaveWeightStart - (storeExp[ptAEBuffer->ae_runtime_data.slaveID].targetBV - BVStart) * (slaveWeightStart - slaveWeightEnd) / (BVEnd - BVStart);
        slaveRatio = (slaveRatio > slaveWeightStart ? slaveWeightStart : (slaveRatio < slaveWeightEnd ? slaveWeightEnd : slaveRatio));
        *targetBV = storeExp[ptAEBuffer->ae_runtime_data.masterID].targetBV * (1 - slaveRatio) + storeExp[ptAEBuffer->ae_runtime_data.slaveID].targetBV * slaveRatio;
        
    }

    if(ptAEBuffer->ucCameraId == ptAEBuffer->ae_runtime_data.masterID)
    {
        ptAEBuffer->ae_runtime_data.multiCamMasterFlag = true;
    }
    LOG_DEBUG("camid %u  master %d mixExposure target master %d  slave %d, ratio %f, flag %d, after target %d, slaveWeight [%f, %f], BVTuning [%d, %d]", 
        ptAEBuffer->ucCameraId, ptAEBuffer->ae_runtime_data.masterID, storeExp[ptAEBuffer->ae_runtime_data.masterID].targetBV, storeExp[ptAEBuffer->ae_runtime_data.slaveID].targetBV, slaveRatio, ptAEBuffer->ae_runtime_data.multiCamMasterFlag ,
        *targetBV,slaveWeightStart, slaveWeightEnd, BVStart, BVEnd);
}

void alAELib_Core_calMeteringRatio(ae_corelib_obj_t* ptAEBuffer, UINT8** metering_table)
{
    
    //left 0, right 1
    if (ptAEBuffer->ucCameraId == ptAEBuffer->ae_runtime_data.masterID)
    {
        return;
    }
    if (ptAEBuffer->ucCameraId == 0)
    {
        *metering_table = ptAEBuffer->ae_tuning_para.weight_table.multiCameraleft;
    }
    else
    {
        *metering_table = ptAEBuffer->ae_tuning_para.weight_table.multiCameraright;
    }
    LOG_DEBUG("cal metering table");
}
