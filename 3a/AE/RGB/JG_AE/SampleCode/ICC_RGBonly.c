/****************************************************************************
*                                                                           *
*       This software and proprietary is confidential and may be used       *
*        only as expressly authorized by a licensing agreement from         *
*                                                                           *
*                            Altek Semiconductor                            *
*                                                                           *
*                   (C) COPYRIGHT 2021 ALTEK SEMICONDUCTOR                  *
*                            ALL RIGHTS RESERVED                            *
*                                                                           *
*                 The entire notice above must be reproduced                *
*                  on all copies and should not be removed.                 *
*                                                                           *
*****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "allib_LSC.h"
#include "alLSCLib_Error_Code.h"
#include "allib_afd.h"
#include "alAFDlib_Error_Code.h"
#include "allib_ae.h"
#include "allib_ae_errcode.h"
#include "allib_awb.h"
#include "allib_awb_errcode.h"

#define _SIMULATE_LSC_RUN                     (20)
#define _SIMULATE_AFD_RUN                     (50)
#define SCANNER_HEIGHT                        (1088)
#define STATS_DATA_SIZE                       (1072)
#define _G_CHANNEL_STATS_FILE_NAME            "pseudo_g_channel_stats.bin"
#define _LSC_TUNE_FILENAME                    "LSC_RGB_default_tuning.bin"
#define _AE_STATS_FILENAME                    "pseudo_ae_stats.bin"
#define _YHIS_STATS_FILENAME                  "pseudo_yhis_stats.bin"
#define _AWB_STATS_FILENAME                   "pseudo_awb_stats.bin"

typedef struct al4ahandle{
    struct alLSClib_lib_version_t       lsc_ver;
    struct alLSClib_runtime_obj_t       lsc_func_obj;
    struct alLSCLib_set_parameter_t     lsc_set_para;
    struct alLSCLib_get_parameter_t     lsc_get_para;
    struct alLSClib_output_data_t       lsc_default_output_data;
    struct alLSClib_output_data_t       lsc_output;
    void *lsc_obj;

    struct allib_afd_lib_version_t      afd_ver;
    struct allib_afd_runtime_obj_t      afd_func_obj;
    struct allib_afd_set_parameter_t    afd_set_para;
    struct allib_afd_get_parameter_t    afd_get_para;
    struct allib_afd_output_data_t      afd_output;
    void *afd_obj;

    struct alaelib_version              ae_ver;
    struct alaeruntimeobj               ae_func_obj;
    struct ae_set_param_t               ae_set_para;
    struct ae_get_param_t               ae_get_para;
    struct ae_output_data_t             ae_output;
    struct alhw3a_ae_proc_data_t        ae_proc_data;
    void *ae_obj;

	struct allib_awb_lib_version      	awb_ver; 
    struct allib_awb_runtime_obj        awb_obj;
    struct allib_awb_output_data        awb_output;
    struct allib_awb_get_parameter      awb_get_para;
    struct allib_awb_set_parameter      awb_set_para;
}al4ahd;

al4ahd          al4a;
unsigned int    g_udsofidx = 0;
unsigned char   g_ucIsICCAbort = FALSE;
void            *ae_stats = NULL;
void            *yhis_stats = NULL;
unsigned int    adgain_current_valid = 0;
unsigned int    exposure_time_current_valid = 0;
char            LSC_tuning_buf[30 * 1024];
unsigned char   _g_channel_stats_buffer[_SIMULATE_AFD_RUN][STATS_DATA_SIZE];
unsigned char   ae_stats_buffer[8320];
unsigned int    yhis_stats_buffer[1152];
unsigned char   awb_stats[30848];
unsigned int    awb_bin_size = 0;

unsigned int load_tuning_bin_and_stats();
unsigned int load_lib(al4ahd *pal4a);
unsigned int lsc_initial(al4ahd *pal4a);
unsigned int afd_initial(al4ahd *pal4a);
unsigned int ae_initial(al4ahd *pal4a);
unsigned int awb_initial(al4ahd *pal4a);
unsigned int afd_run(al4ahd *pal4a, unsigned int sofidx);
unsigned int ae_run(al4ahd *pal4a, unsigned int sofidx);
unsigned int awb_run(al4ahd *pal4a);
unsigned int lsc_run(al4ahd *pal4a);



int main(void)
{
    unsigned int err = 0;

    /*  ===========================================
    *     Get function version.
    *   =========================================== */
    alLSCLib_getlib_version(&(al4a.lsc_ver));
    printf("LSC ver: %d.%d\n", al4a.lsc_ver.major_version, al4a.lsc_ver.minor_version);

    allib_afd_getlib_version(&(al4a.afd_ver));
    printf("AFD ver: %d.%d\n", al4a.afd_ver.major_version, al4a.afd_ver.minor_version);

    allib_ae_getlib_version(&(al4a.ae_ver));
    printf("AE ver: %d.%d\n", al4a.ae_ver.major_version, al4a.ae_ver.minor_version);

    /* get AWB version */
    allib_awb_getlib_version(&al4a.awb_ver);
    printf("AWB Ver %03d.%03d\r\n", al4a.awb_ver.major_version, al4a.awb_ver.minor_version);

    /*  ===========================================
    *     Load tuning file and psudo stats.
    *   =========================================== */
    err = load_tuning_bin_and_stats();
    if (err){
        return err;
    }

    /*  ===========================================
    *     Load lib.
    *   =========================================== */
    err = load_lib(&al4a);
    if (err){
        return err;
    }

    /*  ===========================================
    *     Initial flow
    *   =========================================== */
    lsc_initial(&al4a);
    printf("[LSC initial] LSC default output result => table_adress_00 : 0x%p, table_adress_01 : 0x%p, table_adress_10 : 0x%p, \
        table_adress_11 : 0x%p\n", al4a.lsc_default_output_data.shading_table_00_address, al4a.lsc_default_output_data.shading_table_01_address, \
        al4a.lsc_default_output_data.shading_table_10_address, al4a.lsc_default_output_data.shading_table_11_address);

    afd_initial(&al4a);
    printf("[AFD initial] enable : %d, enable_close_after_detection : %d, mode : %d\n", \
        al4a.afd_get_para.para.afd_setting.enable, al4a.afd_get_para.para.afd_setting.enable_close_after_detection, \
        al4a.afd_get_para.para.afd_setting.mode);
    
    // set calibration data for AE initial
    // min ISO gain, WB gain r/g/b, afd mode
    al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.min_iso = al4a.lsc_get_para.para.default_ISO.min_iso;
    al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_b_gain = al4a.lsc_get_para.para.default_ISO.gain_b;
    al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain = al4a.lsc_get_para.para.default_ISO.gain_g;
    al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_r_gain = al4a.lsc_get_para.para.default_ISO.gain_r;
    al4a.ae_set_para.set_param.ae_initial_setting.afd_flicker_mode = (enum ae_antiflicker_mode_t)al4a.afd_get_para.para.afd_setting.mode;
    err = ae_initial(&al4a);
    printf("[AE initial] iso %d, ExpT %ld(us), Gain %d, bv_val %ld\n", al4a.ae_get_para.para.ae_get_init_expo_param.iso, \
        al4a.ae_get_para.para.ae_get_init_expo_param.exp_time, al4a.ae_get_para.para.ae_get_init_expo_param.ad_gain, \
        al4a.ae_get_para.para.ae_get_init_expo_param.bv_val);
    
    //set calibration data for AWB initial
    // WB gain r/g/b
    al4a.awb_set_para.para.awb_calib_data.calib_r_gain = al4a.lsc_get_para.para.default_ISO.gain_r;
    al4a.awb_set_para.para.awb_calib_data.calib_g_gain = al4a.lsc_get_para.para.default_ISO.gain_g;
    al4a.awb_set_para.para.awb_calib_data.calib_b_gain = al4a.lsc_get_para.para.default_ISO.gain_b;
    awb_initial(&al4a);
    printf("[AWB initial] final R:%d G:%d B:%d\r\n", al4a.awb_get_para.para.awb_init_data.initial_wbgain.r_gain, \
        al4a.awb_get_para.para.awb_init_data.initial_wbgain.g_gain, al4a.awb_get_para.para.awb_init_data.initial_wbgain.b_gain);
    printf("[AWB initial] CT:%d\r\n", al4a.awb_get_para.para.awb_init_data.color_temperature);
    
    /* the following is a example to describe the timing of ISP initial after 4A*/
    // ISPQ_INPUT *input_initial;
    //input_initial->uwGain_Sensor_RGB = ae_get_para.para.ae_get_init_expo_param.ad_gain;
    //input_initial->uwDGain_IR = 256;
    //input_initial->uwWB_Gain_R = awb_get_parapara.awb_init_data.initial_wbgain.r_gain;
    //input_initial->uwWB_Gain_GR = awb_get_parapara.awb_init_data.initial_wbgain.g_gain;
    //input_initial->uwWB_Gain_GB = awb_get_parapara.awb_init_data.initial_wbgain.g_gain;
    //input_initial->uwWB_Gain_B = awb_get_parapara.awb_init_data.initial_wbgain.b_gain;
    //input_initial->uwWB_CT = awb_get_parapara.awb_init_data.color_temperature;
    //int err;
    //err = ISPQ_Scenario_Case_2_1_Initial(qmerge_obj, &input_initial, &ispq_output);
    
    /*  ===========================================
    *       process flow
    *       sample iteranion run (g_udsofidx)
    *   =========================================== */
    do
    {
        unsigned int err = 0;

        /* TO DO:
        * Wait SOF event and get the effective exposure time and adgain of the current statistics frame from sensor module
        */

        err = ae_run(&al4a, g_udsofidx);
        if (err != _AL_3ALIB_SUCCESS) {
            printf("AE run error: %#x\r\n", err);
            return err;
        }
        printf("[alAELib] Process[%d]: Ymean %d, Lux %ld ExpT %ld(us), Gain %ld, converge %d\n", \
            g_udsofidx, al4a.ae_output.ymean100_8bit, al4a.ae_output.lux_result, al4a.ae_output.udexposure_time, \
            al4a.ae_output.udsensor_ad_gain, al4a.ae_output.ae_converged);

        err = afd_run(&al4a, g_udsofidx);
        if (err != _AL_AFDLIB_SUCCESS) {
            printf("AFD run error: %#x\r\n", err);
            return err;
        }
        printf("[alAFDLib] output result => flag_enable : %d, flag_bending : %d, afd_mode : %d, afd_mode_stable : %d\n", 
            al4a.afd_output.flag_enable, al4a.afd_output.flag_bending, al4a.afd_output.afd_mode, al4a.afd_output.afd_mode_stable);

        err = awb_run(&al4a);
        if(err != _AL_AWBLIB_SUCCESS) {
            printf("AWB run error: %#x\r\n", err);
            return err;
        }
        printf("[alAWBLib] CT: %3d\r\n", (int)al4a.awb_output.color_temp);
        printf("[alAWBLib] WB Gain %3d %3d %3d\r\n", al4a.awb_output.wbgain.r_gain, al4a.awb_output.wbgain.g_gain, al4a.awb_output.wbgain.b_gain);

        err = lsc_run(&al4a);
        if (err != _AL_LSCLIB_SUCCESS) {
            printf("LSC run error: %#x\r\n", err);
            return err;
        }
        printf("[alLSCLib] LSC dynamic process result => table_address_00 : 0x%p, table_address_01 : 0x%p, \
            table_address_10 : 0x%p, table_address_11 : 0x%p\n", al4a.lsc_output.shading_table_00_address, \
            al4a.lsc_output.shading_table_01_address, al4a.lsc_output.shading_table_10_address, al4a.lsc_output.shading_table_11_address);

        /*  ===========================================
        *     ISP update sample
        *     the following is an example to describe the timing of update ISP after 4A
        *   =========================================== 
        */ //the update parameter from RGB AE / RGB AWB library
        //input_initial.uwGain_Sensor_RGB = ae_output.udsensor_ad_gain;
        //input_initial.uwGain_Midtones_RGB = ae_output->midtones_gain;
        //input_initial.uwGain_Shadows_RGB = ae_output->shadows_gain;
        //input_initial.uwWB_Gain_R = awb_output->wbgain.r_gain;
        //input_update.uwWB_Gain_GR = awb_output->wbgain.g_gain;
        //input_initial.uwWB_Gain_GB = awb_output->wbgain.g_gain;
        //input_initial.uwWB_Gain_B = awb_output->wbgain.b_gain;
        //input_initial.uwWB_CT = awb_output->color_temp;
        //ISPQ_Scenario_Case_2_1_Update(&input_initial, &ispq_output);


        // So far, we finished 4A process once
        // from the example we simply run the estimation iteration by SOF index 
        // in real case, please make sure to send the SOF notify with new valid and effective exp-time/adgain 
        g_udsofidx++;

    }while(g_udsofidx <= 2);

    /*  ===========================================
    *     De-init flow
    *   =========================================== */
    err = al4a.afd_func_obj.deinit(al4a.afd_obj);
    if (err != _AL_AFDLIB_SUCCESS) {
        printf("afd deinit error\r\n");
        return err;
    }
    
    err = al4a.ae_func_obj.deinit(al4a.ae_obj);
    if (err != _AL_3ALIB_SUCCESS) {
        printf("ae deinit error\r\n");
        return err;
    }

    err = al4a.awb_obj.deinit(&(al4a.awb_obj));
    if(err != _AL_AWBLIB_SUCCESS){
        printf("awb deinit error\r\n");
        return err;
    }

    err = al4a.lsc_func_obj.deinit(al4a.lsc_obj);
    if (err != _AL_LSCLIB_SUCCESS) {
        printf("lsc deinit error\r\n");
        return err;
    }

    system("PAUSE");
    return 0;

}

unsigned int lsc_run(al4ahd *pal4a)
{
    unsigned int lsc_err = _AL_LSCLIB_SUCCESS;

    void **lsc_obj = &(pal4a->lsc_obj);
    struct alLSClib_runtime_obj_t *lsc_func_obj = &(pal4a->lsc_func_obj); 
    struct alLSCLib_set_parameter_t *lsc_set_para = &(pal4a->lsc_set_para);
    struct alLSClib_output_data_t *lsc_output = &(pal4a->lsc_output);
    struct ae_output_data_t *ae_output = &(pal4a->ae_output);
    struct allib_awb_output_data *awb_output = &(pal4a->awb_output);

    //get ISO, AWB gain, BV, ParaIndex data from awb & ae
    lsc_set_para->type = SET_PARAM_TYPE_LSC_DYNAMIC_PROC_PARA;
    lsc_set_para->para.dynamic_proc_para.current_bv = ae_output->ae_converged;
    lsc_set_para->para.dynamic_proc_para.iso_speed = ae_output->iso;
    lsc_set_para->para.dynamic_proc_para.y_mean = ae_output->ymean100_8bit;
    lsc_set_para->para.dynamic_proc_para.interpolate_ct = awb_output->color_temp;
    lsc_err = lsc_func_obj->set_param(lsc_set_para, *lsc_obj);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }

    lsc_err = lsc_func_obj->dynamic_process(*lsc_obj, lsc_output);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }

    return lsc_err;
}

unsigned int awb_run(al4ahd *pal4a)
{
    unsigned int awb_err = _AL_AWBLIB_SUCCESS;

    struct allib_awb_runtime_obj *awb_obj = &(pal4a->awb_obj); 
    struct allib_awb_set_parameter *awb_set_para = &(pal4a->awb_set_para);
    struct ae_output_data_t *ae_output = &(pal4a->ae_output);
    struct allib_awb_output_data *awb_output = &(pal4a->awb_output);

    awb_set_para->type = ALAWB_SET_PARAM_UPDATE_AE_REPORT;
    awb_set_para->para.ae_report_update.ae_converge       = ae_output->ae_converged;
    awb_set_para->para.ae_report_update.bv                = ae_output->bvresult;
    awb_set_para->para.ae_report_update.non_comp_bv       = ae_output->bg_bvresult;
    awb_set_para->para.ae_report_update.iso               = ae_output->iso;
    awb_set_para->para.ae_report_update.midtones_gain     = ae_output->midtones_gain;
    awb_set_para->para.ae_report_update.shadows_gain      = ae_output->shadows_gain;
    awb_err = awb_obj->set_param(awb_set_para, awb_obj->awb);
    if(awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }

    awb_err = awb_obj->process(awb_stats, awb_obj->awb, awb_output, NULL);
    if(awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }
    return awb_err;
}

unsigned int afd_run(al4ahd *pal4a, unsigned int sofidx)
{
    unsigned int afd_err = _AL_AFDLIB_SUCCESS;

    unsigned int eff_adgain;
    unsigned int eff_exp;
    unsigned short ymean = pal4a->ae_output.ymean100_8bit;

    void **afd_obj = &(pal4a->afd_obj);
    struct allib_afd_runtime_obj_t *afd_func_obj = &(pal4a->afd_func_obj); 
    struct allib_afd_set_parameter_t *afd_set_para = &(pal4a->afd_set_para);
    struct allib_afd_output_data_t *afd_output = &(pal4a->afd_output);

    /* set effective exposure time and effective adgain which getting from sensor
    *  for the sample example,  we assume effective exposure and effective gain 
    *  are the same as AE output report from the previous estimation
    */
    if (sofidx < 1)
    {
        eff_adgain = pal4a->ae_get_para.para.ae_get_init_expo_param.ad_gain;
        eff_exp = pal4a->ae_get_para.para.ae_get_init_expo_param.exp_time;
    }
    else
    {
        eff_adgain = pal4a->ae_output.udsensor_ad_gain;
        eff_exp = pal4a->ae_output.udexposure_time;
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
    afd_set_para->para.stats_size = STATS_DATA_SIZE;
    afd_err = afd_func_obj->set_param(afd_set_para, *afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        return afd_err;
    }
    
    afd_err = afd_func_obj->push_stats(_g_channel_stats_buffer[sofidx], *afd_obj); 
    if (sofidx % 2 == 1)
    {
        afd_err = afd_func_obj->kernel_process(*afd_obj, afd_output);
        if (afd_err != _AL_AFDLIB_SUCCESS) {
            return afd_err;
        }
    } 

    return afd_err;
}

unsigned int ae_run(al4ahd *pal4a, unsigned int sofidx)
{
    unsigned int ae_err = _AL_3ALIB_SUCCESS;

    void **ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj *ae_func_obj = &(pal4a->ae_func_obj); 
    struct ae_set_param_t *ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t *ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t *ae_output = &(pal4a->ae_output);
    struct alhw3a_ae_proc_data_t *ae_proc_data = &(pal4a->ae_proc_data);
    enum ae_antiflicker_mode_t afdmode;

    if (sofidx < 2){
        afdmode = pal4a->afd_get_para.para.afd_setting.mode; 
    }
    else{
        afdmode = pal4a->afd_output.afd_mode_stable;
    }

    /* set effective exposure time and effective adgain which getting from sensor
    *  for the sample example,  we assume effective exposure and effective gain 
    *  are the same as AE output report from the previous estimation
    */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_SOF_NOTIFY;
    if (sofidx == 0)
    {
        ae_set_para->set_param.sof_notify_param.exp_adgain = ae_get_para->para.ae_get_init_expo_param.ad_gain;
        ae_set_para->set_param.sof_notify_param.exp_time = ae_get_para->para.ae_get_init_expo_param.exp_time;
		ae_set_para->set_param.sof_notify_param.midtones_gain = ae_get_para->para.ae_get_init_expo_param.midtones_gain;
    }
    else
    {
        ae_set_para->set_param.sof_notify_param.exp_adgain = ae_output->udsensor_ad_gain;
        ae_set_para->set_param.sof_notify_param.exp_time = ae_output->udexposure_time;
		ae_set_para->set_param.sof_notify_param.midtones_gain = ae_output->midtones_gain;
    }
    ae_set_para->set_param.sof_notify_param.sys_sof_index = sofidx;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /* set anti-flicker mode from AFD output results */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_ANTIFLICKER_MODE;
    ae_set_para->set_param.afd_flicker_mode = afdmode;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /* TO DO:
    * Prepare HW stats data and wait frame done event
    */
    ae_proc_data->udsys_sof_idx = sofidx;

    /* Get AE stats from hw */
    // The address of HW stats data, here is pseudo sample
    ae_stats = (void *)ae_stats_buffer;
	// The address of HW Yhis stats data, here is pseudo sample
	yhis_stats = (void *)yhis_stats_buffer;

    ae_err = ae_func_obj->process(ae_stats, yhis_stats, *ae_proc_data, *ae_obj, ae_output);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        printf("3\r\n");
        return ae_err;
    }

    return ae_err;
}

unsigned int load_tuning_bin_and_stats()
{
    FILE *pf;
    unsigned int size = 0;
   
    if (NULL != (pf = fopen(_LSC_TUNE_FILENAME, "rb"))) {
        fseek(pf, 0, SEEK_END);
        size = ftell(pf);
        rewind(pf);
        fread((void *)LSC_tuning_buf, size, 1, pf);
        fclose(pf);
    }
    else{
        printf("load bin error LSC\r\n");
        return 1;
    }

    if (NULL != (pf = fopen(_G_CHANNEL_STATS_FILE_NAME, "rb"))) {
        fseek(pf, 0, SEEK_SET);
        for (unsigned int index = 0; index < _SIMULATE_AFD_RUN; index++) {
            fread((void*)&_g_channel_stats_buffer[index][0], STATS_DATA_SIZE, 1, pf);
        }
        fclose(pf);
    }
    else{
        printf("load stats error AFD\r\n");
        return 1;
    }

    if (NULL != (pf = fopen(_AE_STATS_FILENAME, "rb"))) {
        fseek(pf, 0, SEEK_END);
        size = ftell(pf);
        rewind(pf);
        fread((void *)ae_stats_buffer, size, 1, pf);
        fclose(pf);
    }
    else{
        printf("load stats error AE\r\n");
        return 1;
    }

	if (NULL != (pf = fopen(_YHIS_STATS_FILENAME, "rb"))) {
        fseek(pf, 0, SEEK_END);
        size = ftell(pf);
        rewind(pf);
        fread((void *)yhis_stats_buffer, size, 1, pf);
        fclose(pf);
    }
    else{
        printf("load stats error YHIS\r\n");
        return 1;
    }

    if (NULL != (pf = fopen(_AWB_STATS_FILENAME,"rb"))) {
        fseek(pf, 0, SEEK_END);
        size = ftell(pf);
        awb_bin_size = size;
        rewind(pf);
        fread((void *)awb_stats, size, 1, pf);
        fclose(pf);
    }
    else{
        printf("load stats error AWB\r\n");
        return 1;
    }

    return 0;
}

unsigned int load_lib(al4ahd *pal4a)
{
    unsigned int ae_err = _AL_3ALIB_SUCCESS;
    unsigned int lsc_err = _AL_LSCLIB_SUCCESS;
    unsigned int afd_err = _AL_AFDLIB_SUCCESS;
    unsigned int awb_err = _AL_AWBLIB_SUCCESS;

    lsc_err = alLSCLib_loadfunc(&(pal4a->lsc_func_obj), 0);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        printf("ERROR %x\r\n", (unsigned int)lsc_err);
        return lsc_err;
    }

    afd_err = allib_afd_loadfunc(&(pal4a->afd_func_obj), 0);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        printf("ERROR %x\r\n", (unsigned int)afd_err);
        return afd_err;
    }

    ae_err = allib_ae_loadfunc(&(pal4a->ae_func_obj), 0);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        printf("ERROR %x\r\n", (unsigned int)ae_err);
        return ae_err;
    }

    awb_err = allib_awb_loadfunc(&(pal4a->awb_obj));
    if (awb_err != _AL_AWBLIB_SUCCESS) {
        printf("ERROR %x\r\n", (unsigned int)awb_err);
        return awb_err;
    }
    return 0;
}

unsigned int lsc_initial(al4ahd *pal4a)
{
    unsigned int lsc_err = _AL_LSCLIB_SUCCESS;

    void **lsc_obj = &(pal4a->lsc_obj);
    struct alLSClib_runtime_obj_t *lsc_func_obj = &(pal4a->lsc_func_obj); 
    struct alLSCLib_set_parameter_t *lsc_set_para = &(pal4a->lsc_set_para);
    struct alLSCLib_get_parameter_t *lsc_get_para = &(pal4a->lsc_get_para);
    struct alLSClib_output_data_t *lsc_default_output_data = &(pal4a->lsc_default_output_data);

    lsc_err = lsc_func_obj->initial(lsc_obj);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }

    lsc_set_para->type = SET_PARAM_TYPE_LSC_TUNING_BIN;
    lsc_set_para->para.tuning_data = LSC_tuning_buf;
    lsc_err = lsc_func_obj->set_param(lsc_set_para, *lsc_obj);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }

    lsc_get_para->type = GET_PARAM_TYPE_DEFAULT_ISO;
    lsc_err = lsc_func_obj->get_param(lsc_get_para, *lsc_obj);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }
    
    printf("[Get LSC default ISO AWB FN]\n");
    printf(" min iso : %d\n gain r : %d\n gain g : %d\n gain b : %d\n fn : %d\n"
           , lsc_get_para->para.default_ISO.min_iso
           , lsc_get_para->para.default_ISO.gain_r
           , lsc_get_para->para.default_ISO.gain_g
           , lsc_get_para->para.default_ISO.gain_b
           , lsc_get_para->para.default_ISO.fn);
    
    lsc_get_para->type = GET_PARAM_TYPE_LSC_HW_INIT_PARA;
    lsc_err = lsc_func_obj->get_param(lsc_get_para, *lsc_obj);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }

    lsc_set_para->type = SET_PARAM_TYPE_LSC_COLOR_ORDER;
    lsc_set_para->para.color_order = TYPE_LSC_COLOR_ORDER_RGGB;
    lsc_err = lsc_func_obj->set_param(lsc_set_para, *lsc_obj);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }

    lsc_err = lsc_func_obj->calc_default_table(*lsc_obj, lsc_default_output_data);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }

    return lsc_err;
}

unsigned int afd_initial(al4ahd *pal4a)
{
    unsigned int afd_err = _AL_AFDLIB_SUCCESS;

    void **afd_obj = &(pal4a->afd_obj);
    struct allib_afd_runtime_obj_t *afd_func_obj = &(pal4a->afd_func_obj); 
    struct allib_afd_set_parameter_t *afd_set_para = &(pal4a->afd_set_para);
    struct allib_afd_get_parameter_t *afd_get_para = &(pal4a->afd_get_para);

    afd_err = afd_func_obj->initial(afd_obj);
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        return afd_err;
    }
    printf("afd initial obj addr:%p, %p, %p\r\n", afd_obj, &(pal4a->afd_obj), pal4a->afd_obj);

    afd_set_para->type = ALAFD_SET_PARAM_SENSOR_INFO;
    afd_set_para->para.sensor_info.line_time_ns = 29800;
    afd_set_para->para.sensor_info.sensor_width = 1920;
    afd_set_para->para.sensor_info.sensor_height = 1088;
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
    if (afd_err != _AL_AFDLIB_SUCCESS) {
        return afd_err;
    }

    return afd_err;
}

unsigned int ae_initial(al4ahd *pal4a)
{
    unsigned int ae_err = _AL_3ALIB_SUCCESS;

    void **ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj *ae_func_obj = &(pal4a->ae_func_obj); 
    struct ae_set_param_t *ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t *ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t *ae_output = &(pal4a->ae_output);

    ae_err = ae_func_obj->initial(ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ae_set_para->ae_set_param_type = AE_SET_PARAM_INIT_SETTING;
    ae_set_para->set_param.ae_initial_setting.ae_enable = 1;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ae_get_para->ae_get_param_type = AE_GET_INIT_EXPOSURE_PARAM;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    return ae_err;
}

unsigned int awb_initial(al4ahd *pal4a)
{
    unsigned int awb_err = _AL_AWBLIB_SUCCESS;

    struct allib_awb_runtime_obj *awb_obj = &(pal4a->awb_obj); 
    struct allib_awb_set_parameter *awb_set_para = &(pal4a->awb_set_para);
    struct allib_awb_get_parameter *awb_get_para = &(pal4a->awb_get_para);


    awb_err = awb_obj->initial(awb_obj, awb_bin_size);
    if(awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }

    awb_set_para->type = ALAWB_SET_PARAM_CAMERA_CALIB_DATA;
    awb_err = awb_obj->set_param(awb_set_para, awb_obj->awb);
    if(awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }

    awb_get_para->type = ALAWB_GET_PARAM_INIT_SETTING;
    awb_err = awb_obj->get_param(awb_get_para, awb_obj->awb);
    if(awb_err != _AL_AWBLIB_SUCCESS) {
        return awb_err;
    }

    return awb_err;
}