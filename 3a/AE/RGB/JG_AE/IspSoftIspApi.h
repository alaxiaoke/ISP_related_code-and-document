#ifndef __ISP_SOFT_ISP_API_H__
#define __ISP_SOFT_ISP_API_H__


#include "IspSoftApi.h"


#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum IspSoftIspProcessType
    {
        ISP_SOFT_ISP_PROCESS_IN_IMAGE = 0,
		ISP_SOFT_ISP_PROCESS_IN_TUNING,
        ISP_SOFT_ISP_PROCESS_IN_DEBUG
    } IspSoftIspProcessType;


    typedef enum IspSoftIspPostType
    {
        ISP_SOFT_ISP_POST_OUT_STATS_READY = 0,
        ISP_SOFT_ISP_POST_OUT_IMAGE_READY
    } IspSoftIspPostType;


    typedef void (ISP_SOFT_CALL_TYPE *IspSoft_OnNotifyGPUCompute)(void *owner);


    typedef struct IspSoftIspInitParam
    {
        uint8_t          camera_id;
        bool             out_debug_rgb;     // 为 true 时输出 debug_rgb，为 false 时输出 yuv
        uint32_t         rotation;
        IspSoftImageInfo in_image_info;

        union
        {
            IspSoftImageInfo debug_rgb_info;
            IspSoftImageInfo out_yuv_info;
        };

        IspSoftImageInfo out_rgb_info;
        IspSoftBuffer    tuning_bin;

        IspSoft_OnNotifyGPUCompute OnNotifyGPUCompute;  // 每次进行 GPU 计算前需要调用，等待 GPU 释放资源
    } IspSoftIspInitParam;


    typedef struct IspSoftIspInImage
    {
        uint32_t         frame_id;
        uint64_t         timestamp;
        IspSoftBuffer    awb_out;
        IspSoftBuffer    ae_out;
        uint32_t         sensor_shutter_us;
        uint32_t         sensor_adgain;
        IspSoftBuffer    rawdata;
    } IspSoftIspInImage;


    typedef struct IspSoftIspStatusReady
    {
        uint32_t         frame_id;
        uint64_t         timestamp;
        IspSoftBuffer    awb_stats;
        IspSoftBuffer    ae_stats;
        IspSoftBuffer    atf_stats;
    } IspSoftIspStatusReady;


    typedef struct IspSoftIspYuv
    {
        IspSoftBuffer    y;
        IspSoftBuffer    cbcr;
    } IspSoftIspYuv;


    typedef struct IspSoftIspOutImageReady
    {
        uint32_t            frame_id;
        uint64_t            timestamp;

        union
        {
            IspSoftBuffer   debug_rgb;
            IspSoftIspYuv   out_yuv;    // GPU
        };

        IspSoftBuffer       out_rgb;    // CPU: matting/handmask
    } IspSoftIspOutImageReady;


    typedef IspSoftBuffer IspSoftIspTuning;


    typedef IspSoftBuffer IspSoftIspDebug;


    ISP_SOFT_API(IspSoftModule) IspSoftIspLoad();


#ifdef __cplusplus
}
#endif


#endif