#pragma once

void cl_simple_filter(uchar const* src, uchar* dst, int rows, int cols);

void spafilter(short* src, short* dst, int rows, int cols,
	uchar const* filter_th_0, uchar const* filter_th_1, uchar const* filter_th_2, uchar const* filter_th_3,
	uchar const* filter_th_power, uchar const* mae_preserve_lut, int blending_weight, int cfa_order);

 void tnr_pre(short* src, short* pre, short* spares, uchar* confidence_map, int rows, int cols,
	uchar const* filter_th_0, uchar const* filter_th_1, uchar const* filter_th_2, uchar const* filter_th_3,
	uchar const* filter_th_power, uchar const* mae_preserve_lut, int blending_weight, int cfa_order,
	ushort const* var_r, ushort const* var_g, uchar const* discount_lut, uchar const* anti_ghost_lut, int use_calivar_only);

 void tnr_post(short* src, short* pre, short* spares, uchar* confidence_map, int rows, int cols, int tw, uchar* confidence_debug);

