#ifndef _AE_HANDLE_H_
#define _AE_HANDLE_H_

#include "../Include/allib_ae.h"
#include "../Include/allib_ae_errcode.h"

#ifdef _AE_USAGE_H_
#define GXR_AE _declspec( dllexport )
#else
#define GXR_AE _declspec( dllimport )
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct gxr_aehd {
    uint8_t                         cameraId;
    void*                           al4a;
    void*                           ae_stats;
    void*                           yhis_stats;
    struct ae_set_param_t           ae_set_para;
    struct ae_get_param_t           ae_get_para;
    struct ae_output_data_t         ae_output;
};

GXR_AE unsigned int ae_initial(gxr_aehd* gxr_ae);
GXR_AE unsigned int ae_deinit(gxr_aehd* gxr_ae);
GXR_AE unsigned int ae_run(gxr_aehd* gxr_ae);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // !_AE_HANDLE_H_
