#ifndef ALTEK_LSC_LIB_ERRORCODE_
#define ALTEK_LSC_LIB_ERRORCODE_

enum altek_LSC_error_code {
    _AL_LSCLIB_SUCCESS = (0x0000),

    _AL_LSCLIB_INVALID_INPUT = (0x0100),

    _AL_LSCLIB_INVALID_PARAM = (0x0200),
    _AL_LSCLIB_INVALID_TUNING_FILE_NULL,
    _AL_LSCLIB_INVALID_TUNING_FILE_VERSION,
    _AL_LSCLIB_SCENARIO_CASE_NOT_MATCH_TUNING_FILE,
    //_AL_LSCLIB_INVALID_TUNING_FILE_CHECKSUM,

    _AL_LSCLIB_INIT = (0x0300),
    _AL_LSCLIB_INIT_NULL_OBJ,
    _AL_LSCLIB_INIT_BUFFER_NULL,

    _AL_LSCLIB_SET_PARAM = (0x0400),
    _AL_LSCLIB_SET_PARAM_INPUT_NULL,
    _AL_LSCLIB_SET_PARAM_INPUT_TYPE,

    _AL_LSCLIB_GET_PARAM = (0x0500),
    _AL_LSCLIB_GET_PARAM_INPUT_NULL,
    _AL_LSCLIB_GET_PARAM_INPUT_TYPE,

    _AL_LSCLIB_CALC_DATAULT_TABLE = (0x0600),
    _AL_LSCLIB_CALC_DATAULT_TABLE_INPUT_NULL,
    _AL_LSCLIB_CALC_DATAULT_TABLE_USE_WRONG_COLOR_ORDER,

    _AL_LSCLIB_DYNAMIC_PROCESS = (0x0700),
    _AL_LSCLIB_DYNAMIC_PROCESS_INPUT_NULL,
    _AL_LSCLIB_DYNAMIC_HW3A_STATS_DATA_NULL,
    _AL_LSCLIB_DYNAMIC_PING_PONG_BUFFER_NULL,

    _AL_LSCLIB_DEINIT = (0x0800),
    _AL_LSCLIB_DEINIT_NULL_OBJ,
};

#endif