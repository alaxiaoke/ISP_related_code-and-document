#pragma once

#ifndef GravityXR_Grouping_H_FILE
#define GravityXR_Grouping_H_FILE

#include <stdint.h>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cassert>

typedef signed char schar;
typedef unsigned char uchar;
typedef unsigned short ushort;

static_assert(sizeof(uchar) == 1 && sizeof(short) == 2
	&& sizeof(int) == 4 && sizeof(float) == 4
	&& sizeof(double) == 8, "size required");

enum Grouping_Constants
{
	Grouping_INTER_NUM_TAPS_R4 = 5,
	Grouping_INTER_NUM_TAPS_R2 = 3,
	Grouping_INTER_COEF_BITS = 8,
	Grouping_INTER_COEF_SCALE = 1 << Grouping_INTER_COEF_BITS,
};

typedef struct
{
	int32_t Reg_Enable; // unsigned, 1bit [0, 1]
	int32_t Reg_HorEnable; // unsigned, 1bit [0, 1]
	int32_t Reg_VerEnable; // unsigned, 1bit [0, 1]
	int32_t Reg_SrcHeight; // unsigned, 12bit [0, 4094]
	int32_t Reg_SrcWidth; // unsigned, 12bit [0, 4094]
	int32_t Reg_DstHeight; // unsigned, 12bit [0, 4094]
	int32_t Reg_DstWidth; // unsigned, 12bit [0, 4094]
	int32_t Reg_HorGroupBoundary[4]; // unsigned, 12bit [0, 4094]
	int32_t Reg_VerGroupBoundary[4]; // unsigned, 12bit [0, 4094]
	int32_t Reg_Coef[Grouping_INTER_NUM_TAPS_R4 + Grouping_INTER_NUM_TAPS_R2 - 2]; // signed, 15bit [-16384, 16383]
} Grouping_PARAMS;

class Grouping
{
public:
	void parse_config(void* config_file, Grouping_PARAMS* p = nullptr);
	void updata_config(Grouping_PARAMS* p = nullptr);

public:
	void init();
	void release();

public:
	Grouping_PARAMS params;

public:
	Grouping();
	~Grouping();
	void process(void* src, void* dst, void* p = nullptr); // ÊäÈëÊä³ö, YUV 422 semi-planar interleaved

private:
	int32_t Reg_Enable; // unsigned, 1bit [0, 1]
	int32_t Reg_HorEnable; // unsigned, 1bit [0, 1]
	int32_t Reg_VerEnable; // unsigned, 1bit [0, 1]
	int32_t Reg_SrcHeight; // unsigned, 12bit [0, 4094]
	int32_t Reg_SrcWidth; // unsigned, 12bit [0, 4094]
	int32_t Reg_DstHeight; // unsigned, 12bit [0, 4094]
	int32_t Reg_DstWidth; // unsigned, 12bit [0, 4094]
	int32_t Reg_Coef[Grouping_INTER_NUM_TAPS_R4 + Grouping_INTER_NUM_TAPS_R2 - 2]; // signed, 15bit [-16384, 16383]

	int64_t reg_enable;
	int64_t reg_hor_enable;
	int64_t reg_ver_enable;
	int64_t reg_src_rows;
	int64_t reg_src_cols;
	int64_t reg_dst_rows;
	int64_t reg_dst_cols;
	int64_t reg_hor_group_boundary[4];
	int64_t reg_ver_group_boundary[4];
	int64_t reg_coef[Grouping_INTER_NUM_TAPS_R4 + Grouping_INTER_NUM_TAPS_R2 - 2];

	ushort* stream_src_y, * stream_src_uv;
	ushort* stream_dst_y, * stream_dst_uv;
	ushort* stream_hordst_y, * stream_hordst_uv;
	ushort* buffer_hordst_y[Grouping_INTER_NUM_TAPS_R4], * buffer_hordst_uv[Grouping_INTER_NUM_TAPS_R4];

	void prepare();
	int64_t select_groupid(int64_t index, int64_t group_boundary[4]);
	int64_t* select_coef(int64_t groupid);
	int64_t select_step(int64_t groupid);
	void hor_grouping();
	void ver_grouping();
};

#endif