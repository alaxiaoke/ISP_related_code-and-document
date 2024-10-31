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
*   \file       AEDebugLog.h
*   \brief      Header of Debug Log
*   \author     Rachel Lin
*   \date       2022/04/27
*/
#ifndef _AEDEBUGAELOG_H_
#define _AEDEBUGAELOG_H_




#ifdef __cplusplus
extern "C"
{
#endif
//#include "stdio.h"
#ifdef _AL_AE_C_MODEL_
#include <stdio.h>
#endif

#define ENABLE_AE_DEBUGLOG      (0)

#if(ENABLE_AE_DEBUGLOG)
    //#define _COMMONLOG_
    //#define _ANDROID_LOG_
    //#define _LIB_OUTPUT_DEBUG_LOG_TXT_

    #define LOG_TAG "alAELib"
    #ifdef _COMMONLOG_
        #include <CommonFunc.h>
        #define alAE_Log(arg ...)       COMMON_LevelLogMsg(LOG_LEVEL_IQ_INFO, LOG_TAG, arg)
    #elif defined _ANDROID_LOG_
        #include <android/log.h>
        #define alAE_Log(...)           __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)  // LOGD, debug log
    #elif defined _BUILD_RELEASE_AE_LIB_DLL_
        #define alAE_Log(...)           alAELib_Msg2File(LOG_TAG", " __VA_ARGS__)
    #elif defined _LIB_OUTPUT_DEBUG_LOG_TXT_  //Please define _AL_AE_C_MODEL_ first (Search: _AL_AE_C_MODEL_)
        #define alAE_Log(...)           alAELib_Msg2File(LOG_TAG", " __VA_ARGS__)
    #elif defined _AL_AE_C_MODEL_
        #define alAE_Log(...)           printf(LOG_TAG", " __VA_ARGS__)
    #else
        #define alAE_Log(...)           //printf(LOG_TAG", " __VA_ARGS__)//AL_Log_SaveMsg(1, LOG_TAG, __VA_ARGS__)
    #endif

#else
    #define alAE_Log(...)

#endif  //#if(ENABLE_AE_DEBUGLOG)


/**
@enum ae_debug_log_level
@brief Definition of debug log level
*/
enum ae_debug_log_level {
    AE_DEBUG_LOG_LEVEL_DISABLE              =   0,
    AE_DEBUG_LOG_LEVEL_LV1                  =   1,              // Available for customer
    AE_DEBUG_LOG_LEVEL_LV2                  =   1 << 1,         // Engineer only
    AE_DEBUG_LOG_LEVEL_LV3                  =   1 << 2,         // Engineer only
    AE_DEBUG_LOG_LEVEL_AE_STATS             =   1 << 3,         // Engineer only
	AE_DEBUG_LOG_LEVEL_AE_STATS_Y_10BIT     =   1 << 4,         // Available for customer
    AE_DEBUG_LOG_LEVEL_BLOCK_Y_MEAN_8BIT    =   1 << 5,         // Available for customer
    AE_DEBUG_LOG_LEVEL_SETTING_FILE         =   1 << 6,         // Engineer only
    AE_DEBUG_LOG_LEVEL_METERING_TABLE       =   1 << 7,         // Engineer only
	AE_DEBUG_LOG_LEVEL_YWEIGHT_TABLE		=	1 << 8,         // Engineer only
	AE_DEBUG_LOG_LEVEL_DISPATCH_HW3A        =   1 << 9,         // Engineer only
    AE_DEBUG_LOG_LEVEL_MINOR_LOG            =   1 << 10,        // Engineer only
};


#ifdef __cplusplus
}  // extern "C"
#endif

#endif  //_AEDEBUGAELOG_H_
