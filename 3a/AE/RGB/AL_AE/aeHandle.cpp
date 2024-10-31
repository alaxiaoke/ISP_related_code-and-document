#ifndef _AE_HANDLE_C_
#define _AE_HANDLE_C_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <string>

#include "./Include/allib_ae.h"
#include "./Include/allib_ae_errcode.h"
#include "./aeHandle.h"

unsigned int ae_initial(gxr_aehd* gxr_ae)
{
    /*  ===========================================
    *     Get function version.
    *   =========================================== */
    al4ahd* pal4a = (al4ahd*)malloc(sizeof(al4ahd));
    gxr_ae->al4a = pal4a;

    allib_ae_getlib_version(&pal4a->ae_ver);
    printf("AE ver: %d.%d\n", pal4a->ae_ver.major_version, pal4a->ae_ver.minor_version);
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
    unsigned int ae_err = _AL_3ALIB_SUCCESS;

    ae_err = allib_ae_loadfunc(&(pal4a->ae_func_obj), gxr_ae->cameraId);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        printf("ERROR %x\r\n", (unsigned int)ae_err);
        //gxr_ae->err_code = ae_err;
        return ae_err;
    }

    void** ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj* ae_func_obj = &(pal4a->ae_func_obj);
    struct ae_set_param_t* ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t* ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t* ae_output = &(pal4a->ae_output);

    ae_err = ae_func_obj->initial(ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        //gxr_ae->err_code = ae_err;
        return ae_err;
    }

    ae_set_para->ae_set_param_type = AE_SET_PARAM_INIT_SETTING;
    memcpy(&ae_set_para->set_param.ae_initial_setting,&gxr_ae->ae_set_para.set_param.ae_initial_setting,sizeof(ae_set_parameter_init_t));
    ae_set_para->set_param.cameraId = ae_func_obj->identityid;

    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        //gxr_ae->err_code = ae_err;
        return ae_err;
    }

    ae_get_para->ae_get_param_type = AE_GET_INIT_EXPOSURE_PARAM;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        //gxr_ae->err_code = ae_err;
        return ae_err;
    }
    memcpy(&gxr_ae->ae_get_para.para.ae_get_init_expo_param, &ae_get_para->para.ae_get_init_expo_param, sizeof(ae_init_exposure_param_t));
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
        return ae_err;
    }
    return ae_err;
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

    memcpy(&ae_set_para->set_param, &gxr_ae->ae_set_para.set_param, sizeof(ae_set_param_content_t));

    ae_set_para->ae_set_param_type = AE_SET_PARAM_SOF_NOTIFY;

    //ae_set_para->set_param.sof_notify_param.sys_sof_index = gxr_ae->ae_set_para.set_param.sof_notify_param.sys_sof_index;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        //gxr_ae->err_code = ae_err;
        return ae_err;
    }


    /* set anti-flicker mode from AFD output results */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_ANTIFLICKER_MODE;
    //ae_set_para->set_param.afd_flicker_mode = (ae_antiflicker_mode_t) gxr_ae->atf_mode;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        //gxr_ae->err_code = ae_err;
        return ae_err;
    }

    ///* set manu ae  */
    //ae_set_para->ae_set_param_type = AE_SET_PARAM_MANUAL_EXPTIME;
    //ae_set_para->set_param.manual_exptime = 4000;
    //ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    //if (ae_err != _AL_3ALIB_SUCCESS) {
    //    return ae_err;
    //}

    /* TO DO:
    * Prepare HW stats data and wait frame done event
    */
    //ae_proc_data->udsys_sof_idx = gxr_ae->frameId;
    ae_proc_data->udsys_sof_idx = gxr_ae->ae_set_para.set_param.sof_notify_param.sys_sof_index;

    /* Get AE stats from hw */
    //// The address of HW stats data, here is pseudo sample
    //ae_stats = (void*)ae_stats_buffer;
    //// The address of HW Yhis stats data, here is pseudo sample
    //yhis_stats = (void*)yhis_stats_buffer;

    ae_err = ae_func_obj->process(gxr_ae->ae_stats, gxr_ae->yhis_stats, *ae_proc_data, *ae_obj, ae_output);
    memcpy(&gxr_ae->ae_output, ae_output, sizeof(ae_output_data_t));
    //out
    //gxr_ae->ae_adgain = ae_output->udsensor_ad_gain;
    //gxr_ae->ae_shutter = ae_output->udexposure_time;
    //gxr_ae->ae_converged = ae_output->ae_converged;
    //gxr_ae->Ymean = ae_output->ymean100_8bit;
    //gxr_ae->ae_lux_result = ae_output->bvresult;

    if (ae_err != _AL_3ALIB_SUCCESS) {
        printf("3\r\n");
        //gxr_ae->err_code = ae_err;
        return ae_err;
    }
    //gxr_ae->err_code = ae_err;
    return ae_err;
}

#endif // !_AE_HANDLE_C_