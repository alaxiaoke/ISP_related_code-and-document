//#ifndef _AE_HANDLE_C_
//#define _AE_HANDLE_C_
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdint>


#include <fstream>
#include <iostream>
#include <direct.h>
#include <queue>

#include "./Include/allib_ae.h"
#include "./Include/allib_ae_errcode.h"
#include "./aeHandlepcmr.h"
#include <queue>
#include "./core/include/MyLog.h"
using namespace std;


enum LogLevel                   last_debug_level;  //default off
bool                            last_save_to_file;  //default false
bool                            simTuningEnableLast;
queue<vector<float>>            expousureList;

enum LogLevel_em w_emCurLogLevel = LogLevel_OFF;
int g_log_terminal = 1;
int g_log_to_file = 0;
char* logbuffer = nullptr;

int isSimulate = 0;

unsigned int ae_initial(gxr_aehd* gxr_ae)
{
    if (gxr_ae->err_code == _AL_3ALIB_SIMULATE)
    {
        isSimulate = 1;
    }

    /*  ===========================================
    *     Get function version.
    *   =========================================== */
    al4ahd* pal4a = (al4ahd*)malloc(sizeof(al4ahd));
    gxr_ae->al4a = pal4a;
    unsigned int ae_err = _AL_3ALIB_SUCCESS;

    //debug level get from init
    ae_err = allib_ae_set_debug_log_flag((enum LogLevel_em)Info, false);

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

    ae_err = allib_ae_loadfunc(&(pal4a->ae_func_obj), gxr_ae->cameraId);
    if (ae_err) {
        LOG_DEBUG("ERROR %x\r\n", (unsigned int)ae_err);
        gxr_ae->err_code = ae_err;
        return ae_err;
    }

    void** ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj* ae_func_obj = &(pal4a->ae_func_obj);
    struct ae_set_param_t* ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t* ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t* ae_output = &(pal4a->ae_output);

    ae_err = ae_func_obj->initial(ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        gxr_ae->err_code = ae_err;
        return ae_err;
    }

    ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.min_iso = gxr_ae->min_iso;
    ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.calib_b_gain = gxr_ae->calib_b_gain;
    ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain = gxr_ae->calib_g_gain;
    ae_set_para->set_param.ae_initial_setting.ae_calib_wb_gain.calib_r_gain = gxr_ae->calib_r_gain;

    ae_set_para->set_param.ae_initial_setting.afd_flicker_mode = (ae_antiflicker_mode_t)gxr_ae->atf_mode;

    ae_set_para->set_param.ae_initial_setting.ae_enable = gxr_ae->ae_enable;

    ae_set_para->set_param.cameraId = ae_func_obj->identityid;

    ae_set_para->ae_set_param_type = AE_SET_PARAM_INIT_SETTING;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        gxr_ae->err_code = ae_err;
        return ae_err;
    }

    ae_get_para->ae_get_param_type = AE_GET_INIT_EXPOSURE_PARAM;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        gxr_ae->err_code = ae_err;
        return ae_err;
    }

    if (isSimulate)
    {
        ae_set_para->ae_set_param_type = AE_SET_PARAM_INIT_EXPO;
        ae_set_para->set_param.sof_notify_param.exp_time = gxr_ae->ae_shutter;
        ae_set_para->set_param.sof_notify_param.exp_adgain = gxr_ae->ae_adgain;
        ae_set_para->set_param.sof_notify_param.midtones_gain = gxr_ae->midtones_gain;
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            gxr_ae->err_code = ae_err;
            return ae_err;
        }
    }




    last_debug_level = Info;  //default off
    last_save_to_file = false;  //default false
    simTuningEnableLast = false;
    gxr_ae->err_code = ae_err;

    UINT8 pipeDelay = 4;
    ae_get_para->ae_get_param_type = AE_GET_INIT_PIPEDELAY;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);

    pipeDelay = ae_get_para->para.pipeDelay;
    LOG_INFO("Init pipeDelay %u", pipeDelay);
    vector<float> expoInit;
    expoInit.push_back(3900.0f); //init exptime
    expoInit.push_back(100.0f);  //init adgain
    expoInit.push_back(1.0f);  //init ispgain
    expoInit.push_back(0.0f);    //sofid
    expoInit.push_back(100.0f);  //init midtones gain

    for (int i = 0; i < pipeDelay; i++)
    {
        expoInit[3] = i * 1.0f;
        expousureList.push(expoInit);
    }

    return ae_err;
}

unsigned int ae_deinit(gxr_aehd* gxr_ae)
{
    al4ahd* pal4a = (al4ahd*)(gxr_ae->al4a);
    unsigned int ae_err = _AL_3ALIB_SUCCESS;
    ae_err = pal4a->ae_func_obj.deinit(pal4a->ae_obj);

    if (NULL != pal4a) {
        free(pal4a);
        pal4a = NULL;
    }

    if (ae_err != _AL_3ALIB_SUCCESS) {
        printf("ae deinit error\r\n");
        gxr_ae->err_code = ae_err;
        return ae_err;
    }
    gxr_ae->err_code = ae_err;
    return ae_err;
}

string UnicodeToAnsi_str(const wchar_t* szStr)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
    if (nLen == 0)
    {
        return NULL;
    }
    char* pResult = new char[nLen];
    WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
    string  str(pResult);
    delete[] pResult;
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
    string strpath = dllAddr.substr(0, dwfind);
    string strname = dllAddr.substr(dwfind + 1);
    LOG_DEBUG("strpath %s\n", strpath.c_str());

    return strpath;
}

unsigned int ae_run(gxr_aehd* gxr_ae)
{
    al4ahd* pal4a = (al4ahd*)(gxr_ae->al4a);
    unsigned int ae_err = _AL_3ALIB_SUCCESS;

    void** ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj* ae_func_obj = &(pal4a->ae_func_obj);
    struct ae_set_param_t* ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t* ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t* ae_output = &(pal4a->ae_output);
    struct alhw3a_ae_proc_data_t* ae_proc_data = &(pal4a->ae_proc_data);

    /* set effective exposure time and effective adgain which getting from sensor
    *  for the sample example,  we assume effective exposure and effective gain
    *  are the same as AE output report from the previous estimation
    */
    
    string dllAddr = GetCurrMoudle();
    string qtAddr;
    qtAddr = dllAddr + "\\QtWidgetsApplication1.exe";
    const char* qtAddrSystem = qtAddr.data();
    if (simTuningEnableLast == 0 && gxr_ae->tuning_enable == 1)
    //if (gxr_ae->tuning_enable == 1)
    {
        //SHELLEXECUTEINFO ShExecInfo = { 0 };
        //ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        //ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        //ShExecInfo.hwnd = NULL;
        //ShExecInfo.lpVerb = NULL;

        //WCHAR testPFlie[2048];
        //memset(testPFlie, 0, sizeof(testPFlie));
        //MultiByteToWideChar(CP_ACP, 0, qtAddrSystem, strlen(qtAddrSystem) + 1, testPFlie, sizeof(testPFlie) / sizeof(testPFlie[0]));
        //ShExecInfo.lpFile = testPFlie;
        //ShExecInfo.lpParameters = L"";
        //ShExecInfo.lpDirectory = NULL;
        //ShExecInfo.nShow = SW_SHOWNORMAL;
        //ShExecInfo.hInstApp = NULL;
        //ShellExecuteEx(&ShExecInfo);
        //WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
        
        string txtAddr;
        txtAddr = dllAddr + "\\AEParam.txt";
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
            LOG_ERROR("´ò¿ªAEParam.txtÊ§°Ü£¡ %s\n", txtAddr.c_str());
        }
        /* set weight table mode */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_METERING_MODE;
        ae_set_para->set_param.ae_metering_mode = (ae_metering_mode_type_t)paramSim.paramSingleInt[ae_metering_mode][1];  //  sync with ae_metering_mode_type_t: 0: average, 1: user define
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            gxr_ae->err_code = ae_err;
            return ae_err;
        }

        ///* set manual_isolevel  */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_MANUAL_ISO_LEVEL;
        ae_set_para->set_param.manual_isolevel = (ae_iso_mode_t)paramSim.paramSingleInt[manual_isolevel][1];
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }

        /* set manu ae  */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_MANUAL_EXPTIME;
        ae_set_para->set_param.manual_exptime = paramSim.paramSingleInt[manual_exptime][1];
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }

        /* set manu ae  */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_MANUAL_ADGAIN;
        ae_set_para->set_param.manual_adgain = paramSim.paramSingleInt[manual_adgain][1];
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }

        ///* set manual_ae_target  */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_MANUAL_AE_TARGET;
        ae_set_para->set_param.manual_ae_target = paramSim.paramSingleInt[manual_ae_target][1];
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }

        ///* set wdrEnable  */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_WDR_ENABLE;
        ae_set_para->set_param.wdr_enable = paramSim.wdrEnable;
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }

        /*set iae tuning param*/
        ae_set_para->ae_set_param_type = AE_SET_PARAM_iAE_TUNING_PARAM;
        for (int i = 0; i < maxIaeParamSwitchCase; i++)
        {
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_high = paramSim.paramIAEHist[0][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_low = paramSim.paramIAEHist[1][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_high = paramSim.paramIAEHist[2][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_low = paramSim.paramIAEHist[3][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_end[0] = paramSim.paramIAEHist[4][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_end[1] = paramSim.paramIAEHist[5][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_end[2] = paramSim.paramIAEHist[6][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_start[0] = paramSim.paramIAEHist[7][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_start[1] = paramSim.paramIAEHist[8][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_dark_range_start[2] = paramSim.paramIAEHist[9][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_end[0] = paramSim.paramIAEHist[10][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_end[1] = paramSim.paramIAEHist[11][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_end[2] = paramSim.paramIAEHist[12][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_start[0] = paramSim.paramIAEHist[13][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_start[1] = paramSim.paramIAEHist[14][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushdown_range_start[2] = paramSim.paramIAEHist[15][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_end[0] = paramSim.paramIAEHist[16][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_end[1] = paramSim.paramIAEHist[17][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_end[2] = paramSim.paramIAEHist[18][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_start[0] = paramSim.paramIAEHist[19][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_start[1] = paramSim.paramIAEHist[20][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].hist_sat_pushup_range_start[2] = paramSim.paramIAEHist[21][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].max_adjust_ratio = paramSim.paramIAEHist[22][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].min_adjust_ratio = paramSim.paramIAEHist[23][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].max_midtones_gain = paramSim.paramIAEHist[24][i];
            ae_set_para->set_param.iAE_param.iaeTuningCoreParam[i].max_shadows_gain = paramSim.paramIAEHist[25][i];

        }
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }
        /*set iae tuning param*/
        ae_set_para->ae_set_param_type = AE_SET_PARAM_iAE_PARAM_BV_TH;
        for (int i = 0; i < maxIaeBVThCount; i++)
        {
            ae_set_para->set_param.iAE_param.iaeluxIdxTh[i] = paramSim.paramIAEBVTh[i];
        }

        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }

        /*set iae HistRatio tuning param*/
        ae_set_para->ae_set_param_type = AE_SET_PARAM_iAE_PARAM_HIST_RATIO;
        for (int i = 0; i < maxIaeHistRatioCount; i++)
        {
            ae_set_para->set_param.iAE_param.iaeHistRatio[i] = paramSim.paramIAEHistRatio[i];
        }

        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }

        ///* set smooth level  */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_CONVERGE_SPD;
        ae_set_para->set_param.converge_speedlv = (ae_converge_level_type_t)paramSim.paramSingleInt[converge_speedlv][1];
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }

        ///* set targetTable  */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_TARGET_TABLE;
        for (int i = 0; i < sceneCnt; i++)
        {
            ae_set_para->set_param.ae_target_table[i].aeTargetEn = paramSim.targetTable[i].aeTargetEn;
            memcpy(ae_set_para->set_param.ae_target_table[i].table, paramSim.targetTable[i].aetargetTable, sizeof(ae_set_para->set_param.ae_target_table[i].table));
        }
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }

        ///* set expoTable  */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_EXPO_TABLE;
        for (int i = 0; i < aeTable_Cnt; i++)
        {
            ae_set_para->set_param.ae_expo_table[i].aeExpoEn = paramSim.expousureTable[i].aeExpoEn;
            memcpy(ae_set_para->set_param.ae_expo_table[i].table, paramSim.expousureTable[i].aeExpoTable, sizeof(ae_set_para->set_param.ae_expo_table[i].table));
        }
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            return ae_err;
        }

        /* set target mode */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_TARGET_MODE;
        ae_set_para->set_param.ae_target_mode = (ae_target_mode_type_t)paramSim.paramSingleInt[ae_target_mode][1];
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            gxr_ae->err_code = ae_err;
            return ae_err;
        }

        /* set EXPO mode */
        ae_set_para->ae_set_param_type = AE_SET_PARAM_EXPO_MODE;
        ae_set_para->set_param.ae_expo_mode = (ae_expo_mode_type_t)paramSim.paramSingleInt[ae_expo_mode][1];
        ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
        if (ae_err != _AL_3ALIB_SUCCESS) {
            gxr_ae->err_code = ae_err;
            return ae_err;
        }

    }  
    simTuningEnableLast = gxr_ae->tuning_enable;

    //DEBUG SET
    if (gxr_ae->debug_level != last_debug_level || gxr_ae->save_to_file != last_save_to_file) {  //or another tag for log level modify action   //or expose the func, tooler directly call to modify 
        ae_err = allib_ae_set_debug_log_flag((enum LogLevel_em)gxr_ae->debug_level, gxr_ae->save_to_file);
        last_debug_level = gxr_ae->debug_level;
        last_save_to_file = gxr_ae->save_to_file;
    }

    LOG_DEBUG("expoSOF1 %f, %f, %f\n", expousureList.front()[0], expousureList.front()[1], expousureList.front()[4]);
    vector<float> expoSOF;
    expoSOF = expousureList.front();

    UINT8 pipeDelay = 4;
    ae_get_para->ae_get_param_type = AE_GET_INIT_PIPEDELAY;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    pipeDelay = ae_get_para->para.pipeDelay;

    if (gxr_ae->frameId - expoSOF[3] >= pipeDelay + 1)
    {
        int interval = min(gxr_ae->frameId - expoSOF[3], pipeDelay + pipeDelay - 1);
        for (int i = 0; i < interval - pipeDelay; i++)
        {
            expousureList.pop();
        }
        expoSOF = expousureList.front();
        vector<float> expoSOFBack;
        expoSOFBack = expousureList.back();
        for (int i = interval - pipeDelay; i > 0; i--)
        {
            expoSOFBack[3] = gxr_ae->frameId - i;
            expousureList.push(expoSOFBack);
        }
        expousureList.pop();
    }
    else
    {
        expousureList.pop();
    }


    LOG_DEBUG("expoSOF2 %f, %f, %f\n", expousureList.front()[0], expousureList.front()[1], expousureList.front()[4]);
    uint32_t sofidx = gxr_ae->frameId;
    ae_set_para->ae_set_param_type = AE_SET_PARAM_SOF_NOTIFY;

    vector<float> expoSOFBack;
    expoSOFBack = expousureList.back();

    if (isSimulate)
    {
        ae_set_para->set_param.sof_notify_param.exp_adgain = gxr_ae->sensorgain;
        ae_set_para->set_param.sof_notify_param.exp_time = gxr_ae->shutter;
        ae_set_para->set_param.sof_notify_param.midtones_gain = gxr_ae->midtones_gain;
        LOG_DEBUG("inSimulate, not use expoSOF[]");
    }
    else
    {
        ae_set_para->set_param.sof_notify_param.exp_adgain = expoSOF[1] * expoSOF[2];
        ae_set_para->set_param.sof_notify_param.exp_time = expoSOF[0];
        ae_set_para->set_param.sof_notify_param.midtones_gain = expoSOF[4];
    }

    ae_set_para->set_param.sof_notify_param.sys_sof_index = sofidx;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        gxr_ae->err_code = ae_err;
        return ae_err;
    }

    /*  set enable*/
    ae_set_para->ae_set_param_type = AE_SET_PARAM_ENABLE;
    ae_set_para->set_param.ae_enable = gxr_ae->ae_enable;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        gxr_ae->err_code = ae_err;
        return ae_err;
    }

    /* set anti-flicker mode from AFD output results */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_ANTIFLICKER_MODE;
    ae_set_para->set_param.afd_flicker_mode = (ae_antiflicker_mode_t) gxr_ae->atf_mode;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        gxr_ae->err_code = ae_err;
        return ae_err;
    }
 
    ///* set stats blk size and modift weight table */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_WEIGHT_TABLE_PARAM;
    ae_set_para->set_param.statsBankNum = gxr_ae->statsBankNum;
    ae_set_para->set_param.statsBlockNum = gxr_ae->statsBolckNum;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        gxr_ae->err_code = ae_err;
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
    ae_proc_data->udsys_sof_idx = gxr_ae->frameId;

    /* Get AE stats from hw */
    ae_err = ae_func_obj->process(gxr_ae->ae_stats, gxr_ae->yhis_stats, *ae_proc_data, *ae_obj, ae_output);
    //out


    if (isSimulate)
    {
        gxr_ae->ae_adgain = ae_output->udsensor_ad_gain;
        gxr_ae->ae_ispgain = 1.0f;
    }
    else
    {
        gxr_ae->ae_adgain = min(ae_output->udsensor_ad_gain/100.0f, 16.0f);
        gxr_ae->ae_ispgain = expousureList.front()[2];
    }

    gxr_ae->ae_shutter = ae_output->udexposure_time;
    gxr_ae->ae_converged = ae_output->ae_converged;
    gxr_ae->Ymean = ae_output->ymean100_8bit;   //avg y mean
    gxr_ae->ae_lux_result = ae_output->lux_result;   // avg bv-->lux 
    gxr_ae->bgbv = ae_output->bg_bvresult;   // avg bv
    gxr_ae->nextbv = ae_output->bvresult;   // next bv
    gxr_ae->iso = ae_output->iso;
    gxr_ae->midtones_gain = ae_output->midtones_gain;
    gxr_ae->shadows_gain = ae_output->shadows_gain;

    vector<float> expoout;
    expoout.push_back(ae_output->udexposure_time);
    expoout.push_back(gxr_ae->ae_adgain * 100);
    //expoout.push_back(min(ae_output->udsensor_ad_gain, 1600.0f));
    expoout.push_back((ae_output->udsensor_ad_gain > 1600) ? (ae_output->udsensor_ad_gain / 1600.0f) : 1.0f);
    expoout.push_back(gxr_ae->frameId * 1.0f);
    expoout.push_back(gxr_ae->midtones_gain);
    expousureList.push(expoout);

    if (ae_err != _AL_3ALIB_SUCCESS) {
        printf("3\r\n");
        gxr_ae->err_code = ae_err;
        return ae_err;
    }
    ae_get_para->ae_get_param_type = AE_GET_DEBUG_INFO;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        gxr_ae->err_code = ae_err;
        return ae_err;
    }
    memcpy(&gxr_ae->exif_info, &ae_get_para->para.exif_info, sizeof(struct gxr_AE_exif_info_t));
    gxr_ae->err_code = ae_err;

    //saveFileLog(gxr_ae->save_to_file);
    LOG_SAVE;

    return ae_err;
}