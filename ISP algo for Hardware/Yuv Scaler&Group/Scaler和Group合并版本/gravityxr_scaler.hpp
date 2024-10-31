#pragma once

#ifndef GravityXR_SCLAER_H_FILE
#define GravityXR_SCLAER_H_FILE

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

enum Scaler_Constants
{
	Scaler_NUM_INTER_PHASE_BITS = 4,
	Scaler_NUM_INTER_PHASE = 1 << Scaler_NUM_INTER_PHASE_BITS,
	Scaler_NUM_TAPS = 6,
	Scaler_COORD_BITS = 17,
	Scaler_COORD_SCALE = 1 << Scaler_COORD_BITS,
	Scaler_COEF_BITS = 14,
	Scaler_COEF_SCALE = 1 << Scaler_COEF_BITS,
	Scaler_MAX_RATIO = 5,
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
	int32_t Reg_HorDeltaPhase[5]; // unsigned, 20bit
	int32_t Reg_VerDeltaPhase[5]; // unsigned, 20bit
	int32_t Reg_HorInitPhase[5]; // signed, 30bit
	int32_t Reg_HorUVInitPhase[5]; // signed, 30bit
	int32_t Reg_VerInitPhase[5]; // signed, 30bit
	int32_t Reg_HorCoef[(Scaler_NUM_INTER_PHASE + 2) * (Scaler_NUM_TAPS - 1)]; // signed, 16bit [-16384, 16384]
	int32_t Reg_VerCoef[(Scaler_NUM_INTER_PHASE + 2) * (Scaler_NUM_TAPS - 1)]; // signed, 16bit [-16384, 16384]
	int32_t Reg_HorGroupBoundary[4]; // unsigned, 12bit [0, 4094]
	int32_t Reg_VerGroupBoundary[4]; // unsigned, 12bit [0, 4094]
} Scaler_PARAMS;

class Scaler
{
public:
	void parse_config(void* config_file, Scaler_PARAMS* p = nullptr);
	void updata_config(Scaler_PARAMS* p = nullptr);

public:
	void init();
	void release();

public:
	Scaler_PARAMS params;

public:
	Scaler(int ybitdepth); // scaler ybitdepth = 8, grouping ybitdepth = 12
	~Scaler();
	void process(void* src, void* dst, void* p = nullptr); // ÊäÈëÊä³ö, YUV 422 semi-planar interleaved

private:
	int64_t YBITDEPTH;

	int64_t reg_enable;
	int64_t reg_hor_enable;
	int64_t reg_ver_enable;
	int64_t reg_src_rows;
	int64_t reg_src_cols;
	int64_t reg_dst_rows;
	int64_t reg_dst_cols;
	int64_t reg_ver_deltaphase[5];
	int64_t reg_hor_deltaphase[5];
	int64_t reg_ver_initphase[5];
	int64_t reg_hor_initphase[5];
	int64_t reg_hor_initphase_uv[5];
	int64_t reg_ver_coef[(Scaler_NUM_INTER_PHASE + 2) * (Scaler_NUM_TAPS - 1)];
	int64_t reg_hor_coef[(Scaler_NUM_INTER_PHASE + 2) * (Scaler_NUM_TAPS - 1)];
	int64_t reg_hor_group_boundary[4];
	int64_t reg_ver_group_boundary[4];

	ushort* stream_src_y, * stream_dst_y;
	ushort* stream_src_uv, * stream_dst_uv;
	ushort* stream_hordst_y, * stream_hordst_uv;
	ushort* buffer_hordst_y[Scaler_NUM_TAPS], * buffer_hordst_uv[Scaler_NUM_TAPS];

	void prepare();
};

#endif