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
@file allib_lsc_core.h
@brief Parameter of lsc library
@author Han Tseng
@date 2023/06/29
*/

#ifndef _ALLIB_LSC_CORE_H_
#define _ALLIB_LSC_CORE_H_

/**
@defgroup LSC_CORE LSC_CORE
@brief Lens Shading Compensation
@ingroup  LSC
*/

/**
@addtogroup LSC_CORE
@{
*/


/****************************************************************************
*                               Include File                                *
****************************************************************************/

#include "allib_LSC.h" 

/****************************************************************************
*                         Public Constant Definition                        *
****************************************************************************/

#define LSC_VERSION_MAJOR                       (1)
#define LSC_VERSION_MINOR                       (0)
#define LSC_TUNING_STRUCT_VERSION               (0)

//tuning header info
#define LSC_HEADER_TAG                          "header_tag"
#define LSC_PROJECT_NAME                        "project_name"
#define LSC_SENSOR_NAME                         "sensor_name"
#define LSC_MODULE_HOUSE_NAME                   "module_house_name"
#define LSC_VERSION_DATETIME                    "202303221600"

//Common
#define STRING_LEN                              (20)
#define DATE_TIME_LEN                           (12)
#define SESSION_MAX                             (3)
#define DEFAULT_CALIBRATION_GAIN                (512)
#define SHAD_BASE                               (512)
#define SHAD_MAX                                (4095)
#define CONVERGENCE_THRESHOLD                   (100)
#define PROGRESSIVE_STEP                        (50)
#define LSC_TABLE_W                             (28)
#define LSC_TABLE_H                             (20)
#define LSC_TABLE_SIZE                          (LSC_TABLE_W * LSC_TABLE_H)
#define ISO_NODE_NUM                            (32)
#define INITIAL_PREVIOUS_ISO                    (1400)

//Default High ISO decrease gain nodes
#define DEFAULT_X_NODES {\
0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500,\
1600, 1700, 1800, 1900, 2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900, 3000, 3100,}

//base on 1024
#define DEFAULT_Y_DOWN_RATIO {\
1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 896, 768, 640, 512, 384,\
256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,}

//from LIGHT_SOURCE_CT_0 to LIGHT_SOURCE_CT_4
#define DEFAULT_CT_VALUE {\
2800, 3500, 4100, 5000, 6500, }

//record different type light source
typedef enum
{
    LIGHT_SOURCE_CT_0 = 0,   /* CT lowest */
    LIGHT_SOURCE_CT_1,
    LIGHT_SOURCE_CT_2,
    LIGHT_SOURCE_CT_3,
    LIGHT_SOURCE_CT_4,       /* CT highest */

    LIGHT_SOURCE_CT_NUMBER,
}   LSC_LIGHT_SOURCE;

#pragma pack(push) //push current alignment setting to stack
#pragma pack(1)    //new alignment setting
typedef struct
{
    char                                header_tag[STRING_LEN];
    uint32_t                            total_file_size;
    uint32_t                            total_file_size_exclude_finetuneable;
    char                                project_name[STRING_LEN];
    char                                sensor_name[STRING_LEN];
    char                                module_house_name[STRING_LEN];
    uint8_t                             sensor_id;
    uint8_t                             input_table_tool_version_number;
    uint16_t                            struct_version;
    int8_t                              version_date_time[DATE_TIME_LEN];
    uint32_t                            calibration_version;
    uint32_t                            table_check_sum;
    uint16_t                            OTP_LSC_size;
    uint8_t                             shading_table_width;
    uint8_t                             shading_table_height;
    char                                reserved[44];
}   LSC_HEADER;
#pragma pack(pop)

typedef struct
{
    uint16_t                            color_temperature;
    uint16_t                            shading_table_r[LSC_TABLE_SIZE];
    uint16_t                            shading_table_gr[LSC_TABLE_SIZE];
    uint16_t                            shading_table_gb[LSC_TABLE_SIZE];
    uint16_t                            shading_table_b[LSC_TABLE_SIZE];
}   LSC_TABLE;

#pragma pack(push) //push current alignment setting to stack
#pragma pack(4)    //new alignment setting
typedef struct
{
    uint16_t                            y_mean_threshold;                                       //Average Y (8-bit), scale 100
    uint16_t                            x_iso_nodes[ISO_NODE_NUM];
    uint16_t                            y_iso_values[ISO_NODE_NUM];                          //base : 1024 (ex : if set 256, gain down ratio = 256 / 1024 = 0.25)
    struct alLSCLib_default_iso_t       default_iso_calibration;
    int32_t                             bv_threshold;                                           //-7500
    uint16_t                            iso_step;                                               //50 
    LSC_TABLE                           shading_tables[LIGHT_SOURCE_CT_NUMBER];      //different index assigned different color temperature light source
    char                                reserved[1024];
}   LSC_TUNING_INFO;
#pragma pack(pop)

typedef struct
{
    LSC_HEADER                          header;
    LSC_TUNING_INFO                     tuning_info;
}   LSC_SCENARIO_TUNING_DATA;

typedef struct
{
    uint16_t                            shading_table_r[LSC_TABLE_SIZE];
    uint16_t                            shading_table_gr[LSC_TABLE_SIZE];
    uint16_t                            shading_table_gb[LSC_TABLE_SIZE];
    uint16_t                            shading_table_b[LSC_TABLE_SIZE];
}   LSC_RESULT_TABLE;

typedef struct {
    uint16_t                        previous_iso;
    uint16_t                        interpolate_ct;
    uint16_t                        interpolate_index_low;
    uint16_t                        interpolate_index_high;
    uint16_t                        interpolate_weight_low;
    uint16_t                        interpolate_weight_high;
}   LSC_DEBUG;

typedef struct
{
    LSC_HEADER                          header;
    LSC_TUNING_INFO                     tuning_info;
    LSC_DEBUG                           debug_info;

    enum alLSClib_color_order           color_order;
    uint8_t                             shading_bypass;
    uint8_t                             ping_pong_index;
    uint16_t                            previous_iso;
    int16_t                             current_bv;
    uint16_t                            iso_speed;
    uint16_t                            y_mean;
    uint16_t                            interpolate_ct;

    LSC_RESULT_TABLE                    ping_pong_buffer_0;
    LSC_RESULT_TABLE                    ping_pong_buffer_1;

}   LSC_RUNTIME_DATA;

#endif
