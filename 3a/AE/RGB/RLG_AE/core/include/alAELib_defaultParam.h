/****************************************************************************
*                                                                           *
*       This software and proprietary is confidential and may be used       *
*        only as expressly authorized by a licensing agreement from         *
*                                                                           *
*                            Altek Semiconductor                            *
*                                                                           *
*                   (C) COPYRIGHT 2018 ALTEK SEMICONDUCTOR                  *
*                            ALL RIGHTS RESERVED                            *
*                                                                           *
*                 The entire notice above must be reproduced                *
*                  on all copies and should not be removed.                 *
*                                                                           *
*****************************************************************************/
/**
*   \file       alAELib_defaultParam.h
*   \brief      Default values of AE Library.
*   \author     Hubert Huang
*   \date       2018/04/19
*/
#ifndef _AL_AE_DEFAULT_PARA_H_
#define _AL_AE_DEFAULT_PARA_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* AE Setting */

// Default Initial Setting when device turn on
#define INIT_AE_ISO                                 (100)
#define INIT_AE_ADGAIN                              (100)       // 100 base, should refer to base units
#define INIT_AE_BV                                  (5270)      // scale by 1000, means 5.0 BV, equal to LV 10
#define INIT_AE_EXPOSURE                            (3900)      // units: us, 1.0s = 1000000, current means 3.9 ms
#define INIT_AE_MIDTONES_GAIN                       (100)       // 100 base, should refer to base units
#define INIT_AE_EXPINDEX                            (219.12)

// AE exposure setting
#define AE_TARGET_MEAN                              (3825)      // scale 100 of 8 bits domain
#define AE_FLICKER_PRIORITY                         (0)         // Set 1 means higher priority avoiding flicker when manual adgain, otherwise set 0.
#define AE_FLICKER_MODE                             (3) 	    // sync with allib_ae.h antiflicker mode :  0: off, 1: fix 50Hz, 2: fix 60Hz, 3: AUTO(AFD)
#define _DEFAULT_AE_CONVERGE_TH                     (300)       // BV, scale 1000
#define _DEFAULT_AE_50HZ_TIME_US                    (10000)     // unit: us
#define _DEFAULT_AE_60HZ_TIME_US                    (8333)      // unit: us
#define _DEFAULT_TIME_PER_LINE						(20.0)      // unit: us
#define DEFAULT_MAX_MANUAL_EV_COMP                  (2000)      // scale by 1000, means 2.0 EV
#define DEFAULT_Min_MANUAL_EV_COMP                  (-2000)     // scale by 1000, means -2.0 EV
#define DEFAULT_MIN_FPS                             (1500)      // scale by 100, means 15 fps
#define DEFAULT_MAX_FPS                             (3000)      // scale by 100, means 30 fps
#define DEFAULT_MIN_LINE                            (1)         // min exposure line count
#define DEFAULT_MAX_LINE                            (65535)     // max exposure line count
#define DEFAULT_MIN_EXP_TIME                        (100)       // unit: us
#define DEFAULT_MAX_EXP_TIME                        (11111)     // unit: us
#define DEFAULT_MIN_GAIN                            (100)       // scale by 100, means 1.0x
#define DEFAULT_MAX_GAIN                            (6400)      // scale by 100, means 64.0x
#define DEFAULT_FN                                  (2200)      // scale by 1000, means 2.0 F-number

// Default calibration data
#define _DEFAULT_AE_CALIB_MIN_ISO                   (100)
#define _DEFAULT_AE_CALIB_R_GAIN                    (1500)      // 1000 base
#define _DEFAULT_AE_CALIB_G_GAIN                    (1000)      // 1000 base
#define _DEFAULT_AE_CALIB_B_GAIN                    (1500)      // 1000 base

// AE converge speed
#define AE_SPEED_MODE                               (AE_CONVERGE_SMOOTH)    //sync with ae_converge_level_type_t: 0:normal, 1:direct, 2:fast, 3:smooth, 4:slow
//#define AE_SPEED_MODE                               (AE_CONVERGE_SLOW)    //sync with ae_converge_level_type_t: 0:normal, 1:direct, 2:fast, 3:smooth, 4:slow
#define AE_SPEED_FBASED_SPEED                       (0.00001)
#define AE_SPEED_FMAX_STEP                          (0.9)
#define AE_SPEED_NORMAL                             (40)
#define AE_SPEED_DIRECT                             (100)
#define AE_SPEED_SLOW                               (5)
#define AE_SPEED_SMOOTH                             (20)
#define AE_SPEED_FAST                               (70)

// AE wdr control setting
#define AE_WDR_ENABLE                               (1)             // enable wdr or not, 0: disable, 1: enable
#define AE_WDR_EXPINDEX_TH                          {280, 300, 310, 315, 332, 332, 332, 332, 332, 332, 332, 332, 332, 332, 332, 332, 332, 332}
#define AE_WDR_HIST_RATIO                           {116, 2, 4, 30, 2, 50}

#define AE_WDR_MAX_MIDTONES_GAIN                    {500, 300, 100, 100, 100, 100, 100, 100, 100, 100} // scale by 100, valid range: 100~800, means 1.0x/3.0x/5.0x for low/middle/high region
#define AE_WDR_MAX_SHADOWS_GAIN                     {200, 200, 100, 100, 100, 100, 100, 100, 100, 100} // scale by 100, valid range: 100~400, means 1.0x/2.0x/2.0x for low/middle/high region
#define AE_WDR_HIST_HIGHLIGHT_TARGET                {200, 200, 200, 200, 200, 200, 200, 200, 200, 200} // 8bit, for low/middle/high region
#define AE_WDR_HIST_SHADOWS_TARGET                  {  4,  4,  4, 4,  4,  4, 4,  4,  4, 4}   // 8bit, for low/middle/high region
#define AE_WDR_HIST_HIGHLIGHT_PERCENT_RANGE         {  3,  3,  3, 3,  3,  3, 3,  3,  3, 3}   // scale by 100, means 3% for low/middle/high region
#define AE_WDR_HIST_SHADOWS_PERCENT_RANGE           { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10}   // scale by 100, means 10% for low/middle/high region

// AE TARGET & EXPO setting
#define AE_TARGET_MODE                              (AE_TARGET_NORMAL)  //sync with ae_target_mode_type_t: 0: fix, 1: normal mdoe, 2: night mode, 3: user define
#define AE_EXPO_MODE                                (AE_EXPO_NORMAL)    //sync with ae_expo_mode_type_t: 0: time fix, 1: iso fix, 2: normal mdoe, 3: night mode, 4: user define

// AE metering mode setting
#define AE_METERING_MODE                            (AE_METERING_AVERAGE)   // sync with ae_metering_mode_type_t: 0: average, 1: user define
//big
#define AE_METERING_TABLE_AVERAGE_16x16 {\
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,\
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,\
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,\
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,\
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,\
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,\
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,\
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,\
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,\
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,\
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,\
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,\
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,\
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,\
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,\
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 \
}

//small
//#define AE_METERING_TABLE_AVERAGE_16x16 {\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 \
//}

#define AE_METERING_TABLE_USER_DEFINE_16x16	{\
    1, 1, 1, 1, 1, 2, 3, 4, 4, 3, 2, 1, 1, 1, 1, 1,\
    1, 1, 1, 2, 3, 3, 4, 6, 6, 4, 3, 3, 2, 1, 1, 1,\
    1, 1, 2, 3, 4, 4, 6,10,10, 6, 4, 4, 3, 2, 1, 1,\
    1, 2, 2, 4, 6, 9,11,13,13,11, 9, 6, 4, 2, 2, 1,\
    1, 2, 4, 6, 8,12,13,15,15,13,12, 8, 6, 4, 2, 1,\
    2, 3, 5, 8,10,12,14,17,17,14,12,10, 8, 5, 3, 2,\
    2, 4, 8, 9,11,13,16,20,20,16,13,11, 9, 8, 4, 2,\
    3, 6,10,10,13,17,22,25,25,22,17,13,10,10, 6, 3,\
    3, 6,10,10,13,17,22,25,25,22,17,13,10,10, 6, 3,\
    2, 4, 8, 9,11,13,16,20,20,16,13,11, 9, 8, 4, 2,\
    2, 3, 5, 8,10,12,14,17,17,14,12,10, 8, 5, 3, 2,\
    1, 2, 4, 6, 8,12,13,15,15,13,12, 8, 6, 4, 2, 1,\
    1, 2, 2, 4, 6, 9,11,13,13,11, 9, 6, 4, 2, 2, 1,\
    1, 1, 2, 3, 4, 4, 6,10,10, 6, 4, 4, 3, 2, 1, 1,\
    1, 1, 1, 2, 3, 3, 4, 6, 6, 4, 3, 3, 2, 1, 1, 1,\
    1, 1, 1, 1, 1, 2, 3, 4, 4, 3, 2, 1, 1, 1, 1, 1 \
}

#define AE_Y_WEIGHT_TABLE {\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 \
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // _AL_AE_DEFAULT_PARA_H_
