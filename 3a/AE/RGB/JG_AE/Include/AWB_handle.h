#ifndef _AWB_HANDLE_H_
#define _AWB_HANDLE_H_

#include "allib_awb.h"
#include "allib_ae.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct gxr_awbhd{

    void*                               awb_obj;
    int*                                awb_stats;
    struct ae_output_data_t             ae_output;
    struct allib_awb_output_data        awb_output;
    struct allib_awb_get_parameter      awb_get_para;
    struct allib_awb_set_parameter      awb_set_para;
} gxr_awb ;

__declspec(dllexport) unsigned int awb_initial(gxr_awb* gxr_awb);
__declspec(dllexport) unsigned int awb_run(gxr_awb* gxr_wb);
__declspec(dllexport) unsigned int awb_deinit(gxr_awb* gxr_awb);




#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // !_AWB_HANDLE_H_