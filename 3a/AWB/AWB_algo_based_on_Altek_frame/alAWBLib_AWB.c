#include "alAWBLib_AWBApi.h"
#include "alAWBLib_Version.h"
#include "alwrapper.h"
#include "BU_AwbPara.h"
//#include "allib_awb.h"
//#include "MyLog.h"

static AWB_WORKING_BUFFER        g_AWB_working_buffer;

unsigned int allib_awb_set_debug_log_flag(enum LogLevel_em ae_debug_log_flag, int file_tag)
{
    int errRet = 0;

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
    //LOG_INFO("debug level/to_file: %d/%d", w_emCurLogLevel, g_log_to_file);
    //LOG_DEBUG("debug level/to_file: %d/%d", w_emCurLogLevel, g_log_to_file);
    return errRet;
}

/**
*@brief                     Get initial parameters
*@param[In]   *alAWBLib_runtime_data_t 	 	    AWB runtime    data
*@param[OUT]  *allib_awb_initial_data_t	 	 	 AWB initial data
*@return                    Error code
*/
enum altek_awb_error_code  alAWBLib_get_init_param(alAWBLib_runtime_data_t* awb_runtime_d, struct allib_awb_initial_data_t* awb_init_param_d)
{
    short                           iso_r_128, iso_b_128;
    IQAWB_INPUT_PARA                inputPara;
    IQAWB_OUTPUT_PARA               outputPara;
    enum altek_awb_error_code       error = _AL_AWBLIB_SUCCESS;

    if (0 == awb_init_param_d)
        return _AL_AWBLIB_INIT_BUFFER_NULL;

    iso_r_128 = awb_runtime_d->iso_r_128;
    iso_b_128 = awb_runtime_d->iso_b_128;

    inputPara.awb_mem_address = awb_runtime_d->awb_buffer;

    if ((0 == iso_r_128) || (0 == iso_b_128))
    {
        error = _AL_AWBLIB_INVALID_CALIB_OTP;
        return error;
    }

    inputPara.iso_r_128 = iso_r_128;
    inputPara.iso_b_128 = iso_b_128;
    error = ALTEK_AWB_Init(&inputPara, &outputPara);
    if (_AL_AWBLIB_SUCCESS != error)
    {
        return error;
    }

    awb_init_param_d->initial_wbgain.r_gain = awb_runtime_d->awb_output_backup.wbgain.r_gain;
    awb_init_param_d->initial_wbgain.g_gain = awb_runtime_d->awb_output_backup.wbgain.g_gain;
    awb_init_param_d->initial_wbgain.b_gain = awb_runtime_d->awb_output_backup.wbgain.b_gain;
    awb_init_param_d->color_temperature = awb_runtime_d->awb_output_backup.color_temp;


    return error;

}

/**
*@brief                     Get calibration data 	   and default wb gain (128 based)
*@param[In]   *awb_calibration_data_t 			in
*@param[OUT]   *short 	 	iso_r_gain
*@param[OUT]   *short 	 	iso_b_gain
*@param[OUT]  *awb_calibration_data_t	 	 	 out
*@return                    Error code
*/
enum altek_awb_error_code   alAWBLib_Derive_ISO128(short* iso_r_128, short* iso_b_128, struct awb_calibration_data_t* in, struct awb_calibration_data_t* out)
{
    unsigned int  r, g, b;
    short  iso_r = 0, iso_b = 0;
    enum altek_awb_error_code   error = _AL_AWBLIB_SUCCESS;

    r = in->calib_r_gain;
    g = in->calib_g_gain;
    b = in->calib_b_gain;

    if ((0 == r) || (0 == g) || (0 == b))
    {
        error = _AL_AWBLIB_INVALID_CALIB_OTP;
    }
    else
    {
        iso_r = alAWBLib_Transform_CalibData(r, g);
        iso_b = alAWBLib_Transform_CalibData(b, g);
        if ((0 == iso_r) || (0 == iso_b))
        {
            error = _AL_AWBLIB_INVALID_CALIB_OTP;
        }
    }

    if (_AL_AWBLIB_SUCCESS == error)
    {
        out->calib_r_gain = r;
        out->calib_g_gain = g;
        out->calib_b_gain = b;
        *iso_r_128 = iso_r;
        *iso_b_128 = iso_b;
    }
    else
    {
        out->calib_r_gain = 0;
        out->calib_g_gain = 0;
        out->calib_b_gain = 0;
        *iso_r_128 = 0;
        *iso_b_128 = 0;
    }
    return error;
}


/**
*@brief            Get stats and info from stats data
*@param[In]   *void 			HW3a_stats_Data
*@param[In]   *alAWBLib_runtime_data_t 	 	 	awb_runtime_data
*@param[OUT]  *al3awrapper_stats_awb_t	 	 	al3aWrapper_stat_awb
*@return                    Error code
*/
enum altek_awb_error_code	 alAWBLib_Dispatch_HW3A(void* HW3a_stats_Data, alAWBLib_runtime_data_t* awb_runtime_data, struct al3awrapper_stats_awb_t* al3aWrapper_stat_awb)
{
    enum altek_awb_error_code   Error = _AL_AWBLIB_SUCCESS;
    //unsigned char *hw_stats_info;
    unsigned int* stats;
    unsigned char HwStatsInfo_blk_num = 0, HwStatsInfo_bnk_num = 0;

    // check if stats ptr valid?
    if (HW3a_stats_Data == 0)
    {
        return _AL_AWBLIB_PROCESS_STATS_NULL;
    }
    else
    {
        // hw_stats_info = (unsigned char *)HW3a_stats_Data;
        // stats = (unsigned char *)HW3a_stats_Data + AWB_HW3A_BLOCK_INFO_SIZE;
        stats = (unsigned int*)HW3a_stats_Data;
    }

    // Parse stats data
    // HwStatsInfo_blk_num = hw_stats_info[14] & 0xFF;
    // HwStatsInfo_bnk_num = hw_stats_info[15] & 0x7F;
 //   HwStatsInfo_blk_num = awbStatsH;
    //HwStatsInfo_bnk_num = awbStatsW;
    al3aWrapper_stat_awb->pawb_stats = stats;
    // al3aWrapper_stat_awb->ucvalidblocks = HwStatsInfo_blk_num;
    // al3aWrapper_stat_awb->ucvalidbanks = HwStatsInfo_bnk_num;
    al3aWrapper_stat_awb->ucstats_format = 0;     /* 0: ISP format */

    return Error;
}



//////////////////////////////////////////
// Normal flow function
//////////////////////////////////////////

/**
*@brief                     Initial AWB parameters
*@param[In]   *inputPara    Input AWB Algorithm runtime data
*@param[OUT]  *outputPara   Fill up AWB first initialed data
*@return                    Error code
*/
unsigned int  alAWBLib_init(void* obj, int statsSize)
{
    enum altek_awb_error_code		error = _AL_AWBLIB_SUCCESS;
    unsigned int  offset = 0;
    unsigned int  algorithmBufferSize = ALTEK_Init_GetMemSize();
    unsigned int  runtimeBufferSize = sizeof(alAWBLib_runtime_data_t);
    unsigned int  totalBufferSize;
    unsigned char* awb_workingBuffer_tmp;
    AWB_WORKING_BUFFER* awb_workingBuffer = &g_AWB_working_buffer;
    alAWBLib_runtime_data_t* awb_runtime_data;
    alAWBLib_stats_setting_t* awb_stats_data;
    struct allib_awb_runtime_obj* awb_obj;
    AWB_MEM_ALGORITHM* mem_buffer;
    IQAWB_ALGORITHM_PARA* algorithm_Para;

    if (0 == obj)
        return _AL_AWBLIB_INIT_NULL_OBJ;

    awb_obj = (struct allib_awb_runtime_obj*)obj;

    totalBufferSize = runtimeBufferSize +
        algorithmBufferSize +
        alAWBLib_Private_WorkingBuffer_Reserved;


    //. reset
    if (0 == awb_workingBuffer)
    {
        return _AL_AWBLIB_INIT_BUFFER_NULL;
    }
    memset(awb_workingBuffer, 0, totalBufferSize);
    awb_workingBuffer_tmp = (unsigned char*)awb_workingBuffer;

    awb_runtime_data = (alAWBLib_runtime_data_t*)awb_workingBuffer_tmp;
    awb_stats_data = (alAWBLib_stats_setting_t*)&(awb_runtime_data->awb_stats_setting);
    offset = runtimeBufferSize;

    //. algorithm buffer
    awb_runtime_data->awb_buffer = (void*)(awb_workingBuffer_tmp + offset);
    //. If input tuning, read tuningAddr;
    mem_buffer = ((AWB_MEM_ALGORITHM*)awb_runtime_data->awb_buffer);
    algorithm_Para = ((IQAWB_ALGORITHM_PARA*)&(mem_buffer->awb_algorithm_para));
    IQAWB_INPUT_PARA inputPara;
    memset(&inputPara, 0, sizeof(inputPara));
    inputPara.enterFromProcess = 0;
    error = AWB_LoadTuningFileAddr(&inputPara);
    if (_AL_AWBLIB_SUCCESS != error)
        return error;

    awb_stats_data->hw3a_awb_stats_size = statsSize;

    awb_obj->awb = (void*)awb_workingBuffer_tmp;


    return _AL_AWBLIB_SUCCESS;

}

/**
*@brief                     Initial AWB parameters
*@param[In]   module handler
*@return                    Error code
*/
unsigned int  alAWBLib_deinit(void* obj)
{
    struct allib_awb_runtime_obj* awb_obj;
    enum altek_awb_error_code       error = _AL_AWBLIB_SUCCESS;

    if (0 == obj)
        return _AL_AWBLIB_DEINIT_NULL_OBJ;

    awb_obj = (struct allib_awb_runtime_obj*)obj;

    error = ALTEK_AWB_Deinit();

    //wrapper_free( awb_obj->awb );

    awb_obj->obj_verification = 0;

    awb_obj->awb = 0;
    awb_obj->initial = 0;
    awb_obj->deinit = 0;
    awb_obj->set_param = 0;
    awb_obj->get_param = 0;
    awb_obj->process = 0;

    return error;

}

/**
*@brief                    Algorithm parameters setting
*@param[In]   *inputPara   Input AWB Algorithm Data
*@param[OUT]  *mem_addr    The memory pointer of algorithm parameters
*@return                   Error code
*/
unsigned int  alAWBLib_set_param(struct allib_awb_set_parameter* param, void* awb_dat)
{
    alAWBLib_runtime_data_t* awb_runtime_data;
    enum altek_awb_error_code   error = _AL_AWBLIB_SUCCESS;

    if (0 == param || 0 == awb_dat)
    {
        error = _AL_AWBLIB_SET_PARAM_INPUT_NULL;
        return error;
    }

    awb_runtime_data = (alAWBLib_runtime_data_t*)awb_dat;

    switch (param->type)
    {

    case ALAWB_SET_PARAM_AWB_ENABLE:
        break;

    case ALAWB_SET_PARAM_CAMERA_CALIB_DATA:
        error = alAWBLib_Derive_ISO128(&(awb_runtime_data->iso_r_128),
            &(awb_runtime_data->iso_b_128),
            &(param->para.awb_calib_data),
            &(awb_runtime_data->awb_calib_data));
        break;

    case ALAWB_SET_PARAM_TUNING_FILE:
        break;

    case ALAWB_SET_PARAM_AWB_MODE_SETTING:
        break;

    case ALAWB_SET_PARAM_UPDATE_AE_REPORT:
        memcpy(&(awb_runtime_data->awb_ae_param_setting), &(param->para.ae_report_update), sizeof(struct allib_awb_ae_param_setting_t));
        break;

    case ALAWB_SET_PARAM_WBGAIN_LOCK:
        break;
    case ALAWB_SET_PARAM_WBGAIN_UNLOCK:
        break;
    case ALAWB_SET_PARAM_CT_LOCK:
        break;
    case ALAWB_SET_PARAM_CT_UNLOCK:
        break;

    default:
        error = _AL_AWBLIB_SET_PARAM_INPUT_TYPE;
        break;
    }

    return error;

}


/**
*@brief                    Algorithm parameters setting
*@param[In]   *inputPara   Input AWB Algorithm Data
*@param[OUT]  *mem_addr    The memory pointer of algorithm parameters
*@return                   Error code
*/
unsigned int  alAWBLib_get_param(struct allib_awb_get_parameter* param, void* awb_dat)
{
    alAWBLib_runtime_data_t* awb_runtime_data;
    struct allib_awb_output_data* runtime_output;
    enum altek_awb_error_code       error = _AL_AWBLIB_SUCCESS;

    if (0 == param || 0 == awb_dat)
    {
        error = _AL_AWBLIB_GET_PARAM_INPUT_NULL;
        return error;
    }

    awb_runtime_data = (alAWBLib_runtime_data_t*)awb_dat;
    runtime_output = &(awb_runtime_data->awb_output_backup);
    switch (param->type)
    {
    case ALAWB_GET_PARAM_INIT_SETTING:
        error = alAWBLib_get_init_param(awb_runtime_data, &(param->para.awb_init_data));
        if (_AL_AWBLIB_SUCCESS != error)
        {
            return error;
        }

        break;

    case ALAWB_GET_PARAM_WBGAIN:
        param->para.wbgain.r_gain = runtime_output->wbgain.r_gain;
        param->para.wbgain.g_gain = runtime_output->wbgain.g_gain;
        param->para.wbgain.b_gain = runtime_output->wbgain.b_gain;
        break;

    case ALAWB_GET_PARAM_COLOR_TEMPERATURE:
        param->para.color_temp = runtime_output->color_temp;
        break;

    case ALAWB_GET_PARAM_AWB_STATES:
        break;

    case ALAWB_GET_PARAM_LOCK_CT_SETTING:
        break;

    default:
        error = _AL_AWBLIB_GET_PARAM_INPUT_TYPE;
        break;
    }
    return error;
}

/**
*@brief                    AWB Algorithm Estimation
*@param[In]   *inputPara   Input AWB Algorithm Data
*@param[OUT]  *outputPara  Output Estimation report
*@return                   Error code
*/
unsigned int  alAWBLib_process(void* HW3a_stats_Data, void* awb_dat, struct allib_awb_output_data* awb_output_d, int H, int W, void* debugbuf, unsigned short cameraID, txt_params* params)
{
    short                           iso_r_128, iso_b_128;
    IQAWB_INPUT_PARA                inputPara;
    IQAWB_OUTPUT_PARA               outputPara;
    alAWBLib_runtime_data_t* awb_runtime_data;
    struct al3awrapper_stats_awb_t  al3aWrapper_stat_awb;
    enum altek_awb_error_code       error = _AL_AWBLIB_SUCCESS;

    if (0 == awb_dat)
    {
        error = _AL_AWBLIB_INVALID_INPUT;
        return error;
    }
    if (cameraID > AWB_HANDLE_NUM_MAX || cameraID < 0)
    {
        error = _AL_AWBLIB_INVALID_INPUT;
        return error;
    }

    memset(&al3aWrapper_stat_awb, 0, sizeof(al3aWrapper_stat_awb));
    awb_runtime_data = (alAWBLib_runtime_data_t*)awb_dat;
    //txt_params* txtParam = NULL;
    //txtParam = (txt_params*) params;
    //memset(txtParam, 0, sizeof(txtParam));
    //memcpy(txtParam,params, sizeof(txt_params));

    if (0 == awb_runtime_data->awb_buffer)
    {
        error = _AL_AWBLIB_PROCESS_MEM_NULL;
        return error;
    }

    error = alAWBLib_Dispatch_HW3A(HW3a_stats_Data, awb_runtime_data, &al3aWrapper_stat_awb);
    if (_AL_AWBLIB_SUCCESS != error)
        return error;

    //check stats w/h	
    // if((AWB_STATS_WIDTH != al3aWrapper_stat_awb.ucvalidblocks) || (AWB_STATS_HEIGHT != al3aWrapper_stat_awb.ucvalidbanks))
    //     return _AL_AWBLIB_PROCESS_STATS_SIZE;

    awb_runtime_data->awb_stats_setting.hw3a_awb_block_num = H;
    awb_runtime_data->awb_stats_setting.hw3a_awb_bank_num = W;
    awb_runtime_data->awb_stats_setting.hw3a_awb_stats_size = H * W;
    awb_runtime_data->awb_stats_setting.hw3a_awb_stats = (unsigned int*)al3aWrapper_stat_awb.pawb_stats;

    // Check OTP data
    iso_r_128 = awb_runtime_data->iso_r_128;
    iso_b_128 = awb_runtime_data->iso_b_128;
    if ((0 == iso_r_128) || (0 == iso_b_128))
    {
        error = _AL_AWBLIB_INVALID_CALIB_OTP;
        return error;
    }

    outputPara.valid_report = 1;
    inputPara.iso_r_128 = iso_r_128;
    inputPara.iso_b_128 = iso_b_128;
    inputPara.tuning_file = awb_runtime_data->tuning_addr;
    inputPara.bv = awb_runtime_data->awb_ae_param_setting.non_comp_bv;
    inputPara.addr = awb_runtime_data->awb_stats_setting.hw3a_awb_stats;
    inputPara.width = awb_runtime_data->awb_stats_setting.hw3a_awb_block_num;
    inputPara.height = awb_runtime_data->awb_stats_setting.hw3a_awb_bank_num;
    inputPara.totalsize = awb_runtime_data->awb_stats_setting.hw3a_awb_stats_size;
    inputPara.color_order = AWB_BAYER_RG;
    inputPara.awb_mem_address = awb_runtime_data->awb_buffer;
    inputPara.flag_enable_stylish = 1;
    inputPara.load_params_from_debug_txt = params->load_params_from_debug_txt;
    inputPara.load_params_from_buffer = 0;
    inputPara.params_from_txt = *params;
    inputPara.enterFromProcess = 1;
    //通过txt读取参数修改AWB参数
    //memcpy(&inputPara.params_from_txt, &params_from_txt, sizeof(txt_params));
    if (inputPara.load_params_from_debug_txt) {
        AWB_MEM_ALGORITHM* mem_buffer;
        IQAWB_ALGORITHM_PARA* algorithm_Para;

        mem_buffer = ((AWB_MEM_ALGORITHM*)awb_runtime_data->awb_buffer);
        algorithm_Para = ((IQAWB_ALGORITHM_PARA*)&(mem_buffer->awb_algorithm_para));
        AWB_LoadTuningFileAddr(&inputPara);
        //params->load_params_from_debug_txt = 0;
        inputPara.load_params_from_buffer = 1;
    }
    //. ESTIMATION	
    error = ALTEK_AWB_Estimation(cameraID, &inputPara, &outputPara, debugbuf);
    if (_AL_AWBLIB_SUCCESS != error)
    {
        return error;
    }
    awb_output_d->color_temp = outputPara.color_temperature;
    awb_output_d->wbgain.r_gain = outputPara.r_gain_final;
    awb_output_d->wbgain.g_gain = outputPara.g_gain_final;
    awb_output_d->wbgain.b_gain = outputPara.b_gain_final;
    awb_output_d->rg_allbalance = outputPara.rg_allbalance;
    awb_output_d->bg_allbalance = outputPara.bg_allbalance;

    return error;
}

//////////////////////////////////////////
// public function
//////////////////////////////////////////
ATTRIBUTE_VISIBILITY_DEFAULT
unsigned int   allib_awb_getsize_workingbuffer()
{
    unsigned int  workingBufferSize = ALTEK_Init_GetMemSize();
    unsigned int  runtimeBufferSize = sizeof(alAWBLib_runtime_data_t);
    unsigned int  totalBufferSize = 0;

    totalBufferSize = runtimeBufferSize +
        workingBufferSize +
        alAWBLib_Private_WorkingBuffer_Reserved;

    return totalBufferSize;
}


ATTRIBUTE_VISIBILITY_DEFAULT
void    allib_awb_getlib_version(struct allib_awb_lib_version* AWB_LibVersion)
{
    //ALTEK_GetAwbCoreVersion(&AWB_LibVersion->major_version, &AWB_LibVersion->minor_version);
    AWB_LibVersion->major_version = ALTEK_AWB_LIB_MAJOR_VER;
    AWB_LibVersion->minor_version = ALTEK_AWB_LIB_MINOR_VER;
}

// loading function interface, not directly public API name to outer space
ATTRIBUTE_VISIBILITY_DEFAULT
unsigned int    allib_awb_loadfunc(struct allib_awb_runtime_obj* awb_run_obj)
{
    if (0 == awb_run_obj)   // check input data address validity
    {
        return _AL_AWBLIB_INIT_NULL_OBJ;
    }

    /* Verify lib.h is same as .so */
    // awb_run_obj->obj_verification = sizeof( struct allib_awb_output_data );

    awb_run_obj->awb = 0;
    awb_run_obj->initial = alAWBLib_init;
    awb_run_obj->deinit = alAWBLib_deinit;
    awb_run_obj->set_param = alAWBLib_set_param;
    awb_run_obj->get_param = alAWBLib_get_param;
    awb_run_obj->process = alAWBLib_process;

    return _AL_AWBLIB_SUCCESS;

}

