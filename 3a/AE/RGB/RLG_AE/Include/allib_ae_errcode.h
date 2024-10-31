/****************************************************************************
*                                                                           *
*       This software and proprietary is confidential and may be used       *
*        only as expressly authorized by a licensing agreement from         *
*                                                                           *
*                            Altek Semiconductor                            *
*                                                                           *
*                   (C) COPYRIGHT 2022 ALTEK SEMICONDUCTOR                  *
*                            ALL RIGHTS RESERVED                            *
*                                                                           *
*                 The entire notice above must be reproduced                *
*                  on all copies and should not be removed.                 *
*                                                                           *
*****************************************************************************/
/**
@file allib_ae_errcode.h
@brief Error code of AE lib.
@author Rachel Lin
@date 2022/04/27
*/

#ifndef _ALAELIB_ERR_H_
#define _ALAELIB_ERR_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define _AL_3ALIB_SUCCESS                           (0x00)
#define _AL_3ALIB_SIMULATE                          (0x01)

#define _AL_AELIB_GLOBAL_ERR_OFFSET                 (0xA000)
#define _AL_AELIB_WRP_AE                            (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x100)
#define _AL_AELIB_SETTINGFILE                       (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x200)
#define _AL_AELIB_LIBPROCESS                        (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x300)
#define _AL_AELIB_SETPARAM                          (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x400)

#define _AL_AELIB_INIT_BUFFER_FAIL                  (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x01)
#define _AL_AELIB_INVALID_ADDR                      (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x02)
#define _AL_AELIB_FAIL_INIT_AE_SET                  (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x03)
#define _AL_AELIB_INVALID_PARAM                     (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x04)
#define _AL_AELIB_MISMATCH_STATS_SIZE               (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x05)
#define _AL_AELIB_INVALID_CALIB_GAIN                (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x06)
#define _AL_AELIB_INVALID_ISOLEVEL                  (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x07)
#define _AL_AELIB_INCONSIST_SETDAT                  (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x08)
#define _AL_AELIB_SOF_FRAME_INDEX_MISMATCH          (_AL_AELIB_GLOBAL_ERR_OFFSET + 0x09)

#define _AL_WRP_AE_EMPTY_METADATA                   (_AL_AELIB_WRP_AE + 0x01)
#define _AL_WRP_AE_HW_FRAME_INDEX_MISMATCH          (_AL_AELIB_WRP_AE + 0x02)
#define _AL_WRP_AE_STATS_SIZE_EXCEED                (_AL_AELIB_WRP_AE + 0x03)
#define _AL_WRP_AE_INVALID_HW_ENGINE_ID             (_AL_AELIB_WRP_AE + 0x04)
#define _AL_WRP_AE_INVALID_INPUT_PARAM              (_AL_AELIB_WRP_AE + 0x05)
#define _AL_WRP_AE_INVALID_BLOCKS                   (_AL_AELIB_WRP_AE + 0x06)
#define _AL_WRP_AE_INVALID_INPUT_WB                 (_AL_AELIB_WRP_AE + 0x07)
#define _AL_WRP_AE_INVALID_PIXEL_PER_BLOCKS         (_AL_AELIB_WRP_AE + 0x08)
#define _AL_WRP_AE_INVALID_STATS_ADDR               (_AL_AELIB_WRP_AE + 0x09)
#define _AL_WRP_AE_INVALID_HW_POS_CFG               (_AL_AELIB_WRP_AE + 0x0A)

/* for setting file */
#define _AL_AELIB_INVALID_TARGETMEAN                (_AL_AELIB_SETTINGFILE + 0x01)
#define _AL_AELIB_INVALID_INIT_PARAM                (_AL_AELIB_SETTINGFILE + 0x02)
#define _AL_AELIB_INVALID_FLICKER_PRIORITY_MODE     (_AL_AELIB_SETTINGFILE + 0x03)
#define _AL_AELIB_INVALID_BIN_FILE                  (_AL_AELIB_SETTINGFILE + 0x04)
#define _AL_AELIB_INVALID_BLOCK_NUM                 (_AL_AELIB_SETTINGFILE + 0x05)
#define _AL_AELIB_INVALID_TARGETMEAN_ROIAE          (_AL_AELIB_SETTINGFILE + 0x06)
#define _AL_AELIB_INVALID_IR_CTRL_CFG				(_AL_AELIB_SETTINGFILE + 0x07)
#define _AL_AELIB_INVALID_WDR_ENABLE                (_AL_AELIB_SETTINGFILE + 0x08)
#define _AL_AELIB_INVALID_PRIVACY_ENABLE			(_AL_AELIB_SETTINGFILE + 0x09)
#define _AL_AELIB_INVALID_SMOOTH_QUEUE_NUM			(_AL_AELIB_SETTINGFILE + 0x0A)
#define _AL_AELIB_INVALID_STABLE_LOCK_CTRL			(_AL_AELIB_SETTINGFILE + 0x0B)
#define _AL_AELIB_INVALID_STEADY_PARA				(_AL_AELIB_SETTINGFILE + 0x0C)
#define _AL_AELIB_INVALID_ROICOMP_PARA				(_AL_AELIB_SETTINGFILE + 0x0D)

/* for Lib processing */
#define _AL_AELIB_ERR_PROCESS_BLOCKS                (_AL_AELIB_LIBPROCESS + 0x01)
#define _AL_AELIB_INVALID_PCURVE_BUILD              (_AL_AELIB_LIBPROCESS + 0x02)

/* for set-param */
#define _AL_AELIB_INVALID_FPS                       (_AL_AELIB_SETPARAM + 0x01)
#define _AL_AELIB_INVALID_GAIN                      (_AL_AELIB_SETPARAM + 0x02)
#define _AL_AELIB_INVALID_FNUMBER                   (_AL_AELIB_SETPARAM + 0x03)
#define _AL_AELIB_INVALID_LINE                      (_AL_AELIB_SETPARAM + 0x04)
#define _AL_AELIB_INVALID_EV_COMPENSATION           (_AL_AELIB_SETPARAM + 0x05)
#define _AL_AELIB_INVALID_CONVERGE_SPEED_LEVEL      (_AL_AELIB_SETPARAM + 0x06)
#define _AL_AELIB_INVALID_EXP_TIME                  (_AL_AELIB_SETPARAM + 0x07)
#define _AL_AELIB_INVALID_CURRENT_IR_PARAM          (_AL_AELIB_SETPARAM + 0x08)
#define _AL_AELIB_INVALID_FLICKER_MODE              (_AL_AELIB_SETPARAM + 0x09)
#define _AL_AELIB_INVALID_MIDTONES                  (_AL_AELIB_SETPARAM + 0x0A)


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _ALAELIB_ERR_H_ */
