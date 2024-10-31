/****************************************************************************
*                                                                           *
*       This software and proprietary is confidential and may be used       *
*        only as expressly authorized by a licensing agreement from         *
*                                                                           *
*                            Altek Semiconductor                            *
*                                                                           *
*                   (C) COPYRIGHT 2023 ALTEK SEMICONDUCTOR                  *
*                            ALL RIGHTS RESERVED                            *
*                                                                           *
*                 The entire notice above must be reproduced                *
*                  on all copies and should not be removed.                 *
*                                                                           *
*****************************************************************************/

/**
@file allib_lsc.h
@brief Definition of LSC lib. type declaration
@author Han Tseng
@date 2023/06/29
*/

#ifndef _ALLIB_LSC_H_
#define _ALLIB_LSC_H_

/**
@defgroup LSC LSC
@brief Lens Shading Compensation
@ingroup  ISPQ
*/

/**
@addtogroup LSC
@{
*/


/******************************************************************************
* Include files
*******************************************************************************/
#include "stdint.h"//"mtype.h"

/******************************************************************************
*  Macro definitions
******************************************************************************/

/********************************************************************************
*                           Type Declaration                                    *
********************************************************************************/

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct alLSClib_lib_version_t {
    uint16_t                      major_version;
    uint16_t                      minor_version;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

enum alLSClib_color_order {
    TYPE_LSC_COLOR_ORDER_RGGB = 0,
    TYPE_LSC_COLOR_ORDER_GRBG,
    TYPE_LSC_COLOR_ORDER_GBRG,
    TYPE_LSC_COLOR_ORDER_BGGR,
};

enum alLSClib_set_parameter_type {
    SET_PARAM_TYPE_LSC_TUNING_BIN = 0,
    SET_PARAM_TYPE_LSC_HW_INIT_PARA,
    SET_PARAM_TYPE_LSC_DYNAMIC_PROC_PARA,
    SET_PARAM_TYPE_LSC_COLOR_ORDER,
	SET_PARAM_TYPE_LSC_AWB_PROC_PARA,
    SET_PARAM_TYPE_LSC_MAX,
};

enum alLSClib_get_parameter_type {
    GET_PARAM_TYPE_LSC_HW_INIT_PARA,
    GET_PARAM_TYPE_LSC_DYNAMIC_PROC_PARA,
    GET_PARAM_TYPE_LSC_COLOR_ORDER,
    GET_PARAM_TYPE_DEFAULT_ISO,
	GET_PARAM_TYPE_LSC_AWB_PROC_PARA,
    GET_PARAM_TYPE_LSC_MAX,
};

/*!< Shd_Reg, rw, Unit: B, Limitation 2B alignment */
struct alLSClib_output_data_t {
    void*                           shading_table_00_address;
    void*                           shading_table_01_address;
    void*                           shading_table_10_address;
    void*                           shading_table_11_address;
};

struct alLSCLib_hw_init_para_t {
    uint8_t                         shade_bypass;
};

struct alLSCLib_dynamic_proc_para_t {
    int16_t                         current_bv;
    uint16_t                        iso_speed;
    uint16_t                        y_mean;
    uint16_t                        interpolate_ct;
};

//AWB information not use in Gravity
struct alLSCLib_awb_proc_para_t     {
	uint16_t                        Rgain;
	uint16_t                        Bgain;
	void*                           data;
};

struct alLSCLib_default_iso_t {
    uint16_t                        min_iso;
    uint16_t                        gain_r;
    uint16_t                        gain_g;
    uint16_t                        gain_b;
    uint32_t                        fn;
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct alLSCLib_set_parameter_t {
    enum alLSClib_set_parameter_type            type;
    union {
        void                                    *tuning_data;
        enum alLSClib_color_order               color_order;
        struct alLSCLib_hw_init_para_t          hw_init_para;
        struct alLSCLib_dynamic_proc_para_t     dynamic_proc_para;
		struct alLSCLib_awb_proc_para_t         awb_proc_para;
    }   para;
};
#pragma pack(pop)  /* restore old alignment setting from stack */


#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct alLSCLib_get_parameter_t {
    enum alLSClib_get_parameter_type            type;
    union {
        enum alLSClib_color_order               color_order;
        struct alLSCLib_hw_init_para_t          hw_init_para;
        struct alLSCLib_dynamic_proc_para_t     dynamic_proc_para;
		struct alLSCLib_awb_proc_para_t         awb_proc_para;
        struct alLSCLib_default_iso_t           default_ISO;
    }   para;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

typedef uint32_t(*alLSClib_init_func)(void **lsc_initial_buffer);
typedef uint32_t(*alLSClib_deinit_func)(void *lsc_obj);
typedef uint32_t(*alLSClib_set_param_func)(struct alLSCLib_set_parameter_t *param, void *lsc_dat);
typedef uint32_t(*alLSClib_get_param_func)(struct alLSCLib_get_parameter_t *param, void *lsc_dat);
typedef uint32_t(*alLSClib_calc_default_table_func)(void *buffer, struct alLSClib_output_data_t *default_output);
typedef uint32_t(*alLSClib_dynamic_process_func)(void *buffer, struct alLSClib_output_data_t *output);

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct alLSClib_runtime_obj_t {
    uint32_t                                   identityid;  /*!< framework tag for current instance */
    void                                       *data;
    alLSClib_init_func                         initial;
    alLSClib_deinit_func                       deinit;
    alLSClib_set_param_func                    set_param;
    alLSClib_get_param_func                    get_param;
    alLSClib_calc_default_table_func           calc_default_table;
    alLSClib_dynamic_process_func              dynamic_process;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

/********************************************************************************
*                           Public Function Prototype                          *
********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

uint32_t    alLSCLib_getlib_runtimebuf_size(void);
void        alLSCLib_getlib_version(struct alLSClib_lib_version_t *lib_version);


/* Return: TRUE: loading with no error , FALSE: false loading function APIs address. */
uint32_t    alLSCLib_loadfunc(struct alLSClib_runtime_obj_t *lsc_run_obj, uint32_t identityid);

#ifdef __cplusplus
}
#endif //__cplusplus

/**
@}
*/  /*end addtogroup LSC*/

#endif
