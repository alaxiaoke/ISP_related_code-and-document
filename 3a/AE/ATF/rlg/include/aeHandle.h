#ifndef _AE_HANDLE_H_
#define _AE_HANDLE_H_

#ifdef _AE_HANDLE_C_
#define GXR_AE _declspec( dllexport )
#else
#define GXR_AE _declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum LogLevel
{
    Log_OFF = 0,

    Trace = 1 << 0,
    Debug = 1 << 1,
    Info = 1 << 2,
    Warn = 1 << 3,
    Error = 1 << 4,
    Fatal = 1 << 5,

    Log_End
};

struct gxr_AE_exif_info_t {
    //*current expo param
    uint32_t  cur_exp_time;   /*!< unit:us, for initial, exposure time is only for reference, exposure line would be more precise */
    uint16_t  cur_ad_gain;    /*!< scale 100, if 1.0x, set 100 */

    //*cur process exif
    uint16_t  target;
    int32_t   bvY_8bit;       /*!< Luma value for current brightness (not update when AE disabled)*/
    int32_t   bgYmean_8bit;    /*!< Luma value from average mean (not update when AE disabled), for awb ae_non_comp_bv_val */
    uint32_t  lux_result;         /*!< Lux value from average mean (not update when AE disabled)*/
    int32_t   next_bv;

    //*out
    uint32_t  next_exp_time;      /*!< unit:us, for initial, exposure time is only for reference, exposure line would be more precise */
    uint16_t  next_ad_gain;       /*!< scale 100, if 1.0x, set 100 */
    uint16_t  midtones_gain;      /*!< scale 100, if 1.0x, set 100 */
    int16_t   ae_converged;       /*!< 0:Not converged, 1:Converged */
};

typedef struct gxr_aehd {
    void*   al4a;
    void*   ae_stats;
    void*   yhis_stats;
    void*   tuning_file;
    void*   debug_file;

    uint8_t     cameraId;
    //enable 
    uint8_t     ae_enable;   // 0 or 1
    uint32_t    frameId;
    // calib param OTP
    uint32_t    min_iso;         /*!< minimum ISO */
    uint32_t    calib_r_gain;    /*!< scale 1000 */
    uint32_t    calib_g_gain;    /*!< scale 1000 */
    uint32_t    calib_b_gain;    /*!< scale 1000 */
    // stats info get from ini at init
    int8_t      statsBankNum;      /*!< max 24 */
    int8_t      statsBolckNum;     /*!< max 36 */
    //atf mode
    int         atf_mode;   //0: off, 1: 50HZ; 2: 60HZ
    // sensor info 
    float       shutter;    //input
    float       sensorgain;

    // out
    float       ae_shutter;      //us
    float       ae_adgain;     //scale 100
    float       ae_ispgain;    //scale 1  out for isp
    int         ae_lux_result;   //scale 1000
    float       Ymean;        //scale 100
    int16_t     ae_converged;  //converged tag
    int         nextbv;     // after weight and smooth 
    int         bgbv;       // avg bv to, awb
    uint32_t    iso;
    uint16_t    midtones_gain;
    uint16_t    shadows_gain;
    
    int32_t     err_code;

    //debug print
    struct gxr_AE_exif_info_t       exif_info;
    enum LogLevel                   debug_level;  //default off
    bool                            save_to_file;  //default false
    //tuning tag
    bool        tuning_enable;
}gxr_aehd;

GXR_AE unsigned int ae_initial(gxr_aehd* gxr_ae);
GXR_AE unsigned int ae_deinit(gxr_aehd* gxr_ae);
GXR_AE unsigned int ae_run(gxr_aehd* gxr_ae);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // !_AE_HANDLE_H_
