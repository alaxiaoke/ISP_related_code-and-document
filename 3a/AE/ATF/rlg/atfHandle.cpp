#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdint>

#include <fstream>
#include <iostream>
#include <direct.h>
#include <windows.h>

#include "atfHandle.h"
#include "alAFDLib_Error_Code.h"
#include "allib_afd.h"

using namespace std;

enum LogLevel_em w_emCurLogLevel = LogLevel_OFF;
int g_log_terminal = 0;
int g_log_to_file = 0;
char* logbuffer = NULL;
//int tuning_enable = 0;


unsigned int atf_initial(gxr_atfhd* gxr_atf)
{
    al4ahd* pal4a = (al4ahd*)malloc(sizeof(al4ahd));
    gxr_atf->al4a = pal4a;

	allib_afd_getlib_version(&(pal4a->afd_ver));
	//printf("AFD ver: %d.%d\n", pal4a->afd_ver.major_version, pal4a->afd_ver.minor_version);
    LOG_DEBUG("AFD ver: %d.%d\n", pal4a->afd_ver.major_version, pal4a->afd_ver.minor_version);
	
	unsigned int afd_err = _AL_AFDLIB_SUCCESS;

	/*  ===========================================
*     Load tuning file and psudo stats.
*   =========================================== */
	//err = load_tuning_bin_and_stats();
	//if (err) {
	//    return err;
	//}

	afd_err = allib_afd_loadfunc(&(pal4a->afd_func_obj), 0);
	if (afd_err != _AL_AFDLIB_SUCCESS) {
		printf("ERROR %x\r\n", (unsigned int)afd_err);
		return afd_err;
	}

    void** afd_obj = &(pal4a->afd_obj);
    struct allib_afd_runtime_obj_t* afd_func_obj = &(pal4a->afd_func_obj);
    struct allib_afd_set_parameter_t* afd_set_para = &(pal4a->afd_set_para);
    struct allib_afd_get_parameter_t* afd_get_para = &(pal4a->afd_get_para);

    afd_err = afd_func_obj->initial(afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        return afd_err;
    }
    //printf("afd initial obj addr:%p, %p, %p\r\n", afd_obj, &(pal4a->afd_obj), pal4a->afd_obj);

    afd_set_para->type = ALAFD_SET_PARAM_SENSOR_INFO;
    afd_set_para->para.sensor_info.line_time_ns = gxr_atf->line_time_ns;  //29800  6371 = 1e9/90/1744
    afd_set_para->para.sensor_info.sensor_width = gxr_atf->sensor_width;
    afd_set_para->para.sensor_info.sensor_height = gxr_atf->sensor_height;
    LOG_DEBUG("[alAFDLib] RLG sensor_height = %d", afd_set_para->para.sensor_info.sensor_height);

    //afd_set_para->para.sensor_info.line_time_ns = 6371;  //29800  8822 = 1e9/65/1744
    //afd_set_para->para.sensor_info.sensor_width = 2328;
    //afd_set_para->para.sensor_info.sensor_height = 1744;

    LOG_DEBUG("[alAFDLib] line_time_ns = %d", afd_set_para->para.sensor_info.line_time_ns);
    LOG_DEBUG("[alAFDLib] sensor_height = %d", afd_set_para->para.sensor_info.sensor_height);

    //default
    afd_set_para->para.sensor_info.a3engine_width = 0;
    afd_set_para->para.sensor_info.a3engine_height = 0;
    afd_set_para->para.sensor_info.sensor_woi_x = 0;
    afd_set_para->para.sensor_info.sensor_woi_y = 0;
    afd_set_para->para.sensor_info.sensor_woi_w = 0;
    afd_set_para->para.sensor_info.sensor_woi_h = 0;
    afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        return afd_err;
    }

    afd_get_para->type = ALAFD_GET_PARAM_SETTING;
    afd_err = afd_func_obj->get_param(afd_get_para, *afd_obj);
    memcpy(&gxr_atf->atf_currSetpara, &afd_get_para->para.afd_setting, sizeof(afd_setting_t));

    if (afd_err != _AL_AFDLIB_SUCCESS) {
        return afd_err;
    }

    return afd_err;
}

unsigned int atf_deinit(gxr_atfhd* gxr_atf)
{
    al4ahd* pal4a = (al4ahd*)(gxr_atf->al4a);
    unsigned int afd_err = _AL_AFDLIB_SUCCESS;
    afd_err = pal4a->afd_func_obj.deinit(pal4a->afd_obj);

    if (NULL != pal4a) {
        free(pal4a);
        pal4a = NULL;
    }

    if (afd_err != _AL_AFDLIB_SUCCESS) {
        printf("afd deinit error\r\n");
        return afd_err;
    }

	return afd_err;
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
    delete(pResult);
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
    //printf("strpath %s\n", strpath.c_str());

    return strpath;
}


unsigned int atf_run(gxr_atfhd* gxr_atf)
{
    al4ahd* pal4a = (al4ahd*)(gxr_atf->al4a);
    UINT32 sofidx = gxr_atf->sofIndex;
    LOG_DEBUG("frameID: %d",gxr_atf->sofIndex);
    unsigned int afd_err = _AL_AFDLIB_SUCCESS;

    //unsigned int eff_adgain = gxr_atf->gain;
    //unsigned int eff_exp = gxr_atf->exposure_time;
    //unsigned short ymean = gxr_atf->average_value;
    unsigned int eff_adgain = 400;
    unsigned int eff_exp = 7000;
    unsigned short ymean = 3825;

    void** afd_obj = &(pal4a->afd_obj);
    struct allib_afd_runtime_obj_t* afd_func_obj = &(pal4a->afd_func_obj);
    struct allib_afd_set_parameter_t* afd_set_para = &(pal4a->afd_set_para);
    struct allib_afd_output_data_t* afd_output = &(pal4a->afd_output);

    /* set effective exposure time and effective adgain which getting from sensor
    *  for the sample example,  we assume effective exposure and effective gain
    *  are the same as AE output report from the previous estimation
    */
    //if (sofidx < 1)
    //{
    //    eff_adgain = pal4a->ae_get_para.para.ae_get_init_expo_param.ad_gain;
    //    eff_exp = pal4a->ae_get_para.para.ae_get_init_expo_param.exp_time;
    //}
    //else
    //{
    //    eff_adgain = pal4a->ae_output.udsensor_ad_gain;
    //    eff_exp = pal4a->ae_output.udexposure_time;
    //}

        //set fs result for algo
    if (gxr_atf->fs_result.update_enable)
    {
        printf("ATF: max_freq:%d, amp:%d \n", gxr_atf->fs_result.freq1.freq, gxr_atf->fs_result.freq1.amp);
        printf("ATF: freq2:%d, amp2:%d \n", gxr_atf->fs_result.freq2.freq, gxr_atf->fs_result.freq2.amp);
        printf("ATF: freq3:%d, amp3:%d \n\n", gxr_atf->fs_result.freq3.freq, gxr_atf->fs_result.freq3.amp);
        
        afd_set_para->type = ALAFD_SET_PARAM_FSRESULT;
        memcpy(&afd_set_para->para.fs_result.freq1, &gxr_atf->fs_result.freq1,sizeof(Freq));
        memcpy(&afd_set_para->para.fs_result.freq2, &gxr_atf->fs_result.freq2,sizeof(Freq));
        memcpy(&afd_set_para->para.fs_result.freq3, &gxr_atf->fs_result.freq3,sizeof(Freq));
        afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
        if (afd_err != _AL_AFDLIB_SUCCESS) {
            return afd_err;
        }
        gxr_atf->fs_result.update_enable = false;
    }

    afd_set_para->type = ALAFD_SET_PARAM_EXPOSURE_INFO;
    afd_set_para->para.exposure_info.exposure_time = eff_exp;
    afd_set_para->para.exposure_info.gain = eff_adgain;
    afd_set_para->para.exposure_info.average_value = ymean;
    afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        return afd_err;
    }

    afd_set_para->type = ALAFD_SET_PARAM_STATS_SIZE;
    //afd_set_para->para.stats_size = STATS_DATA_SIZE;
    afd_set_para->para.stats_size = gxr_atf->sensor_height;
    afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        return afd_err;
    }

    if (gxr_atf->tuning_enable)
    {
        //parse tuning param
        string dllAddr = GetCurrMoudle();
        string txtAddr;
        //txtAddr = dllAddr + "\\ATFTuningParam.bin";
        txtAddr = dllAddr + "\\ATFTuningParam.txt";
        //txtAddr = "D:\\work\\ATF\\code\\AL_ATF\\lib\\ATFTuningParam.txt";  //for simu only

        fstream fin;
        fin.open(txtAddr, ios::binary | ios::in);
        parseParam paramSim;
        memset(&paramSim, 0, sizeof(parseParam));
        if (fin.is_open())
        {
            fin.read((char*)&paramSim, sizeof(paramSim)* 2);
            fin.close();
        }
        else
        {
            printf("open ATFTuningParam.txt failed£¡ %s\n", txtAddr);
        }

        // parse tuning txt to struct
        afd_tuning_t tuningParam;
        memset(&tuningParam, 0, sizeof(afd_tuning_t));
        UINT16* param = (UINT16*)malloc(sizeof(UINT16) * (DEFAULT_PARAM_NUM + EFFECT_PARAM_NUM));
        for (int i = 0; i < DEFAULT_PARAM_NUM; i++)
        {
            param[i] = paramSim.defaultParam[i][1];
        }
        for (int i = 0; i < EFFECT_PARAM_NUM; i++)
        {
            param[i+ DEFAULT_PARAM_NUM] = paramSim.setEffectParam[i][1];
        }
        memcpy(&tuningParam, param, sizeof(afd_tuning_t));

        afd_set_para->type = ALAFD_SET_PARAM_TUNING;
        memcpy(&afd_set_para->para.tuning_setting, &tuningParam, sizeof(afd_tuning_t));
        afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
        if (afd_err != _AL_AFDLIB_SUCCESS) {
            return afd_err;
        }

        afd_setting_t enable_flag;
        UINT16* flag = (UINT16 *) malloc((sizeof(UINT16) * ENABLE_PARAM_NUM));
        for (int i = 0; i < ENABLE_PARAM_NUM; i++)
        {
            flag[i] = paramSim.enableFlag[i][1];
        }
        enable_flag.enable = (UINT8)flag[0];
        enable_flag.enable_close_after_detection = (UINT8)flag[1];
        enable_flag.mode = (afd_mode)flag[2];

        afd_set_para->type = ALAFD_SET_PARAM_ENABLE;
        memcpy(&afd_set_para->para.afd_setting, &enable_flag, sizeof(afd_setting_t));
        afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
        if (afd_err != _AL_AFDLIB_SUCCESS) {
            return afd_err;
        }

        //fs tuning 
        UINT16* fs_calc = (UINT16*)malloc((sizeof(UINT16) * FS_CALC_NUM));
        for (int i = 0; i < FS_CALC_NUM; i++)
        {
            fs_calc[i] = paramSim.setFScalcParam[i][1];
        }
        afd_set_para->type = ALAFD_SET_PARAM_TUNING_FS;
        memcpy(&afd_set_para->para.fs_tuning_setting, fs_calc, sizeof(afd_fs_tuning_t));
        afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
        if (afd_err != _AL_AFDLIB_SUCCESS) {
            return afd_err;
        }
    }

    //ungroup?
    if (true)
    {

    }


    afd_err = afd_func_obj->push_stats(gxr_atf->atf_stats, *afd_obj);
    LOG_DEBUG("finished push_stats");

    if (sofidx % 2 == 1)
    {
        afd_err = afd_func_obj->kernel_process(*afd_obj, afd_output);
        if (afd_err != _AL_AFDLIB_SUCCESS) {
            return afd_err;
        }
        gxr_atf->atf_output.flag_enable = afd_output->flag_enable;
        gxr_atf->atf_output.flag_bending = afd_output->flag_bending;
        gxr_atf->atf_output.atf_mode_stable = (atf_mode) afd_output->afd_mode_stable;
        gxr_atf->atf_output.atf_mode =(atf_mode) afd_output->afd_mode;
        //memcpy(&gxr_atf->atf_output, afd_output, sizeof(atf_output_data_t));
        LOG_DEBUG("[%02d] [alAFDLib] output result => flag_enable : %d, flag_banding : %d, afd_mode : %d, afd_mode_stable : %d\n",
           sofidx, gxr_atf->atf_output.flag_enable, gxr_atf->atf_output.flag_bending, gxr_atf->atf_output.atf_mode, gxr_atf->atf_output.atf_mode_stable);
    }

    LOG_SAVE;

	return afd_err;
}
