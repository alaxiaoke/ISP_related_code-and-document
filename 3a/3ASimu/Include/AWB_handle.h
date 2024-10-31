#ifndef _AWB_HANDLE_H_
#define _AWB_HANDLE_H_

#include "allib_awb.h"
#include "allib_ae.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum LogLevelAWB
{
    AWBLog_OFF = 0,
    AWBTrace = 1 << 0,
    AWBDebug = 1 << 1,
    AWBInfo = 1 << 2,
    AWBWarn = 1 << 3,
    AWBError = 1 << 4,
    AWBFatal = 1 << 5,
    AWBLog_End
};

typedef struct gxr_awbhd{

    void*                               awb_obj;
    int*                                awb_stats;
    struct ae_output_data_t             ae_output;
    struct allib_awb_output_data        awb_output;
    struct allib_awb_get_parameter      awb_get_para;
    struct allib_awb_set_parameter      awb_set_para;
    int                                 identityid;
    enum LogLevelAWB                       debug_level;  //default off
    int                                 save_to_file;  //default false
    bool                                tuning_enable;
} gxr_awb ;

__declspec(dllexport) unsigned int awb_initial(gxr_awb* gxr_awb);
__declspec(dllexport) unsigned int awb_run(gxr_awb* gxr_wb);
__declspec(dllexport) unsigned int awb_deinit(gxr_awb* gxr_awb);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // !_AWB_HANDLE_H_