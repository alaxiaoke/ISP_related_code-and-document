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

#ifndef ALTEK_AWB_LIB_ERRORCODE_
#define ALTEK_AWB_LIB_ERRORCODE_

enum altek_awb_error_code {
    _AL_AWBLIB_SUCCESS               = (0x0000),

	_AL_AWBLIB_GLOBAL_ERR_OFFSET     = (0xB000),

    _AL_AWBLIB_INVALID_INPUT   = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0100)),
    _AL_AWBLIB_INVALID_TABLE_SIZE,
    _AL_AWBLIB_INVALID_CALIB_OTP,
	_AL_AWB_ERROR_INVALID_STABLE_SIZE,
	_AL_AWB_ERROR_INVALID_STABLE_TYPE,

    _AL_AWBLIB_INVALID_PARAM   = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0200)),
    _AL_AWBLIB_INVALID_TUNING_FILE_VERSION,
    _AL_AWBLIB_INVALID_TUNING_FILE_DATA,
    _AL_AWBLIB_INVALID_TUNING_FILE_CHECKSUM,

    _AL_AWBLIB_INIT            = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0300)),
    _AL_AWBLIB_INIT_NULL_OBJ,
    _AL_AWBLIB_INIT_BUFFER_NULL,
    _AL_AWBLIB_INIT_TUNING_FILE_NULL,
    _AL_AWBLIB_INIT_HANDLER_OVER_SIZE,
    _AL_AWBLIB_INIT_PRIVATE_MEM_NOTMATCH,
    _AL_AWBLIB_INIT_MEM_OVERLAP,
    _AL_AWBLIB_INIT_MEM_FORMAT_WRONG,

    _AL_AWBLIB_FLOW            = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0400)),
    _AL_AWBLIB_FLOW_HANDLER_WRONG,

    _AL_AWBLIB_PROCESS      = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0500)),
    _AL_AWBLIB_PROCESS_MEM_NULL,
    _AL_AWBLIB_PROCESS_STATS_FORMAT,
    _AL_AWBLIB_PROCESS_STATS_SIZE,
    _AL_AWBLIB_PROCESS_STATS_NULL,

    _AL_AWBLIB_SET_PARAM       = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0600)),
    _AL_AWBLIB_SET_PARAM_INPUT_NULL,
    _AL_AWBLIB_SET_PARAM_INPUT_TYPE,
    _AL_AWBLIB_SET_PARAM_LOCL_CT_LIMIT,

    _AL_AWBLIB_GET_PARAM       = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0700)),
    _AL_AWBLIB_GET_PARAM_INPUT_NULL,
    _AL_AWBLIB_GET_PARAM_INPUT_TYPE,

    _AL_AWBLIB_DEINIT          = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0800)),
    _AL_AWBLIB_DEINIT_NULL_OBJ,
};

#endif
