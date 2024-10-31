#pragma once

#ifndef _ALLIB_LSC_CORE_H_
#define _ALLIB_LSC_CORE_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CLIP(value, minVal, maxVal) ((value) < (minVal) ? (minVal) : ((value) > (maxVal) ? (maxVal) : (value)))

#define LSC_TABLE_W                             (17)
#define LSC_TABLE_H                             (13)
#define LSC_TABLE_SIZE                          (LSC_TABLE_W * LSC_TABLE_H)
#define RAW_W                               (3280)
#define RAW_H                               (2448)
#define RAW_SIZE                            (RAW_W * RAW_H)

enum GXR_RawPatternTypes
{
    GXR_Raw_RGGB,
    GXR_Raw_BGGR,
    GXR_Raw_GRBG,
    GXR_Raw_GBRG,
    GXR_Raw_Patt_Impl = 4,
};

typedef struct
{
    uint16_t                            upscale_table_r[RAW_SIZE / 4];
    uint16_t                            upscale_table_gr[RAW_SIZE / 4];
    uint16_t                            upscale_table_gb[RAW_SIZE / 4];
    uint16_t                            upscale_table_b[RAW_SIZE / 4];
}   LSC_UPSCALE_TABLE;

typedef struct
{
    uint16_t                            shading_table_r[LSC_TABLE_SIZE];
    uint16_t                            shading_table_gr[LSC_TABLE_SIZE];
    uint16_t                            shading_table_gb[LSC_TABLE_SIZE];
    uint16_t                            shading_table_b[LSC_TABLE_SIZE];
}   LSC_TABLE;


typedef enum
{
    LIGHT_SOURCE_CT_0 = 0,   /* CT lowest */
    LIGHT_SOURCE_CT_1,
    LIGHT_SOURCE_CT_2,
    LIGHT_SOURCE_CT_3,
    LIGHT_SOURCE_CT_4,       /* CT highest */

    LIGHT_SOURCE_CT_NUMBER,
}   LSC_LIGHT_SOURCE;

void apply_lsc(int bayer_pat, LSC_UPSCALE_TABLE* upscale_table, uint16_t* data);
int32_t tableUpScale(uint16_t* shading_table, uint16_t* upscale_table);

#endif