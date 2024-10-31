#include "gravityxr_scaler.hpp"

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
	stream_conv_hor_y = NULL;
	stream_conv_hor_uv = NULL;
	memset(buffer_conv_hor_y, 0, sizeof(buffer_conv_hor_y));
	memset(buffer_conv_hor_uv, 0, sizeof(buffer_conv_hor_uv));
}


Scaler::~Scaler()
{
	if (stream_conv_hor_y)
	{
		free(stream_conv_hor_y);
		stream_conv_hor_y = NULL;
	}
	if (stream_conv_hor_uv)
	{
		free(stream_conv_hor_uv);
		stream_conv_hor_uv = NULL;
	}
	if (buffer_conv_hor_y[0])
	{
		free(buffer_conv_hor_y[0]);
		memset(buffer_conv_hor_y, 0, sizeof(buffer_conv_hor_y));
	}
	if (buffer_conv_hor_uv[0])
	{
		free(buffer_conv_hor_uv[0]);
		memset(buffer_conv_hor_uv, 0, sizeof(buffer_conv_hor_uv));
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

	if (stream_conv_hor_y)
	{
		free(stream_conv_hor_y);
		stream_conv_hor_y = NULL;
	}
	if (stream_conv_hor_uv)
	{
		free(stream_conv_hor_uv);
		stream_conv_hor_uv = NULL;
	}
	if (buffer_conv_hor_y[0])
	{
		free(buffer_conv_hor_y[0]);
		memset(buffer_conv_hor_y, 0, sizeof(buffer_conv_hor_y));
	}
	if (buffer_conv_hor_uv[0])
	{
		free(buffer_conv_hor_uv[0]);
		memset(buffer_conv_hor_uv, 0, sizeof(buffer_conv_hor_uv));
	}
	stream_conv_hor_y = (ushort*)malloc(reg_src_rows * reg_dst_cols * sizeof(ushort));
	stream_conv_hor_uv = (ushort*)malloc(reg_src_rows * reg_dst_cols * sizeof(ushort));
	buffer_conv_hor_y[0] = (ushort*)malloc(Scaler_NUM_TAPS * reg_dst_cols * sizeof(ushort));
	buffer_conv_hor_uv[0] = (ushort*)malloc(Scaler_NUM_TAPS * reg_dst_cols * sizeof(ushort));
	for (int i = 1; i < Scaler_NUM_TAPS; i++)
	{
		buffer_conv_hor_y[i] = buffer_conv_hor_y[i - 1] + reg_dst_cols;
		buffer_conv_hor_uv[i] = buffer_conv_hor_uv[i - 1] + reg_dst_cols;
	}
}


//==================== process ====================//

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
		ushort* Conv_Hor_Y = stream_conv_hor_y;
		ushort* Conv_Hor_UV = stream_conv_hor_uv;

		while (row_index < reg_src_rows)
		{
			// horizontal_filter_onerow
			int64_t col_index = 0;
			int64_t cur_phase_y = reg_hor_initphase;
			int64_t cur_phase_uv = reg_hor_initphase_uv;

			int64_t coef_index, coef_offset;
			int64_t w0, w1, w2, w3, w4, w5;
			int64_t res;

			while (col_index < reg_dst_cols)
			{
				if (reg_hor_enable)
				{
					// horizontal_filter_onerow_process_y
					w0 = border_replicate((cur_phase_y >> Scaler_COORD_BITS) - 2, reg_src_cols);
					w1 = border_replicate((cur_phase_y >> Scaler_COORD_BITS) - 1, reg_src_cols);
					w2 = border_replicate((cur_phase_y >> Scaler_COORD_BITS), reg_src_cols);
					w3 = border_replicate((cur_phase_y >> Scaler_COORD_BITS) + 1, reg_src_cols);
					w4 = border_replicate((cur_phase_y >> Scaler_COORD_BITS) + 2, reg_src_cols);
					w5 = border_replicate((cur_phase_y >> Scaler_COORD_BITS) + 3, reg_src_cols);

					coef_index = (cur_phase_y & mask) >> (Scaler_COORD_BITS - Scaler_NUM_INTER_PHASE_BITS);
					coef_offset = (coef_index << 2) + coef_index;
					res = (Src_Y[w0] - Src_Y[w5]) * reg_hor_coef[coef_offset] +
						(Src_Y[w1] - Src_Y[w5]) * reg_hor_coef[coef_offset + 1] +
						(Src_Y[w2] - Src_Y[w5]) * reg_hor_coef[coef_offset + 2] +
						(Src_Y[w3] - Src_Y[w5]) * reg_hor_coef[coef_offset + 3] +
						(Src_Y[w4] - Src_Y[w5]) * reg_hor_coef[coef_offset + 4] +
						(Src_Y[w5] << Scaler_COEF_BITS);
					Conv_Hor_Y[col_index] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 3)) >> (Scaler_COEF_BITS - 2), 0, 4095);

					w0 = border_replicate(((cur_phase_y + reg_hor_deltaphase) >> Scaler_COORD_BITS) - 2, reg_src_cols);
					w1 = border_replicate(((cur_phase_y + reg_hor_deltaphase) >> Scaler_COORD_BITS) - 1, reg_src_cols);
					w2 = border_replicate(((cur_phase_y + reg_hor_deltaphase) >> Scaler_COORD_BITS), reg_src_cols);
					w3 = border_replicate(((cur_phase_y + reg_hor_deltaphase) >> Scaler_COORD_BITS) + 1, reg_src_cols);
					w4 = border_replicate(((cur_phase_y + reg_hor_deltaphase) >> Scaler_COORD_BITS) + 2, reg_src_cols);
					w5 = border_replicate(((cur_phase_y + reg_hor_deltaphase) >> Scaler_COORD_BITS) + 3, reg_src_cols);
					coef_index = ((cur_phase_y + reg_hor_deltaphase) & mask) >> (Scaler_COORD_BITS - Scaler_NUM_INTER_PHASE_BITS);
					coef_offset = (coef_index << 2) + coef_index;
					res = (Src_Y[w0] - Src_Y[w5]) * reg_hor_coef[coef_offset] +
						(Src_Y[w1] - Src_Y[w5]) * reg_hor_coef[coef_offset + 1] +
						(Src_Y[w2] - Src_Y[w5]) * reg_hor_coef[coef_offset + 2] +
						(Src_Y[w3] - Src_Y[w5]) * reg_hor_coef[coef_offset + 3] +
						(Src_Y[w4] - Src_Y[w5]) * reg_hor_coef[coef_offset + 4] +
						(Src_Y[w5] << Scaler_COEF_BITS);
					Conv_Hor_Y[col_index + 1] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 3)) >> (Scaler_COEF_BITS - 2), 0, 4095);


					// horizontal_filter_onerow_process_uv
					w0 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS) - 2, reg_src_cols >> 1) << 1;
					w1 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS) - 1, reg_src_cols >> 1) << 1;
					w2 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS), reg_src_cols >> 1) << 1;
					w3 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS) + 1, reg_src_cols >> 1) << 1;
					w4 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS) + 2, reg_src_cols >> 1) << 1;
					w5 = border_replicate((cur_phase_uv >> Scaler_COORD_BITS) + 3, reg_src_cols >> 1) << 1;
					coef_index = (cur_phase_uv & mask) >> (Scaler_COORD_BITS - Scaler_NUM_INTER_PHASE_BITS);
					coef_offset = (coef_index << 2) + coef_index;

					res = (Src_UV[w0] - Src_UV[w5]) * reg_hor_coef[coef_offset] +
						(Src_UV[w1] - Src_UV[w5]) * reg_hor_coef[coef_offset + 1] +
						(Src_UV[w2] - Src_UV[w5]) * reg_hor_coef[coef_offset + 2] +
						(Src_UV[w3] - Src_UV[w5]) * reg_hor_coef[coef_offset + 3] +
						(Src_UV[w4] - Src_UV[w5]) * reg_hor_coef[coef_offset + 4] +
						(Src_UV[w5] << Scaler_COEF_BITS);
					Conv_Hor_UV[col_index] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 3)) >> (Scaler_COEF_BITS - 2), 0, 4095);
					res = (Src_UV[w0 + 1] - Src_UV[w5 + 1]) * reg_hor_coef[coef_offset] +
						(Src_UV[w1 + 1] - Src_UV[w5 + 1]) * reg_hor_coef[coef_offset + 1] +
						(Src_UV[w2 + 1] - Src_UV[w5 + 1]) * reg_hor_coef[coef_offset + 2] +
						(Src_UV[w3 + 1] - Src_UV[w5 + 1]) * reg_hor_coef[coef_offset + 3] +
						(Src_UV[w4 + 1] - Src_UV[w5 + 1]) * reg_hor_coef[coef_offset + 4] +
						(Src_UV[w5 + 1] << Scaler_COEF_BITS);
					Conv_Hor_UV[col_index + 1] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE >> 3)) >> (Scaler_COEF_BITS - 2), 0, 4095);

					col_index += 2;
					cur_phase_y += (reg_hor_deltaphase << 1);
					cur_phase_uv += reg_hor_deltaphase;
				}
				else
				{
					Conv_Hor_Y[col_index] = Src_Y[col_index] << 2;
					Conv_Hor_Y[col_index + 1] = Src_Y[col_index + 1] << 2;
					Conv_Hor_UV[col_index] = Src_UV[col_index] << 2;
					Conv_Hor_UV[col_index + 1] = Src_UV[col_index + 1] << 2;
					col_index += 2;
				}
			}

			row_index += 1;
			Src_Y += reg_src_cols;
			Src_UV += reg_src_cols;
			Conv_Hor_Y += reg_dst_cols;
			Conv_Hor_UV += reg_dst_cols;
		}

		// vertical_filter
		row_index = 0;
		ushort* Dst_Y = stream_dst_y;
		ushort* Dst_UV = stream_dst_uv;
		Conv_Hor_Y = stream_conv_hor_y;
		Conv_Hor_UV = stream_conv_hor_uv;

		int64_t cur_phase = reg_ver_initphase;
		int64_t coef_index, coef_offset;
		int64_t h0, h1, h2, h3, h4, h5;
		if (reg_ver_enable)
		{
			while (row_index < reg_dst_rows)
			{
				// get_coord
				h0 = border_replicate((cur_phase >> Scaler_COORD_BITS) - 2, reg_src_rows);
				h1 = border_replicate((cur_phase >> Scaler_COORD_BITS) - 1, reg_src_rows);
				h2 = border_replicate((cur_phase >> Scaler_COORD_BITS), reg_src_rows);
				h3 = border_replicate((cur_phase >> Scaler_COORD_BITS) + 1, reg_src_rows);
				h4 = border_replicate((cur_phase >> Scaler_COORD_BITS) + 2, reg_src_rows);
				h5 = border_replicate((cur_phase >> Scaler_COORD_BITS) + 3, reg_src_rows);

				// get_hor_stream_to_buffer
				memcpy(buffer_conv_hor_y[0], Conv_Hor_Y + h0 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_conv_hor_y[1], Conv_Hor_Y + h1 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_conv_hor_y[2], Conv_Hor_Y + h2 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_conv_hor_y[3], Conv_Hor_Y + h3 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_conv_hor_y[4], Conv_Hor_Y + h4 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_conv_hor_y[5], Conv_Hor_Y + h5 * reg_dst_cols, reg_dst_cols * sizeof(ushort));

				memcpy(buffer_conv_hor_uv[0], Conv_Hor_UV + h0 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_conv_hor_uv[1], Conv_Hor_UV + h1 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_conv_hor_uv[2], Conv_Hor_UV + h2 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_conv_hor_uv[3], Conv_Hor_UV + h3 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_conv_hor_uv[4], Conv_Hor_UV + h4 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_conv_hor_uv[5], Conv_Hor_UV + h5 * reg_dst_cols, reg_dst_cols * sizeof(ushort));

				// get_ver_coef_offset
				coef_index = (cur_phase & mask) >> (Scaler_COORD_BITS - Scaler_NUM_INTER_PHASE_BITS);
				coef_offset = (coef_index << 2) + coef_index;

				// vertical_filter_onerow
				int64_t col_index = 0;
				int64_t res;
				while (col_index < reg_dst_cols)
				{
					// vertical_filter_onerow_process_y
					res = (buffer_conv_hor_y[0][col_index] - buffer_conv_hor_y[5][col_index]) * reg_ver_coef[coef_offset] +
						(buffer_conv_hor_y[1][col_index] - buffer_conv_hor_y[5][col_index]) * reg_ver_coef[coef_offset + 1] +
						(buffer_conv_hor_y[2][col_index] - buffer_conv_hor_y[5][col_index]) * reg_ver_coef[coef_offset + 2] +
						(buffer_conv_hor_y[3][col_index] - buffer_conv_hor_y[5][col_index]) * reg_ver_coef[coef_offset + 3] +
						(buffer_conv_hor_y[4][col_index] - buffer_conv_hor_y[5][col_index]) * reg_ver_coef[coef_offset + 4] +
						(buffer_conv_hor_y[5][col_index] << Scaler_COEF_BITS);
					Dst_Y[col_index] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE << 1)) >> (Scaler_COEF_BITS + 2), 0, 1023);

					res = (buffer_conv_hor_y[0][col_index + 1] - buffer_conv_hor_y[5][col_index + 1]) * reg_ver_coef[coef_offset] +
						(buffer_conv_hor_y[1][col_index + 1] - buffer_conv_hor_y[5][col_index + 1]) * reg_ver_coef[coef_offset + 1] +
						(buffer_conv_hor_y[2][col_index + 1] - buffer_conv_hor_y[5][col_index + 1]) * reg_ver_coef[coef_offset + 2] +
						(buffer_conv_hor_y[3][col_index + 1] - buffer_conv_hor_y[5][col_index + 1]) * reg_ver_coef[coef_offset + 3] +
						(buffer_conv_hor_y[4][col_index + 1] - buffer_conv_hor_y[5][col_index + 1]) * reg_ver_coef[coef_offset + 4] +
						(buffer_conv_hor_y[5][col_index + 1] << Scaler_COEF_BITS);
					Dst_Y[col_index + 1] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE << 1)) >> (Scaler_COEF_BITS + 2), 0, 1023);


					// vertical_filter_onerow_process_uv
					res = (buffer_conv_hor_uv[0][col_index] - buffer_conv_hor_uv[5][col_index]) * reg_ver_coef[coef_offset] +
						(buffer_conv_hor_uv[1][col_index] - buffer_conv_hor_uv[5][col_index]) * reg_ver_coef[coef_offset + 1] +
						(buffer_conv_hor_uv[2][col_index] - buffer_conv_hor_uv[5][col_index]) * reg_ver_coef[coef_offset + 2] +
						(buffer_conv_hor_uv[3][col_index] - buffer_conv_hor_uv[5][col_index]) * reg_ver_coef[coef_offset + 3] +
						(buffer_conv_hor_uv[4][col_index] - buffer_conv_hor_uv[5][col_index]) * reg_ver_coef[coef_offset + 4] +
						(buffer_conv_hor_uv[5][col_index] << Scaler_COEF_BITS);
					Dst_UV[col_index] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE << 1)) >> (Scaler_COEF_BITS + 2), 0, 1023);

					res = (buffer_conv_hor_uv[0][col_index + 1] - buffer_conv_hor_uv[5][col_index + 1]) * reg_ver_coef[coef_offset] +
						(buffer_conv_hor_uv[1][col_index + 1] - buffer_conv_hor_uv[5][col_index + 1]) * reg_ver_coef[coef_offset + 1] +
						(buffer_conv_hor_uv[2][col_index + 1] - buffer_conv_hor_uv[5][col_index + 1]) * reg_ver_coef[coef_offset + 2] +
						(buffer_conv_hor_uv[3][col_index + 1] - buffer_conv_hor_uv[5][col_index + 1]) * reg_ver_coef[coef_offset + 3] +
						(buffer_conv_hor_uv[4][col_index + 1] - buffer_conv_hor_uv[5][col_index + 1]) * reg_ver_coef[coef_offset + 4] +
						(buffer_conv_hor_uv[5][col_index + 1] << Scaler_COEF_BITS);
					Dst_UV[col_index + 1] = ec_clamp<int64_t>((res + (Scaler_COEF_SCALE << 1)) >> (Scaler_COEF_BITS + 2), 0, 1023);

					col_index += 2;
				}

				row_index += 1;
				Dst_Y += reg_dst_cols;
				Dst_UV += reg_dst_cols;
				cur_phase += reg_ver_deltaphase;
			}
		}
		else
		{
			while (row_index < reg_dst_rows)
			{
				int64_t col_index = 0;
				while (col_index < reg_dst_cols)
				{
					Dst_Y[col_index] = Conv_Hor_Y[col_index] >> 2;
					Dst_Y[col_index + 1] = Conv_Hor_Y[col_index + 1] >> 2;
					Dst_UV[col_index] = Conv_Hor_UV[col_index] >> 2;
					Dst_UV[col_index + 1] = Conv_Hor_UV[col_index + 1] >> 2;
					col_index += 2;
				}
				row_index += 1;
				Dst_Y += reg_dst_cols;
				Dst_UV += reg_dst_cols;
				Conv_Hor_Y += reg_dst_cols;
				Conv_Hor_UV += reg_dst_cols;
			}
		}
	}
}