#ifndef __ISP_SOFT_ATF_API_H__
#define __ISP_SOFT_ATF_API_H__


#include "IspSoftApi.h"


#ifdef __cplusplus
extern "C"
{
#endif


    typedef enum IspSoftAtfProcessType
    {
        ISP_SOFT_ATF_PROCESS_IN_IMAGE = 0,
        ISP_SOFT_ATF_PROCESS_IN_TUNING,
        ISP_SOFT_ATF_PROCESS_IN_DEBUG
    } IspSoftAtfProcessType;


    typedef enum IspSoftAtfPostType
    {
        ISP_SOFT_ATF_POST_OUT_READY = 0
    } IspSoftAtfPostType;


    typedef struct AlsConfig
    {
        uint32_t        sample_rate;
        uint32_t        bin_num;
        float           freq_step;
    } AlsConfig;


    typedef struct IspSoftAtfInitParam {
        // sensor info
        uint32_t                line_time_ns;
        uint32_t                sensor_width;
        uint32_t                sensor_height;

        // als config
        AlsConfig               als_config;

        // Tuning
        IspSoftBuffer           tuning_bin;

        const char*             config_path;
    } IspSoftAtfInitParam;


    typedef struct Freq
    {
        float       freq;
        uint32_t    amp;
    } Freq;


    typedef struct FS_Result
    {
        Freq        freq1;
        Freq        freq2;
        Freq        freq3;
    } FS_Result;


    typedef struct IspSoftAtfInImage
    {
        uint32_t                frame_id;
        uint64_t                timestamp;

        // STATS
        IspSoftBuffer           isp_out;

        // get from ae out
        IspSoftBuffer           ae_out;

        // fs data
        FS_Result               fs_out;
    } IspSoftAtfInImage;


    typedef struct IspSoftAtfOutReady
    {
        uint32_t            frame_id;
        uint64_t            timestamp;

        IspSoftBuffer       atf_out;
    } IspSoftAtfOutReady;


    typedef IspSoftBuffer IspSoftAtfTuning;


    typedef IspSoftBuffer IspSoftAtfDebug;


    ISP_SOFT_API(IspSoftModule) IspSoftAtfLoad();


#ifdef __cplusplus
}
#endif


#endif