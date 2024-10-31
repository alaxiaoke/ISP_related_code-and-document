#include "gravityxr_grouping.hpp"

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


//==================== Grouping ====================//

Grouping::Grouping()
{
	memset(&params, 0, sizeof(params));
	stream_src_y = NULL;
	stream_src_uv = NULL;
	stream_dst_y = NULL;
	stream_dst_uv = NULL;
	stream_hordst_y = NULL;
	stream_hordst_uv = NULL;
	memset(buffer_hordst_y, 0, sizeof(buffer_hordst_y));
	memset(buffer_hordst_uv, 0, sizeof(buffer_hordst_uv));
}


Grouping::~Grouping()
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


void Grouping::init()
{
	prepare();
}


void Grouping::release()
{
}


void Grouping::parse_config(void* config_file, Grouping_PARAMS * p)
{
#define load_array(var) { if (var < sizeof(params->var) / sizeof(params->var[0])) params->var[var++] = static_cast<uint32_t>(val); }
	char key[512], valstr[6144];
	int ninput;
	int Reg_HorGroupBoundary = 0;
	int Reg_VerGroupBoundary = 0;
	int Reg_Coef = 0;

	Grouping_PARAMS* params = &this->params;
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
		else if (!strcmp(key, "hor_groupb") || !strcmp(key, "Reg_HorGroupBoundary"))
			load_array(Reg_HorGroupBoundary)
		else if (!strcmp(key, "ver_groupb") || !strcmp(key, "Reg_VerGroupBoundary"))
			load_array(Reg_VerGroupBoundary)
		else if (!strcmp(key, "coef") || !strcmp(key, "Reg_Coef"))
			load_array(Reg_Coef)
	}
#undef load_array
}


void Grouping::updata_config(Grouping_PARAMS * p)
{
	if (p)
		memcpy(&params, p, sizeof(params));
}


//==================== prepare ====================//

void Grouping::prepare()
{
	reg_enable = params.Reg_Enable;
	reg_hor_enable = params.Reg_HorEnable;
	reg_ver_enable = params.Reg_VerEnable;
	reg_src_rows = params.Reg_SrcHeight;
	reg_src_cols = params.Reg_SrcWidth;
	reg_dst_rows = params.Reg_DstHeight;
	reg_dst_cols = params.Reg_DstWidth;
	for (int i = 0; i < 4; i++)
	{
		reg_hor_group_boundary[i] = params.Reg_HorGroupBoundary[i];
		reg_ver_group_boundary[i] = params.Reg_VerGroupBoundary[i];
	}

	int len = sizeof(params.Reg_Coef) / sizeof(params.Reg_Coef[0]);
	for (int i = 0; i < len; i++)
		reg_coef[i] = params.Reg_Coef[i];

	assert(reg_src_cols % 2 == 0 && reg_dst_cols % 2 == 0);
	if (!reg_enable)
		assert(reg_src_cols == reg_dst_cols && reg_src_rows == reg_dst_rows);
	if (reg_enable && !reg_hor_enable)
		assert(reg_src_cols == reg_dst_cols);
	if (reg_enable && !reg_ver_enable)
		assert(reg_src_rows == reg_dst_rows);

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
	buffer_hordst_y[0] = (ushort*)malloc(Grouping_INTER_NUM_TAPS_R4 * reg_dst_cols * sizeof(ushort));
	buffer_hordst_uv[0] = (ushort*)malloc(Grouping_INTER_NUM_TAPS_R4 * reg_dst_cols * sizeof(ushort));
	for (int i = 1; i < Grouping_INTER_NUM_TAPS_R4; i++)
	{
		buffer_hordst_y[i] = buffer_hordst_y[i - 1] + reg_dst_cols;
		buffer_hordst_uv[i] = buffer_hordst_uv[i - 1] + reg_dst_cols;
	}
}


//==================== process ====================//

int64_t Grouping::select_groupid(int64_t index, int64_t group_boundary[4])
{
	int64_t groupid;
	if (index < group_boundary[0])
		groupid = 0;
	else if (index < group_boundary[1])
		groupid = 1;
	else if (index < group_boundary[2])
		groupid = 2;
	else if (index < group_boundary[3])
		groupid = 1;
	else
		groupid = 0;
	return groupid;
}


int64_t* Grouping::select_coef(int64_t groupid)
{
	assert(groupid == 0 || groupid == 1 || groupid == 2);
	if (groupid == 0)
		return reg_coef;
	else if (groupid == 1)
		return reg_coef + Grouping_INTER_NUM_TAPS_R4 - 1;
}


int64_t Grouping::select_step(int64_t groupid)
{
	assert(groupid == 0 || groupid == 1 || groupid == 2);
	if (groupid == 0)
		return 4;
	else if (groupid == 1)
		return 2;
	else if (groupid == 2)
		return 1;
}


void Grouping::hor_grouping()
{
	// horizontal_grouping
	int64_t row_index = 0;
	ushort* Src_Y = stream_src_y;
	ushort* Src_UV = stream_src_uv;
	ushort* HorDst_Y = stream_hordst_y;
	ushort* HorDst_UV = stream_hordst_uv;

	while (row_index < reg_src_rows)
	{
		// horizontal_grouping_onerow
		int64_t col_index = 0;
		int64_t coord = 0;
		int64_t group_id;
		int64_t* coef = NULL;
		int64_t step;
		int64_t w0, w1, w2, w3, w4;
		int64_t res;

		while (col_index < reg_dst_cols)
		{
			if (reg_hor_enable)
			{
				group_id = select_groupid(col_index, reg_hor_group_boundary);
				coef = select_coef(group_id);
				step = select_step(group_id);

				if (group_id == 0)
				{
					// process y
					w0 = border_replicate(coord - 2, reg_src_cols);
					w1 = border_replicate(coord - 1, reg_src_cols);
					w2 = border_replicate(coord, reg_src_cols);
					w3 = border_replicate(coord + 1, reg_src_cols);
					w4 = border_replicate(coord + 2, reg_src_cols);
					res = (Src_Y[w0] - Src_Y[w4]) * coef[0] +
						(Src_Y[w1] - Src_Y[w4]) * coef[1] +
						(Src_Y[w2] - Src_Y[w4]) * coef[2] +
						(Src_Y[w3] - Src_Y[w4]) * coef[3] +
						(Src_Y[w4] << Grouping_INTER_COEF_BITS);
					HorDst_Y[col_index] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE >> 3)) >> (Grouping_INTER_COEF_BITS - 2), 0, 4095);

					w0 = border_replicate(coord + step - 2, reg_src_cols);
					w1 = border_replicate(coord + step - 1, reg_src_cols);
					w2 = border_replicate(coord + step, reg_src_cols);
					w3 = border_replicate(coord + step + 1, reg_src_cols);
					w4 = border_replicate(coord + step + 2, reg_src_cols);
					res = (Src_Y[w0] - Src_Y[w4]) * coef[0] +
						(Src_Y[w1] - Src_Y[w4]) * coef[1] +
						(Src_Y[w2] - Src_Y[w4]) * coef[2] +
						(Src_Y[w3] - Src_Y[w4]) * coef[3] +
						(Src_Y[w4] << Grouping_INTER_COEF_BITS);
					HorDst_Y[col_index + 1] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE >> 3)) >> (Grouping_INTER_COEF_BITS - 2), 0, 4095);

					// process uv
					w0 = border_replicate((coord >> 1) - 2, reg_src_cols >> 1) << 1;
					w1 = border_replicate((coord >> 1) - 1, reg_src_cols >> 1) << 1;
					w2 = border_replicate(coord >> 1, reg_src_cols >> 1) << 1;
					w3 = border_replicate((coord >> 1) + 1, reg_src_cols >> 1) << 1;
					w4 = border_replicate((coord >> 1) + 2, reg_src_cols >> 1) << 1;
					res = (Src_UV[w0] - Src_UV[w4]) * coef[0] +
						(Src_UV[w1] - Src_UV[w4]) * coef[1] +
						(Src_UV[w2] - Src_UV[w4]) * coef[2] +
						(Src_UV[w3] - Src_UV[w4]) * coef[3] +
						(Src_UV[w4] << Grouping_INTER_COEF_BITS);
					HorDst_UV[col_index] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE >> 3)) >> (Grouping_INTER_COEF_BITS - 2), 0, 4095);
					res = (Src_UV[w0 + 1] - Src_UV[w4 + 1]) * coef[0] +
						(Src_UV[w1 + 1] - Src_UV[w4 + 1]) * coef[1] +
						(Src_UV[w2 + 1] - Src_UV[w4 + 1]) * coef[2] +
						(Src_UV[w3 + 1] - Src_UV[w4 + 1]) * coef[3] +
						(Src_UV[w4 + 1] << Grouping_INTER_COEF_BITS);
					HorDst_UV[col_index + 1] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE >> 3)) >> (Grouping_INTER_COEF_BITS - 2), 0, 4095);
				}
				else if (group_id == 1)
				{
					// process y
					w0 = border_replicate(coord - 1, reg_src_cols);
					w1 = border_replicate(coord, reg_src_cols);
					w2 = border_replicate(coord + 1, reg_src_cols);
					res = (Src_Y[w0] - Src_Y[w2]) * coef[0] +
						(Src_Y[w1] - Src_Y[w2]) * coef[1] +
						(Src_Y[w2] << Grouping_INTER_COEF_BITS);
					HorDst_Y[col_index] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE >> 3)) >> (Grouping_INTER_COEF_BITS - 2), 0, 4095);

					w0 = border_replicate(coord + step - 1, reg_src_cols);
					w1 = border_replicate(coord + step, reg_src_cols);
					w2 = border_replicate(coord + step + 1, reg_src_cols);
					res = (Src_Y[w0] - Src_Y[w2]) * coef[0] +
						(Src_Y[w1] - Src_Y[w2]) * coef[1] +
						(Src_Y[w2] << Grouping_INTER_COEF_BITS);
					HorDst_Y[col_index + 1] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE >> 3)) >> (Grouping_INTER_COEF_BITS - 2), 0, 4095);

					// process uv
					w0 = border_replicate((coord >> 1) - 1, reg_src_cols >> 1) << 1;
					w1 = border_replicate(coord >> 1, reg_src_cols >> 1) << 1;
					w2 = border_replicate((coord >> 1) + 1, reg_src_cols >> 1) << 1;
					res = (Src_UV[w0] - Src_UV[w2]) * coef[0] +
						(Src_UV[w1] - Src_UV[w2]) * coef[1] +
						(Src_UV[w2] << Grouping_INTER_COEF_BITS);
					HorDst_UV[col_index] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE >> 3)) >> (Grouping_INTER_COEF_BITS - 2), 0, 4095);
					res = (Src_UV[w0 + 1] - Src_UV[w2 + 1]) * coef[0] +
						(Src_UV[w1 + 1] - Src_UV[w2 + 1]) * coef[1] +
						(Src_UV[w2 + 1] << Grouping_INTER_COEF_BITS);
					HorDst_UV[col_index + 1] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE >> 3)) >> (Grouping_INTER_COEF_BITS - 2), 0, 4095);
				}
				else if (group_id == 2)
				{
					w0 = border_replicate(coord, reg_src_cols);
					HorDst_Y[col_index] = Src_Y[w0] << 2;
					w0 = border_replicate(coord + step, reg_src_cols);
					HorDst_Y[col_index + 1] = Src_Y[w0] << 2;

					w0 = border_replicate(coord >> 1, reg_src_cols >> 1) << 1;
					HorDst_UV[col_index] = Src_UV[w0] << 2;
					HorDst_UV[col_index + 1] = Src_UV[w0 + 1] << 2;
				}
				col_index += 2;
				coord += (step << 1);
			}
			else
			{
				HorDst_Y[col_index] = Src_Y[col_index] << 2;
				HorDst_Y[col_index + 1] = Src_Y[col_index + 1] << 2;
				HorDst_UV[col_index] = Src_UV[col_index] << 2;
				HorDst_UV[col_index + 1] = Src_UV[col_index + 1] << 2;
				col_index += 2;
			}
		}

		row_index += 1;
		Src_Y += reg_src_cols;
		Src_UV += reg_src_cols;
		HorDst_Y += reg_dst_cols;
		HorDst_UV += reg_dst_cols;
	}
}


void Grouping::ver_grouping()
{
	// vertical_grouping
	ushort* Dst_Y = stream_dst_y;
	ushort* Dst_UV = stream_dst_uv;
	ushort* HorDst_Y = stream_hordst_y;
	ushort* HorDst_UV = stream_hordst_uv;
	int64_t row_index = 0;
	int64_t coord = 0;
	int64_t group_id;
	int64_t* coef = NULL;
	int64_t step;
	int64_t h0, h1, h2, h3, h4;
	if (reg_ver_enable)
	{
		while (row_index < reg_dst_rows)
		{
			group_id = select_groupid(row_index, reg_ver_group_boundary);
			coef = select_coef(group_id);
			step = select_step(group_id);

			if (group_id == 0)
			{
				h0 = border_replicate(coord - 2, reg_src_rows);
				h1 = border_replicate(coord - 1, reg_src_rows);
				h2 = border_replicate(coord, reg_src_rows);
				h3 = border_replicate(coord + 1, reg_src_rows);
				h4 = border_replicate(coord + 2, reg_src_rows);

				// get_hor_stream_to_buffer
				memcpy(buffer_hordst_y[0], HorDst_Y + h0 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_y[1], HorDst_Y + h1 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_y[2], HorDst_Y + h2 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_y[3], HorDst_Y + h3 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_y[4], HorDst_Y + h4 * reg_dst_cols, reg_dst_cols * sizeof(ushort));

				memcpy(buffer_hordst_uv[0], HorDst_UV + h0 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_uv[1], HorDst_UV + h1 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_uv[2], HorDst_UV + h2 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_uv[3], HorDst_UV + h3 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_uv[4], HorDst_UV + h4 * reg_dst_cols, reg_dst_cols * sizeof(ushort));

				// vertical_filter_onerow
				int64_t col_index = 0;
				int64_t res;
				while (col_index < reg_dst_cols)
				{
					// vertical_filter_onerow_process_y
					res = (buffer_hordst_y[0][col_index] - buffer_hordst_y[4][col_index]) * coef[0] +
						(buffer_hordst_y[1][col_index] - buffer_hordst_y[4][col_index]) * coef[1] +
						(buffer_hordst_y[2][col_index] - buffer_hordst_y[4][col_index]) * coef[2] +
						(buffer_hordst_y[3][col_index] - buffer_hordst_y[4][col_index]) * coef[3] +
						(buffer_hordst_y[4][col_index] << Grouping_INTER_COEF_BITS);
					Dst_Y[col_index] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE << 1)) >> (Grouping_INTER_COEF_BITS + 2), 0, 1023);

					// vertical_filter_onerow_process_y
					res = (buffer_hordst_y[0][col_index + 1] - buffer_hordst_y[4][col_index + 1]) * coef[0] +
						(buffer_hordst_y[1][col_index + 1] - buffer_hordst_y[4][col_index + 1]) * coef[1] +
						(buffer_hordst_y[2][col_index + 1] - buffer_hordst_y[4][col_index + 1]) * coef[2] +
						(buffer_hordst_y[3][col_index + 1] - buffer_hordst_y[4][col_index + 1]) * coef[3] +
						(buffer_hordst_y[4][col_index + 1] << Grouping_INTER_COEF_BITS);
					Dst_Y[col_index + 1] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE << 1)) >> (Grouping_INTER_COEF_BITS + 2), 0, 1023);

					// vertical_filter_onerow_process_uv
					res = (buffer_hordst_uv[0][col_index] - buffer_hordst_uv[4][col_index]) * coef[0] +
						(buffer_hordst_uv[1][col_index] - buffer_hordst_uv[4][col_index]) * coef[1] +
						(buffer_hordst_uv[2][col_index] - buffer_hordst_uv[4][col_index]) * coef[2] +
						(buffer_hordst_uv[3][col_index] - buffer_hordst_uv[4][col_index]) * coef[3] +
						(buffer_hordst_uv[4][col_index] << Grouping_INTER_COEF_BITS);
					Dst_UV[col_index] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE << 1)) >> (Grouping_INTER_COEF_BITS + 2), 0, 1023);

					// vertical_filter_onerow_process_uv
					res = (buffer_hordst_uv[0][col_index + 1] - buffer_hordst_uv[4][col_index + 1]) * coef[0] +
						(buffer_hordst_uv[1][col_index + 1] - buffer_hordst_uv[4][col_index + 1]) * coef[1] +
						(buffer_hordst_uv[2][col_index + 1] - buffer_hordst_uv[4][col_index + 1]) * coef[2] +
						(buffer_hordst_uv[3][col_index + 1] - buffer_hordst_uv[4][col_index + 1]) * coef[3] +
						(buffer_hordst_uv[4][col_index + 1] << Grouping_INTER_COEF_BITS);
					Dst_UV[col_index + 1] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE << 1)) >> (Grouping_INTER_COEF_BITS + 2), 0, 1023);

					col_index += 2;
				}
			}
			else if (group_id == 1)
			{
				h0 = border_replicate(coord - 1, reg_src_rows);
				h1 = border_replicate(coord, reg_src_rows);
				h2 = border_replicate(coord + 1, reg_src_rows);

				// get_hor_stream_to_buffer
				memcpy(buffer_hordst_y[0], HorDst_Y + h0 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_y[1], HorDst_Y + h1 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_y[2], HorDst_Y + h2 * reg_dst_cols, reg_dst_cols * sizeof(ushort));

				memcpy(buffer_hordst_uv[0], HorDst_UV + h0 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_uv[1], HorDst_UV + h1 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_uv[2], HorDst_UV + h2 * reg_dst_cols, reg_dst_cols * sizeof(ushort));

				// vertical_filter_onerow
				int64_t col_index = 0;
				int64_t res;
				while (col_index < reg_dst_cols)
				{
					// vertical_filter_onerow_process_y
					res = (buffer_hordst_y[0][col_index] - buffer_hordst_y[2][col_index]) * coef[0] +
						(buffer_hordst_y[1][col_index] - buffer_hordst_y[2][col_index]) * coef[1] +
						(buffer_hordst_y[2][col_index] << Grouping_INTER_COEF_BITS);
					Dst_Y[col_index] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE << 1)) >> (Grouping_INTER_COEF_BITS + 2), 0, 1023);

					// vertical_filter_onerow_process_y
					res = (buffer_hordst_y[0][col_index + 1] - buffer_hordst_y[2][col_index + 1]) * coef[0] +
						(buffer_hordst_y[1][col_index + 1] - buffer_hordst_y[2][col_index + 1]) * coef[1] +
						(buffer_hordst_y[2][col_index + 1] << Grouping_INTER_COEF_BITS);
					Dst_Y[col_index + 1] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE << 1)) >> (Grouping_INTER_COEF_BITS + 2), 0, 1023);

					// vertical_filter_onerow_process_uv
					res = (buffer_hordst_uv[0][col_index] - buffer_hordst_uv[2][col_index]) * coef[0] +
						(buffer_hordst_uv[1][col_index] - buffer_hordst_uv[2][col_index]) * coef[1] +
						(buffer_hordst_uv[2][col_index] << Grouping_INTER_COEF_BITS);
					Dst_UV[col_index] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE << 1)) >> (Grouping_INTER_COEF_BITS + 2), 0, 1023);

					// vertical_filter_onerow_process_uv
					res = (buffer_hordst_uv[0][col_index + 1] - buffer_hordst_uv[2][col_index + 1]) * coef[0] +
						(buffer_hordst_uv[1][col_index + 1] - buffer_hordst_uv[2][col_index + 1]) * coef[1] +
						(buffer_hordst_uv[2][col_index + 1] << Grouping_INTER_COEF_BITS);
					Dst_UV[col_index + 1] = ec_clamp<int64_t>((res + (Grouping_INTER_COEF_SCALE << 1)) >> (Grouping_INTER_COEF_BITS + 2), 0, 1023);

					col_index += 2;
				}
			}
			else if (group_id == 2)
			{
				h0 = border_replicate(coord, reg_src_rows);

				// get_hor_stream_to_buffer
				memcpy(buffer_hordst_y[0], HorDst_Y + h0 * reg_dst_cols, reg_dst_cols * sizeof(ushort));
				memcpy(buffer_hordst_uv[0], HorDst_UV + h0 * reg_dst_cols, reg_dst_cols * sizeof(ushort));

				int64_t col_index = 0;
				while (col_index < reg_dst_cols)
				{
					Dst_Y[col_index] = buffer_hordst_y[0][col_index] >> 2;
					Dst_Y[col_index + 1] = buffer_hordst_y[0][col_index + 1] >> 2;
					Dst_UV[col_index] = buffer_hordst_uv[0][col_index] >> 2;
					Dst_UV[col_index + 1] = buffer_hordst_uv[0][col_index + 1] >> 2;
					col_index += 2;
				}
			}

			row_index += 1;
			coord += step;
			Dst_Y += reg_dst_cols;
			Dst_UV += reg_dst_cols;
		}
	}
	else
	{
		while (row_index < reg_dst_rows)
		{
			int64_t col_index = 0;
			while (col_index < reg_dst_cols)
			{
				Dst_Y[col_index] = HorDst_Y[col_index] >> 2;
				Dst_UV[col_index] = HorDst_UV[col_index] >> 2;
				col_index += 1;
			}
			row_index += 1;
			Dst_Y += reg_dst_cols;
			Dst_UV += reg_dst_cols;
			HorDst_Y += reg_dst_cols;
			HorDst_UV += reg_dst_cols;
		}
	}
}


void Grouping::process(void* src, void* dst, void* p)
{
	if (p)
	{
		updata_config(static_cast<Grouping_PARAMS*>(p));
		prepare();
	}

	stream_src_y = static_cast<ushort*>(src);
	stream_src_uv = stream_src_y + reg_src_rows * reg_src_cols;
	stream_dst_y = static_cast<ushort*>(dst);
	stream_dst_uv = stream_dst_y + reg_dst_rows * reg_dst_cols;

	if (!reg_enable)
		memcpy(dst, src, reg_src_rows * reg_src_cols * 2 * sizeof(ushort));
	else
	{
		hor_grouping();
		ver_grouping();
	}
}