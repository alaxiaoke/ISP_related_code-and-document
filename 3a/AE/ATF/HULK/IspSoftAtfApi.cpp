#include <cstdio>
#include <cstdarg>
#include <cstdlib>
//#include <cmath>
#include <cstring>
#include <cstdint>
#include <cassert>

#include <fstream>
#include <iostream>
#include <direct.h>
#include <windows.h>

#include "./ATF/alAFDLib_Error_Code.h"
#include "./ATF/allib_afd.h"
#include "out.h"
#include "IspSoftAtfApi.h"
//#include "ATF/Mylog.h"

using namespace std;

int w_emCurLogLevel = LogLevel_OFF;       //log level switch  -->add new func to update loglevel
int g_log_terminal = 1;
int g_log_to_file = 0;
thread_local char* logbuffer = nullptr;

static string configPath = "";

bool first_process = true;

uint32_t  AtfTuningParseSet(void* handle, parseParam* data)
{
    al4ahd* pal4a = (al4ahd*)handle;
    unsigned int afd_err = _AL_AFDLIB_SUCCESS;

    void** afd_obj = &(pal4a->afd_obj);
    struct allib_afd_runtime_obj_t* afd_func_obj = &(pal4a->afd_func_obj);
    struct allib_afd_set_parameter_t* afd_set_para = &(pal4a->afd_set_para);
    struct allib_afd_output_data_t* afd_output = &(pal4a->afd_output);

    // parse tuning txt to struct
    afd_tuning_t tuningParam;
    memset(&tuningParam, 0, sizeof(afd_tuning_t));
    UINT16* param = (UINT16*)malloc(sizeof(UINT16) * (DEFAULT_PARAM_NUM + EFFECT_PARAM_NUM));
    for (int i = 0; i < DEFAULT_PARAM_NUM; i++)
    {
        param[i] = data->defaultParam[i][1];
    }
    for (int i = 0; i < EFFECT_PARAM_NUM; i++)
    {
        param[i + DEFAULT_PARAM_NUM] = data->setEffectParam[i][1];
    }
    memcpy(&tuningParam, param, sizeof(afd_tuning_t));

    afd_set_para->type = ALAFD_SET_PARAM_TUNING;
    memcpy(&afd_set_para->para.tuning_setting, &tuningParam, sizeof(afd_tuning_t));

    afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        return afd_err;
    }

    afd_setting_t enable_flag;
    UINT16* flag = (UINT16*)malloc((sizeof(UINT16) * ENABLE_PARAM_NUM));
    for (int i = 0; i < ENABLE_PARAM_NUM; i++)
    {
        flag[i] = data->enableFlag[i][1];
    }
    enable_flag.enable = (UINT8)flag[0];
    enable_flag.fs_enable = (UINT8)flag[1];
    enable_flag.enable_group = (UINT8)flag[2];
    enable_flag.enable_close_after_detection = (UINT8)flag[3];
    enable_flag.mode = (afd_mode)flag[4];

    afd_set_para->type = ALAFD_SET_PARAM_ENABLE;
    memcpy(&afd_set_para->para.afd_setting, &enable_flag, sizeof(afd_setting_t));
    afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);

    //fs tuning 
    UINT16* fs_calc = (UINT16*)malloc((sizeof(UINT16) * FS_CALC_NUM));
    for (int i = 0; i < FS_CALC_NUM; i++)
    {
        fs_calc[i] = data->setFScalcParam[i][1];
    }
    afd_set_para->type = ALAFD_SET_PARAM_TUNING_FS;
    memcpy(&afd_set_para->para.fs_tuning_setting, fs_calc, sizeof(afd_fs_tuning_t));
    afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        return afd_err;
    }

    return afd_err;
}


static IspSoftHandle AtfInit(void* owner, IspSoftPostMessageFunc postMessage, void* data)
{
    assert(owner);
    assert(data);

    // algo init ? 创建算法实例
    //return 初始化函数指针 类似原先 void* al4a 指针
    al4ahd* pal4a = (al4ahd*)malloc(sizeof(al4ahd));
    unsigned int afd_err = _AL_AFDLIB_SUCCESS;

    //debug level get from init
    afd_err = allib_atf_set_debug_log_flag(LogLevel_Info, false, true);
    //LOG_INFO("ATF for HULK");

    first_process = true;

    allib_afd_getlib_version(&(pal4a->afd_ver));
    LOG_INFO("AFD ver: %d.%03d\n", pal4a->afd_ver.major_version, pal4a->afd_ver.minor_version);
    LOG_WHITEBOX("AFD ver: %d.%03d\n", pal4a->afd_ver.major_version, pal4a->afd_ver.minor_version);

    afd_err = allib_afd_loadfunc(&(pal4a->afd_func_obj), 0);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        LOG_INFO("ERROR %x\r\n", (unsigned int)afd_err);
        return 0;
    }

    //config path

    if (((IspSoftAtfInitParam*)data)->config_path != NULL)
    {
        configPath = ((IspSoftAtfInitParam*)data)->config_path;
    }
    else
    {
        LOG_INFO("CONFIG PATH IS NULL !!");
        configPath = ".//";
    }

    void** afd_obj = &(pal4a->afd_obj);
    struct allib_afd_runtime_obj_t* afd_func_obj = &(pal4a->afd_func_obj);
    struct allib_afd_set_parameter_t* afd_set_para = &(pal4a->afd_set_para);
    struct allib_afd_get_parameter_t* afd_get_para = &(pal4a->afd_get_para);

    afd_err = afd_func_obj->initial(afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        LOG_INFO("ERROR %x\r\n", (unsigned int)afd_err);
        return NULL;
    }

    //  记录一些外部owner、postMessage等，而非实际算法config信息配置
    //pal4a->afd_func_obj.postMessage = postMessage;
    //pal4a->afd_func_obj.owner = owner;
    pal4a->postMessage = postMessage;
    pal4a->owner = owner;

    afd_set_para->type = ALAFD_SET_PARAM_SENSOR_INFO;
    afd_set_para->para.sensor_info.line_time_ns = ((IspSoftAtfInitParam*)data)->line_time_ns;
    afd_set_para->para.sensor_info.sensor_width = ((IspSoftAtfInitParam*)data)->sensor_width;
    afd_set_para->para.sensor_info.sensor_height = ((IspSoftAtfInitParam*)data)->sensor_height;
    LOG_INFO("line_time_ns = %d", afd_set_para->para.sensor_info.line_time_ns);
    LOG_INFO("sensor_height = %d", afd_set_para->para.sensor_info.sensor_height);
    LOG_INFO("sensor_width = %d", afd_set_para->para.sensor_info.sensor_width);
    //default
    afd_set_para->para.sensor_info.a3engine_width = 0;
    afd_set_para->para.sensor_info.a3engine_height = ((IspSoftAtfInitParam*)data)->sensor_height; // ATF_STATS_MAX_LINE_SENSOR;
    afd_set_para->para.sensor_info.sensor_woi_x = 0;
    afd_set_para->para.sensor_info.sensor_woi_y = 0;
    afd_set_para->para.sensor_info.sensor_woi_w = 0;
    afd_set_para->para.sensor_info.sensor_woi_h = 0;
    afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        LOG_INFO("ERROR %x\r\n", (unsigned int)afd_err);
        return NULL;
    }
    // set als config
    if (((IspSoftAtfInitParam*)data)->als_config.bin_num != 0)
    {
        memcpy(&afd_set_para->para.flicker_sensor_info, &((IspSoftAtfInitParam*)data)->als_config, sizeof(AlsConfig));
        afd_set_para->type = ALAFD_SET_PARAM_FLICKER_SENSOR_INFO;
        afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
        if (afd_err != _AL_AFDLIB_SUCCESS) {
            LOG_INFO("ERROR %x\r\n", (unsigned int)afd_err);
            return NULL;
        }
    }
    else
    {
        LOG_INFO("init als config setting is null");
    }

    if (((IspSoftAtfInitParam*)data)->tuning_bin.Data != NULL)
    {
        //tuning bin set
        afd_err = AtfTuningParseSet(pal4a, (parseParam*)((IspSoftAtfInitParam*)data)->tuning_bin.Data);

        if (afd_err != _AL_AFDLIB_SUCCESS) {
            LOG_INFO("ERROR %x\r\n", (unsigned int)afd_err);
            return NULL;
        }
    }
    else
    {
        LOG_INFO("init tuning file is null");
    }

    LOG_SAVE;
    afd_err = allib_atf_set_debug_log_flag(LogLevel_OFF, FALSE, TRUE);
    //afd_err = allib_atf_set_debug_log_flag((enum LogLevel_em)LogLevel_Debug, FALSE);

    return pal4a;
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


static uint32_t AtfLoadTuningBin(IspSoftHandle handle, void* data)  //算法tuning参数配置
{
    //parse tuning param
    //string dllAddr = GetCurrMoudle();
    string dllAddr = configPath;
    string txtAddr;
    txtAddr = dllAddr + "\\ATFTuningParam.bin";
    //txtAddr = "D:\\work\\ATF\\code\\AL_ATF\\lib\\ATFTuningParam.txt";  //for simu only
    fstream fin;
    fin.open(txtAddr, ios::binary | ios::in);
    parseParam paramSim;
    memset(&paramSim, 0, sizeof(parseParam));
    if (fin.is_open())
    {
        fin.read((char*)&paramSim, sizeof(paramSim) * 2);
        fin.close();
    }
    else
    {
        LOG_DEBUG("tuning bin path:%s\n", txtAddr.c_str());
        LOG_DEBUG("open ATFTuningParam.bin failed！\n", txtAddr);
        return _AL_AFDLIB_INVALID_TUNING_FILE_NULL;
    }
    // algo set tuning param
    return AtfTuningParseSet(handle, &paramSim);
}


static uint32_t AtfLoadDebugSetting(IspSoftHandle handle, void* data)  //算法debug参数配置
{
    // algo set debug param
    al4ahd* pal4a = (al4ahd*)handle;
    unsigned int afd_err = _AL_AFDLIB_SUCCESS;

    void** afd_obj = &(pal4a->afd_obj);
    struct allib_afd_runtime_obj_t* afd_func_obj = &(pal4a->afd_func_obj);
    struct allib_afd_set_parameter_t* afd_set_para = &(pal4a->afd_set_para);
    struct allib_afd_get_parameter_t* afd_get_para = &(pal4a->afd_get_para);
    struct allib_afd_output_data_t* afd_output = &(pal4a->afd_output);

    //parse tuning param
    //string dllAddr = GetCurrMoudle();
    string dllAddr = configPath;
    string txtAddr;
    txtAddr = dllAddr + "\\AtfDebug.txt";
    //txtAddr = "D:\\work\\ATF\\code\\AL_ATF\\lib\\AtfDebug.txt";  //for simu only
    FILE* fp;
    fp = fopen(txtAddr.c_str(), "rt");
    parseParam paramSim;
    memset(&paramSim, 0, sizeof(parseParam));
    if (fp != NULL)
    {
        char tmp[512];
        int num = 0;
        int len;
        int level = 0;
        BOOL save_flag = false;
        BOOL teminal = false;

        while (fgets(tmp, 512, fp) != NULL)
        {
            len = strlen(tmp);
            tmp[len - 1] = '\0';  /*去掉换行符*/
            //printf("%s %d \n", tmp, len - 1);
            if (num == 0)
            {
                level = atoi(tmp);
            }
            else if(num == 1)
            {
                save_flag = (atoi(tmp) == 1) ? true : false;
            }
            else if (num == 2)
            {
                teminal = (atoi(tmp) == 1) ? true : false;
            }
            num++;
        }
        fclose(fp);

        //LogLevel_em level = LogLevel_Debug;  //todo:parse from txt
        //int level = 63;  //todo:parse from txt
        //BOOL save_flag = true;
        //BOOL teminal = true;

        //set debug level
        afd_err = allib_atf_set_debug_log_flag(level, save_flag, teminal);
    }
    else
    {
        afd_err = _AL_AFDLIB_INVALID_DEBUG_FILE_NULL;
        LOG_ERROR("打开AEParam.txt失败！ %s", txtAddr.c_str());
        LOG_ERROR("ERROR %x\r", (unsigned int)afd_err);
        return afd_err;
    }

    return afd_err;
}


static uint32_t AtfRun(IspSoftHandle handle, IspSoftAtfInImage* data)      //算法每帧参数配置、计算
{
    if (handle==NULL)
    {
        return -1;
    }
    if (data==NULL)
    {
        return _AL_AFDLIB_INVALID_INPUT;
    }

    al4ahd* pal4a = (al4ahd*)handle;
    unsigned int afd_err = _AL_AFDLIB_SUCCESS;

    void** afd_obj = &(pal4a->afd_obj);
    struct allib_afd_runtime_obj_t* afd_func_obj = &(pal4a->afd_func_obj);
    struct allib_afd_set_parameter_t* afd_set_para = &(pal4a->afd_set_para);
    struct allib_afd_get_parameter_t* afd_get_para = &(pal4a->afd_get_para);
    struct allib_afd_output_data_t* afd_output = &(pal4a->afd_output);

    // algo process
    IspSoftAeOut* ae_out = (IspSoftAeOut*)data->ae_out.Data;
    IspSoftIspAtfStatusOut* isp_out = (IspSoftIspAtfStatusOut*)data->isp_out.Data;


    UINT32 sofidx = data->frame_id;
    LOG_INFO("frameID: %d", data->frame_id);

    //set fs result
    if (data->fs_out.freq1.amp!=0 ||(afd_set_para->para.fs_result.freq1.amp != data->fs_out.freq1.amp))
    {
        afd_set_para->type = ALAFD_SET_PARAM_FSRESULT;
        memcpy(&afd_set_para->para.fs_result.freq1, &data->fs_out.freq1, sizeof(Freq));
        memcpy(&afd_set_para->para.fs_result.freq2, &data->fs_out.freq2, sizeof(Freq));
        memcpy(&afd_set_para->para.fs_result.freq3, &data->fs_out.freq3, sizeof(Freq));
        afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
        if (afd_err != _AL_AFDLIB_SUCCESS) {
            LOG_ERROR("ERROR %x\r\n", (unsigned int)afd_err);
            return afd_err;
        }
        LOG_WHITEBOX("-----FS SET IN-----");

        LOG_WHITEBOX("FREQ 1 %d", data->fs_out.freq1.freq);
        LOG_WHITEBOX("FREQ 2 %d", data->fs_out.freq2.freq);
        LOG_WHITEBOX("FREQ 3 %d", data->fs_out.freq3.freq);
        LOG_WHITEBOX("amp 1 %d", data->fs_out.freq1.amp);
        LOG_WHITEBOX("amp 2 %d", data->fs_out.freq2.amp);
        LOG_WHITEBOX("amp 2 %d", data->fs_out.freq3.amp);

    }
    unsigned int eff_adgain ;
    unsigned int eff_exp;
    unsigned short ymean;

    if (isp_out == NULL || isp_out->atf_stats == NULL || isp_out->group_line == NULL 
        || data->isp_out.Length != sizeof(IspSoftIspAtfStatusOut))
    {
        LOG_ERROR("ISP OUT ERROR %x\r\n", _AL_AFDLIB_INVALID_INPUT);
        return _AL_AFDLIB_INVALID_INPUT;
    }
    else
    {
        eff_adgain = isp_out->sensor_adgain;
        eff_exp = isp_out->sensor_shutter_us;
    }


    if (ae_out == NULL && data->ae_out.Length == 0)   //init frame
    {
        ymean = 3285;
    }
    else if (ae_out == NULL)
    {
        LOG_ERROR("AE OUT ERROR %x\r\n", _AL_AFDLIB_INVALID_INPUT);

        return _AL_AFDLIB_INVALID_INPUT;
    }
    else if (data->ae_out.Length != sizeof(IspSoftAeOut))
    {
        LOG_ERROR("AE OUT LENGTH ERROR %x\r\n", _AL_AFDLIB_INVALID_INPUT);

        return _AL_AFDLIB_INVALID_INPUT;
    }
    else
    {
        ymean = ae_out->Ymean;
    }

    LOG_WHITEBOX("frameId: %d", sofidx);
    LOG_WHITEBOX("exposure_time: %d", eff_exp);
    LOG_WHITEBOX("gain: %d", eff_adgain);
    LOG_WHITEBOX("frame average_value: %d", ymean);

    afd_set_para->type = ALAFD_SET_PARAM_EXPOSURE_INFO;
    afd_set_para->para.exposure_info.exposure_time = eff_exp;
    afd_set_para->para.exposure_info.gain = eff_adgain;
    afd_set_para->para.exposure_info.average_value = ymean;
    afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        LOG_ERROR("ERROR %x\r\n", (unsigned int)afd_err);
        return afd_err;
    }

    afd_set_para->type = ALAFD_SET_PARAM_STATS_SIZE;
    //afd_set_para->para.stats_size = ATF_STATS_MAX_LINE_SENSOR;
    afd_set_para->para.stats_size = isp_out->atf_stats_line;
    afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        LOG_ERROR("ERROR %x\r\n", (unsigned int)afd_err);
        return afd_err;
    }
    //get line time us
    afd_get_para->type = ALAFD_GET_PARAM_SENSOR_INFO;
    afd_err = afd_func_obj->get_param(afd_get_para, *afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        LOG_ERROR("ERROR %x\r\n", (unsigned int)afd_err);
        return afd_err;
    }

    if (afd_get_para->para.sensor_info.sensor_woi_h != isp_out->stats_woi_height)
    {
        // set after group size 
        afd_set_para->type = ALAFD_SET_PARAM_SENSOR_INFO;
        afd_set_para->para.sensor_info.line_time_ns = afd_get_para->para.sensor_info.line_time_ns;
        afd_set_para->para.sensor_info.sensor_width = afd_get_para->para.sensor_info.sensor_width;
        afd_set_para->para.sensor_info.sensor_height = afd_get_para->para.sensor_info.sensor_height;
        //LOG_DEBUG("[alAFDLib] RLG sensor_height = %d", afd_set_para->para.sensor_info.sensor_height);
        //default
        //afd_set_para->para.sensor_info.a3engine_width = afd_get_para->para.sensor_info.a3engine_width;
        afd_set_para->para.sensor_info.a3engine_height = afd_get_para->para.sensor_info.a3engine_height;  //64b  size before group: 3000*2920   12M :3496*4656
        afd_set_para->para.sensor_info.a3engine_width = 2920;
        //afd_set_para->para.sensor_info.a3engine_height = 3000;  //64b  size before group: 3000*2920
        afd_set_para->para.sensor_info.sensor_woi_x = 0;
        afd_set_para->para.sensor_info.sensor_woi_y = 0;
        afd_set_para->para.sensor_info.sensor_woi_w = isp_out->stats_woi_width;
        afd_set_para->para.sensor_info.sensor_woi_h = isp_out->stats_woi_height;
        afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
        if (afd_err != _AL_AFDLIB_SUCCESS) {
            LOG_INFO("ERROR %x\r\n", (unsigned int)afd_err);
            return afd_err;
        }
    }


    afd_err = afd_func_obj->push_stats(isp_out->atf_stats, isp_out->group_line, *afd_obj);
    //frame0 protect
    if (first_process == 1)
    {
        afd_output->flag_enable = 0;
        afd_output->flag_bending = 0;
        afd_output->afd_mode = AFD_MODE_OFF;
        afd_output->afd_mode_stable = AFD_MODE_OFF;
        first_process = false;
    }

    if (sofidx % 2 == 1)
    {
        afd_err = afd_func_obj->kernel_process(*afd_obj, afd_output);
        if (afd_err != _AL_AFDLIB_SUCCESS) {
            LOG_ERROR("ERROR %x\r\n", (unsigned int)afd_err);
            return afd_err;
        }

    }

    IspSoftAtfOut atf_out;

    atf_out.flag_enable = afd_output->flag_enable;
    atf_out.flag_bending = afd_output->flag_bending;
    atf_out.atf_mode_stable = (atf_mode)afd_output->afd_mode_stable;
    atf_out.atf_mode = (atf_mode)afd_output->afd_mode;

    //out 调用回调函数,修改填充 postMessage的type 和 data
    IspSoftAtfOutReady outdata;
    outdata.frame_id = data->frame_id;
    outdata.timestamp = data->timestamp;
    outdata.atf_out.Data = (uint8_t*) &atf_out;
    outdata.atf_out.Length = sizeof(atf_out);

    IspSoftPostMessageFunc postMessage = (IspSoftPostMessageFunc)pal4a->postMessage;
    postMessage(pal4a->owner, ISP_SOFT_ATF_POST_OUT_READY, &outdata);

    LOG_SAVE;

    return  afd_err;
}


static int32_t AtfProcess(IspSoftHandle handle, uint32_t processType, void *data)
{
    assert(handle);

    switch (processType)
    {
    case ISP_SOFT_ATF_PROCESS_IN_TUNING:
        return AtfLoadTuningBin(handle, data);   //加载tuning bin

    case ISP_SOFT_ATF_PROCESS_IN_DEBUG:
        return AtfLoadDebugSetting(handle, data);   //加载debug config

    case ISP_SOFT_ATF_PROCESS_IN_IMAGE:
        assert(data);

        return AtfRun(handle, (IspSoftAtfInImage*)data);

    default:
        return -1;
    }
}


static int32_t AtfDeinit(IspSoftHandle handle)
{
    assert(handle);

    al4ahd* pal4a = (al4ahd*)handle;
    unsigned int afd_err = _AL_AFDLIB_SUCCESS;
    afd_err = pal4a->afd_func_obj.deinit(pal4a->afd_obj);

    if (NULL != logbuffer) {
        free(logbuffer);
        logbuffer = NULL;
    }

    if (NULL != pal4a) {
        free(pal4a);
        pal4a = NULL;
    }

    if (afd_err != _AL_AFDLIB_SUCCESS) {
        LOG_DEBUG("afd deinit error\r\n");
        return afd_err;
    }

    return afd_err;
}


IspSoftModule IspSoftAtfLoad()
{
    IspSoftModule result = { 0 };
    result.Init = AtfInit;
    result.Process = AtfProcess;
    result.Deinit = AtfDeinit;

    return result;
}
