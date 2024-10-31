#pragma once
#ifndef _ATF_HANDLE_
#define _ATF_HANDLE_

#ifdef _ATF_HANDLE_C_
#define GXR_ATF _declspec( dllexport )
#else
#define GXR_ATF _declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define _SIMULATE_AFD_RUN                     (50)
#define SCANNER_HEIGHT                        (1088)
#define STATS_DATA_SIZE                       (1744)  //1072


    enum atf_mode {
        ATF_MODE_OFF = 0,
        ATF_MODE_50HZ,
        ATF_MODE_60HZ,
        ATF_MODE_90HZ = 3,
        ATF_MODE_144HZ = 4,
        ATF_MODE_240HZ = 5,
        ATF_MODE_320HZ = 6,
        ATF_MODE_360HZ = 7,
        ATF_MODE_MAX,
    };

    struct Freq
    {
        uint32_t    freq;
        uint32_t    amp;
    };

    struct FS_result
    {
        int    update_enable;
        Freq    freq1;
        Freq    freq2;
        Freq    freq3;
    };

    struct atf_output_data_t {
        uint8_t                                     flag_enable;
        uint8_t                                     flag_bending;
        enum atf_mode                               atf_mode;
        enum atf_mode                               atf_mode_stable;
    };

    struct atf_curSet {
        uint8_t                                     enable;
        uint8_t                                     enable_close_after_detection;
        enum atf_mode                               mode;
    };

    typedef struct gxr_atfhd {
        void*                   al4a;
        void*                   atf_stats;
        //sensor info
        uint32_t                line_time_ns;
        uint32_t                sensor_width;
        uint32_t                sensor_height;
        // exposure info
        uint32_t                sofIndex;
        float                   exposure_time;
        float                   gain;
        //get from ae out
        float                   average_value;

        ////tuning down_sampling
        //uint16_t                jump_sample_num;                //suggest 16
        //uint16_t                jump_sample_do_avg_count;       //suggest 8

        struct atf_curSet               atf_currSetpara;
        struct atf_output_data_t        atf_output;
        //tuning tag
        bool                    tuning_enable;
        //fs
        FS_result               fs_result;
    };


    GXR_ATF unsigned int atf_initial(gxr_atfhd* gxr_atf);
    GXR_ATF unsigned int atf_deinit(gxr_atfhd* gxr_atf);
    GXR_ATF unsigned int atf_run(gxr_atfhd* gxr_atf);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // !_ATF_HANDLE_
