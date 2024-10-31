#pragma once
#ifndef _ATF_HANDLE_
#define _ATF_HANDLE_
//#include "./allib_afd.h"

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

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
    enum atf_mode {
        ATF_MODE_OFF = 0,
        ATF_MODE_50HZ,
        ATF_MODE_60HZ,
        ATF_MODE_MAX,
    };
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
    struct atf_output_data_t {
        bool                                        flag_enable;
        bool                                        flag_bending;
        enum atf_mode                               atf_mode;
        enum atf_mode                               atf_mode_stable;
    };
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
    struct atf_curSet {
        uint8_t                                     enable;
        uint8_t                                     enable_close_after_detection;
        enum atf_mode                               mode;
    };

#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
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

        struct atf_curSet               atf_currSetpara;
        struct atf_output_data_t        atf_output;
    };
#pragma pack(pop)  /* restore old alignment setting from stack */


    GXR_ATF unsigned int atf_initial(gxr_atfhd* gxr_atf);
    GXR_ATF unsigned int atf_deinit(gxr_atfhd* gxr_atf);
    GXR_ATF unsigned int atf_run(gxr_atfhd* gxr_atf);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // !_ATF_HANDLE_
