#pragma once

#define AWB_STATS_MAX_BLOCK_X           64
#define AWB_STATS_MAX_BLOCK_Y           48
#define AE_STATS_MAX_BLOCK_X            36
#define AE_STATS_MAX_BLOCK_Y            24
#define AE_HIST_MAX_BIN                 256
#define ATF_STATS_MAX_LINE_SENSOR       3000
#define ATF_STATS_MAX_LINE_AFTER_GROUP  1736

#define AWB_STATS_MAX_BLOCK_NUM    AWB_STATS_MAX_BLOCK_X*AWB_STATS_MAX_BLOCK_Y
#define AE_STATS_MAX_BLOCK_NUM    AE_STATS_MAX_BLOCK_X*AE_STATS_MAX_BLOCK_Y


typedef struct IspSoftIspAtfStatusOut
{
    uint32_t         sensor_shutter_us;
    uint32_t         sensor_adgain;
    uint32_t         stats_woi_height;
    uint32_t         stats_woi_width;
    uint16_t         atf_stats_line;
    uint16_t         group_line[ATF_STATS_MAX_LINE_SENSOR];  // 对应统计的group前行数
    uint32_t         atf_stats[ATF_STATS_MAX_LINE_AFTER_GROUP];  // max size group 后总行数
} IspSoftIspAtfStatusOut;


typedef struct IspSoftIspAwbStatusOut
{
    uint32_t         stats_woi_height;
    uint32_t         stats_woi_width;
    uint16_t         awb_stats_blockY;  //awb
    uint16_t         awb_stats_blockX;
    uint32_t         awb_stats[5 * AWB_STATS_MAX_BLOCK_NUM];  // max size 5*64*48
} IspSoftIspAwbStatusOut;


typedef struct IspSoftIspAeStatusOut
{
    uint32_t         sensor_shutter_us;
    uint32_t         sensor_adgain;
    uint32_t         stats_woi_height;
    uint32_t         stats_woi_width;
    uint16_t         ae_stats_blockY;  //ae
    uint16_t         ae_stats_blockX;
    uint32_t         ae_stats[8 * AE_STATS_MAX_BLOCK_NUM];  // max size 8*36*24
    uint32_t         ae_hist[AE_HIST_MAX_BIN];  // max size
} IspSoftIspAeStatusOut;


typedef struct IspSoftAeOut
{
    // out
    uint32_t    ae_shutter;      //us
    uint32_t    ae_adgain;     //scale 100
    uint32_t    ae_ispgain;    //scale 100 out for isp
    uint32_t    ae_lux_result;   //scale 1000
    uint16_t    Ymean;        //scale 100
    int16_t     ae_converged;  //converged tag
    int32_t     nextbv;     // after weight and smooth 
    int32_t     bgbv;       // avg bv to, awb
    uint32_t    iso;
    uint16_t    midtones_gain;
    uint16_t    shadows_gain;
} IspSoftAeOut;


typedef struct IspSoftAwbOut
{
    unsigned short                      awb_update;           /* valid awb to update or not */
    unsigned short                      rg_allbalance;        /* Report for capture */
    unsigned short                      bg_allbalance;       /* Report for capture */
    unsigned short                      r_gain;              /* scale 256 */
    unsigned short                      g_gain;              /* scale 256 */
    unsigned short                      b_gain;              /* scale 256 */
    unsigned short                      color_temp;         /* (major) color temperature */
    float                               final_CCM[9];
    float                               final_lsc[13 * 17 * 4];      /*LSC table rggb */
} IspSoftAwbOut;


enum atf_mode {
    ATF_MODE_OFF = 0,
    ATF_MODE_50HZ = 1,
    ATF_MODE_60HZ = 2,
    ATF_MODE_90HZ = 3,
    ATF_MODE_144HZ = 4,
    ATF_MODE_240HZ = 5,
    ATF_MODE_320HZ = 6,
    ATF_MODE_360HZ = 7,
    ATF_MODE_MAX,
};


typedef struct IspSoftAtfOut
{
    // out
    uint8_t                                     flag_enable;
    uint8_t                                     flag_bending;
    enum atf_mode                               atf_mode;
    enum atf_mode                               atf_mode_stable;
} IspSoftAtfOut;


