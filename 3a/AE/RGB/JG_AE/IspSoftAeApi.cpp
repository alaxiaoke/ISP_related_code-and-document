#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <cassert>

#include <fstream>
#include <iostream>
#include <direct.h>

#include "./Include/allib_ae.h"
#include "./Include/allib_ae_errcode.h"
//#include <queue>
#include "./core/include/MyLog.h"

#include "out.h"
#include "IspSoftAeApi.h"

using namespace std;

int w_emCurLogLevel = LogLevel_OFF;
int g_log_terminal = 1;
int g_log_to_file = 0;
char* logbuffer = nullptr;
static string configHALPath = "";
string UnicodeToAnsi_str(const wchar_t* szStr)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
    if (nLen == 0)
    {
        return "";
    }
    char* pResult = new char[nLen];
    WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
    string  str(pResult);
    if (pResult != NULL)
    {
        delete[] pResult;
        pResult = NULL;
    }

    return str;
}

string GetCurrMoudle()
{
    HMODULE hModule = NULL;
    TCHAR dllDir[4096] = { 0 };
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)&GetCurrMoudle, &hModule);
    if (NULL != hModule)
    {
        GetModuleFileName(hModule, (LPWSTR)dllDir, 4096);
    }
    string dllAddr;
    dllAddr = UnicodeToAnsi_str(dllDir);

    int  dwfind = dllAddr.find_last_of("\\");
    string strpath = "";
    string strname = "";
    if (dwfind != string::npos)
    {
        strpath = dllAddr.substr(0, dwfind);
        strname = dllAddr.substr(dwfind + 1);
    }

    LOG_DEBUG("strpath %s\n", strpath.c_str());

    return strpath;
}

static void updateOutData(IspSoftAeOutReady* outData, ae_output_data_t* ae_output, ae_sensorExpoList* expoSOFFront, IspSoftAeOut* ae_out, UINT32 maxSensorGain)
{
    
    outData->ae_shutter_us = ae_output->udexposure_time;
    outData->ae_adgain = min(ae_output->udsensor_ad_gain, maxSensorGain);

    ae_out->ae_shutter = ae_output->udexposure_time;
    ae_out->ae_adgain = min(ae_output->udsensor_ad_gain, maxSensorGain);
    ae_out->ae_ispgain = expoSOFFront->ispGain * 100;
    ae_out->ae_converged = ae_output->ae_converged;
    ae_out->Ymean = ae_output->ymean100_8bit;   //avg y mean
    ae_out->ae_lux_result = ae_output->lux_result;   // avg bv-->lux 
    ae_out->bgbv = ae_output->bg_bvresult;   // avg bv
    ae_out->nextbv = ae_output->bvresult;   // next bv
    ae_out->iso = ae_output->iso;
    ae_out->midtones_gain = expoSOFFront->ispmidtoneGain;
    ae_out->shadows_gain = ae_output->shadows_gain;
    LOG_DEBUG("final output %u  %u  %u", ae_out->ae_adgain, ae_out->ae_shutter, ae_out->ae_ispgain);

}

int32_t AEConfigParamSet(IspSoftHandle handle, parseParam* data)
{
    if (handle == NULL)
    {
        return _AL_AELIB_INVALID_HANDLE;
    }
    al4ahd* pal4a = (al4ahd*)handle;
    unsigned int ae_err = _AL_3ALIB_SUCCESS;

    void** ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj* ae_func_obj = &(pal4a->ae_func_obj);
    struct ae_set_param_t* ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t* ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t* ae_output = &(pal4a->ae_output);
    struct alhw3a_ae_proc_data_t* ae_proc_data = &(pal4a->ae_proc_data);

    /*set pipedelay*/
    ae_set_para->ae_set_param_type = AE_SET_PARAM_PIPEDELAY;
    ae_set_para->set_param.pipeDelay = data->configInfo.pipedelay;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /*set ae enable AE_SET_PARAM_INIT_SETTING*/
    ae_set_para->set_param.ae_initial_setting.ae_enable = data->configInfo.aeEnable;


    /*set preview_sensor_info*/
    /*before AE_SET_PARAM_SENSOR_HW_EXPO_UPDATE because cal minExpBV*/
    ae_set_para->ae_set_param_type = AE_SET_PARAM_SENSOR_GAIN_UPDATE;
    ae_set_para->set_param.preview_sensor_info.max_gain = data->configInfo.maxGain;
    ae_set_para->set_param.preview_sensor_info.min_gain = data->configInfo.minGain;
    ae_set_para->set_param.preview_sensor_info.max_SensorGain = data->configInfo.maxSensorGain;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ae_set_para->ae_set_param_type = AE_SET_PARAM_SENSOR_HW_EXPO_UPDATE;
    ae_set_para->set_param.preview_sensor_info.exposuretime_per_exp_line_ns = data->configInfo.exptimePerLineNs;
    ae_set_para->set_param.preview_sensor_info.max_line_cnt = data->configInfo.maxLineCnt;
    ae_set_para->set_param.preview_sensor_info.min_line_cnt = data->configInfo.minLineCnt;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ae_set_para->ae_set_param_type = AE_SET_PARAM_SENSOR_SW_EXPO_UPDATE;
    ae_set_para->set_param.preview_sensor_info.sw_max_exptime = data->configInfo.swMaxExptime;
    ae_set_para->set_param.preview_sensor_info.sw_min_exptime = data->configInfo.swMinExptime;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ae_set_para->ae_set_param_type = AE_SET_PARAM_SENSOR_FPS_UPDATE;
    ae_set_para->set_param.preview_sensor_info.max_fps = data->configInfo.maxfps;
    ae_set_para->set_param.preview_sensor_info.min_fps = data->configInfo.minfps;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ae_set_para->ae_set_param_type = AE_SET_PARAM_SENSOR_FN_UPDATE;
    ae_set_para->set_param.preview_sensor_info.f_number_x1000 = data->configInfo.fNumberx1000;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    return ae_err;
}

int32_t AeTuningParseSet(IspSoftHandle handle, parseParam* data)
{
    if (handle == NULL)
    {
        return _AL_AELIB_INVALID_HANDLE;
    }
    al4ahd* pal4a = (al4ahd*)handle;
    unsigned int ae_err = _AL_3ALIB_SUCCESS;

    void** ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj* ae_func_obj = &(pal4a->ae_func_obj);
    struct ae_set_param_t* ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t* ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t* ae_output = &(pal4a->ae_output);
    struct alhw3a_ae_proc_data_t* ae_proc_data = &(pal4a->ae_proc_data);

    LOG_DEBUG("aeparam version %f %d", data->paramVer.minorVer, data->paramVer.majorVer);
    /* set weight table mode */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_METERING_MODE;
    ae_set_para->set_param.ae_metering_mode = (ae_metering_mode_type_t)data->paramSingleInt[ae_metering_mode][1];  //  sync with ae_metering_mode_type_t: 0: average, 1: user define
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ///* set manual_isolevel  */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_MANUAL_ISO_LEVEL;
    ae_set_para->set_param.manual_isolevel = (ae_iso_mode_t)data->paramSingleInt[manual_isolevel][1];
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /* set manu ae  */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_MANUAL_EXPTIME;
    ae_set_para->set_param.manual_exptime = data->paramSingleInt[manual_exptime][1];
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /* set manu ae  */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_MANUAL_ADGAIN;
    ae_set_para->set_param.manual_adgain = data->paramSingleInt[manual_adgain][1];
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ///* set manual_ae_target  */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_MANUAL_AE_TARGET;
    ae_set_para->set_param.manual_ae_target = data->paramSingleInt[manual_ae_target][1];
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ///* set wdrEnable  */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_WDR_ENABLE;
    ae_set_para->set_param.wdr_enable = data->wdrEnable;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /*set ucAntiFlickerMode*/
    ae_set_para->ae_set_param_type = AE_SET_PARAM_ANTIFLICKERMODE;
    ae_set_para->set_param.ucAntiFlickerMode = data->paramSingleInt[ucAntiFlickerMode][1];
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /*set iae tuning param*/
    ae_set_para->ae_set_param_type = AE_SET_PARAM_iAE_TUNING_PARAM;
    for (int i = 0; i < maxIaeParamSwitchCase; i++)
    {
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_high = data->paramIAEHist[0][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_low = data->paramIAEHist[1][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_high = data->paramIAEHist[2][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_low = data->paramIAEHist[3][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_end[0] = data->paramIAEHist[4][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_end[1] = data->paramIAEHist[5][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_end[2] = data->paramIAEHist[6][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_start[0] = data->paramIAEHist[7][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_start[1] = data->paramIAEHist[8][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_start[2] = data->paramIAEHist[9][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_end[0] = data->paramIAEHist[10][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_end[1] = data->paramIAEHist[11][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_end[2] = data->paramIAEHist[12][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_start[0] = data->paramIAEHist[13][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_start[1] = data->paramIAEHist[14][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_start[2] = data->paramIAEHist[15][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_end[0] = data->paramIAEHist[16][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_end[1] = data->paramIAEHist[17][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_end[2] = data->paramIAEHist[18][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_start[0] = data->paramIAEHist[19][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_start[1] = data->paramIAEHist[20][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_start[2] = data->paramIAEHist[21][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].max_adjust_ratio = data->paramIAEHist[22][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].min_adjust_ratio = data->paramIAEHist[23][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].max_midtones_gain = data->paramIAEHist[24][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].max_shadows_gain = data->paramIAEHist[25][i];
        ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].max_isp_midtones_gain = data->paramIAEHist[26][i];

    }
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }
    /*set iae tuning param*/
    ae_set_para->ae_set_param_type = AE_SET_PARAM_iAE_PARAM_BV_TH;
    for (int i = 0; i < maxIaeBVThCount; i++)
    {
        ae_set_para->set_param.iAE_param.iaeluxIdxTh[i] = data->paramIAEBVTh[i];
    }

    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /*set iae HistRatio tuning param*/
    ae_set_para->ae_set_param_type = AE_SET_PARAM_iAE_PARAM_HIST_RATIO;
    for (int i = 0; i < maxIaeHistRatioCount; i++)
    {
        ae_set_para->set_param.iAE_param.iaeHistRatio[i] = data->paramIAEHistRatio[i];
    }

    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ///* set smooth level  */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_CONVERGE_SPD;
    ae_set_para->set_param.converge_speedlv = (ae_converge_level_type_t)data->paramSingleInt[converge_speedlv][1];
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ///* set targetTable  */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_TARGET_TABLE;
    for (int i = 0; i < sceneCnt; i++)
    {
        ae_set_para->set_param.ae_target_table[i].aeTargetEn = data->targetTable[i].aeTargetEn;
        memcpy(ae_set_para->set_param.ae_target_table[i].table, data->targetTable[i].aetargetTable, sizeof(ae_set_para->set_param.ae_target_table[i].table));
    }
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ///* set expoTable  */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_EXPO_TABLE;
    for (int i = 0; i < aeTable_Cnt; i++)
    {
        ae_set_para->set_param.ae_expo_table[i].aeExpoEn = data->expousureTable[i].aeExpoEn;
        memcpy(ae_set_para->set_param.ae_expo_table[i].table, data->expousureTable[i].aeExpoTable, sizeof(ae_set_para->set_param.ae_expo_table[i].table));
    }
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /* set target mode */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_TARGET_MODE;
    ae_set_para->set_param.ae_target_mode = (ae_target_mode_type_t)data->paramSingleInt[ae_target_mode][1];
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /* set EXPO mode */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_EXPO_MODE;
    ae_set_para->set_param.ae_expo_mode = (ae_expo_mode_type_t)data->paramSingleInt[ae_expo_mode][1];
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /*set multiCam Param*/
    ae_set_para->ae_set_param_type = AE_SET_PARAM_MULTICAM_PARAM;
    memcpy(&ae_set_para->set_param.multiCamSetParam, &data->multiCamParam, sizeof(ae_set_para->set_param.multiCamSetParam));
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    return ae_err;
}


static IspSoftHandle AeInit(void* owner, IspSoftPostMessageFunc postMessage, void* data)
{

    if (owner == NULL)
    {
        return NULL;
    }
    if (data == NULL)
    {
        return NULL;
    }

    unsigned int ae_err = _AL_3ALIB_SUCCESS;
    //algo init
    /*  ===========================================
    *     Get function version.
    *   =========================================== */
    al4ahd* pal4a = (al4ahd*)malloc(sizeof(al4ahd));
    if (pal4a == nullptr)
    {
        return NULL;
    }
    pal4a->postMessage = postMessage;
    pal4a->owner = owner;

   

    //debug level get from init
    ae_err = allib_ae_set_debug_log_flag(0b011010, TRUE, TRUE);//LOG DEBUG AND INFO AND ERROR 

    allib_ae_getlib_version(&pal4a->ae_ver);
    LOG_INFO("AE ver: %ld_V%.2f\n", pal4a->ae_ver.major_version, pal4a->ae_ver.minor_version);
    /*  ===========================================
    *     Load tuning file and psudo stats.
    *   =========================================== */
    //err = load_tuning_bin_and_stats();
    //if (err) {
    //    return err;
    //}

    /*  ===========================================
    *     Load lib.
    *   =========================================== */

    ae_err = allib_ae_loadfunc(&(pal4a->ae_func_obj), ((IspSoftAeInitParam*)data)->camera_id);
    if (ae_err) {
        LOG_ERROR("ERROR %x\r\n", (unsigned int)ae_err);
        return NULL;
    }

    void** ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj* ae_func_obj = &(pal4a->ae_func_obj);
    struct ae_set_param_t* ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t* ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t* ae_output = &(pal4a->ae_output);

    ae_err = ae_func_obj->initial(ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        LOG_ERROR("ERROR %x\r\n", (unsigned int)ae_err);
        return NULL;
    }

    ae_set_para->set_param.ae_initial_setting.ae_enable = 1;

    if ((parseParam*)((IspSoftAeInitParam*)data)->tuning_bin.Data == NULL || ((IspSoftAeInitParam*)data)->tuning_bin.Length != sizeof(parseParam))
    {
        LOG_ERROR("ERROR tuning bin %x\r\n", (unsigned int)_AL_AELIB_INVALID_BIN_FILE);
    }
    else
    {
        parseParam paramSim;
        memcpy(&paramSim, (parseParam*)((IspSoftAeInitParam*)data)->tuning_bin.Data, sizeof(parseParam));

        /*set configparam when initial*/
        AEConfigParamSet(pal4a, &paramSim);

        /*set when update tuning*/
        AeTuningParseSet(pal4a, &paramSim);
    }

    //if ((ae_calibration_data_t*)((IspSoftAeInitParam*)data)->otp_bin.Data != NULL && ((IspSoftAeInitParam*)data)->otp_bin.Length == sizeof(ae_calibration_data_t))
    //{
    //     ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.min_iso = ((ae_calibration_data_t*)((IspSoftAeInitParam*)data)->otp_bin.Data)->min_iso;
    //     ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.calib_b_gain = ((ae_calibration_data_t*)((IspSoftAeInitParam*)data)->otp_bin.Data)->calib_b_gain;
    //     ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain = ((ae_calibration_data_t*)((IspSoftAeInitParam*)data)->otp_bin.Data)->calib_g_gain;
    //     ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.calib_r_gain = ((ae_calibration_data_t*)((IspSoftAeInitParam*)data)->otp_bin.Data)->calib_r_gain;
    //    
    //} 
    ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.min_iso = 0;
    ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.calib_b_gain = 0;
    ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain = 0;
    ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.calib_r_gain = 0;

    ae_set_para->set_param.ae_initial_setting.afd_flicker_mode = ANTIFLICKER_OFF; 

    ae_set_para->set_param.cameraId = ae_func_obj->identityid;

    ae_set_para->ae_set_param_type = AE_SET_PARAM_INIT_SETTING;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        LOG_ERROR("ERROR %x\r\n", (unsigned int)ae_err);
        return NULL;
    }

    //ae_get_para->ae_get_param_type = AE_GET_INIT_EXPOSURE_PARAM;
    //ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    //if (ae_err != _AL_3ALIB_SUCCESS) {
    //    LOG_ERROR("ERROR %x\r\n", (unsigned int)ae_err);
    //    return NULL;
    //}

   ae_set_para->ae_set_param_type = AE_SET_PARAM_INIT_EXPO;
   ae_set_para->set_param.sof_notify_param.exp_time = ((IspSoftAeInitParam*)data)->initial_sensor_shutter_us;
   ae_set_para->set_param.sof_notify_param.exp_adgain = ((IspSoftAeInitParam*)data)->initial_sensor_adgain;
   ae_set_para->set_param.sof_notify_param.midtones_gain = 100;
   ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
   if (ae_err != _AL_3ALIB_SUCCESS) {
       LOG_ERROR("ERROR %x\r\n", (unsigned int)ae_err);
       return NULL;
   }

    //last_debug_level = LogLevel_Info;  //default off
    //last_save_to_file = false;  //default false

    ae_set_para->ae_set_param_type = AE_SET_PARAM_INITEXPOLIST;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        LOG_ERROR("ERROR %x\r\n", (unsigned int)ae_err);
        return NULL;
    }

    if (((IspSoftAeInitParam*)data)->config_path != NULL)
    {
        configHALPath = ((IspSoftAeInitParam*)data)->config_path;
    }

    ae_err = allib_ae_set_debug_log_flag((enum LogLevel_em)LogLevel_OFF, false, TRUE);
    return pal4a;
}


static int32_t AeLoadTuningBin(IspSoftHandle handle, void* data)  //算法tuning参数配置
{
    if (handle == NULL)
    {
        return _AL_AELIB_INVALID_HANDLE;
    }
    //string configHALPath = GetCurrMoudle();
    string txtAddr;
    txtAddr = configHALPath + "\\AEParam.bin";
    fstream fin;
    fin.open(txtAddr, ios::binary | ios::in);
    parseParam paramSim;
    memset(&paramSim, 0, sizeof(parseParam));
    if (fin.is_open())
    {
        fin.read((char*)&paramSim, sizeof(paramSim));
        fin.close();
    }
    else
    {
        LOG_ERROR("打开AEParam.bin失败！ %s\n", txtAddr.c_str());
        return _AL_AELIB_INVALID_BIN_FILE;
    }
    
    // algo set tuning param
    return AeTuningParseSet(handle, &paramSim);
}


static int32_t AeLoadDebugSetting(IspSoftHandle handle, void* data)  //算法debug参数配置
{
    if (handle == NULL)
    {
        return _AL_AELIB_INVALID_HANDLE;
    }

    unsigned int ae_err = _AL_3ALIB_SUCCESS;
    // algo set init param
    //string configHALPath = GetCurrMoudle();
    string debugAddr;
    debugAddr = configHALPath + "\\AEDebug.txt";
    int level = LogLevel_OFF;
    BOOL save_flag = false;
    BOOL terminal_flag = false;

    FILE* fp;
    fp = fopen(debugAddr.c_str(), "r");
    if (NULL == fp)
    {
        level = LogLevel_Error;
        save_flag = false;
        terminal_flag = true;
        //set debug level
        ae_err = allib_ae_set_debug_log_flag(level, save_flag, terminal_flag);
        LOG_ERROR("Fail open AEDebug.txt!  %s", debugAddr.c_str());
    }
    else
    {
        char buf[1024];
        for(int i = 0; i < 3; i++)
        {
            fgets(buf,1024,fp);
            if (i == 0)
            {
                int loglevel = atoi(buf);
                level = loglevel;
            }
            if (i == 1)
            {
                int saveflag = atoi(buf);
                save_flag = (saveflag == 1) ? TRUE : FALSE;
            }
            if (i == 2)
            {
                int terflag = atoi(buf);
                terminal_flag = (terflag == 1) ? TRUE : FALSE;
            }
        }
        //set debug level
        ae_err = allib_ae_set_debug_log_flag(level, save_flag, terminal_flag);
        LOG_DEBUG("set loglevel %d  saveflag %d  terminal_flag %d", level, save_flag, terminal_flag);
        fclose(fp);
    }
    return ae_err;
}


static int32_t AeRun(IspSoftHandle handle, IspSoftAeInImage* data) 
{
    if (handle == NULL)
    {
        return _AL_AELIB_INVALID_HANDLE;
    }
    if (data == NULL)
    {
        return _AL_AELIB_INVALID_INPUTDATA;
    }
    // algo process
    if (data->isp_out.Data == NULL || data->isp_out.Length != sizeof(IspSoftIspAeStatusOut))
    {
        return _AL_WRP_AE_EMPTY_ISPDATA;
    }

    al4ahd* pal4a = (al4ahd*)handle;
    unsigned int ae_err = _AL_3ALIB_SUCCESS;
    void** ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj* ae_func_obj = &(pal4a->ae_func_obj);
    struct ae_set_param_t* ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t* ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t* ae_output = &(pal4a->ae_output);
    struct alhw3a_ae_proc_data_t* ae_proc_data = &(pal4a->ae_proc_data);

    IspSoftIspAeStatusOut* isp_out = (IspSoftIspAeStatusOut*)data->isp_out.Data;
    IspSoftAtfOut* atf_out = (IspSoftAtfOut*)data->atf_out.Data;
    IspSoftAwbOut* awb_out = (IspSoftAwbOut*)data->awb_out.Data;

    IspSoftAtfOut atf_tmp_out;
    if (data->atf_out.Data == NULL && data->atf_out.Length == 0)
    {
        atf_tmp_out.atf_mode = (atf_mode)0;
        atf_tmp_out.flag_enable = 0;
        atf_tmp_out.atf_mode_stable = (atf_mode)0;
        atf_tmp_out.flag_bending = 0;
        atf_out = &atf_tmp_out;
    }
    else if (data->atf_out.Data == NULL || data->atf_out.Length != sizeof(IspSoftAtfOut))
    {
        return _AL_WRP_AE_EMPTY_ATFDATA;
    }

    IspSoftAwbOut awb_tmp_out;
    if (data->awb_out.Data == NULL && data->awb_out.Length == 0)
    {
        awb_tmp_out.awb_update = 0;
        awb_tmp_out.rg_allbalance = 57; //scale 100, (rgbalance/100) * 256 
        awb_tmp_out.bg_allbalance = 63;
        awb_tmp_out.color_temp = 6500;
        awb_tmp_out.r_gain = 512;
        awb_tmp_out.g_gain = 256;
        awb_tmp_out.b_gain = 512;
        for (int i = 0; i < 9; i++)
        {
            awb_tmp_out.final_CCM[i] = 1.0f;
        }
        for (int i = 0; i < 13 * 17 * 4; i++)
        {
            awb_tmp_out.final_lsc[i] = 1.0f;
        }
        awb_out = &awb_tmp_out;
    }
    else if (data->awb_out.Data == NULL || data->awb_out.Length != sizeof(IspSoftAwbOut))
    {
        return _AL_WRP_AE_EMPTY_AWBDATA;
    }


    /* set effective exposure time and effective adgain which getting from sensor
    *  for the sample example,  we assume effective exposure and effective gain
    *  are the same as AE output report from the previous estimation
    */
    ae_set_para->set_param.frameid = data->frame_id;
    ae_set_para->ae_set_param_type = AE_SET_PARAM_FRAMEID;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }
    ae_sensorExpoList expoSOF;
    ae_get_para->ae_get_param_type = AE_GET_CUREXPOLIST;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }
    memcpy(&expoSOF, &ae_get_para->para.expoSOF,sizeof(ae_sensorExpoList)) ;

    uint32_t sofidx = data->frame_id;
    ae_set_para->ae_set_param_type = AE_SET_PARAM_SOF_NOTIFY;

    LOG_DEBUG("check ISPOut gain %u  shutter %u, check expSOF gain %f  shutter %f  ispgain %f  midtonegain %f", isp_out->sensor_adgain, isp_out->sensor_shutter_us, expoSOF.sensorGain, expoSOF.shutter, expoSOF.ispGain, expoSOF.midtoneGain);
    LOG_WARN("check ISPOut gain %u  shutter %u, check expSOF gain %f  shutter %f  ispgain %f  midtonegain %f", isp_out->sensor_adgain, isp_out->sensor_shutter_us, expoSOF.sensorGain, expoSOF.shutter, expoSOF.ispGain, expoSOF.midtoneGain);
    ae_set_para->set_param.sof_notify_param.exp_adgain = isp_out->sensor_adgain * expoSOF.ispGain;
    ae_set_para->set_param.sof_notify_param.exp_time = isp_out->sensor_shutter_us;
    ae_set_para->set_param.sof_notify_param.midtones_gain = expoSOF.midtoneGain;
    ae_set_para->set_param.sof_notify_param.sys_sof_index = sofidx;

    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }


    /* set anti-flicker mode from AFD output results */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_ANTIFLICKER_MODE;
    ae_set_para->set_param.afd_flicker_mode = (ae_antiflicker_mode_t)atf_out->atf_mode_stable;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }
    ///* set stats blk size and modift weight table */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_WEIGHT_TABLE_PARAM;
    ae_set_para->set_param.statsBankNum = isp_out->ae_stats_blockX;
    ae_set_para->set_param.statsBlockNum = isp_out->ae_stats_blockY;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }
    ///* set awbgain */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_AWBGAIN;
    ae_set_para->set_param.awbInfo.rGain = awb_out->r_gain;
    ae_set_para->set_param.awbInfo.gGain = awb_out->g_gain;
    ae_set_para->set_param.awbInfo.bGain = awb_out->b_gain;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }
    ///* set gaze ae enable  */
    //ae_set_para->ae_set_param_type = AE_SET_PARAM_GAZE_ENABLE;
    //ae_set_para->set_param.gaze_enable = 1;
    //ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    //if (ae_err != _AL_3ALIB_SUCCESS) {
    //    return ae_err;
    //}

    /////* set gaze ae ROI  */
    //ae_set_para->ae_set_param_type = AE_SET_PARAM_OBJECT_ROI;
    //ae_set_para->set_param.ae_set_object_roi_setting.roi_count = 1;
    //ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    //if (ae_err != _AL_3ALIB_SUCCESS) {
    //    return ae_err;
    //}

    /* TO DO:
    * Prepare HW stats data and wait frame done event
    */
    ae_proc_data->udsys_sof_idx = data->frame_id;

    /* Get AE stats from hw */
    ae_err = ae_func_obj->process(isp_out->ae_stats, isp_out->ae_hist, *ae_proc_data, *ae_obj, ae_output);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    
    //out
    ae_sensorExpoList expoSOFFront;
    ae_get_para->ae_get_param_type = AE_GET_CUREXPOLISTFRONT;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    memcpy(&expoSOFFront, &ae_get_para->para.expoSOF, sizeof(ae_sensorExpoList));

    UINT32 maxSensorGain;
    ae_get_para->ae_get_param_type = AE_GET_MAXSENSORGAIN;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    maxSensorGain = ae_get_para->para.maxSensorGain;

    ae_set_para->ae_set_param_type = AE_SET_PARAM_EXPOSOFOUT;
    ae_set_para->set_param.expoSOFOut.frameID = data->frame_id;
    ae_set_para->set_param.expoSOFOut.shutter = ae_output->udexposure_time;
    ae_set_para->set_param.expoSOFOut.sensorGain =  min(ae_output->udsensor_ad_gain / 100.0f, maxSensorGain / 100.0f);
    ae_set_para->set_param.expoSOFOut.ispGain = (ae_output->udsensor_ad_gain > maxSensorGain) ? (ae_output->udsensor_ad_gain * 1.0f / maxSensorGain) : 1.0f;
    ae_set_para->set_param.expoSOFOut.midtoneGain = ae_output->midtones_gain;
    ae_set_para->set_param.expoSOFOut.ispmidtoneGain = ae_output->isp_midtones_gain;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    //out  update postMessage type and data
    uint32_t postType;
    IspSoftPostMessageFunc postMessage = (IspSoftPostMessageFunc)pal4a->postMessage;

    //outData
    IspSoftAeOutReady outData;
    IspSoftAeOut ae_out;
    outData.frame_id = data->frame_id;
    outData.timestamp = data->timestamp;
    updateOutData(&outData,ae_output, &expoSOFFront, &ae_out, maxSensorGain);
    outData.ae_out.Data = (uint8_t*) & ae_out;
    outData.ae_out.Length = sizeof(IspSoftAeOut);
    postType = ISP_SOFT_AE_POST_OUT_READY;
    postMessage(pal4a->owner,postType, &outData);

    //exifdata
    ae_get_para->ae_get_param_type = AE_GET_DEBUG_INFO;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }


    LOG_SAVE;
   
    return ae_err;
}

static int32_t AeProcess(IspSoftHandle handle, uint32_t processType, void *data)
{
    // ?
    unsigned int ae_err = _AL_3ALIB_SUCCESS;
    switch (processType)
    {
    case ISP_SOFT_AE_PROCESS_IN_TUNING:
        return AeLoadTuningBin(handle, data);   //load tuning bin

    case ISP_SOFT_AE_PROCESS_IN_DEBUG:
        return AeLoadDebugSetting(handle, data);   //load debug config

    case ISP_SOFT_AE_PROCESS_IN_IMAGE:
        return AeRun(handle,(IspSoftAeInImage*)data);  //algo process
    default:
        break;
    }
    return ae_err;
}


static int32_t AeDeinit(IspSoftHandle handle)
{
    al4ahd* pal4a = (al4ahd*)handle;
    unsigned int ae_err = _AL_3ALIB_SUCCESS;
    ae_err = pal4a->ae_func_obj.deinit(pal4a->ae_obj);

    if (NULL != pal4a) {
        free(pal4a);
        pal4a = NULL;
    }

    if (ae_err != _AL_3ALIB_SUCCESS) {
        printf("ae deinit error\r\n");
        return ae_err;
    }
    return ae_err;
}


IspSoftModule IspSoftAeLoad()
{
    IspSoftModule result = { 0 };
    result.Init = AeInit;
    result.Process = AeProcess;
    result.Deinit = AeDeinit;

    return result;
}