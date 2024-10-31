/****************************************************************************
*                                                                           *
*       This software and proprietary is confidential and may be used       *
*        only as expressly authorized by a licensing agreement from         *
*                                                                           *
*                            G Semiconductor                            *
*                                                                           *
*                   (C) COPYRIGHT 2021 G SEMICONDUCTOR                  *
*                            ALL RIGHTS RESERVED                            *
*                                                                           *
*                 The entire notice above must be reproduced                *
*                  on all copies and should not be removed.                 *
*                                                                           *
*****************************************************************************/

#ifndef G_AFD_LIB_ERRORCODE_
#define G_AFD_LIB_ERRORCODE_

enum G_AFD_error_code {
    _AL_AFDLIB_SUCCESS = (0x0000),

    _AL_AFDLIB_INVALID_INPUT = (0x0100),
    _AL_AFDLIB_INVALID_MWB_SIZE,
    _AL_AFDLIB_INVALID_TABLE_SIZE,
    _AL_AFDLIB_INVALID_CALIB_OTP,

    _AL_AFDLIB_INVALID_PARAM = (0x0200),
    _AL_AFDLIB_INVALID_TUNING_FILE_NULL,
    _AL_AFDLIB_INVALID_DEBUG_FILE_NULL,
    _AL_AFDLIB_INVALID_TUNING_FILE_VERSION,
    _AL_AFDLIB_INVALID_TUNING_FILE_CHECKSUM,

    _AL_AFDLIB_INIT = (0x0300),
    _AL_AFDLIB_INIT_NULL_OBJ,
    _AL_AFDLIB_INIT_BUFFER_NULL,
    _AL_AFDLIB_INIT_PRIVATE_MEM_NULL,
    _AL_AFDLIB_INIT_HANDLER_OVER_SIZE,
    _AL_AFDLIB_INIT_PRIVATE_MEM_NOTMATCH,
    _AL_AFDLIB_INIT_MEM_OVERLAP,
    _AL_AFDLIB_INIT_MEM_VERSION_WRONG,
    _AL_AFDLIB_INIT_MEM_FORMAT_WRONG,

    _AL_AFDLIB_FLOW = (0x0400),
    _AL_AFDLIB_FLOW_RUNNING_LOCK,
    _AL_AFDLIB_FLOW_HANDLER_WRONG,

    _AL_AFDLIB_PROCESS = (0x0500),
    _AL_AFDLIB_PROCESS_STATS_NULL,
    _AL_AFDLIB_PROCESS_STATS_GROUP_NULL,
    _AL_AFDLIB_PROCESS_MEM_NULL,
    _AL_AFDLIB_PROCESS_OUTPUT_NULL,
    _AL_AFDLIB_PROCESS_STATS_DISPATCH_ERROR,
    _AL_AFDLIB_PROCESS_FREQUENCY_TRANSFORM_ERROR,
    _AL_AFDLIB_PROCESS_GROUP_LINE_OVER_SENSOR_HEIGHT,
    _AL_AFDLIB_PROCESS_GROUP_LINE_OVER_STATS_HEIGHT,
    _AL_AFDLIB_PROCESS_STATS_OVER_SAT,

    _AL_AFDLIB_SET_PARAM = (0x0600),
    _AL_AFDLIB_SET_PARAM_INPUT_NULL,
    _AL_AFDLIB_SET_PARAM_INPUT_TYPE,
    _AL_AFDLIB_SET_PARAM_STATS_SIZE_EMPTY,
    _AL_AFDLIB_SET_PARAM_SENSOR_INFO_EMPTY,
    _AL_AFDLIB_SET_PARAM_SENSOR_INFO_INVALID_WOI,
    _AL_AFDLIB_SET_PARAM_DETECTION_INVALID_SETTING,
    _AL_AFDLIB_SET_PARAM_DEBUG_TYPE_NULL,

    _AL_AFDLIB_GET_PARAM = (0x0700),
    _AL_AFDLIB_GET_PARAM_INPUT_NULL,
    _AL_AFDLIB_GET_PARAM_INPUT_TYPE,

    _AL_AFDLIB_DEINIT = (0x0800),
    _AL_AFDLIB_DEINIT_NULL_OBJ,
};

#endif