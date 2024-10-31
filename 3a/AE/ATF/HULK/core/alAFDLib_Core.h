#ifndef _AFD_KERNEL_
#define _AFD_KERNEL_

#include "allib_afd.h"

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

#include "allib_mtype.h"
#include "alAFDLib_DFT.h"

#define G_AFD_VERSION_MAJOR             (1)
#define G_AFD_VERSION_MINOR             (313)
#define G_AFD_TUNING_STRUCT_VERSION     (0)

#define AFD_HWSTATS_INFO_LOCATION           (0)     //0:before stats; 1:after stats
#define AFD_HWSTATS_INFO_SIZE               (128)
#define AFD_MAX_G_CHANNEL_STATS_INPUT_SIZE  (128)
#define AFD_MAX_POINTS_REGION               (64)
#define AFD_DEFAULT_POINTS_1ST_REGION       (16)
#define AFD_DEFAULT_POINTS_2ND_REGION       (8)

#define AFD_STATS_QUEUE_SIZE                (5)
#define AFD_STABLE_QUEUE_SIZE               (16)
#define AFD_DFT_DATA_LENGTH                 (2 * AFD_MAX_POINTS_REGION + 1)       // 4 = DFT(2) * DELTA(2)
#define AFD_HEADER_STRING                   "AFD TuningBin"
#define AFD_HEADER_STRING_LENGTH            (32)

#define AFD_EXP_TIME_DIFF_THD               (500)
#define AFD_BASIC_EXP_TIME_50HZ             (10000)
#define AFD_BASIC_EXP_TIME_60HZ             (8333)

#define ABS(a)                              ((a) > 0 ? (a) : (-(a)))
#define MAX(a,b)                            ((a) > (b) ? (a) : (b))
#define MIN(a,b)                            ((a) > (b) ? (b) : (a))

typedef enum
{
    AFD_DETECT_STATUS_NONE = 0,
    AFD_DETECT_STATUS_OK,
    AFD_DETECT_STATUS_STATS_NOT_STABLE,
    AFD_DETECT_STATUS_PEAK_NOT_ENOUGH,
    AFD_DETECT_STATUS_PEAK_DC_OVER_TH,
    AFD_DETECT_STATUS_PEAK_2ND_OVER_TH,
    AFD_DETECT_STATUS_PEAK_OUT_OF_RANGE,
    AFD_DETECT_STATUS_PEAK_2ND_AREA_OVER_TH,
    AFD_DETECT_STATUS_PEAK_NOT_SYMMETRIC,
    AFD_DETECT_STATUS_STABLE_TH,

    AFD_DETECT_STATUS_MAX,
}   AFD_DETECT_STATUS;

typedef enum
{
    FS_DETECT_STATUS_NONE = 0,
    FS_DETECT_STATUS_OK,
    FS_DETECT_STATUS_PEAK_NOT_ENOUGH,
    FS_DETECT_STATUS_PEAK_2ND_OVER_TH,
    FS_DETECT_STATUS_STABLE_NOT_IN_BOX,
    FS_DETECT_STATUS_STABLE_CNT_NOT_ENOUGH,

    FS_DETECT_STATUS_MAX,
}   FS_DETECT_STATUS;

#pragma pack(push) //push current alignment setting to stack
#pragma pack(1)    //new alignment setting
typedef struct {
    char                                    tag[AFD_HEADER_STRING_LENGTH];
    char                                    user_name[AFD_HEADER_STRING_LENGTH];
    char                                    module_name[AFD_HEADER_STRING_LENGTH];
    UINT16                                  struct_version;
    UINT16                                  date_y;
    UINT16                                  date_m;
    UINT16                                  date_d;
    UINT32                                  lib_version_major;
    UINT32                                  lib_version_minor;
    FLOAT32                                 tuning_version;
    char                                    reserved[10];
    UINT16                                  check_sum;
}   AFD_TUNING_HEADER;
#pragma pack(pop)  //restore old alignment setting from stack

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(1)    /* new alignment setting */
typedef struct
{
    AFD_TUNING_HEADER                       header;

    UINT32                                  line_time_ns;
    UINT16                                  sensor_width;
    UINT16                                  sensor_height;
    UINT16                                  a3engine_width;                 // 0 if don't know
    UINT16                                  a3engine_height;                // 0 if don't know
    UINT16                                  woi_x;
    UINT16                                  woi_y;
    UINT16                                  woi_w;
    UINT16                                  woi_h;
    
    UINT16                                  default_hz;                     // 1 : 50hz; 2 : 60hz
    UINT16                                  level_50;                       // 0 ~ 2, strong to weak
    UINT16                                  level_60;                       // 0 ~ 2, strong to weak
    UINT16                                  level_stable_th;                // 0 ~ 2, strong to weak
    UINT16                                  base;                           // 8192
    UINT16                                  ratio_shift;                    // 10

    // stable
    UINT16                                  th_frame_average;               // 5

    UINT16                                  ratio_exp_gain_max;             // base 1024
    UINT16                                  ratio_exp_gain_min;             // base 1024
    UINT16                                  ratio_ae_y_max;                 // base 1024
    UINT16                                  ratio_ae_y_min;                 // base 1024

    // detection
    UINT16                                  enable_2nd_region;              // 1
    UINT16                                  ratio_max_range;                // base 1024
    UINT16                                  points_1st_region;              // dft points on 1st region; up to AFD_MAX_POINTS_REGION
    UINT16                                  points_2nd_region;              // dft points on 2nd region; up to AFD_MAX_POINTS_REGION
    UINT16                                  th_amp;                         // 60
    UINT16                                  th_amp_min;                     // 36
    UINT16                                  ratio_peak_dc;                  // base 1024
    UINT16                                  ratio_peak_2nd;                 // base 1024
    UINT16                                  index_range_50_0;               // base 10, 50
    UINT16                                  index_range_50_1;               // base 10, 35
    UINT16                                  index_range_50_2;               // base 10, 20
    UINT16                                  index_range_60_0;               // base 10, 50
    UINT16                                  index_range_60_1;               // base 10, 35
    UINT16                                  index_range_60_2;               // base 10, 20
    UINT16                                  th_symm_min;                    // 4
    UINT16                                  size_symmetric;                 // 3, max to 4
    UINT16                                  ratio_symmetric[4];             // base 1024, ratio * ratio * 1024
    UINT16                                  th_symm_50hz;                   // 24
    UINT16                                  th_symm_60hz;                   // 24
    UINT16                                  peak_decay_count;               // 2
    UINT16                                  ratio_low_amp;                  // base 1024

    // judgement
    UINT16                                  stable_th_0;                    // 1 ~ 16
    UINT16                                  stable_th_1;                    // 1 ~ 16
    UINT16                                  stable_th_2;                    // 1 ~ 16

    //jump sample info
    UINT16                                  jump_sample_num;                //suggest 16
    UINT16                                  jump_sample_do_avg_count;       //suggest 8

    //Because of sample rate, 50Hz/60Hz flicker pos maybe different.
    UINT16                                  _50Hz_flicker_pos;
    UINT16                                  _60Hz_flicker_pos;
    UINT8                                   enable_close_after_change_mode;
    UINT8                                   enable_debug_log;
    UINT16                                  frame_interval_50;              // 1 ~ 4
    UINT16                                  frame_interval_60;              // 1 ~ 4

    //flicker sensor result judgement
    UINT16                                  fs_th_dc_amp;
    UINT32                                  amp_2nd_peak_ratio;
    UINT16                                  freq_box[7];
    UINT16                                  vote_delta_th;
    UINT8                                   fs_th_stable;
    UINT8                                   fs_enable_close_after_change_mode;

    char                                    reserved[274];
}   AFD_TUNING_DATA;
#pragma pack(pop)  /* restore old alignment setting from stack */

typedef struct
{
    UINT32                                  hw_index;
    UINT32                                  frame_average;
    UINT32                                  avg_info[AFD_MAX_G_CHANNEL_STATS_INPUT_SIZE];
}   AFD_STATS_DATA;

typedef struct
{
    BOOL                                    contain_stats_data;
    struct afd_exposure_info_t              info;
    AFD_STATS_DATA                          stats_data;
}   AFD_STATS_QUEUE_DATA;

typedef struct
{
    enum afd_mode                           target_hz;

    UINT16                                  range_min_50hz;
    UINT16                                  range_max_50hz;
    UINT16                                  range_sym_50hz;

    UINT16                                  range_min_60hz;
    UINT16                                  range_max_60hz;
    UINT16                                  range_sym_60hz;

    UINT16                                  count_1st;
    UINT16                                  delta_1st;
    UINT16                                  points_1st[AFD_MAX_POINTS_REGION];
    UINT16                                  count_2nd;
    UINT16                                  delta_2nd;
    UINT16                                  points_2nd[AFD_MAX_POINTS_REGION];

    UINT16                                  th_amp;
    UINT16                                  th_stable;
	FLOAT32                                 unit;
}   AFD_CONDITIONS;

typedef struct FLICKER_JUDGE_REPORT
{
    AFD_DETECT_STATUS                       detect_status;
    enum afd_mode                           detect_hz;
}   AFD_STABLE_DATA;

typedef struct FS_JUDGE_REPORT
{
    FS_DETECT_STATUS                       detect_status;
    enum afd_mode                           detect_hz;
}   FS_STABLE_DATA;

typedef struct
{
    BOOL                                    is_banding;
    AFD_STABLE_DATA                         result;

    int                                     data_diff[AFD_MAX_G_CHANNEL_STATS_INPUT_SIZE];
    FLOAT32                                 data_diff_normalize[AFD_MAX_G_CHANNEL_STATS_INPUT_SIZE];

    UINT32                                  amp_max;
    UINT32                                  data_fft[AFD_DFT_DATA_LENGTH];

    FLOAT32                                 unit;
}   AFD_WORKING_BUFFER;

typedef struct
{
    BOOL                                    is_banding;
    FS_STABLE_DATA                          result;
}   FS_WORKING_BUFFER;

typedef struct
{
    UINT32    freq;
    UINT32    amp;
} FREQ;

typedef struct
{
    FREQ    freq1;
    FREQ    freq2;
    FREQ    freq3;
} AFD_FSRESULT;

typedef struct
{
    struct allib_afd_lib_version_t          version;
    struct allib_afd_output_data_t          output_report;
    AFD_TUNING_DATA                         tuning_data;
    UINT32                                  stats_size;
    AFD_FSRESULT                            fs_result;

    // set
    struct afd_setting_t                    afd_setting;
    UINT8                                   flag_sensor_info;
    struct afd_sensor_info_t                sensor_info;
    UINT8                                   flag_flicker_sensor_info;
    struct afd_flicker_sensor_info_t        flicker_sensor_info;
    UINT8                                   flag_detection;
    struct afd_parameter_detect_t           detection;

    // process
    UINT8                                   flag_update_range;
    struct afd_exposure_info_t              exposure_info;

    AFD_STATS_DATA                          stats_data;
    UINT16                                  row_count;
    UINT16                                  input_data_num;

    // stats queue
    UINT8                                   status_queue_index_0;
    UINT8                                   status_queue_index_1;
    AFD_STATS_QUEUE_DATA                    queue_stats[AFD_STATS_QUEUE_SIZE];

    AFD_CONDITIONS                          conditions;
    AFD_WORKING_BUFFER                      working_buffer;
    FS_WORKING_BUFFER                       fs_working_buffer;

    // stable queue
    UINT8                                   stable_queue_index;
    UINT8                                   fs_stable_queue_index;
    UINT16                                  th_stable;
    AFD_STABLE_DATA                         queue_stable[AFD_STABLE_QUEUE_SIZE];
    FS_STABLE_DATA                          fs_queue_stable[AFD_STABLE_QUEUE_SIZE];
    AFD_STABLE_DATA                         stable_result;
    FS_STABLE_DATA                          fs_stable_result;

    UINT16                                  stable_50hz_cnt;
    UINT16                                  stable_60hz_cnt;
	
}   AFD_RUNTIME_DATA;

//=======

void    allib_afd_add_header(void* ptr, float version);
void    allib_afd_initial_tuning_data(void *ptr);
UINT16  allib_afd_header_check_sum(void *data, int size_header, int size_tuning_data);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // !_AFD_KERNEL_