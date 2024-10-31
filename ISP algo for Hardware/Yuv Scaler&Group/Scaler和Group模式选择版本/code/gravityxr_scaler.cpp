#include "gravityxr_scaler.hpp"

#define EC_Debuging 1

#ifndef border_replicate
#define border_replicate(p, len) ((p) < 0 ? 0 : (p) < (len) ? (p) : (len) - 1)
#endif

//==================== STD ====================//

template<class Tp> inline constexpr
Tp const& ec_min(Tp const& x, Tp const& y)
{
	return y < x ? y : x;
}

template<class Tp> inline constexpr
Tp const& ec_max(Tp const& x, Tp const& y)
{
	return y < x ? x : y;
}

template<class Tp> inline constexpr
Tp const& ec_clamp(Tp const& x, Tp const& lo, Tp const& hi)
{
	return x < lo ? lo : (hi < x ? hi : x);
}


//==================== Scaler ====================//

Scaler::Scaler()
{
	memset(&params, 0, sizeof(params));
	stream_src_y = NULL;
	stream_dst_y = NULL;
	stream_src_uv = NULL;
	stream_dst_uv = NULL;
	stream_hordst_y = NULL;
	stream_hordst_uv = NULL;
	memset(buffer_hordst_y, 0, sizeof(buffer_hordst_y));
	memset(buffer_hordst_uv, 0, sizeof(buffer_hordst_uv));
}


Scaler::~Scaler()
{
	if (stream_hordst_y)
	{
		free(stream_hordst_y);
		stream_hordst_y = NULL;
	}
	if (stream_hordst_uv)
	{
		free(stream_hordst_uv);
		stream_hordst_uv = NULL;
	}
	if (buffer_hordst_y[0])
	{
		free(buffer_hordst_y[0]);
		memset(buffer_hordst_y, 0, sizeof(buffer_hordst_y));
	}
	if (buffer_hordst_uv[0])
	{
		free(buffer_hordst_uv[0]);
		memset(buffer_hordst_uv, 0, sizeof(buffer_hordst_uv));
	}
}


void Scaler::init()
{
	prepare();
}


void Scaler::release()
{
}


void Scaler::parse_config(void* config_file, Scaler_PARAMS* p)
{
#define load_array(var) { if (var < sizeof(params->var) / sizeof(params->var[0])) params->var[var++] = static_cast<uint32_t>(val); }
	char key[512], valstr[6144];
	int ninput;
	int Reg_VerCoef = 0;
	int Reg_HorCoef = 0;

	Scaler_PARAMS* params = &this->params;
	if (p)
		params = p;

	FILE* fid = static_cast<FILE*>(config_file);
	assert(fid);
	while (!feof(fid))
	{
		ninput = fscanf(fid, " %511[^=\n]=%6143[^;\n]%*[^\n]", key, valstr);
		if (key[0] == ';') continue;
		if (key[0] == '[') break;
		double val = atof(valstr);
		if (ninput != 2)
			continue;
		else if (!strcmp(key, "enable") || !strcmp(key, "Reg_Enable"))
			params->Reg_Enable = static_cast<int32_t>(val);
		else if (!strcmp(key, "hor_enable") || !strcmp(key, "Reg_HorEnable"))
			params->Reg_HorEnable = static_cast<int32_t>(val);
		else if (!strcmp(key, "ver_enable") || !strcmp(key, "Reg_VerEnable"))
			params->Reg_VerEnable = static_cast<int32_t>(val);
		else if (!strcmp(key, "srows") || !strcmp(key, "Reg_SrcHeight"))
			params->Reg_SrcHeight = static_cast<int32_t>(val);
		else if (!strcmp(key, "scols") || !strcmp(key, "Reg_SrcWidth"))
			params->Reg_SrcWidth = static_cast<int32_t>(val);
		else if (!strcmp(key, "drows") || !strcmp(key, "Reg_DstHeight"))
			params->Reg_DstHeight = static_cast<int32_t>(val);
		else if (!strcmp(key, "dcols") || !strcmp(key, "Reg_DstWidth"))
			params->Reg_DstWidth = static_cast<int32_t>(val);
		else if (!strcmp(key, "vdp") || !strcmp(key, "Reg_VerDeltaPhase"))
			params->Reg_VerDeltaPhase = static_cast<int32_t>(val);
		else if (!strcmp(key, "hdp") || !strcmp(key, "Reg_HorDeltaPhase"))
			params->Reg_HorDeltaPhase = static_cast<int32_t>(val);
		else if (!strcmp(key, "vip") || !strcmp(key, "Reg_VerInitPhase"))
			params->Reg_VerInitPhase = static_cast<int32_t>(val);
		else if (!strcmp(key, "hip") || !strcmp(key, "Reg_HorInitPhase"))
			params->Reg_HorInitPhase = static_cast<int32_t>(val);
		else if (!strcmp(key, "huvip") || !strcmp(key, "Reg_HorUVInitPhase"))
			params->Reg_HorUVInitPhase = static_cast<int32_t>(val);
		else if (!strcmp(key, "vcoef") || !strcmp(key, "Reg_VerCoef"))
			load_array(Reg_VerCoef)
		else if (!strcmp(key, "hcoef") || !strcmp(key, "Reg_HorCoef"))
			load_array(Reg_HorCoef)
	}
#undef load_array
}


void Scaler::updata_config(Scaler_PARAMS* p)
{
	if (p)
		memcpy(&params, p, sizeof(params));
}


//==================== prepare ====================//

void Scaler::prepare()
{
	reg_enable = params.Reg_Enable;
	reg_hor_enable = params.Reg_HorEnable;
	reg_ver_enable = params.Reg_VerEnable;
	reg_src_rows = params.Reg_SrcHeight;
	reg_src_cols = params.Reg_SrcWidth;
	reg_dst_rows = params.Reg_DstHeight;
	reg_dst_cols = params.Reg_DstWidth;
	reg_ver_deltaphase = params.Reg_VerDeltaPhase;
	reg_hor_deltaphase = params.Reg_HorDeltaPhase;
	reg_ver_initphase = params.Reg_VerInitPhase;
	reg_hor_initphase = params.Reg_HorInitPhase;
	reg_hor_initphase_uv = params.Reg_HorUVInitPhase;

	int len = sizeof(params.Reg_VerCoef) / sizeof(params.Reg_VerCoef[0]);
	for (int i = 0; i < len; i++)
		 reg_ver_coef[i] = params.Reg_VerCoef[i];
	len = sizeof(params.Reg_HorCoef) / sizeof(params.Reg_HorCoef[0]);
	for (size_t i = 0; i < len; i++)
		reg_hor_coef[i] = params.Reg_HorCoef[i];

	reg_mode = params.Reg_Mode;
	for (int i = 0; i < 4; i++)
	{
		reg_hor_group_boundary[i] = params.Reg_HorGroupBoundary[i];
		reg_ver_group_boundary[i] = params.Reg_VerGroupBoundary[i];
	}

#if EC_Debuging
	if (reg_mode)
	{
		for (int i = 0; i < 3; i++)
		{
			assert(reg_hor_group_boundary[i] <= reg_hor_group_boundary[i + 1]);
			assert(reg_ver_group_boundary[i] <= reg_ver_group_boundary[i + 1]);
			assert(reg_hor_group_boundary[i] % 2 == 0);
		}
		assert(reg_hor_group_boundary[3] <= reg_dst_cols);
		assert(reg_ver_group_boundary[3] <= reg_dst_rows);
		assert(reg_hor_group_boundary[3] % 2 == 0);
	}
	assert(reg_src_cols % 2 == 0 && reg_dst_cols % 2 == 0);
#endif

	if (stream_hordst_y)
	{
		free(stream_hordst_y);
		stream_hordst_y = NULL;
	}
	if (stream_hordst_uv)
	{
		free(stream_hordst_uv);
		stream_hordst_uv = NULL;
	}
	if (buffer_hordst_y[0])
	{
		free(buffer_hordst_y[0]);
		memset(buffer_hordst_y, 0, sizeof(buffer_hordst_y));
	}
	if (buffer_hordst_uv[0])
	{
		free(buffer_hordst_uv[0]);
		memset(buffer_hordst_uv, 0, sizeof(buffer_hordst_uv));
	}
	stream_hordst_y = (ushort*)malloc(reg_src_rows * reg_dst_cols * sizeof(ushort));
	stream_hordst_uv = (ushort*)malloc(reg_src_rows * reg_dst_cols * sizeof(ushort));
	buffer_hordst_y[0] = (ushort*)malloc(Scaler_NUM_TAPS * reg_dst_cols * sizeof(ushort));
	buffer_hordst_uv[0] = (ushort*)malloc(Scaler_NUM_TAPS * reg_dst_cols * sizeof(ushort));
	for (int i = 1; i < Scaler_NUM_TAPS; i++)
	{
		buffer_hordst_y[i] = buffer_hordst_y[i - 1] + reg_dst_cols;
		buffer_hordst_uv[i] = buffer_hordst_uv[i - 1] + reg_dst_cols;
	}
}


//==================== process ====================//

static int64_t select_coef_index(int64_t index, int64_t group_boundary[4])
{
	if (index < group_boundary[0])
		return 0;
	else if (index < group_boundary[1])
		return 1;
	else if (index < group_boundary[2])
		return 2;
	else if (index < group_boundary[3])
		return 1;
	else
		return 0;
}


static int64_t select_delta_phase(int64_t index, int64_t group_boundary[4])
{
	if (index < group_boundary[0])
		return 4 << Scaler_COORD_BITS;
	else if (index < group_boundary[1])
		return 2 << Scaler_COORD_BITS;
	else if (index < group_boundary[2])
		return 1 << Scaler_COORD_BITS;
	else if (index < group_boundary[3])
		return 2 << Scaler_COORD_BITS;
	else
		return 4 << Scaler_COORD_BITS;
}


void Scaler::process(void* src, void* dst, void* p)
{
	if (p)
	{
		updata_config(static_cast<Scaler_PARAMS*>(p));
		prepare();
	}

	if (!reg_enable)
		memcpy(dst, src, reg_src_rows * reg_src_cols * 2 * sizeof(ushort));
	else
	{
		stream_src_y = static_cast<ushort*>(src);
		stream_src_uv = stream_src_y + reg_src_rows * reg_src_cols;
		stream_dst_y = static_cast<ushort*>(dst);
		stream_dst_uv = stream_dst_y + reg_dst_rows * reg_dst_cols;

		int64_t const mask = (1 << Scaler_COORD_BITS) - 1;

		// horizontal_filter
		int64_t row_index = 0;
		ushort* Src_Y = stream_src_y;
		ushort* Src_UV = stream_src_uv;
		ushort* HorDst_Y = stream_hordst_y;
		ushort* HorDst_UV = stream_hordst_uv;

		while (row_index < reg_src_rows)
		{
			// horizontal_filter_onerow
			int64_t col_index = 0;
			int64_t cur_phase_y = reg_mode ? 0 : reg_hor_initphase;
			int64_t cur_phase_uv = reg_mode ? 0 : reg_hor_initphase_uv;
			int64_t cur_delta_phase;
			int64_t coef_index, coef_offset;
			int64_t w0, w1, w2, w3, w4, w5;
			int64_t res;

			while (col_index < reg_dst_cols)
			{
				if (reg_hor_enable)
				{
					// horizontal_filter_onerow_process_y
					cur_delta_phase = reg_mode ? select_delta_phase(col_index, reg_hor_group_boundary) : reg_hor_deltaphase;

					w0 = border_replicate((cur_phase_y >> Scaler_COORD_BITS) - 2, reg_src_cols);
					w1 = border_replicate((cur_phase_y >> Scaler_COORD_BITS) - 1, reg_src_cols);
					w2 = border_replicate((cur_phase_y >> Scaler_COORD_BITS), reg_src_cols);
					w3 = border_replicate((cur_phase_y >> Scaler_COORD_BITS) + 1, reg_src_cols);
					w4 = border_replicate((cur_phase_y >> Scaler_COORD_BITS) + 2, reg_src_cols);
					w5 = border_replicate((cur_phase_y >> Scaler_COORD_BITS) + 3, reg_src_cols);

					coef_index = reg_mode ? select_coef_index(col_index, reg_hor_group_boundary) : (cur_phase_y & mask) >> (Scaler_COORD_BITS - Scaler_NUM_INTER_PHASE_BITS);
					coef_offset = (coef_index << 2) + coef_index;
					res = (Src_Y[w0] - Src_Y[w5]) * reg_hor_coef[coef_offset] + // 中间乘法结果 S25
						(Src_Y[w1] - Src_Y[w5]) * reg_hor_coef[coef_offset + 1] +
						(Src_Y[w2] - Src_Y[w5]) * reg_hor_coef[coef_offset + 2] +
						(Src_Y[w3] - Src_Y[w5]) * reg_hor_coef[coef_offset + 3] +
						(Src_Y[w4] - Src_Y[w5]) * reg_hor_coef[coef_offset + 4] +
						(Src_Y[w5] << Scaler_COEF_BITS); // res U24
					HorDst_Y[col_index] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 1)) >> Scaler_COEF_BITS, 0, 1023);

					w0 = border_replicate(((cur_phase_y + cur_delta_phase) >> Scaler_COORD_BITS) - 2, reg_src_cols);
					w1 = border_replicate(((cur_phase_y + cur_delta_phase) >> Scaler_COORD_BITS) - 1, reg_src_cols);
					w2 = border_replicate(((cur_phase_y + cur_delta_phase) >> Scaler_COORD_BITS), reg_src_cols);
					w3 = border_replicate(((cur_phase_y + cur_delta_phase) >> Scaler_COORD_BITS) + 1, reg_src_cols);
					w4 = border_replicate(((cur_phase_y + cur_delta_phase) >> Scaler_COORD_BITS) + 2, reg_src_cols);
					w5 = border_replicate(((cur_phase_y + cur_delta_phase) >> Scaler_COORD_BITS) + 3, reg_src_cols);
					coef_index = reg_mode ? select_coef_index(col_index, reg_hor_group_boundary) : ((cur_phase_y + cur_delta_phase) & mask) >> (Scaler_COORD_BITS - Scaler_NUM_INTER_PHASE_BITS);
					coef_offset = (coef_index << 2) + coef_index;
					res = (Src_Y[w0] - Src_Y[w5]) * reg_hor_coef[coef_offset] +	// 中间乘法结果 S25
						(Src_Y[w1] - Src_Y[w5]) * reg_hor_coef[coef_offset + 1] +
						(Src_Y[w2] - Src_Y[w5]) * reg_hor_coef[coef_offset + 2] +
						(Src_Y[w3] - Src_Y[w5]) * reg_hor_coef[coef_offset + 3] +
						(Src_Y[w4] - Src_Y[w5]) * reg_hor_coef[coef_offset + 4] +
						(Src_Y[w5] << Scaler_COEF_BITS); // res U24
					HorDst_Y[col_index + 1] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 1)) >> Scaler_COEF_BITS, 0, 1023);


					// horizontal_filter_onerow_process_uv
					w0 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS) - 2, reg_src_cols >> 1) << 1;
					w1 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS) - 1, reg_src_cols >> 1) << 1;
					w2 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS), reg_src_cols >> 1) << 1;
					w3 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS) + 1, reg_src_cols >> 1) << 1;
					w4 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS) + 2, reg_src_cols >> 1) << 1;
					w5 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS) + 3, reg_src_cols >> 1) << 1;
					coef_index = reg_mode ? select_coef_index(col_index, reg_hor_group_boundary) : (cur_phase_uv & mask) >> (Scaler_COORD_BITS - Scaler_NUM_INTER_PHASE_BITS);
					coef_offset = (coef_index << 2) + coef_index;

					res = (Src_UV[w0] - Src_UV[w5]) * reg_hor_coef[coef_offset] + // 中间乘法结果 S25
						(Src_UV[w1] - Src_UV[w5]) * reg_hor_coef[coef_offset + 1] +
						(Src_UV[w2] - Src_UV[w5]) * reg_hor_coef[coef_offset + 2] +
						(Src_UV[w3] - Src_UV[w5]) * reg_hor_coef[coef_offset + 3] +
						(Src_UV[w4] - Src_UV[w5]) * reg_hor_coef[coef_offset + 4] +
						(Src_UV[w5] << Scaler_COEF_BITS); // res U24
					HorDst_UV[col_index] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 1)) >> Scaler_COEF_BITS, 0, 1023);
					res = (Src_UV[w0 + 1] - Src_UV[w5 + 1]) * reg_hor_coef[coef_offset] + // 中间乘法结果 S25
						(Src_UV[w1 + 1] - Src_UV[w5 + 1]) * reg_hor_coef[coef_offset + 1] +
						(Src_UV[w2 + 1] - Src_UV[w5 + 1]) * reg_hor_coef[coef_offset + 2] +
						(Src_UV[w3 + 1] - Src_UV[w5 + 1]) * reg_hor_coef[coef_offset + 3] +
						(Src_UV[w4 + 1] - Src_UV[w5 + 1]) * reg_hor_coef[coef_offset + 4] +
						(Src_UV[w5 + 1] << Scaler_COEF_BITS); // res U24
					HorDst_UV[col_index + 1] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 1)) >> Scaler_COEF_BITS, 0, 1023);

					col_index += 2;
					cur_phase_y += (cur_delta_phase << 1);
					cur_phase_uv += cur_delta_phase;
				}
				else
				{
					HorDst_Y[col_index] = Src_Y[col_index];
					HorDst_Y[col_index + 1] = Src_Y[col_index + 1];
					HorDst_UV[col_index] = Src_UV[col_index];
					HorDst_UV[col_index + 1] = Src_UV[col_index + 1];
					col_index += 2;
				}
			}

			row_index += 1;
			Src_Y += reg_src_cols;
			Src_UV += reg_src_cols;
			HorDst_Y += reg_dst_cols;
			HorDst_UV += reg_dst_cols;
		}

		// vertical_filter
		row_index = 0;
		ushort* Dst_Y = stream_dst_y;
		ushort* Dst_UV = stream_dst_uv;
		HorDst_Y = stream_hordst_y;
		HorDst_UV = stream_hordst_uv;

		int64_t cur_phase = reg_mode ? 0 : reg_ver_initphase;
		int64_t cur_delta_phase;
		int64_t coef_index, coef_offset;
		int64_t coord, last_coord, shift_num;
		if (reg_ver_enable)
		{
			while (row_index < reg_dst_rows)
			{
				cur_delta_phase = reg_mode ? select_delta_phase(row_index, reg_ver_group_boundary) : reg_ver_deltaphase;

				// get_coord
				coord = border_replicate((cur_phase >> Scaler_COORD_BITS), reg_src_rows);

				// get_hor_stream_to_buffer
				shift_num = row_index == 0 ? 6 : ec_min<int>(coord - last_coord, 6);
				last_coord = coord;
				for (int i = 0; i < 6 - shift_num; i++) // 保留6-shift_num条
				{
					memcpy(buffer_hordst_y[i], buffer_hordst_y[i + shift_num], reg_dst_cols * sizeof(ushort));
					memcpy(buffer_hordst_uv[i], buffer_hordst_uv[i + shift_num], reg_dst_cols * sizeof(ushort));
				}
				for (int i = 4 - shift_num; i < 4; i++) // 新增shift_num条
				{
					int64_t cood_i = border_replicate((cur_phase >> Scaler_COORD_BITS) + i, reg_src_rows);
					memcpy(buffer_hordst_y[i + 2], HorDst_Y + cood_i * reg_dst_cols, reg_dst_cols * sizeof(ushort));
					memcpy(buffer_hordst_uv[i + 2], HorDst_UV + cood_i * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				}

				// get_ver_coef_offset
				coef_index = reg_mode ? select_coef_index(row_index, reg_ver_group_boundary) : (cur_phase & mask) >> (Scaler_COORD_BITS - Scaler_NUM_INTER_PHASE_BITS);
				coef_offset = (coef_index << 2) + coef_index;

				// vertical_filter_onerow
				int64_t col_index = 0;
				int64_t res;
				while (col_index < reg_dst_cols)
				{
					// vertical_filter_onerow_process_y
					res = (buffer_hordst_y[0][col_index] - buffer_hordst_y[5][col_index]) * reg_ver_coef[coef_offset] +	// 中间乘法结果 S25
						(buffer_hordst_y[1][col_index] - buffer_hordst_y[5][col_index]) * reg_ver_coef[coef_offset + 1] +
						(buffer_hordst_y[2][col_index] - buffer_hordst_y[5][col_index]) * reg_ver_coef[coef_offset + 2] +
						(buffer_hordst_y[3][col_index] - buffer_hordst_y[5][col_index]) * reg_ver_coef[coef_offset + 3] +
						(buffer_hordst_y[4][col_index] - buffer_hordst_y[5][col_index]) * reg_ver_coef[coef_offset + 4] +
						(buffer_hordst_y[5][col_index] << Scaler_COEF_BITS);	// res U24
					Dst_Y[col_index] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 1)) >> Scaler_COEF_BITS, 0, 1023);

					res = (buffer_hordst_y[0][col_index + 1] - buffer_hordst_y[5][col_index + 1]) * reg_ver_coef[coef_offset] +	// 中间乘法结果 S25
						(buffer_hordst_y[1][col_index + 1] - buffer_hordst_y[5][col_index + 1]) * reg_ver_coef[coef_offset + 1] +
						(buffer_hordst_y[2][col_index + 1] - buffer_hordst_y[5][col_index + 1]) * reg_ver_coef[coef_offset + 2] +
						(buffer_hordst_y[3][col_index + 1] - buffer_hordst_y[5][col_index + 1]) * reg_ver_coef[coef_offset + 3] +
						(buffer_hordst_y[4][col_index + 1] - buffer_hordst_y[5][col_index + 1]) * reg_ver_coef[coef_offset + 4] +
						(buffer_hordst_y[5][col_index + 1] << Scaler_COEF_BITS);	// res U24
					Dst_Y[col_index + 1] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 1)) >> Scaler_COEF_BITS, 0, 1023);


					// vertical_filter_onerow_process_uv
					res = (buffer_hordst_uv[0][col_index] - buffer_hordst_uv[5][col_index]) * reg_ver_coef[coef_offset] +	// 中间乘法结果 S25
						(buffer_hordst_uv[1][col_index] - buffer_hordst_uv[5][col_index]) * reg_ver_coef[coef_offset + 1] +
						(buffer_hordst_uv[2][col_index] - buffer_hordst_uv[5][col_index]) * reg_ver_coef[coef_offset + 2] +
						(buffer_hordst_uv[3][col_index] - buffer_hordst_uv[5][col_index]) * reg_ver_coef[coef_offset + 3] +
						(buffer_hordst_uv[4][col_index] - buffer_hordst_uv[5][col_index]) * reg_ver_coef[coef_offset + 4] +
						(buffer_hordst_uv[5][col_index] << Scaler_COEF_BITS);	// res U24
					Dst_UV[col_index] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 1)) >> Scaler_COEF_BITS, 0, 1023);

					res = (buffer_hordst_uv[0][col_index + 1] - buffer_hordst_uv[5][col_index + 1]) * reg_ver_coef[coef_offset] +	// 中间乘法结果 S25
						(buffer_hordst_uv[1][col_index + 1] - buffer_hordst_uv[5][col_index + 1]) * reg_ver_coef[coef_offset + 1] +
						(buffer_hordst_uv[2][col_index + 1] - buffer_hordst_uv[5][col_index + 1]) * reg_ver_coef[coef_offset + 2] +
						(buffer_hordst_uv[3][col_index + 1] - buffer_hordst_uv[5][col_index + 1]) * reg_ver_coef[coef_offset + 3] +
						(buffer_hordst_uv[4][col_index + 1] - buffer_hordst_uv[5][col_index + 1]) * reg_ver_coef[coef_offset + 4] +
						(buffer_hordst_uv[5][col_index + 1] << Scaler_COEF_BITS);	// res U24
					Dst_UV[col_index + 1] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 1)) >> Scaler_COEF_BITS, 0, 1023);

					col_index += 2;
				}

				row_index += 1;
				Dst_Y += reg_dst_cols;
				Dst_UV += reg_dst_cols;
				cur_phase += cur_delta_phase;
			}
		}
		else
		{
			while (row_index < reg_dst_rows)
			{
				int64_t col_index = 0;
				while (col_index < reg_dst_cols)
				{
					Dst_Y[col_index] = HorDst_Y[col_index];
					Dst_Y[col_index + 1] = HorDst_Y[col_index + 1];
					Dst_UV[col_index] = HorDst_UV[col_index];
					Dst_UV[col_index + 1] = HorDst_UV[col_index + 1];
					col_index += 2;
				}
				row_index += 1;
				Dst_Y += reg_dst_cols;
				Dst_UV += reg_dst_cols;
				HorDst_Y += reg_dst_cols;
				HorDst_UV += reg_dst_cols;
			}
		}
	}
}