#ifndef _AE_HANDLE_H_
#define _AE_HANDLE_H_

#include "./Include/allib_ae.h"
//#include "../Include/allib_ae_errcode.h"

#ifdef _AE_HANDLE_C_
#define GXR_AE _declspec( dllexport )
#else
#define GXR_AE _declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct gxr_aehd {
    UINT8                           cameraId;
    void*                           al4a;
    void*                           ae_stats;
    void*                           yhis_stats;

    struct ae_set_param_t           ae_set_para;
    struct ae_get_param_t           ae_get_para;
    struct ae_output_data_t         ae_output;

    ////enable 
    //int     ae_enable;   // 0 or 1
    //uint32_t frameId;
    //// calib param
    //uint32_t min_iso;         /*!< minimum ISO */
    //uint32_t calib_r_gain;    /*!< scale 1000 */
    //uint32_t calib_g_gain;    /*!< scale 1000 */
    //uint32_t calib_b_gain;    /*!< scale 1000 */
    ////atf mode
    //int atf_mode;   //0: off, 1: 50HZ; 2: 60HZ
    //// sensor info 
    //float shutter;
    //float sensorgain;

    //// out
    //float ae_shutter;      //us
    //float ae_adgain;     //scale 100
    //float ae_ispgain;    //scale 1  out for isp
    //int ae_lux_result;   //scale 1000
    //float Ymean;        //scale 100
    //int16_t   ae_converged;
    //
    //int32_t  err_code;
};

GXR_AE unsigned int ae_initial(gxr_aehd* gxr_ae);
GXR_AE unsigned int ae_deinit(gxr_aehd* gxr_ae);
GXR_AE unsigned int ae_run(gxr_aehd* gxr_ae);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // !_AE_HANDLE_H_
