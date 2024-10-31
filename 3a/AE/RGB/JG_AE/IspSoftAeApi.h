#ifndef __ISP_SOFT_AE_API_H__
#define __ISP_SOFT_AE_API_H__


#include "IspSoftApi.h"


#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum IspSoftAeProcessType
    {
        ISP_SOFT_AE_PROCESS_IN_IMAGE = 0,
        ISP_SOFT_AE_PROCESS_IN_TUNING,
        ISP_SOFT_AE_PROCESS_IN_DEBUG
    } IspSoftAeProcessType;


    typedef enum IspSoftAePostType
    {
        ISP_SOFT_AE_POST_OUT_READY = 0
    } IspSoftAePostType;


    typedef struct IspSoftAeInitParam
    {
        uint8_t         camera_id;

        // sensor ini set
        uint32_t        initial_sensor_shutter_us;
        uint32_t        initial_sensor_adgain;

        // calib param OTP
        IspSoftBuffer   otp_bin;

        // TUNING
        IspSoftBuffer   tuning_bin;

        const char      *config_path;
    } IspSoftAeInitParam;


    typedef struct IspSoftAeInImage
    {
        uint32_t        frame_id;
        uint64_t        timestamp;

        // STATS
        IspSoftBuffer   isp_out;

        // ATF out
        IspSoftBuffer   atf_out;

        // AWB out
        IspSoftBuffer   awb_out;
    } IspSoftAeInImage;


    typedef struct IspSoftAeOutReady
    {
        uint32_t        frame_id;
        uint64_t        timestamp;

        // out
        uint32_t        ae_shutter_us;
        uint32_t        ae_adgain;
        IspSoftBuffer   ae_out;
    } IspSoftAeOutReady;


    typedef IspSoftBuffer IspSoftAeTuning;


    typedef IspSoftBuffer IspSoftAeDebug;


    ISP_SOFT_API(IspSoftModule) IspSoftAeLoad();


#ifdef __cplusplus
}
#endif


#endif