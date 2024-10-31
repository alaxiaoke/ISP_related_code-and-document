
#include "AWB_handle.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

//#include "MyLog.h"

#include <Windows.h>
#include <fstream>
#include <iostream>
#include <direct.h>
#include <queue>
using namespace std;



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
    printf("strpath %s\n", strpath.c_str());

    return strpath;
}

void sort_wp(int* a, int l, int* min, int* min2);

enum LogLevel_em w_emCurLogLevel = LogLevel_OFF;
int g_log_terminal = 1;
int g_log_to_file = 0;
char* logbuffer = NULL;

//float high_ccm[] = { 1.254f,-0.431f, 0.188f,
//-0.110f,1.415f,-0.305f,
//-0.0369f,-0.374f,1.421f };
//float D65_ccm[] = { 1.254f,-0.431f, 0.188f,
//-0.110f,1.401f,-0.295f,
//-0.0369f,-0.374f,1.421f };
//float cwf_ccm[9] = { 1.22f, -0.34f, 0.12f,
//-0.15f, 1.34f, -0.19f,
//0.05f, -0.45f, 1.40f };
//float tl84_ccm[] = { 1.22f, -0.34f, 0.12f,
//-0.15f, 1.34f, -0.19f,
//0.05f, -0.45f, 1.40f };
//float A_ccm[] = { 1.32f, -0.24f, 0.08f,
//-0.19f, 1.32f, -0.13f,
//0.18f, -0.58f, 1.40f };

float high_ccm[] =
{ 1.10f,-0.14f, 0.04f,
-0.0f,1.22f,-0.22f,
0.029f,-0.179f,1.15f };
float D65_ccm[] =
{ 1.10f,-0.14f, 0.04f,
-0.0f,1.22f,-0.22f,
0.029f,-0.179f,1.15f };
float cwf_ccm[9] =
{ 1.10f,-0.14f, 0.04f,
-0.0f,1.22f,-0.22f,
0.089f,-0.439f,1.35f };
float tl84_ccm[] =
{ 1.21f,-0.31f, 0.10f,
-0.08f,1.32f,-0.24f,
0.089f,-0.439f,1.35f };
float A_ccm[] =
{ 1.21f,-0.31f, 0.10f,
-0.08f,1.32f,-0.24f,
0.089f,-0.439f,1.35f };

float* ccm_list[7] = { A_ccm, tl84_ccm, cwf_ccm,D65_ccm, D65_ccm, high_ccm ,high_ccm };

//static float* lsctable0[7] = { d50_lsc0, d50_lsc0, d50_lsc0, d50_lsc0,d50_lsc0 ,d50_lsc0 , d50_lsc0 };
//static float* lsctable1[7] = { d50_lsc1, d50_lsc1, d50_lsc1, d50_lsc1,d50_lsc1 ,d50_lsc1 , d50_lsc1 };
//static float* lsctable2[7] = { d50_lsc2, d50_lsc2, d50_lsc2, d50_lsc2,d50_lsc2 ,d50_lsc2 , d50_lsc2 };

//static float* lsctable0[7] = { d65_lsc0, d65_lsc0, d65_lsc0, d65_lsc0,d65_lsc0 ,d65_lsc0 , d65_lsc0 };
//static float* lsctable1[7] = { d65_lsc1, d65_lsc1, d65_lsc1, d65_lsc1,d65_lsc1 ,d65_lsc1 , d65_lsc1 };
//static float* lsctable2[7] = { d65_lsc2, d65_lsc2, d65_lsc2, d65_lsc2,d65_lsc2 ,d65_lsc2 , d65_lsc2 };

//static float* lsctable0[7] = { tl84_lsc0, tl84_lsc0, tl84_lsc0, tl84_lsc0,tl84_lsc0 ,tl84_lsc0 , tl84_lsc0 };
//static float* lsctable1[7] = { tl84_lsc1, tl84_lsc1, tl84_lsc1, tl84_lsc1,tl84_lsc1 ,tl84_lsc1 , tl84_lsc1 };
//static float* lsctable2[7] = { tl84_lsc2, tl84_lsc2, tl84_lsc2, tl84_lsc2,tl84_lsc2 ,tl84_lsc2 , tl84_lsc2 };

//static float* lsctable0[7] = { A_lsc0, A_lsc0, A_lsc0, A_lsc0,A_lsc0 ,A_lsc0 , A_lsc0 };
//static float* lsctable1[7] = { A_lsc1, A_lsc1, A_lsc1, A_lsc1,A_lsc1 ,A_lsc1 , A_lsc1 };
//static float* lsctable2[7] = { A_lsc2, A_lsc2, A_lsc2, A_lsc2,A_lsc2 ,A_lsc2 , A_lsc2 };

//static float* lsctable0[7] = { cwf_lsc0, cwf_lsc0, cwf_lsc0, cwf_lsc0, cwf_lsc0 ,cwf_lsc0 , cwf_lsc0 };
//static float* lsctable1[7] = { cwf_lsc1, cwf_lsc1, cwf_lsc1, cwf_lsc1, cwf_lsc1 ,cwf_lsc1 , cwf_lsc1 };
//static float* lsctable2[7] = { cwf_lsc2, cwf_lsc2, cwf_lsc2, cwf_lsc2, cwf_lsc2 ,cwf_lsc2 , cwf_lsc2 };

//static float* lsctable0[7] = { high_lsc0, high_lsc0, high_lsc0, high_lsc0,high_lsc0 ,high_lsc0 , high_lsc0 };
//static float* lsctable1[7] = { high_lsc1, high_lsc1, high_lsc1, high_lsc1,high_lsc1 ,high_lsc1 , high_lsc1 };
//static float* lsctable2[7] = { high_lsc2,  high_lsc2, high_lsc2, high_lsc2,high_lsc2 ,high_lsc2 , high_lsc2 };

//static float* lsctable0[7] = { H_lsc0, H_lsc0, H_lsc0, H_lsc0,H_lsc0 ,H_lsc0 , H_lsc0 };
//static float* lsctable1[7] = { H_lsc1, H_lsc1, H_lsc1, H_lsc1,H_lsc1 ,H_lsc1 , H_lsc1 };
//static float* lsctable2[7] = { H_lsc2, H_lsc2, H_lsc2, H_lsc2,H_lsc2 ,H_lsc2 , H_lsc2 };

static float* lsctable0[7] = { H_lsc0, A_lsc0, tl84_lsc0, cwf_lsc0, d50_lsc0 ,d65_lsc0 , high_lsc0 };
static float* lsctable1[7] = { H_lsc1, A_lsc1, tl84_lsc1, cwf_lsc1, d50_lsc1 ,d65_lsc1 , high_lsc1 };
static float* lsctable2[7] = { H_lsc2, A_lsc2, tl84_lsc2, cwf_lsc2, d50_lsc2 ,d65_lsc2 , high_lsc2 };



enum LogLevelAWB                   last_debug_level;  //default off
int                                last_save_to_file;  //default false


void two_camera_strategy(gxr_awb* gxr_awb) {
    float left_r, left_b, right_r, right_b, delta_left, delta_right, avg_left_r, avg_left_b, avg_right_r, avg_right_b, avg_mixed_r = 0, avg_mixed_b = 0;

    //gxr_awb->awb_output.wbgain.r_gain = 1;
    //gxr_awb->awb_output.wbgain.b_gain = 1;

    if (gxr_awb->awb_handle_param.left_frame_num <= timeSeriesLength && gxr_awb->awb_handle_param.right_frame_num <= timeSeriesLength) {
        gxr_awb->awb_handle_param.mixed_rg_list[gxr_awb->awb_handle_param.left_frame_num - 1] = gxr_awb->awb_output.wbgain.r_gain;
        gxr_awb->awb_handle_param.mixed_bg_list[gxr_awb->awb_handle_param.left_frame_num - 1] = gxr_awb->awb_output.wbgain.b_gain;
    }
    else if (gxr_awb->awb_handle_param.left_frame_num > timeSeriesLength && gxr_awb->awb_handle_param.right_frame_num > timeSeriesLength)
    {
        for (int j = 0; j < timeSeriesLength - 1; j++)
        {
            gxr_awb->awb_handle_param.mixed_rg_list[j] = gxr_awb->awb_handle_param.mixed_rg_list[j + 1];
            gxr_awb->awb_handle_param.mixed_bg_list[j] = gxr_awb->awb_handle_param.mixed_bg_list[j + 1];
            avg_mixed_r += gxr_awb->awb_handle_param.mixed_rg_list[j];
            avg_mixed_b += gxr_awb->awb_handle_param.mixed_bg_list[j];
        }
        left_r = gxr_awb->awb_handle_param.left_rg_list[timeSeriesLength - 1] * 100;
        left_b = gxr_awb->awb_handle_param.left_bg_list[timeSeriesLength - 1] * 100;
        right_r = gxr_awb->awb_handle_param.right_rg_list[timeSeriesLength - 1] * 100;
        right_b = gxr_awb->awb_handle_param.right_bg_list[timeSeriesLength - 1] * 100;
        if (abs(left_r - right_r) + abs(left_b - right_b) < 20)
        {
            gxr_awb->awb_handle_param.mixed_rg_list[timeSeriesLength - 1] = gxr_awb->awb_output.wbgain.r_gain;
            gxr_awb->awb_handle_param.mixed_bg_list[timeSeriesLength - 1] = gxr_awb->awb_output.wbgain.b_gain;
        }
        else {
            float tmp_avg_r = 0, tmp_avg_b = 0;
            //下面算mix均值的时候，除以timeSeriesLength-1，因为最后一帧还没有放进去。
            tmp_avg_r = avg_mixed_r / (float)(timeSeriesLength - 1);
            tmp_avg_b = avg_mixed_b / (float)(timeSeriesLength - 1);
            delta_left = abs(left_r - tmp_avg_r) + abs(left_b - tmp_avg_b);
            delta_right = abs(right_r - tmp_avg_r) + abs(right_b - tmp_avg_b);
            if (delta_left > delta_right) {
                gxr_awb->awb_handle_param.mixed_rg_list[timeSeriesLength - 1] = right_r;
                gxr_awb->awb_handle_param.mixed_bg_list[timeSeriesLength - 1] = right_b;
            }
            else {
                gxr_awb->awb_handle_param.mixed_rg_list[timeSeriesLength - 1] = left_r;
                gxr_awb->awb_handle_param.mixed_bg_list[timeSeriesLength - 1] = left_b;
            }
        }
        avg_mixed_r += gxr_awb->awb_handle_param.mixed_rg_list[timeSeriesLength - 1];
        avg_mixed_b += gxr_awb->awb_handle_param.mixed_bg_list[timeSeriesLength - 1];
        gxr_awb->awb_output.wbgain.r_gain = avg_mixed_r / (float)timeSeriesLength;
        gxr_awb->awb_output.wbgain.b_gain = avg_mixed_b / (float)timeSeriesLength;
    }
}

//用于计算没有经过shift的rg,bg的时序
void time_series_balance(gxr_awb* gxr_awb, int identityid, float rg, float bg) {
    if (identityid == 0) {
        if (gxr_awb->awb_handle_param.left_frame_balance_num < timeSeriesLength) {
            gxr_awb->awb_handle_param.left_rg_balance_list[gxr_awb->awb_handle_param.left_frame_balance_num] = rg;
            gxr_awb->awb_handle_param.left_bg_balance_list[gxr_awb->awb_handle_param.left_frame_balance_num] = bg;
        }
        else
        {
            for (int j = 0; j < timeSeriesLength - 1; j++) {
                gxr_awb->awb_handle_param.left_rg_balance_list[j] = gxr_awb->awb_handle_param.left_rg_balance_list[j + 1];
                gxr_awb->awb_handle_param.left_bg_balance_list[j] = gxr_awb->awb_handle_param.left_bg_balance_list[j + 1];
            }
            gxr_awb->awb_handle_param.left_rg_balance_list[timeSeriesLength - 1] = rg;
            gxr_awb->awb_handle_param.left_bg_balance_list[timeSeriesLength - 1] = bg;
        }
        gxr_awb->awb_handle_param.left_frame_balance_num++;
        if (gxr_awb->awb_handle_param.left_frame_balance_num > 10000) {
            gxr_awb->awb_handle_param.left_frame_balance_num = timeSeriesLength;
        }
    }
    else {
        if (gxr_awb->awb_handle_param.right_frame_balance_num < timeSeriesLength) {
            gxr_awb->awb_handle_param.right_rg_balance_list[gxr_awb->awb_handle_param.right_frame_balance_num] = rg;
            gxr_awb->awb_handle_param.right_bg_balance_list[gxr_awb->awb_handle_param.right_frame_balance_num] = bg;
        }
        else
        {
            for (int j = 0; j < timeSeriesLength - 1; j++) {
                gxr_awb->awb_handle_param.right_rg_balance_list[j] = gxr_awb->awb_handle_param.right_rg_balance_list[j + 1];
                gxr_awb->awb_handle_param.right_bg_balance_list[j] = gxr_awb->awb_handle_param.right_bg_balance_list[j + 1];
            }
            gxr_awb->awb_handle_param.right_rg_balance_list[timeSeriesLength - 1] = rg;
            gxr_awb->awb_handle_param.right_bg_balance_list[timeSeriesLength - 1] = bg;
        }
        gxr_awb->awb_handle_param.right_frame_balance_num++;
        if (gxr_awb->awb_handle_param.right_frame_balance_num > 10000) {
            gxr_awb->awb_handle_param.right_frame_balance_num = timeSeriesLength;
        }
    }
};

void time_series(gxr_awb* gxr_awb, int identityid, float rg, float bg) {

    if (identityid == 0) {
        if (gxr_awb->awb_handle_param.left_frame_num < timeSeriesLength) {
            gxr_awb->awb_handle_param.left_rg_list[gxr_awb->awb_handle_param.left_frame_num] = rg;
            gxr_awb->awb_handle_param.left_bg_list[gxr_awb->awb_handle_param.left_frame_num] = bg;
        }
        else
        {
            for (int j = 0; j < timeSeriesLength - 1; j++) {
                gxr_awb->awb_handle_param.left_rg_list[j] = gxr_awb->awb_handle_param.left_rg_list[j + 1];
                gxr_awb->awb_handle_param.left_bg_list[j] = gxr_awb->awb_handle_param.left_bg_list[j + 1];
            }
            gxr_awb->awb_handle_param.left_rg_list[timeSeriesLength - 1] = rg;
            gxr_awb->awb_handle_param.left_bg_list[timeSeriesLength - 1] = bg;
        }
        gxr_awb->awb_handle_param.left_frame_num++;
        if (gxr_awb->awb_handle_param.left_frame_num > 10000) {
            gxr_awb->awb_handle_param.left_frame_num = timeSeriesLength;
        }
    }
    else {
        if (gxr_awb->awb_handle_param.right_frame_num < timeSeriesLength) {
            gxr_awb->awb_handle_param.right_rg_list[gxr_awb->awb_handle_param.right_frame_num] = rg;
            gxr_awb->awb_handle_param.right_bg_list[gxr_awb->awb_handle_param.right_frame_num] = bg;
        }
        else
        {
            for (int j = 0; j < timeSeriesLength - 1; j++) {
                gxr_awb->awb_handle_param.right_rg_list[j] = gxr_awb->awb_handle_param.right_rg_list[j + 1];
                gxr_awb->awb_handle_param.right_bg_list[j] = gxr_awb->awb_handle_param.right_bg_list[j + 1];
            }
            gxr_awb->awb_handle_param.right_rg_list[timeSeriesLength - 1] = rg;
            gxr_awb->awb_handle_param.right_bg_list[timeSeriesLength - 1] = bg;
        }
        gxr_awb->awb_handle_param.right_frame_num++;
        if (gxr_awb->awb_handle_param.right_frame_num > 10000) {
            gxr_awb->awb_handle_param.right_frame_num = timeSeriesLength;
        }
    }
    //for (int j = 0; j < 19; j++) {
    //    printf("frame:  %d,  left rg: %f, right rg: %f\r\n", j, left_rg_list[j], right_rg_list[j]);
    //}
};

unsigned int awb_initial(gxr_awb* gxr_awb)
{

    unsigned int awb_err = _AL_AWBLIB_SUCCESS;
    unsigned int awb_bin_size = 32 * 32;
    struct allib_awb_runtime_obj* awb_obj = (struct allib_awb_runtime_obj*)gxr_awb->awb_obj;
    awb_obj = (struct allib_awb_runtime_obj*)malloc(sizeof(struct allib_awb_runtime_obj));

    awb_err = allib_awb_loadfunc(awb_obj);
    if (awb_err != _AL_AWBLIB_SUCCESS) {
        printf("ERROR %x\r\n", (unsigned int)awb_err);
        return awb_err;
    }

    // struct allib_awb_runtime_obj *awb_obj = gxr_awb->awb_obj; 
    struct allib_awb_set_parameter* awb_set_para = &(gxr_awb->awb_set_para);
    struct allib_awb_get_parameter* awb_get_para = &(gxr_awb->awb_get_para);

    awb_err = awb_obj->initial(awb_obj, awb_bin_size);
    if (awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }

    awb_set_para->type = ALAWB_SET_PARAM_CAMERA_CALIB_DATA;
    awb_err = awb_obj->set_param(awb_set_para, awb_obj->awb);
    if (awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }

    awb_get_para->type = ALAWB_GET_PARAM_INIT_SETTING;
    awb_err = awb_obj->get_param(awb_get_para, awb_obj->awb);
    if (awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }

    for (int i = 0; i < timeSeriesLength; i++) {
        gxr_awb->awb_handle_param.cct_list[i] = 0;
        gxr_awb->awb_handle_param.left_rg_list[i] = 0;
        gxr_awb->awb_handle_param.left_bg_list[i] = 0;
        gxr_awb->awb_handle_param.right_rg_list[i] = 0;
        gxr_awb->awb_handle_param.right_bg_list[i] = 0;
        gxr_awb->awb_handle_param.mixed_rg_list[i] = 0;
        gxr_awb->awb_handle_param.mixed_bg_list[i] = 0;
        gxr_awb->awb_handle_param.left_rg_balance_list[i] = 0;
        gxr_awb->awb_handle_param.left_bg_balance_list[i] = 0;
        gxr_awb->awb_handle_param.right_rg_balance_list[i] = 0;
        gxr_awb->awb_handle_param.right_bg_balance_list[i] = 0;
    }
    gxr_awb->awb_handle_param.left_frame_num = 0;
    gxr_awb->awb_handle_param.right_frame_num = 0;
    gxr_awb->awb_handle_param.left_frame_balance_num = 0;
    gxr_awb->awb_handle_param.right_frame_balance_num = 0;

    gxr_awb->awb_obj = awb_obj;

    //debug level get from init
    allib_awb_set_debug_log_flag((enum LogLevel_em)AWBLog_OFF, 0);
    last_debug_level = AWBLog_OFF;  //default off
    last_save_to_file = 0;  //default false

    return awb_err;
};

unsigned int awb_run(gxr_awb* gxr_awb)
{

    gxr_awb->tuning_enable = 1;
    unsigned int awb_err = _AL_AWBLIB_SUCCESS;

    struct allib_awb_runtime_obj* awb_obj = (struct allib_awb_runtime_obj*)gxr_awb->awb_obj;
    struct allib_awb_set_parameter* awb_set_para = &(gxr_awb->awb_set_para);
    struct allib_awb_output_data* awb_output = &(gxr_awb->awb_output);
    struct awb_param_for_handle* handle_param = &(gxr_awb->awb_handle_param);

    if (gxr_awb->debug_level != last_debug_level || gxr_awb->save_to_file != last_save_to_file) {  //or another tag for log level modify action   //or expose the func, tooler directly call to modify 
        allib_awb_set_debug_log_flag((enum LogLevel_em)gxr_awb->debug_level, gxr_awb->save_to_file);

        last_debug_level = gxr_awb->debug_level;
        last_save_to_file = gxr_awb->save_to_file;
    }

    awb_err = awb_obj->set_param(awb_set_para, awb_obj->awb);
    if (awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }

    if (gxr_awb->tuning_enable == 1) {
        string dllAddr = GetCurrMoudle();
        string txtAddr;
        txtAddr = dllAddr + ".\\AWBParam.bin";
        fstream fin;
        fin.open(txtAddr, ios::binary | ios::in);
        if (fin.is_open())
        {
            parseParam param;
            fin.read((char*)&param, sizeof(param));
            for (int i = 0; i < 8; i++) {
                gxr_awb->awb_handle_param.params_from_txt.greyRoi[i] = (unsigned short)param.greyRegions[0][i];
                gxr_awb->awb_handle_param.params_from_txt.greyRoi2[i] = (unsigned short)param.greyRegions[1][i];
                gxr_awb->awb_handle_param.params_from_txt.greyRoi3[i] = (unsigned short)param.greyRegions[2][i];
                gxr_awb->awb_handle_param.params_from_txt.greyRoi4[i] = (unsigned short)param.greyRegions[3][i];
                gxr_awb->awb_handle_param.params_from_txt.greyRoi5[i] = (unsigned short)param.greyRegions[4][i];
                gxr_awb->awb_handle_param.params_from_txt.greyRoi6[i] = (unsigned short)param.greyRegions[5][i];
                gxr_awb->awb_handle_param.params_from_txt.greyRoi7[i] = (unsigned short)param.greyRegions[6][i];

                gxr_awb->awb_handle_param.params_from_txt.red_region[i] = (unsigned short)param.colorRegions[0][i];
                gxr_awb->awb_handle_param.params_from_txt.blue_region[i] = (unsigned short)param.colorRegions[1][i];
                gxr_awb->awb_handle_param.params_from_txt.yellow_region[i] = (unsigned short)param.colorRegions[2][i];
                gxr_awb->awb_handle_param.params_from_txt.green_region[i] = (unsigned short)param.colorRegions[3][i];

                gxr_awb->awb_handle_param.params_from_txt.misguideRoi_1[i] = (unsigned short)param.misColorRegions[0][i];
                gxr_awb->awb_handle_param.params_from_txt.misguideRoi_2[i] = (unsigned short)param.misColorRegions[1][i];
                gxr_awb->awb_handle_param.params_from_txt.misguideRoi_3[i] = (unsigned short)param.misColorRegions[2][i];
                gxr_awb->awb_handle_param.params_from_txt.misguideRoi_4[i] = (unsigned short)param.misColorRegions[3][i];
                gxr_awb->awb_handle_param.params_from_txt.misguideRoi_5[i] = (unsigned short)param.misColorRegions[4][i];
            }

            for (int i = 0; i < 3; i++) {
                gxr_awb->awb_handle_param.params_from_txt.redRoiOffset[i] = (unsigned short)param.colorRegionsParams[0][i];
                gxr_awb->awb_handle_param.params_from_txt.blueRoiOffset[i] = (unsigned short)param.colorRegionsParams[1][i];
                gxr_awb->awb_handle_param.params_from_txt.yellowRoiOffset[i] = (unsigned short)param.colorRegionsParams[2][i];
                gxr_awb->awb_handle_param.params_from_txt.greenRoiOffset[i] = (unsigned short)param.colorRegionsParams[3][i];
            }

            for (int i = 0; i < 2; i++) {
                gxr_awb->awb_handle_param.params_from_txt.greyLightSource[i] = (unsigned short)param.lightSourceTable[0][i];
                gxr_awb->awb_handle_param.params_from_txt.greyLightSource2[i] = (unsigned short)param.lightSourceTable[1][i];
                gxr_awb->awb_handle_param.params_from_txt.greyLightSource3[i] = (unsigned short)param.lightSourceTable[2][i];
                gxr_awb->awb_handle_param.params_from_txt.greyLightSource4[i] = (unsigned short)param.lightSourceTable[3][i];
                gxr_awb->awb_handle_param.params_from_txt.greyLightSource5[i] = (unsigned short)param.lightSourceTable[4][i];
                gxr_awb->awb_handle_param.params_from_txt.greyLightSource6[i] = (unsigned short)param.lightSourceTable[5][i];
                gxr_awb->awb_handle_param.params_from_txt.greyLightSource7[i] = (unsigned short)param.lightSourceTable[6][i];
            }

            for (int i = 0; i < 9; i++) {
                A_ccm[i] = (float)param.ccmParams[0][i] / 1000.0f;
                tl84_ccm[i] = (float)param.ccmParams[1][i] / 1000.0f;
                cwf_ccm[i] = (float)param.ccmParams[2][i] / 1000.0f;
                D65_ccm[i] = (float)param.ccmParams[3][i] / 1000.0f;
                high_ccm[i] = (float)param.ccmParams[4][i] / 1000.0f;
            }

            int index = 0;
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 13; j++) {
                    for (int k = 0; k < 13; k++) {
                        gxr_awb->awb_handle_param.params_from_txt.shiftRG[index] = param.shiftRG[i][j][k];
                        gxr_awb->awb_handle_param.params_from_txt.shiftBG[index] = param.shiftBG[i][j][k];
                        index++;
                    }
                }
            }
            for (int i = 0; i < 7; i++) {
                gxr_awb->awb_handle_param.params_from_txt.lightSourceWeight1[i] = (unsigned short)param.cctWeightTable[0][i];
                gxr_awb->awb_handle_param.params_from_txt.lightSourceWeight2[i] = (unsigned short)param.cctWeightTable[1][i];
                gxr_awb->awb_handle_param.params_from_txt.lightSourceWeight3[i] = (unsigned short)param.cctWeightTable[2][i];
                gxr_awb->awb_handle_param.params_from_txt.lightSourceWeight4[i] = (unsigned short)param.cctWeightTable[3][i];
                gxr_awb->awb_handle_param.params_from_txt.lightSourceWeight5[i] = (unsigned short)param.cctWeightTable[4][i];
            }

            for (int i = 0; i < 7; i++) {
                gxr_awb->awb_handle_param.params_from_txt.LightSourceShift[2*i] = (short)param.lightSourceShift[i][0];
                gxr_awb->awb_handle_param.params_from_txt.LightSourceShift[2*i+1] = (short)param.lightSourceShift[i][1];
            }

            //gxr_awb->awb_handle_param.params_from_txt.load_params_from_debug_txt = 1;
            //gxr_awb->awb_handle_param.params_from_txt.load_params_from_buffer = 1;
        }
    }

    int H = (int)32;
    int W = (int)32;

    awb_err = awb_obj->process(gxr_awb->awb_stats, awb_obj->awb, awb_output,H, W, NULL, (unsigned short)gxr_awb->identityid, &(gxr_awb->awb_handle_param.params_from_txt));
    if(awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }

    if (gxr_awb->awb_output.wbgain.r_gain == 0) {
        gxr_awb->awb_output.wbgain.r_gain = std_wp[5][0];
        gxr_awb->awb_output.wbgain.g_gain = 100;
        gxr_awb->awb_output.wbgain.b_gain = std_wp[5][1];
        gxr_awb->awb_output.color_temp = 6449;
    }
    else {
        gxr_awb->awb_output.wbgain.r_gain = (int)(gxr_awb->awb_output.wbgain.r_gain / 256.0f * 100);
        gxr_awb->awb_output.wbgain.g_gain = 100;
        gxr_awb->awb_output.wbgain.b_gain = (int)(gxr_awb->awb_output.wbgain.b_gain / 256.0f * 100);
    }

    //LOG_WARN("[WhiteBox] before smoothRGB: %3d %3d %3d\r\n", gxr_awb->awb_output.wbgain.r_gain, gxr_awb->awb_output.wbgain.g_gain, gxr_awb->awb_output.wbgain.b_gain);

    //**********************************时序及双目算法部分，离线仿真的时候应该关闭*************************************************
#ifdef TIMESERIES
    float rg, bg;

    time_series(gxr_awb, gxr_awb->identityid, gxr_awb->awb_output.wbgain.r_gain / 100.0f, gxr_awb->awb_output.wbgain.b_gain / 100.0f);
    float sum_rg = 0, sum_bg = 0;
    if (handle_param->left_frame_num >= timeSeriesLength || handle_param->right_frame_num >= timeSeriesLength) {
        if (gxr_awb->identityid == 0 && handle_param->left_frame_num >= timeSeriesLength) {       //默认用左边的camera做主camera来计算
            for (int i = 0; i < timeSeriesLength; i++) {
                sum_rg += handle_param->left_rg_list[i];
                sum_bg += handle_param->left_bg_list[i];
            }
            rg = sum_rg / (float)timeSeriesLength;
            bg = sum_bg / (float)timeSeriesLength;
            gxr_awb->awb_output.wbgain.r_gain = (int)(rg * 100);
            gxr_awb->awb_output.wbgain.g_gain = 100;
            gxr_awb->awb_output.wbgain.b_gain = (int)(bg * 100);
        }
        else if (handle_param->right_frame_num >= timeSeriesLength)
        {
            for (int i = 0; i < timeSeriesLength; i++) {
                sum_rg += handle_param->right_rg_list[i];
                sum_bg += handle_param->right_bg_list[i];
            }
            rg = sum_rg / (float)timeSeriesLength;
            bg = sum_bg / (float)timeSeriesLength;
            gxr_awb->awb_output.wbgain.r_gain = (int)(rg * 100);
            gxr_awb->awb_output.wbgain.g_gain = 100;
            gxr_awb->awb_output.wbgain.b_gain = (int)(bg * 100);
        }
    }

    //调用双目算法
    if (handle_param->left_frame_num > 0 && handle_param->right_frame_num > 0) {
        two_camera_strategy(gxr_awb);
    }

    float rg_balance, bg_balance;
    //没有shift的gain的时序
    time_series_balance(gxr_awb, gxr_awb->identityid, gxr_awb->awb_output.rg_allbalance, gxr_awb->awb_output.bg_allbalance);
    float sum_rg_balance = 0, sum_bg_balance = 0;
    if (handle_param->left_frame_balance_num >= timeSeriesLength || handle_param->right_frame_balance_num >= timeSeriesLength) {
        if (gxr_awb->identityid == 0 && handle_param->left_frame_balance_num >= timeSeriesLength) {
            for (int i = 0; i < timeSeriesLength; i++) {
                sum_rg_balance += handle_param->left_rg_balance_list[i];
                sum_bg_balance += handle_param->left_bg_balance_list[i];
            }
            rg_balance = sum_rg_balance / (float)timeSeriesLength;
            bg_balance = sum_bg_balance / (float)timeSeriesLength;
            gxr_awb->awb_output.rg_allbalance = rg_balance;
            gxr_awb->awb_output.bg_allbalance = bg_balance;
        }
        else if (handle_param->right_frame_balance_num >= timeSeriesLength)
        {
            for (int i = 0; i < timeSeriesLength; i++) {
                sum_rg_balance += handle_param->right_rg_balance_list[i];
                sum_bg_balance += handle_param->right_bg_balance_list[i];
            }
            rg_balance = sum_rg_balance / (float)timeSeriesLength;
            bg_balance = sum_bg_balance / (float)timeSeriesLength;
            gxr_awb->awb_output.rg_allbalance = rg_balance;
            gxr_awb->awb_output.bg_allbalance = bg_balance;
        }
    }

#endif
    //**********************************otp计算*************************************************
    //unsigned short otp_r = 688;   //55
    //unsigned short otp_g = 1000;
    //unsigned short otp_b = 601;
    //unsigned short otp_r = 703;   //46
    //unsigned short otp_g = 1000;
    //unsigned short otp_b = 605;
    //unsigned short otp_r = 706;   //20
    //unsigned short otp_g = 1000;
    //unsigned short otp_b = 603;
    //unsigned short otp_r = 686;   //15      
    //unsigned short otp_g = 1000;
    //unsigned short otp_b = 595;
    unsigned short otp_r = 686;   //31
    unsigned short otp_g = 1000;
    unsigned short otp_b = 602;
    //unsigned short otp_r = 688;   //18
    //unsigned short otp_g = 1000;
    //unsigned short otp_b = 602;
    //unsigned short otp_r = 682;   //1
    //unsigned short otp_g = 1000;
    //unsigned short otp_b = 597;
    //unsigned short otp_r = 693;   //32
    //unsigned short otp_g = 1000;
    //unsigned short otp_b = 604;

    //gxr_awb->awb_output.wbgain.r_gain = (int)((float)gxr_awb->awb_output.wbgain.r_gain * (golden_r / (float)otp_r) * 1000);
    //gxr_awb->awb_output.wbgain.b_gain = (int)((float)gxr_awb->awb_output.wbgain.b_gain * (golden_b / (float)otp_b) * 1000);

    //gxr_awb->awb_output.rg_allbalance = (int)((float)gxr_awb->awb_output.rg_allbalance * (golden_r / (float)otp_r) * 1000);
    //gxr_awb->awb_output.bg_allbalance = (int)((float)gxr_awb->awb_output.bg_allbalance * (golden_b / (float)otp_b) * 1000);
    //gxr_awb->awb_output.wbgain.r_gain = (int)((float)100 * (golden_r / (float)otp_r) * 1000);
    //gxr_awb->awb_output.wbgain.b_gain = (int)((float)100 * (golden_b / (float)otp_b) * 1000);
    //gxr_awb->awb_output.wbgain.r_gain = (int)((float)100);
    //gxr_awb->awb_output.wbgain.b_gain = (int)((float)100);
    //**********************************otp计算*************************************************
 
    //**********************************时序及双目算法部分，离线仿真的时候应该关闭*************************************************

    //**********************************色温计算*************************************************
    int r_gain = gxr_awb->awb_output.rg_allbalance;
    int b_gain = gxr_awb->awb_output.bg_allbalance;
    int min_indx, dist, minDist = 65534;
    float dist1, dist2;
    for (int i = 0; i < cctCurvePointNum; i++) {
        dist = (int)sqrt(pow(r_gain - (int)(cct_curve[i][0]), 2) + pow(b_gain - (int)(cct_curve[i][1]), 2));
        if (dist < minDist)
        {
            minDist = dist;
            min_indx = i;
        }
    }
    int cct = 6500;
    if (min_indx == 0) {
        if (r_gain < cct_curve[min_indx][0]) {
            cct = cct_curve[min_indx][2];
        }
        else {
            dist1 = (int)sqrt(pow(r_gain - (int)(cct_curve[0][0]), 2) + pow(b_gain - (int)(cct_curve[0][1]), 2));
            dist2 = (int)sqrt(pow(r_gain - (int)(cct_curve[1][0]), 2) + pow(b_gain - (int)(cct_curve[1][1]), 2));
            cct = cct_curve[0][2] + dist1 / (dist1 + dist2) * (cct_curve[1][2] - cct_curve[0][2]);
        }
    }
    else if (min_indx == cctCurvePointNum - 1) {
        if (r_gain > cct_curve[cctCurvePointNum - 1][0]) {
            cct = cct_curve[min_indx][2];
        }
        else {
            dist1 = (int)sqrt(pow(r_gain - (int)(cct_curve[min_indx][0]), 2) + pow(b_gain - (int)(cct_curve[min_indx][1]), 2));
            dist2 = (int)sqrt(pow(r_gain - (int)(cct_curve[min_indx - 1][0]), 2) + pow(b_gain - (int)(cct_curve[min_indx - 1][1]), 2));
            cct = cct_curve[min_indx][2] - dist1 / (dist1 + dist2) * (cct_curve[min_indx][2] - cct_curve[min_indx - 1][2]);
        }
    }
    else {
        dist1 = (int)sqrt(pow(r_gain - (int)(cct_curve[min_indx - 1][0]), 2) + pow(b_gain - (int)(cct_curve[min_indx - 1][1]), 2));
        dist2 = (int)sqrt(pow(r_gain - (int)(cct_curve[min_indx + 1][0]), 2) + pow(b_gain - (int)(cct_curve[min_indx + 1][1]), 2));
        if (dist1 <= dist2) {
            cct = cct_curve[min_indx - 1][2] + dist1 / (dist1 + dist) * (cct_curve[min_indx][2] - cct_curve[min_indx - 1][2]);
        }
        else {
            cct = cct_curve[min_indx][2] + dist2 / (dist2 + dist) * (cct_curve[min_indx + 1][2] - cct_curve[min_indx][2]);
        }
    }
    gxr_awb->awb_output.color_temp = cct;
    //**********************************色温时序，仿真关闭*************************************************
#ifdef TIMESERIES
    if (gxr_awb->identityid == 0) {
        int sum_cct = 0;
        if (handle_param->left_frame_balance_num <= timeSeriesLength) {
            handle_param->cct_list[handle_param->left_frame_balance_num - 1] = cct;     //time_series_balance里面已经对left_frame_balance_num加一了
            for (int i = 0; i < handle_param->left_frame_balance_num; i++) {
                sum_cct += handle_param->cct_list[i];
            }
            sum_cct = sum_cct / handle_param->left_frame_balance_num;
            gxr_awb->awb_output.color_temp = sum_cct;
        }
        else if (handle_param->left_frame_balance_num > timeSeriesLength)
        {
            for (int j = 0; j < timeSeriesLength - 1; j++) {
                handle_param->cct_list[j] = handle_param->cct_list[j + 1];
            }
            handle_param->cct_list[timeSeriesLength - 1] = cct;
            for (int i = 0; i < timeSeriesLength; i++) {
                sum_cct += handle_param->cct_list[i];
            }
            sum_cct = sum_cct / timeSeriesLength;
            gxr_awb->awb_output.color_temp = sum_cct;
        }
    }
    else {
        int sum_cct = 0;
        if (handle_param->right_frame_balance_num <= timeSeriesLength) {
            handle_param->cct_list[handle_param->right_frame_balance_num - 1] = cct;     //time_series_balance里面已经对left_frame_balance_num加一了
            for (int i = 0; i < handle_param->right_frame_balance_num; i++) {
                sum_cct += handle_param->cct_list[i];
            }
            sum_cct = sum_cct / handle_param->right_frame_balance_num;
            gxr_awb->awb_output.color_temp = sum_cct;
        }
        else if (handle_param->right_frame_balance_num > timeSeriesLength)
        {
            for (int j = 0; j < timeSeriesLength - 1; j++) {
                handle_param->cct_list[j] = handle_param->cct_list[j + 1];
            }
            handle_param->cct_list[timeSeriesLength - 1] = cct;
            for (int i = 0; i < timeSeriesLength; i++) {
                sum_cct += handle_param->cct_list[i];
            }
            sum_cct = sum_cct / timeSeriesLength;
            gxr_awb->awb_output.color_temp = sum_cct;
        }
    }
#endif
    //**********************************色温时序，仿真关闭***************************************
    //**********************************色温计算*************************************************

    int r_ = gxr_awb->awb_output.rg_allbalance;
    int b_ = gxr_awb->awb_output.bg_allbalance;

    int min_ind, min2_ind;
    float weight1, weight2;
    int dists[7] = { 0 };
    for (int i = 0; i < 7; i++) {
        dists[i] = (int)sqrt(pow(r_ - (int)(std_wp[i][0]), 2) + pow(b_ - (int)(std_wp[i][1]), 2));
        //dist[i] = 144 + i;
    }
    sort_wp(dists, 7, &min_ind, &min2_ind);

    weight1 = dists[min2_ind] / (float)(dists[min_ind] + dists[min2_ind]);
    weight2 = 1 - weight1;

    for (int i = 0; i < 9; i++) {
        gxr_awb->awb_output.final_CCM[i] = weight1 * ccm_list[min_ind][i] + weight2 * ccm_list[min2_ind][i];
    }

    //gxr_awb->awb_output.final_CCM[0] = 1.0;
    //gxr_awb->awb_output.final_CCM[1] = 0.0;
    //gxr_awb->awb_output.final_CCM[2] = 0.0;
    //gxr_awb->awb_output.final_CCM[3] = 0.0;
    //gxr_awb->awb_output.final_CCM[4] = 1.0;
    //gxr_awb->awb_output.final_CCM[5] = 0.0;
    //gxr_awb->awb_output.final_CCM[6] = 0.0;
    //gxr_awb->awb_output.final_CCM[7] = 0.0;
    //gxr_awb->awb_output.final_CCM[8] = 1.0;

    //**********************************选择LSC table*************************************************
    cct = gxr_awb->awb_output.color_temp;
    if (cct <= 2300) {
        min_ind = 0;
        min2_ind = 0;
    }
    else if (cct <= 3000) {
        weight2 = (cct - 2300) / (float)(3000 - 2300);
        weight1 = 1 - weight2;
        min_ind = 0;
        min2_ind = 1;
    }
    else if (cct <= 4000) {
        weight2 = (cct - 3000) / (float)(4000 - 3000);
        weight1 = 1 - weight2;
        min_ind = 1;
        min2_ind = 2;
    }
    else if (cct <= 5000) {
        weight2 = (cct - 4000) / (float)(5000 - 4000);
        weight1 = 1 - weight2;
        min_ind = 2;
        min2_ind = 4;
    }
    else if (cct <= 6500) {
        weight2 = (cct - 5000) / (float)(6500 - 5000);
        weight1 = 1 - weight2;
        min_ind = 4;
        min2_ind = 5;
    }
    else if (cct <= 7500) {
        weight2 = (cct - 6500) / (float)(7500 - 6500);
        weight1 = 1 - weight2;
        min_ind = 5;
        min2_ind = 6;
    }
    else if (cct > 7500) {
        min_ind = 6;
        min2_ind = 6;
    }

    for (int i = 0; i < 13 * 17; i++) {
        gxr_awb->awb_output.final_lsc[i] = weight1 * lsctable0[min_ind][i] + weight2 * lsctable0[min2_ind][i];
        gxr_awb->awb_output.final_lsc[i + 13 * 17] = weight1 * lsctable1[min_ind][i] + weight2 * lsctable1[min2_ind][i];
        gxr_awb->awb_output.final_lsc[i + 2 * 13 * 17] = weight1 * lsctable1[min_ind][i] + weight2 * lsctable1[min2_ind][i];
        gxr_awb->awb_output.final_lsc[i + 3 * 13 * 17] = weight1 * lsctable2[min_ind][i] + weight2 * lsctable2[min2_ind][i];
        //gxr_awb->awb_output.final_lsc[i] = (weight1 * lsctable0[min_ind][i] + weight2 * lsctable0[min2_ind][i]) * (golden_r / (float)otp_r) * 1000;
        //gxr_awb->awb_output.final_lsc[i + 13 * 17] = (weight1 * lsctable1[min_ind][i] + weight2 * lsctable1[min2_ind][i]) * (golden_r / (float)otp_r) * 1000;
        //gxr_awb->awb_output.final_lsc[i + 2 * 13 * 17] = (weight1 * lsctable1[min_ind][i] + weight2 * lsctable1[min2_ind][i]) * (golden_r / (float)otp_r) * 1000;
        //gxr_awb->awb_output.final_lsc[i + 3 * 13 * 17] = (weight1 * lsctable2[min_ind][i] + weight2 * lsctable2[min2_ind][i]) * (golden_r / (float)otp_r) * 1000;

    }
    //**********************************选择LSC table*************************************************
    LOG_WARN("[WhiteBox] code version %3d %3d\r\n", 06, 06);
    LOG_WARN("[WhiteBox] CT: %3d\r\n", (int)gxr_awb->awb_output.color_temp);
    LOG_WARN("[WhiteBox] bv: %3d\r\n", (int)gxr_awb->ae_output.bvresult);
    LOG_WARN("[WhiteBox] WB Gain %3d %3d %3d\r\n", gxr_awb->awb_output.wbgain.r_gain, gxr_awb->awb_output.wbgain.g_gain, gxr_awb->awb_output.wbgain.b_gain);
    LOG_WARN("[WhiteBox] (rg,bg): %3f %3f \r\n", 100 / (float)gxr_awb->awb_output.wbgain.r_gain, 100 / (float)gxr_awb->awb_output.wbgain.b_gain);
    LOG_WARN("[WhiteBox] CCM %3f %3f %3f %3f %3f %3f %3f %3f %3f\r\n",
        gxr_awb->awb_output.final_CCM[0], gxr_awb->awb_output.final_CCM[1], gxr_awb->awb_output.final_CCM[2],
        gxr_awb->awb_output.final_CCM[3], gxr_awb->awb_output.final_CCM[4], gxr_awb->awb_output.final_CCM[5],
        gxr_awb->awb_output.final_CCM[6], gxr_awb->awb_output.final_CCM[7], gxr_awb->awb_output.final_CCM[8]);//[alAWBLib] CT: 5875 [alAWBLib] WB Gain 202 100 170
    LOG_WARN("[WhiteBox] final_lsc %3f %3f \r\n", gxr_awb->awb_output.final_lsc[13 * 17], gxr_awb->awb_output.final_lsc[3 * 13 * 17 - 1]);

    LOG_SAVE;
    return awb_err;
}


unsigned int awb_deinit(gxr_awb* gxr_awb)
{
    unsigned int awb_err = _AL_AWBLIB_SUCCESS;
    struct allib_awb_runtime_obj* awb_obj = (struct allib_awb_runtime_obj*)gxr_awb->awb_obj;

    awb_err = awb_obj->deinit(awb_obj);
    if (awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }
    return awb_err;
}


void sort_wp(int* a, int l, int* minInd, int* min2Ind)//a为数组地址，l为数组长度。
{
    int i;
    int min = 100000;
    int min2 = 100000;
    int min_index = 0; int min2_index = 0;
    //排序主体
    for (i = 0; i < l; i++)
        if (a[i] < min2)
        {
            min2 = a[i];
            min2_index = i;
            if (a[i] < min) {
                min2 = min;
                min = a[i];
                min2_index = min_index;
                min_index = i;
            }
        }
    *minInd = min_index;
    *min2Ind = min2_index;
}




