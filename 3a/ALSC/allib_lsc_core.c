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

/*******************************************************************************
 *                               Include File                                   *
 *******************************************************************************/
#include "allib_LSC.h" 
#include "allib_lsc_core.h"
#include "alLSCLib_Error_Code.h" //"allib_lsc_error_code.h"
#include "alwrapper.h"

//#define NULL                                    ((void*)0)
#define ABS(a)                                  ((a) > 0 ? (a) : (-(a)))
#define MAX(a,b)                                ((a) > (b) ? (a) : (b))
#define MIN(a,b)                                ((a) > (b) ? (b) : (a))

/*******************************************************************************
 *                              Private Global Variable                         *
 *******************************************************************************/

static LSC_RUNTIME_DATA g_LSC_working_buffer; //global array in bss region (not set initial)

uint32_t alLSCLib_getlib_runtimebuf_size(void) {
	return sizeof(LSC_RUNTIME_DATA);
}

void alLSCLib_getlib_version(struct alLSClib_lib_version_t* lib_version) {
	lib_version->major_version = LSC_VERSION_MAJOR;
	lib_version->minor_version = LSC_VERSION_MINOR;
}

void alLSClib_add_header(void* ptr) {
	LSC_HEADER* header = (LSC_HEADER*) ptr;

	strncpy(&header->header_tag[0], LSC_HEADER_TAG, strlen(LSC_HEADER_TAG));

	header->total_file_size = sizeof(LSC_SCENARIO_TUNING_DATA);

	header->total_file_size_exclude_finetuneable = header->total_file_size;
	strncpy(&header->project_name[0], LSC_PROJECT_NAME,
			strlen(LSC_PROJECT_NAME));
	strncpy(&header->sensor_name[0], LSC_SENSOR_NAME, strlen(LSC_SENSOR_NAME));
	strncpy(&header->module_house_name[0], LSC_MODULE_HOUSE_NAME,
			strlen(LSC_MODULE_HOUSE_NAME));
	header->sensor_id = 0;
	header->input_table_tool_version_number = 0;
	//strncpy(&header->version_date_time[0], LSC_VERSION_DATETIME, strlen(LSC_VERSION_DATETIME));

	header->calibration_version = 0;
	header->table_check_sum = 0;
	header->OTP_LSC_size = 0;
	header->shading_table_width = LSC_TABLE_W;
	header->shading_table_height = LSC_TABLE_H;
}

void alLSClib_initial_tuning_data(void* ptr) {
	LSC_TUNING_INFO* tuning = (LSC_TUNING_INFO*) ptr;
	uint32_t i = 0, j = 0;

	tuning->y_mean_threshold = 300;

	uint16_t X_node[ISO_NODE_NUM] = DEFAULT_X_NODES;
	uint16_t Y_node[ISO_NODE_NUM] = DEFAULT_Y_DOWN_RATIO;
	for (i = 0; i < ISO_NODE_NUM; i++) {
		tuning->x_iso_nodes[i] = X_node[i];
		tuning->y_iso_values[i] = Y_node[i];
	}

	tuning->default_iso_calibration.min_iso = 804;
	tuning->default_iso_calibration.gain_r = DEFAULT_CALIBRATION_GAIN;
	tuning->default_iso_calibration.gain_g = DEFAULT_CALIBRATION_GAIN;
	tuning->default_iso_calibration.gain_b = DEFAULT_CALIBRATION_GAIN;
	tuning->default_iso_calibration.fn = 2000;

	tuning->bv_threshold = -7500;
	tuning->iso_step = 50;

	//set table default
	uint16_t default_ct[5] = DEFAULT_CT_VALUE;
	for (i = 0; i < (int) LIGHT_SOURCE_CT_NUMBER; i++) {
		tuning->shading_tables[i].color_temperature = default_ct[i];
		for (j = 0; j < LSC_TABLE_SIZE; j++) {
			tuning->shading_tables[i].shading_table_r[j] = SHAD_BASE;
			tuning->shading_tables[i].shading_table_gr[j] = SHAD_BASE;
			tuning->shading_tables[i].shading_table_gb[j] = SHAD_BASE;
			tuning->shading_tables[i].shading_table_b[j] = SHAD_BASE;
		}
	}
}

//if need set default LSC rundata info, set here
void _load_default_parameters_lsc(LSC_RUNTIME_DATA* runtime_data) {
	alLSClib_add_header(&runtime_data->header);
	alLSClib_initial_tuning_data(&runtime_data->tuning_info);
}

uint32_t alLSCLib_init(void** lsc_initial_buffer) {
	LSC_RUNTIME_DATA* runtime_data = &g_LSC_working_buffer;

	if ((void*) 0 == runtime_data) {
		return _AL_LSCLIB_INIT_BUFFER_NULL;
	}

	*lsc_initial_buffer = runtime_data;

	memset(runtime_data, 0, alLSCLib_getlib_runtimebuf_size());

	_load_default_parameters_lsc(runtime_data);

	//set runtime default value
	runtime_data->ping_pong_index = 0;
	runtime_data->previous_iso = INITIAL_PREVIOUS_ISO;

	return _AL_LSCLIB_SUCCESS;
}

uint32_t alLSCLib_deinit(void* lsc_obj) {
	if ((void*) 0 == lsc_obj) {
		return _AL_LSCLIB_DEINIT_NULL_OBJ;
	}

	memset(lsc_obj, 0, alLSCLib_getlib_runtimebuf_size());

	return _AL_LSCLIB_SUCCESS;
}

uint32_t set_lsc_tunning_info(void* tunning_file, void* lsc_obj) {
	if ((void*) 0 == tunning_file)
		return _AL_LSCLIB_INVALID_TUNING_FILE_NULL;

	LSC_RUNTIME_DATA* runtime_data = (LSC_RUNTIME_DATA*) lsc_obj;

	memcpy(&runtime_data->header, tunning_file, sizeof(LSC_HEADER));

	if (LSC_TUNING_STRUCT_VERSION != runtime_data->header.struct_version) {
		return _AL_LSCLIB_INVALID_TUNING_FILE_VERSION;
	}

	LSC_SCENARIO_TUNING_DATA* tuning_data =
			(LSC_SCENARIO_TUNING_DATA*) tunning_file;
	memcpy(&runtime_data->tuning_info, &tuning_data->tuning_info,
			sizeof(LSC_TUNING_INFO));

	return _AL_LSCLIB_SUCCESS;
}

uint32_t alLSCLib_set_param(struct alLSCLib_set_parameter_t* a_ptParam,
		void* lsc_obj) {
	LSC_RUNTIME_DATA* runtime_data = (LSC_RUNTIME_DATA*) lsc_obj;

	if ((void*) 0 == a_ptParam || (void*) 0 == lsc_obj) {
		return _AL_LSCLIB_SET_PARAM_INPUT_NULL;
	}

	switch (a_ptParam->type) {
	default:
		return _AL_LSCLIB_SET_PARAM_INPUT_TYPE;
	case SET_PARAM_TYPE_LSC_TUNING_BIN:
		if ((void*) 0 != a_ptParam->para.tuning_data) {
			uint32_t err = set_lsc_tunning_info(a_ptParam->para.tuning_data,
					lsc_obj);
			if (_AL_LSCLIB_SUCCESS != err)
				return err;
		}
		break;
	case SET_PARAM_TYPE_LSC_HW_INIT_PARA:
		runtime_data->shading_bypass =
				a_ptParam->para.hw_init_para.shade_bypass;
		break;
	case SET_PARAM_TYPE_LSC_DYNAMIC_PROC_PARA:
		runtime_data->current_bv = a_ptParam->para.dynamic_proc_para.current_bv;
		runtime_data->iso_speed = a_ptParam->para.dynamic_proc_para.iso_speed;
		runtime_data->y_mean = a_ptParam->para.dynamic_proc_para.y_mean;
		runtime_data->interpolate_ct =
				a_ptParam->para.dynamic_proc_para.interpolate_ct;
		break;
	case SET_PARAM_TYPE_LSC_COLOR_ORDER:
		runtime_data->color_order = a_ptParam->para.color_order;
		break;
	}

	return _AL_LSCLIB_SUCCESS;
}

uint32_t alLSCLib_get_param(struct alLSCLib_get_parameter_t* a_ptParam,
		void* lsc_obj) {
	LSC_RUNTIME_DATA* runtime_data = (LSC_RUNTIME_DATA*) lsc_obj;

	if ((void*) 0 == a_ptParam || (void*) 0 == lsc_obj) {
		return _AL_LSCLIB_GET_PARAM_INPUT_NULL;
	}

	switch (a_ptParam->type) {
	default:
		return _AL_LSCLIB_GET_PARAM_INPUT_TYPE;
	case GET_PARAM_TYPE_LSC_HW_INIT_PARA:
		a_ptParam->para.hw_init_para.shade_bypass =
				runtime_data->shading_bypass;
		break;
	case GET_PARAM_TYPE_LSC_DYNAMIC_PROC_PARA:
		a_ptParam->para.dynamic_proc_para.current_bv = runtime_data->current_bv;
		a_ptParam->para.dynamic_proc_para.iso_speed = runtime_data->iso_speed;
		a_ptParam->para.dynamic_proc_para.y_mean = runtime_data->y_mean;
		a_ptParam->para.dynamic_proc_para.interpolate_ct =
				runtime_data->interpolate_ct;
		break;
	case GET_PARAM_TYPE_DEFAULT_ISO:
		memcpy(&a_ptParam->para.default_ISO,
				&runtime_data->tuning_info.default_iso_calibration,
				sizeof(struct alLSCLib_default_iso_t));
		break;
	case GET_PARAM_TYPE_LSC_COLOR_ORDER:
		a_ptParam->para.color_order = runtime_data->color_order;
		break;
	}

	return _AL_LSCLIB_SUCCESS;
}

//Using different light source shading table do interpolate
void interpolate_LSC_tables(LSC_RUNTIME_DATA* runtime_data,
		LSC_RESULT_TABLE* ping_pong) {
	int input_CT = runtime_data->interpolate_ct;
	uint16_t base = 1024;
	uint16_t w_s = 0, w_e = 0;
	uint16_t ct_s = 1, ct_e = 12800;
	uint16_t idx_s = 0, idx_e = 0;
	uint16_t* ping_pong_R = &ping_pong->shading_table_r[0];
	uint16_t* ping_pong_Gr = &ping_pong->shading_table_gr[0];
	uint16_t* ping_pong_Gb = &ping_pong->shading_table_gb[0];
	uint16_t* ping_pong_B = &ping_pong->shading_table_b[0];
	uint16_t i, value_num = LSC_TABLE_W * LSC_TABLE_H;

	memset(&runtime_data->debug_info, 0, sizeof(LSC_DEBUG));

	for (i = 0; i < (uint16_t) LIGHT_SOURCE_CT_NUMBER; i++) {
		if (runtime_data->tuning_info.shading_tables[i].color_temperature
				<= input_CT
				&& ct_s
						< runtime_data->tuning_info.shading_tables[i].color_temperature) {
			ct_s =
					runtime_data->tuning_info.shading_tables[i].color_temperature;
			idx_s = i;
			w_s = 1024;
		}

		if (input_CT
				<= runtime_data->tuning_info.shading_tables[i].color_temperature
				&& runtime_data->tuning_info.shading_tables[i].color_temperature
						< ct_e) {
			ct_e =
					runtime_data->tuning_info.shading_tables[i].color_temperature;
			idx_e = i;
			w_e = 1024;
		}
	}

	if (1024 == w_s && 1024 == w_e && ct_e != ct_s) {
		uint32_t r = 0, gr = 0, gb = 0, b = 0;
		uint32_t total_r = 0, total_gr = 0, total_gb = 0, total_b = 0;

		w_e = base * (runtime_data->interpolate_ct - ct_s) / (ct_e - ct_s);
		w_s = base - w_e;
		for (i = 0; i < value_num; i++) {
			total_r = total_gr = total_gb = total_b = 0;

			total_r +=
					runtime_data->tuning_info.shading_tables[idx_s].shading_table_r[i]
							* w_s;
			total_gr +=
					runtime_data->tuning_info.shading_tables[idx_s].shading_table_gr[i]
							* w_s;
			total_gb +=
					runtime_data->tuning_info.shading_tables[idx_s].shading_table_gb[i]
							* w_s;
			total_b +=
					runtime_data->tuning_info.shading_tables[idx_s].shading_table_b[i]
							* w_s;

			total_r +=
					runtime_data->tuning_info.shading_tables[idx_e].shading_table_r[i]
							* w_e;
			total_gr +=
					runtime_data->tuning_info.shading_tables[idx_e].shading_table_gr[i]
							* w_e;
			total_gb +=
					runtime_data->tuning_info.shading_tables[idx_e].shading_table_gb[i]
							* w_e;
			total_b +=
					runtime_data->tuning_info.shading_tables[idx_e].shading_table_b[i]
							* w_e;

			//set to ping pong buffer
			r = (uint16_t) (total_r >> 10);
			gr = (uint16_t) (total_gr >> 10);
			gb = (uint16_t) (total_gb >> 10);
			b = (uint16_t) (total_b >> 10);

			*(ping_pong_R + i) = MAX(SHAD_BASE, MIN(SHAD_MAX, r));
			*(ping_pong_Gr + i) = MAX(SHAD_BASE, MIN(SHAD_MAX, gr));
			*(ping_pong_Gb + i) = MAX(SHAD_BASE, MIN(SHAD_MAX, gb));
			*(ping_pong_B + i) = MAX(SHAD_BASE, MIN(SHAD_MAX, b));
		}
	} else if (1024 == w_s) {
		memcpy(ping_pong_R,
				&runtime_data->tuning_info.shading_tables[idx_s].shading_table_r[0],
				LSC_TABLE_SIZE * sizeof(uint16_t));
		memcpy(ping_pong_Gr,
				&runtime_data->tuning_info.shading_tables[idx_s].shading_table_gr[0],
				LSC_TABLE_SIZE * sizeof(uint16_t));
		memcpy(ping_pong_Gb,
				&runtime_data->tuning_info.shading_tables[idx_s].shading_table_gb[0],
				LSC_TABLE_SIZE * sizeof(uint16_t));
		memcpy(ping_pong_B,
				&runtime_data->tuning_info.shading_tables[idx_s].shading_table_b[0],
				LSC_TABLE_SIZE * sizeof(uint16_t));
		w_e = 0;
	} else {
		memcpy(ping_pong_R,
				&runtime_data->tuning_info.shading_tables[idx_e].shading_table_r[0],
				LSC_TABLE_SIZE * sizeof(uint16_t));
		memcpy(ping_pong_Gr,
				&runtime_data->tuning_info.shading_tables[idx_e].shading_table_gr[0],
				LSC_TABLE_SIZE * sizeof(uint16_t));
		memcpy(ping_pong_Gb,
				&runtime_data->tuning_info.shading_tables[idx_e].shading_table_gb[0],
				LSC_TABLE_SIZE * sizeof(uint16_t));
		memcpy(ping_pong_B,
				&runtime_data->tuning_info.shading_tables[idx_e].shading_table_b[0],
				LSC_TABLE_SIZE * sizeof(uint16_t));
		w_s = 0;
	}

	runtime_data->debug_info.interpolate_ct = input_CT;
	runtime_data->debug_info.interpolate_index_low = idx_s;
	runtime_data->debug_info.interpolate_index_high = idx_e;
	runtime_data->debug_info.interpolate_weight_low = w_s;
	runtime_data->debug_info.interpolate_weight_high = w_e;
}

//in order to prevent screen flicker, ISO need to decrease smooth.
uint16_t get_smooth_ISO(LSC_RUNTIME_DATA* runtime_data) {
	uint16_t smooth_ISO = runtime_data->previous_iso;
	uint16_t ae_ISO_speed = runtime_data->iso_speed;
	uint16_t ISO_smooth_decrease_value = runtime_data->tuning_info.iso_step;
	uint32_t diff = ABS(ae_ISO_speed - runtime_data->previous_iso);

	if (diff > CONVERGENCE_THRESHOLD
			&& ae_ISO_speed != runtime_data->previous_iso) {
		if (diff <= ISO_smooth_decrease_value)
			smooth_ISO = ae_ISO_speed;
		else if (ae_ISO_speed > runtime_data->previous_iso)
			smooth_ISO = runtime_data->previous_iso + ISO_smooth_decrease_value;
		else
			smooth_ISO = runtime_data->previous_iso - ISO_smooth_decrease_value;
	}

	runtime_data->previous_iso = smooth_ISO;
	return smooth_ISO;
}

//High ISO Decrease Shading Gain Run
void high_ISO_decrease_gain_node_interpolate(LSC_RUNTIME_DATA* runtime_data,
		uint16_t ISO_speed, LSC_RESULT_TABLE* ping_pong) {
	uint32_t i = 0;
	uint16_t index = 0;
	uint32_t gain_down_ratio = 0;
	uint16_t left_X_node = 0, right_X_node = 0;
	uint16_t left_Y_ratio = 0, right_Y_ratio = 0;
	uint16_t* ping_pong_R = &ping_pong->shading_table_r[0];
	uint16_t* ping_pong_Gr = &ping_pong->shading_table_gr[0];
	uint16_t* ping_pong_Gb = &ping_pong->shading_table_gb[0];
	uint16_t* ping_pong_B = &ping_pong->shading_table_b[0];

	for (i = 0; i < ISO_NODE_NUM; i++) {
		if (ISO_speed >= runtime_data->tuning_info.x_iso_nodes[i])
			index = i;
	}

	if (index == ISO_NODE_NUM - 1)
		gain_down_ratio = runtime_data->tuning_info.y_iso_values[index];
	else {
		left_X_node = runtime_data->tuning_info.x_iso_nodes[index];
		right_X_node = runtime_data->tuning_info.x_iso_nodes[index + 1];
		left_Y_ratio = runtime_data->tuning_info.y_iso_values[index];
		right_Y_ratio = runtime_data->tuning_info.y_iso_values[index + 1];

		gain_down_ratio = left_Y_ratio * (right_X_node - ISO_speed)
				+ right_Y_ratio * (ISO_speed - left_X_node);
		gain_down_ratio /= (right_X_node - left_X_node);
	}

	//If gain ratio equal to gain base, not do ISO decrease gain, optimize performance
	uint8_t offset = 10;
	int gain_base = 1 << offset;
	if (gain_down_ratio == gain_base)
		return;

	for (i = 0; i < LSC_TABLE_SIZE; i++) {
		if (*(ping_pong_Gr + i) <= SHAD_BASE || *(ping_pong_Gb + i) <= SHAD_BASE) {
			*(ping_pong_R + i) = SHAD_BASE;
			*(ping_pong_Gr + i) = SHAD_BASE;
			*(ping_pong_Gb + i) = SHAD_BASE;
			*(ping_pong_B + i) = SHAD_BASE;
		} else {
			uint32_t ratio_R = ((uint32_t) (*(ping_pong_R + i)) << offset)
					/ *(ping_pong_Gr + i);
			uint32_t ratio_B = ((uint32_t) (*(ping_pong_B + i)) << offset)
					/ *(ping_pong_Gb + i);
			uint32_t temp_r = ((*(ping_pong_Gr + i) - SHAD_BASE)
					* gain_down_ratio) >> offset;
			uint32_t temp_b = ((*(ping_pong_Gb + i) - SHAD_BASE)
					* gain_down_ratio) >> offset;

			*(ping_pong_Gr + i) = temp_r + SHAD_BASE;
			*(ping_pong_Gr + i) =
					(*(ping_pong_Gr + i) > SHAD_MAX) ?
							SHAD_MAX : *(ping_pong_Gr + i);

			*(ping_pong_Gb + i) = temp_b + SHAD_BASE;
			*(ping_pong_Gb + i) =
					(*(ping_pong_Gb + i) > SHAD_MAX) ?
							SHAD_MAX : *(ping_pong_Gb + i);

			*(ping_pong_R + i) = (uint16_t) ((*(ping_pong_Gr + i) * ratio_R)
					>> offset);
			*(ping_pong_B + i) = (uint16_t) ((*(ping_pong_Gb + i) * ratio_B)
					>> offset);

			*(ping_pong_R + i) =
					(*(ping_pong_R + i) > SHAD_MAX) ?
							SHAD_MAX : *(ping_pong_R + i);
			*(ping_pong_B + i) =
					(*(ping_pong_B + i) > SHAD_MAX) ?
							SHAD_MAX : *(ping_pong_B + i);
		}
	}
}

void set_lsc_table_address(LSC_RUNTIME_DATA* runtime_data,
		LSC_RESULT_TABLE* ping_pong, struct alLSClib_output_data_t* output) {
	switch (runtime_data->color_order) {
	default:
		break;
	case TYPE_LSC_COLOR_ORDER_RGGB:
		output->shading_table_00_address = &ping_pong->shading_table_r[0];
		output->shading_table_01_address = &ping_pong->shading_table_gr[0];
		output->shading_table_10_address = &ping_pong->shading_table_gb[0];
		output->shading_table_11_address = &ping_pong->shading_table_b[0];
		break;
	case TYPE_LSC_COLOR_ORDER_GRBG:
		output->shading_table_00_address = &ping_pong->shading_table_gr[0];
		output->shading_table_01_address = &ping_pong->shading_table_r[0];
		output->shading_table_10_address = &ping_pong->shading_table_b[0];
		output->shading_table_11_address = &ping_pong->shading_table_gb[0];
		break;
	case TYPE_LSC_COLOR_ORDER_GBRG:
		output->shading_table_00_address = &ping_pong->shading_table_gb[0];
		output->shading_table_01_address = &ping_pong->shading_table_b[0];
		output->shading_table_10_address = &ping_pong->shading_table_r[0];
		output->shading_table_11_address = &ping_pong->shading_table_gr[0];
		break;
	case TYPE_LSC_COLOR_ORDER_BGGR:
		output->shading_table_00_address = &ping_pong->shading_table_b[0];
		output->shading_table_01_address = &ping_pong->shading_table_gb[0];
		output->shading_table_10_address = &ping_pong->shading_table_gr[0];
		output->shading_table_11_address = &ping_pong->shading_table_r[0];
		break;
	}
}

//calculate default table
uint32_t alLSCLib_CalcDefaultTable(void* lsc_obj,
		struct alLSClib_output_data_t* default_output) {
	LSC_RUNTIME_DATA* runtime_data = (LSC_RUNTIME_DATA*) lsc_obj;

	if ((void*) 0 == default_output || (void*) 0 == lsc_obj) {
		return _AL_LSCLIB_CALC_DATAULT_TABLE_INPUT_NULL;
	}

	LSC_RESULT_TABLE* ping_pong = &runtime_data->ping_pong_buffer_0;
	runtime_data->ping_pong_index = 1;

	memcpy(&ping_pong->shading_table_r[0],
			&runtime_data->tuning_info.shading_tables[0].shading_table_r[0],
			sizeof(uint16_t) * LSC_TABLE_SIZE);
	memcpy(&ping_pong->shading_table_gr[0],
			&runtime_data->tuning_info.shading_tables[0].shading_table_gr[0],
			sizeof(uint16_t) * LSC_TABLE_SIZE);
	memcpy(&ping_pong->shading_table_gb[0],
			&runtime_data->tuning_info.shading_tables[0].shading_table_gb[0],
			sizeof(uint16_t) * LSC_TABLE_SIZE);
	memcpy(&ping_pong->shading_table_b[0],
			&runtime_data->tuning_info.shading_tables[0].shading_table_b[0],
			sizeof(uint16_t) * LSC_TABLE_SIZE);

	set_lsc_table_address(runtime_data, ping_pong, default_output);

	return _AL_LSCLIB_SUCCESS;
}

uint32_t alLSCLib_DynamicProcess(void* lsc_obj,
		struct alLSClib_output_data_t* output) {
	LSC_RUNTIME_DATA* runtime_data = (LSC_RUNTIME_DATA*) lsc_obj;

	if ((void*) 0 == lsc_obj || (void*) 0 == output) {
		return _AL_LSCLIB_DYNAMIC_PROCESS_INPUT_NULL;
	}

	if (runtime_data->shading_bypass) {
		return _AL_LSCLIB_SUCCESS;
	}

	//decide use ping pong 0 or ping pong 1
	LSC_RESULT_TABLE* ping_pong = (void*) 0;
	if (runtime_data->ping_pong_index == 0) {
		ping_pong = &runtime_data->ping_pong_buffer_0;
		runtime_data->ping_pong_index = 1;
	} else if (runtime_data->ping_pong_index == 1) {
		ping_pong = &runtime_data->ping_pong_buffer_1;
		runtime_data->ping_pong_index = 0;
	}

	if ((void*) 0 == ping_pong) {
		return _AL_LSCLIB_DYNAMIC_PING_PONG_BUFFER_NULL;
	}

	//Use Ymean and BV information to judge black
	if (runtime_data->current_bv <= runtime_data->tuning_info.bv_threshold
			&& runtime_data->y_mean
					<= runtime_data->tuning_info.y_mean_threshold) {
		uint32_t i = 0;
		uint16_t* ping_pong_R = &ping_pong->shading_table_r[0];
		uint16_t* ping_pong_Gr = &ping_pong->shading_table_gr[0];
		uint16_t* ping_pong_Gb = &ping_pong->shading_table_gb[0];
		uint16_t* ping_pong_B = &ping_pong->shading_table_b[0];

		for (i = 0; i < LSC_TABLE_SIZE; i++) {
			*(ping_pong_R + i) = SHAD_BASE;
			*(ping_pong_Gr + i) = SHAD_BASE;
			*(ping_pong_Gb + i) = SHAD_BASE;
			*(ping_pong_B + i) = SHAD_BASE;
		}
	} else {
		interpolate_LSC_tables(runtime_data, ping_pong);
		uint16_t smooth_ISO = get_smooth_ISO(runtime_data);
		high_ISO_decrease_gain_node_interpolate(runtime_data, smooth_ISO,
				ping_pong);
	}

	memset(output, 0, sizeof(struct alLSClib_output_data_t));
	set_lsc_table_address(runtime_data, ping_pong, output);

	return _AL_LSCLIB_SUCCESS;
}

uint32_t alLSCLib_loadfunc(struct alLSClib_runtime_obj_t* lsc_run_obj,
		uint32_t identity_id) {
	if ((void*) 0 == lsc_run_obj)   // check input data address validity
			{
		return _AL_LSCLIB_INIT_NULL_OBJ;
	}

	lsc_run_obj->identityid = identity_id; // tag identity ID to object, for framework recognition
	lsc_run_obj->initial = alLSCLib_init;
	lsc_run_obj->deinit = alLSCLib_deinit;
	lsc_run_obj->set_param = alLSCLib_set_param;
	lsc_run_obj->get_param = alLSCLib_get_param;
	lsc_run_obj->calc_default_table = alLSCLib_CalcDefaultTable;
	lsc_run_obj->dynamic_process = alLSCLib_DynamicProcess;

	return _AL_LSCLIB_SUCCESS;
}
