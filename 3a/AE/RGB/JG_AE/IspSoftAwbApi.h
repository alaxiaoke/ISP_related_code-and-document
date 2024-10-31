#ifndef __ISP_SOFT_AWB_API_H__
#define __ISP_SOFT_AWB_API_H__


#include "IspSoftApi.h"


#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum IspSoftAwbProcessType
    {
        ISP_SOFT_AWB_PROCESS_IN_IMAGE = 0,
        ISP_SOFT_AWB_PROCESS_IN_TUNING,
        ISP_SOFT_AWB_PROCESS_IN_DEBUG
    } IspSoftAwbProcessType;


    typedef enum IspSoftAwbPostType
    {
        ISP_SOFT_AWB_POST_OUT_READY = 0
    } IspSoftAwbPostType;


    typedef struct IspSoftAwbInitParam
    {
        uint8_t             camera_id;
        IspSoftBuffer       otp_bin;
        IspSoftBuffer       tuning_bin;
    } IspSoftAwbInitParam;


    typedef struct IspSoftAwbInImage
    {
        uint32_t            frame_id;
        uint64_t            timestamp;

        // STATS
        IspSoftBuffer       isp_out;
        IspSoftBuffer       ae_out;
    } IspSoftAwbInImage;


    typedef struct IspSoftAwbOutReady
    {
        uint32_t            frame_id;
        uint64_t            timestamp;

        IspSoftBuffer       awb_out;
    } IspSoftAwbOutReady;


    typedef IspSoftBuffer IspSoftAwbTuning;


    typedef IspSoftBuffer IspSoftAwbDebug;


    ISP_SOFT_API(IspSoftModule) IspSoftAwbLoad();


#ifdef __cplusplus
}
#endif


#endif