#include "alAFDLib_Core.h"
#include "alAFDLib_Error_Code.h"
#include "FS_process.h"

static AFD_RUNTIME_DATA g_afd_runtime_data_array;

UINT16  allib_afd_header_check_sum(void *data, int size_header, int size_tuning_data)
{
    unsigned char *ptr = (unsigned char*)data + size_header;
    unsigned long checksum = 0;

    for (UINT16 i = size_header; i < size_tuning_data; i++, ptr++)
        checksum += *ptr;

    checksum = checksum % 65536;
    return (UINT16)checksum;
}

void    allib_afd_add_header(void* ptr, float version)
{
    AFD_TUNING_DATA *data = (AFD_TUNING_DATA*)ptr;

    memset(&data->header.tag[0], 0, AFD_HEADER_STRING_LENGTH);
    strncpy(&data->header.tag[0], AFD_HEADER_STRING, strlen(AFD_HEADER_STRING));

    data->header.struct_version = ALTEK_AFD_TUNING_STRUCT_VERSION;
    data->header.lib_version_major = ALTEK_AFD_VERSION_MAJOR;
    data->header.lib_version_minor = ALTEK_AFD_VERSION_MINOR;
    data->header.tuning_version = version;
    data->header.check_sum = allib_afd_header_check_sum(ptr, sizeof(AFD_TUNING_HEADER), sizeof(AFD_TUNING_DATA));
    memset(&data->header.reserved[0], 0, 10);
}

void    allib_afd_initial_tuning_data(void *ptr)
{
    AFD_TUNING_DATA *data = (AFD_TUNING_DATA*)ptr;
    memset(data, 0, sizeof(AFD_TUNING_DATA));

    data->header.struct_version = ALTEK_AFD_TUNING_STRUCT_VERSION;

    data->line_time_ns = 29800;
    data->sensor_width = 2328;
    data->sensor_height = 1744;
    data->woi_x = 0;
    data->woi_y = 0;
    data->woi_w = 0;
    data->woi_h = 0;

    data->default_hz = AFD_MODE_50HZ;
    data->points_1st_region = AFD_DEFAULT_POINTS_1ST_REGION;
    data->points_2nd_region = AFD_DEFAULT_POINTS_2ND_REGION;
    data->enable_2nd_region = 0;

    data->base = 8192;
    data->level_50 = 0;
    data->index_range_50_0 = 70;
    data->index_range_50_1 = 60;
    data->index_range_50_2 = 50;
    data->level_60 = 1;
    data->index_range_60_0 = 70;
    data->index_range_60_1 = 60;
    data->index_range_60_2 = 50;
    data->th_frame_average = 5;
    data->th_amp_min = 36;
    data->th_amp = 60;
    data->ratio_shift = 10;
    data->ratio_exp_gain_min = 870;     // 0.85 * 1024
    data->ratio_exp_gain_max = 1178;    // 1.15 * 1024
    data->ratio_ae_y_min = 870;         // 0.85 * 1024
    data->ratio_ae_y_max = 1178;        // 1.15 * 1024
    data->ratio_peak_dc = 332;          // 0.9 * (0.6)^2  * 1024
    data->ratio_peak_2nd = 332;         // 0.9 * (0.6)^2  * 1024
    data->th_symm_min = 3;
    data->th_symm_50hz = 24;
    data->th_symm_60hz = 24;
    data->size_symmetric = 3;
    data->ratio_symmetric[0] = 830;     // 0.9^2 * 1024
    data->ratio_symmetric[1] = 502;     // 0.7^2 * 1024
    data->ratio_symmetric[2] = 256;     // 0.5^2 * 1024
    data->ratio_max_range = 666;        // 0.650 * 1024
    data->ratio_low_amp = 154;          // 0.150 * 1024
    data->peak_decay_count = 2;
    data->level_stable_th = 1;
    data->stable_th_0 = 11;
    data->stable_th_1 = 9;
    data->stable_th_2 = 7;

    data->jump_sample_num = 64;
    data->jump_sample_do_avg_count = 10;

    data->_50Hz_flicker_pos = 100;
    data->_60Hz_flicker_pos = 120;
    data->enable_close_after_change_mode = 0;
    data->enable_debug_log = 0;
    data->frame_interval_50 = 4;
    data->frame_interval_60 = 1;

    //FS
    data->fs_th_dc_amp = 154;          // 0.150 * 1024
    data->amp_2nd_peak_ratio = 614;         // 0.6  * 1024
    memcpy(&data->freq_box[0], &BOX[0],sizeof(UINT16) * VOTE_NUM );
    data->vote_delta_th = 10;
    data->fs_th_stable = 5;
    data->fs_enable_close_after_change_mode = 0;

    allib_afd_add_header(data, 1.0F);
}

void    _update_sensor_info_from_tuning_file(AFD_RUNTIME_DATA *runtime_data)
{
    runtime_data->sensor_info.line_time_ns = runtime_data->tuning_data.line_time_ns;
    runtime_data->sensor_info.sensor_width = runtime_data->tuning_data.sensor_width;
    runtime_data->sensor_info.sensor_height = runtime_data->tuning_data.sensor_height;
    runtime_data->sensor_info.a3engine_width = runtime_data->tuning_data.a3engine_width;
    runtime_data->sensor_info.a3engine_height = runtime_data->tuning_data.a3engine_height;
}

enum afd_level  convert_afd_level(UINT16 level)
{
    enum afd_level afd_level = AFD_LEVEL_STRONG;
    switch (level)
    {
    case 0:
        afd_level = AFD_LEVEL_STRONG;
        break;
    case 1:
        afd_level = AFD_LEVEL_MIDDLE;
        break;
    case 2:
        afd_level = AFD_LEVEL_WEAK;
        break;
    }
    return afd_level;
}

void    _update_detection_setting_from_tuning_file(AFD_RUNTIME_DATA *runtime_data)
{ 
    runtime_data->detection.afd_level_50hz = convert_afd_level(runtime_data->tuning_data.level_50);
    runtime_data->detection.afd_level_60hz = convert_afd_level(runtime_data->tuning_data.level_60);
    runtime_data->detection.afd_level_detect_th = convert_afd_level(runtime_data->tuning_data.level_stable_th);
}

void    _load_default_parameters(AFD_RUNTIME_DATA *runtime_data)
{
    memset(runtime_data, 0, sizeof(AFD_RUNTIME_DATA));

    allib_afd_getlib_version(&runtime_data->version);
    runtime_data->output_report.afd_mode = AFD_MODE_OFF;
    runtime_data->output_report.afd_mode_stable = AFD_MODE_OFF;
    runtime_data->output_report.flag_enable = 1;

    allib_afd_initial_tuning_data(&runtime_data->tuning_data);

    runtime_data->stats_size = 61568;

    runtime_data->afd_setting.enable = 1;
    runtime_data->afd_setting.fs_enable = 1;
    runtime_data->afd_setting.enable_close_after_detection = 0;
    runtime_data->afd_setting.mode = AFD_MODE_OFF;

    //fs reslt
    memset(&runtime_data->fs_result, 0, sizeof(AFD_FSRESULT));

    runtime_data->flag_sensor_info = 0;
    _update_sensor_info_from_tuning_file(runtime_data);

    runtime_data->flag_detection = 0;
    _update_detection_setting_from_tuning_file(runtime_data);
}

void    _reset_queue_stats(AFD_RUNTIME_DATA *runtime_data)
{
    runtime_data->status_queue_index_0 = 0;
    runtime_data->status_queue_index_1 = 0;
    memset(&runtime_data->queue_stats, 0, sizeof(AFD_STATS_QUEUE_DATA) * AFD_STATS_QUEUE_SIZE);
}

void    _reset_queue_stable(AFD_RUNTIME_DATA *runtime_data)
{
    runtime_data->stable_queue_index = 0;
    memset(&runtime_data->queue_stable, 0, sizeof(AFD_STABLE_DATA) * AFD_STABLE_QUEUE_SIZE);
}

//int stats_cnt = 0;

UINT32  _dispatch_avgG_channel_stats(void *g_channel_rows_avg_data, AFD_RUNTIME_DATA *runtime_data)
{
    UINT16 i = 0, j = 0;
    UINT32* avg_G_info = (UINT32*)g_channel_rows_avg_data;
    AFD_STATS_DATA *stats_data = &runtime_data->stats_data;
    AFD_TUNING_DATA *tuning_data = &runtime_data->tuning_data;
    struct afd_sensor_info_t *sensor = &runtime_data->sensor_info;

    UINT16 row_count = (0 != sensor->sensor_woi_h) ? sensor->sensor_woi_h : sensor->sensor_height;
    row_count = (0 != sensor->a3engine_height) ? sensor->a3engine_height : row_count;

    if (row_count > runtime_data->stats_size)
        row_count = runtime_data->stats_size;

    if (row_count < tuning_data->jump_sample_num)
    {
        return _AL_AFDLIB_PROCESS_STATS_DISPATCH_ERROR;
    }

    //if row count is different, reset stats queue and stable queue
    if (0 == runtime_data->row_count)
    {
        runtime_data->row_count = row_count;
    }
    else
    {
        if (runtime_data->row_count != row_count)
        {
            runtime_data->row_count = row_count;
            _reset_queue_stats(runtime_data);
            _reset_queue_stable(runtime_data);
        }
    }
 
    UINT16 jump_row_num = row_count / tuning_data->jump_sample_num;
    UINT16 jump_sample_do_avg_count = (tuning_data->jump_sample_do_avg_count > jump_row_num) ? 1 : tuning_data->jump_sample_do_avg_count;

    UINT16 avg_jump_index = jump_row_num / jump_sample_do_avg_count;
    UINT32 sum_frame_avg = 0;

    memset(stats_data, 0, sizeof(AFD_STATS_DATA));
    runtime_data->input_data_num = tuning_data->jump_sample_num;
   
    //LOG_DEBUG("down sampling stats print");

    for (i = 0; i < tuning_data->jump_sample_num; i++)
    {
        UINT32 sum = 0;
        for (j = 0; j < jump_sample_do_avg_count; j++)
        {
            sum += avg_G_info[i * jump_row_num + avg_jump_index * j];
        }
        stats_data->avg_info[i] = sum / jump_sample_do_avg_count;

        //for (j = 0; j < jump_row_num; j++)
        //{
        //    sum += avg_G_info[i * tuning_data->jump_sample_num +  j];
        //}
        //stats_data->avg_info[i] = sum / jump_row_num;

        //if (stats_cnt < 2)
        //{
        //    LOG_DEBUG("stats %d : %d", i, stats_data->avg_info[i]);
        //}
        sum_frame_avg += stats_data->avg_info[i];
    }
    //LOG_DEBUG("ori stats %d : %d", 872, avg_G_info[871]);
    //LOG_DEBUG("ori stats %d : %d", 1744, avg_G_info[1743]);
    stats_data->frame_average = sum_frame_avg / tuning_data->jump_sample_num;

    //stats_cnt++;

    return _AL_AFDLIB_SUCCESS;
}


UINT32  _get_exposure_time_diff(UINT32 expos, UINT32 base)
{
    INT32 diff_0 = expos - (expos / base) * base;
    INT32 diff_1 = (expos / base + 1) * base - expos;
    return MIN(ABS(diff_0), ABS(diff_1));
}

void    _push_stats(AFD_RUNTIME_DATA *runtime_data)
{
    UINT8 interval;
    UINT32 diff_50 = _get_exposure_time_diff(runtime_data->exposure_info.exposure_time, AFD_BASIC_EXP_TIME_50HZ);
    UINT32 diff_60 = _get_exposure_time_diff(runtime_data->exposure_info.exposure_time, AFD_BASIC_EXP_TIME_60HZ);
    AFD_STATS_QUEUE_DATA* frame_n = &runtime_data->queue_stats[runtime_data->status_queue_index_0];

    if (AFD_EXP_TIME_DIFF_THD >= diff_50)
    {
        runtime_data->conditions.target_hz = AFD_MODE_60HZ;
        interval = runtime_data->tuning_data.frame_interval_60;
    }
    else if (AFD_EXP_TIME_DIFF_THD >= diff_60)
    {
        runtime_data->conditions.target_hz = AFD_MODE_50HZ;
        interval = runtime_data->tuning_data.frame_interval_50;
    }
    else
    {
        runtime_data->conditions.target_hz = AFD_MODE_OFF;
        interval = (runtime_data->tuning_data.frame_interval_50 + runtime_data->tuning_data.frame_interval_60 + 1) / 2;
        //interval = 0;
    }

    memcpy(&frame_n->info, &runtime_data->exposure_info, sizeof(struct afd_exposure_info_t));
    memcpy(&frame_n->stats_data, &runtime_data->stats_data, sizeof(AFD_STATS_DATA));

    runtime_data->status_queue_index_1 = ((AFD_STATS_QUEUE_SIZE + runtime_data->status_queue_index_0 - interval) % AFD_STATS_QUEUE_SIZE);
}

BOOL    _check_stats_stable(AFD_RUNTIME_DATA *runtime_data)
{
    INT32 diff;
    AFD_TUNING_DATA *tuning = &runtime_data->tuning_data;
    AFD_STATS_QUEUE_DATA* frame_n = &runtime_data->queue_stats[runtime_data->status_queue_index_0];
    AFD_STATS_QUEUE_DATA* frame_p = &runtime_data->queue_stats[runtime_data->status_queue_index_1];

    diff = frame_n->stats_data.frame_average - frame_p->stats_data.frame_average;
    LOG_DEBUG("frame_diff is : %d\n", diff);
    LOG_DEBUG("frame_avg is  %d and  %d\n", frame_n->stats_data.frame_average, frame_p->stats_data.frame_average);

    if (frame_n->info.exposure_time < frame_p->info.exposure_time - 500 ||
        frame_n->info.exposure_time > frame_p->info.exposure_time + 500 ||
        frame_n->info.gain < ((tuning->ratio_exp_gain_min * frame_p->info.gain) >> tuning->ratio_shift) ||
        frame_n->info.gain >((tuning->ratio_exp_gain_max * frame_p->info.gain) >> tuning->ratio_shift) ||
        frame_n->info.average_value < ((tuning->ratio_ae_y_min * frame_p->info.average_value) >> tuning->ratio_shift) ||
        frame_n->info.average_value >((tuning->ratio_ae_y_max * frame_p->info.average_value) >> tuning->ratio_shift) ||
        ABS(diff) > tuning->th_frame_average
        )
        return TRUE;

    return FALSE;
}

UINT16  _compute_range_points(UINT16 *out_count, UINT16 *out_table, UINT16 left, UINT16 right, UINT16 points, UINT16 default_pt)
{
    UINT16 i, pt;
    UINT16 len, jump, count;

    if (0 == points)
        pt = default_pt;
    else if (points >= AFD_MAX_POINTS_REGION)
        pt = AFD_MAX_POINTS_REGION - 1;
    else
        pt = points;

    len = right - left;
    if (len <= pt)
    {
        jump = 1;
        count = len + 1;
        for (i = 0; i < count; i++)
            out_table[i] = left + i;
    }
    else
    {
        jump = (len + pt - 1) / pt;
        count = pt + 1;
        for (i = 0; i < count - 1; i++)
            out_table[i] = left + (UINT16)((i * len + pt / 2) / pt);
        out_table[count - 1] = right;
    }
    *out_count = count;
    return jump;
}

UINT32  _compute_flicker_range(AFD_RUNTIME_DATA *runtime_data)
{
    struct afd_sensor_info_t *sensor = &runtime_data->sensor_info;
    struct afd_parameter_detect_t *detection = &runtime_data->detection;
    AFD_TUNING_DATA *tuning = &runtime_data->tuning_data;
    AFD_CONDITIONS *conditions = &runtime_data->conditions;
    UINT16 left, right;
    UINT16 shift = tuning->base / AFD_DFT_LEN;   //4
    
    UINT32 input_height = (0 != sensor->sensor_woi_h) ? sensor->sensor_woi_h : sensor->sensor_height;
    UINT32 total_line_time = input_height * sensor->line_time_ns;
    LOG_DEBUG("line_time_ns = %d", sensor->line_time_ns);
    LOG_DEBUG("input_height = %d", input_height);

    UINT32 stats_height = (0 != sensor->a3engine_height) ? sensor->a3engine_height : input_height;

    if (stats_height > runtime_data->stats_size)
        stats_height = runtime_data->stats_size;

    UINT32 stats_line_time = total_line_time / stats_height;
    UINT32 jump_row_num = stats_height / tuning->jump_sample_num;

    FLOAT32 second = 1.0f;
    FLOAT32 sample_rate = second * 1000.0f * 1000.0f * 1000.0f / stats_line_time / jump_row_num;  //second => ms => us => ns
    FLOAT32 unit = 10 * sample_rate / AFD_DFT_LEN;
	conditions->unit = unit;
	
    LOG_DEBUG("stats_line_time : %d", stats_line_time);
    LOG_DEBUG("stats_height : %d", stats_height);
    LOG_DEBUG("tuning->jump_sample_num = %d", tuning->jump_sample_num);
    LOG_DEBUG("jump_row_num = %d", jump_row_num);
    LOG_DEBUG("sample_rate = %.2f", sample_rate);
    LOG_DEBUG("unit = %.2f \n", unit);

    if (0 == unit)
	{
		return _AL_AFDLIB_PROCESS_FREQUENCY_TRANSFORM_ERROR;
	}
        
	UINT16 _50Hz_flicker_pos = tuning->_50Hz_flicker_pos * 10;
    UINT16 _60Hz_flicker_pos = tuning->_60Hz_flicker_pos * 10;
    
    switch (detection->afd_level_50hz)
    {
    default:
    case AFD_LEVEL_STRONG:
        conditions->range_min_50hz = (UINT16)((_50Hz_flicker_pos - tuning->index_range_50_0) / unit);
        conditions->range_max_50hz = (UINT16)((_50Hz_flicker_pos + tuning->index_range_50_0) / unit);
        break;
    case AFD_LEVEL_MIDDLE:
        conditions->range_min_50hz = (UINT16)((_50Hz_flicker_pos - tuning->index_range_50_1) / unit);
        conditions->range_max_50hz = (UINT16)((_50Hz_flicker_pos + tuning->index_range_50_1) / unit);
        break;
    case AFD_LEVEL_WEAK:
        conditions->range_min_50hz = (UINT16)((_50Hz_flicker_pos - tuning->index_range_50_2) / unit);
        conditions->range_max_50hz = (UINT16)((_50Hz_flicker_pos + tuning->index_range_50_2) / unit);
        break;
    }

    switch (detection->afd_level_60hz)
    {
    default:
    case AFD_LEVEL_STRONG:
        conditions->range_min_60hz = (UINT16)((_60Hz_flicker_pos - tuning->index_range_60_0) / unit);
        conditions->range_max_60hz = (UINT16)((_60Hz_flicker_pos + tuning->index_range_60_0) / unit);
        break;
    case AFD_LEVEL_MIDDLE:
        conditions->range_min_60hz = (UINT16)((_60Hz_flicker_pos - tuning->index_range_60_1) / unit);
        conditions->range_max_60hz = (UINT16)((_60Hz_flicker_pos + tuning->index_range_60_1) / unit);
        break;
    case AFD_LEVEL_WEAK:
        conditions->range_min_60hz = (UINT16)((_60Hz_flicker_pos - tuning->index_range_60_2) / unit);
        conditions->range_max_60hz = (UINT16)((_60Hz_flicker_pos + tuning->index_range_60_2) / unit);
        break;
    }

    switch (detection->afd_level_detect_th)
    {
    default:
    case AFD_LEVEL_STRONG:
        conditions->th_stable = tuning->stable_th_0;
        break;
    case AFD_LEVEL_MIDDLE:
        conditions->th_stable = tuning->stable_th_1;
        break;
    case AFD_LEVEL_WEAK:
        conditions->th_stable = tuning->stable_th_2;
        break;
    }
    conditions->th_amp = MAX(tuning->th_amp_min, (tuning->th_amp + shift / 2) / shift);
    conditions->range_sym_50hz = MAX(tuning->th_symm_min, (tuning->th_symm_50hz / shift));
    conditions->range_sym_60hz = MAX(tuning->th_symm_min, (tuning->th_symm_60hz / shift));

    memset(conditions->points_1st, 0, sizeof(UINT16) * AFD_MAX_POINTS_REGION);
    memset(conditions->points_2nd, 0, sizeof(UINT16) * AFD_MAX_POINTS_REGION);

    left = 0;
    right = 2 * conditions->range_max_60hz - conditions->range_min_60hz;
    conditions->delta_1st = _compute_range_points(&conditions->count_1st, conditions->points_1st, left, right, tuning->points_1st_region, AFD_DEFAULT_POINTS_1ST_REGION);

    left = right;
    right = ((tuning->ratio_max_range * AFD_DFT_LEN) >> (tuning->ratio_shift + 1));
    conditions->delta_2nd = _compute_range_points(&conditions->count_2nd, conditions->points_2nd, left, right, tuning->points_2nd_region, AFD_DEFAULT_POINTS_2ND_REGION);

    return _AL_AFDLIB_SUCCESS;
}

BOOL    _flicker_detection_frame_diff(AFD_RUNTIME_DATA *runtime_data)
{
    UINT16 h;
    UINT32 diff_abs, max = 0;
    INT32 diff;
    int  *data_d = &runtime_data->working_buffer.data_diff[0];
    FLOAT32 *data_n = &runtime_data->working_buffer.data_diff_normalize[0];
    UINT32 *data_0 = &runtime_data->queue_stats[runtime_data->status_queue_index_0].stats_data.avg_info[0];
    UINT32 *data_1 = &runtime_data->queue_stats[runtime_data->status_queue_index_1].stats_data.avg_info[0];
    LOG_DEBUG("calculate frame is %d and %d", runtime_data->status_queue_index_0, runtime_data->status_queue_index_1);
    for (h = 0; h < runtime_data->input_data_num; h++)
    {
        diff = data_1[h] - data_0[h];
        data_d[h] = diff;

        diff_abs = ABS(diff);
        if (max < diff_abs)
            max = diff_abs;
    }

    if (0 == max)
    {
        runtime_data->working_buffer.result.detect_status = AFD_DETECT_STATUS_PEAK_NOT_ENOUGH;
        printf("_flicker_detection_frame_diff detect_status: %d \n", runtime_data->working_buffer.result.detect_status);
        return TRUE;
    }

	FLOAT32 max_float = (FLOAT32)max;
	if (0.0f == max_float)
    {
        runtime_data->working_buffer.result.detect_status = AFD_DETECT_STATUS_PEAK_NOT_ENOUGH;
        printf("_flicker_detection_frame_diff detect_status: %d \n", runtime_data->working_buffer.result.detect_status);
        return TRUE;
    }
	
    for (h = 0; h < runtime_data->input_data_num; h++)
	{
		FLOAT32 input_float = (FLOAT32)data_d[h];		
		data_n[h] = (10.0f * input_float / max_float);
	}
	
    return FALSE;
}

BOOL    _flicker_scan_1st_region(AFD_RUNTIME_DATA *runtime_data)
{
    AFD_TUNING_DATA *tuning = &runtime_data->tuning_data;
    AFD_CONDITIONS *conditions = &runtime_data->conditions;
    AFD_WORKING_BUFFER *working = &runtime_data->working_buffer;
    FLOAT32 re, im;
    INT32 symm_diff;
    UINT16 i = 0, j = 0;
    UINT16 th_symm = 0;
    UINT16 left, right;
    UINT16 index, peak_index;

    UINT16 min50 = conditions->range_min_50hz - conditions->delta_1st;
    UINT16 max50 = conditions->range_max_50hz + conditions->delta_1st;
    UINT16 min60 = conditions->range_min_60hz - conditions->delta_1st;
    UINT16 max60 = conditions->range_max_60hz + conditions->delta_1st;
    
    UINT32 amp, amp_max, amp_max2;
    UINT32 *fft = &working->data_fft[0];

    amp_max = 0;
			
    for (i = 0; i < conditions->count_1st; i++)
    {
        DFTIndex(working->data_diff_normalize, runtime_data->input_data_num, conditions->points_1st[i], &re, &im);
        amp = (UINT32)(re * re + im * im + 0.5);
        fft[i] = amp;
        if (amp_max < amp)
            amp_max = amp;
    }
    working->amp_max = amp_max;
    //printf("amp_max is %d \n", amp_max);
    //printf("dc_amp is %d \n", fft[0]);
    //printf("th_amp is %d \n", conditions->th_amp);

    LOG_DEBUG("amp_max is %d", amp_max);
    LOG_DEBUG("dc_amp is %d", fft[0]);
    LOG_DEBUG("th_amp is %d", conditions->th_amp);
     //peak dc
    if (fft[0] >((amp_max * tuning->ratio_peak_dc) >> tuning->ratio_shift))
    {
        working->result.detect_status = AFD_DETECT_STATUS_PEAK_DC_OVER_TH;
        LOG_DEBUG("peak dc detect_status: %d", working->result.detect_status);
        return TRUE;
    }

     //peak amp
    if (working->amp_max < (UINT32)(conditions->th_amp * conditions->th_amp))
    {
        working->result.detect_status = AFD_DETECT_STATUS_PEAK_NOT_ENOUGH;
        LOG_DEBUG("peak amp detect_status: %d", working->result.detect_status);
        return TRUE;
    }

    // find peak
    index = 65535;
    amp_max = fft[0];
    amp_max2 = fft[0];
    for (i = 1; i < conditions->count_1st - 1; i++)
    {
        if (fft[i - 1] >= fft[i] || fft[i] < fft[i + 1])
            continue;

        if (amp_max < fft[i])
        {
            if (amp_max2 < amp_max)
                amp_max2 = amp_max;

            index = i;
            amp_max = fft[i];
        }
        else if (amp_max2 < fft[i])
        {
            amp_max2 = fft[i];
        }
    }

    // NO peak
    if (65535 == index)
    {
        working->result.detect_status = AFD_DETECT_STATUS_PEAK_NOT_ENOUGH;
        LOG_DEBUG("NO peak detect_status: %d", working->result.detect_status);
        return TRUE;
    }

    // 2nd peak over 1st * ratio
    if (amp_max2 > ((amp_max * tuning->ratio_peak_2nd) >> tuning->ratio_shift))
    {
        working->result.detect_status = AFD_DETECT_STATUS_PEAK_2ND_OVER_TH;
        LOG_DEBUG("2nd peak detect_status: %d", working->result.detect_status);
        return TRUE;
    }
    LOG_DEBUG("amp_max2 is %d", amp_max2);

    // check peak range
    peak_index = conditions->points_1st[index];

    //printf("peak_index is %d \n", peak_index);
    //printf("min 50 -- max 50 is %d -- %d \n", min50, max50);
    //printf("min 60 -- max 60 is %d -- %d \n", min60, max60);
    //printf("conditions->target_hz %d \n", conditions->target_hz);

    LOG_DEBUG("conditions->target_hz %d", conditions->target_hz);
    LOG_DEBUG("peak_index is %d", peak_index);
    LOG_DEBUG("min 50 -- max 50 is %d -- %d ", min50, max50);
    LOG_DEBUG("min 60 -- max 60 is %d -- %d", min60, max60);

    if (AFD_MODE_50HZ == conditions->target_hz)
    {
        if (peak_index < min50 || max50 < peak_index)
        {
            working->result.detect_status = AFD_DETECT_STATUS_PEAK_OUT_OF_RANGE;
            LOG_DEBUG("50hz detect_status: %d", working->result.detect_status);
            return TRUE;
        }
				
        working->is_banding = TRUE;
        working->result.detect_hz = AFD_MODE_50HZ;
        th_symm = conditions->range_sym_50hz;
    }
    else if (AFD_MODE_60HZ == conditions->target_hz)
    {
        if (peak_index < min60 || max60 < peak_index)
        {
            working->result.detect_status = AFD_DETECT_STATUS_PEAK_OUT_OF_RANGE;
            LOG_DEBUG("60hz detect_status: %d", working->result.detect_status);
            return TRUE;
        }
        working->is_banding = TRUE;
        working->result.detect_hz = AFD_MODE_60HZ;
        th_symm = conditions->range_sym_60hz;
    }
    else
    {
        // in this case, exposure is not based on 10ms or 8.3ms, to detect 50/60 both
        if (peak_index < min50 || max60 < peak_index)
        {
            working->result.detect_status = AFD_DETECT_STATUS_PEAK_OUT_OF_RANGE;
            LOG_DEBUG("50/60hz detect_status: %d", working->result.detect_status);
            return TRUE;
        }
        else
        {
            // base on center, judge to 50/60 hz by peak index is closed to 50 or 60;
            // center might be right a little, so it is 60 hz if peak_index = center;
            UINT16 center = (max50 + min60 + 1) / 2;
            if (peak_index < center)
            {
                working->result.detect_hz = AFD_MODE_50HZ;
                th_symm = conditions->range_sym_50hz;
            }
            else
            {
                working->result.detect_hz = AFD_MODE_60HZ;
                th_symm = conditions->range_sym_60hz;
            }
            working->is_banding = TRUE;
        }
    }
    //printf("1 current detect freq is %d \n", working->result.detect_hz);
    LOG_DEBUG("1 current detect freq is %d", working->result.detect_hz);
    th_symm = 2 * (1 + MAX(1, (th_symm + conditions->delta_1st - 1) / conditions->delta_1st));
    //printf("conditions->delta_1st is %d \n", conditions->delta_1st);
    //printf("th_symm is %d \n", th_symm);
    LOG_DEBUG("th_symm is %d", th_symm);

    // check symmetric
    for (i = 0; i < tuning->size_symmetric; i++)
    {
        amp = ((amp_max * tuning->ratio_symmetric[i]) >> tuning->ratio_shift);

        left = 0;

        for (j = index - 1; j != 0; j--)
        {
            // decreasing
            if (fft[j] < fft[j - 1])
            {
                working->result.detect_hz = AFD_MODE_OFF;
                working->result.detect_status = AFD_DETECT_STATUS_PEAK_NOT_SYMMETRIC;
                return TRUE;
            }

            if (fft[j] <= amp)
            {
                left = j;
                break;
            }
        }

        right = conditions->count_1st;
        for (j = index + 1; j < conditions->count_1st; j++)
        {
            // decreasing
            if (fft[j] < fft[j + 1])
            {
                working->result.detect_hz = AFD_MODE_OFF;
                working->result.detect_status = AFD_DETECT_STATUS_PEAK_NOT_SYMMETRIC;
                return TRUE;
            }

            if (fft[j] <= amp)
            {
                right = j;
                break;
            }
        }

        symm_diff = 2 * index - left - right;
        //printf("symm_diff is %d \n", symm_diff);
        LOG_DEBUG("symm_diff is %d", symm_diff);

        if (ABS(symm_diff) > th_symm)
        {
            working->result.detect_hz = AFD_MODE_OFF;
            working->result.detect_status = AFD_DETECT_STATUS_PEAK_NOT_SYMMETRIC;
            return TRUE;
        }
    }
    //printf("2 current detect freq is %d \n", working->result.detect_hz);
    LOG_DEBUG("2 current detect freq is %d", working->result.detect_hz);

    working->result.detect_status = AFD_DETECT_STATUS_OK;
    return FALSE;
}

BOOL    _flicker_scan_2nd_region(AFD_RUNTIME_DATA *runtime_data)
{
    AFD_TUNING_DATA *tuning = &runtime_data->tuning_data;
    AFD_CONDITIONS *conditions = &runtime_data->conditions;
    AFD_WORKING_BUFFER *working = &runtime_data->working_buffer;
    FLOAT32 re, im;
    UINT16 i, index;
    UINT16 peak_decay_cnt = 0;
    UINT16 peak, peak_decay = working->amp_max;
    UINT16 amp_max = working->amp_max;
    UINT16 amp_max_th_2nd = ((amp_max * tuning->ratio_peak_2nd) >> tuning->ratio_shift);
    UINT16 amp_max_th_low = ((amp_max * tuning->ratio_low_amp) >> tuning->ratio_shift);
    UINT32 *fft = &working->data_fft[0];

    for (i = 1; i < conditions->count_2nd; i++)
    {
        DFTIndex(working->data_diff_normalize, runtime_data->input_data_num, conditions->points_2nd[i], &re, &im);

        index = conditions->count_1st + i - 1;
        fft[index] = (UINT32)(re * re + im * im);

        if ((fft[index - 2] < fft[index - 1]) && (fft[index - 1] > fft[index]))
        {
            peak = fft[index - 1];

            // 2nd peak over 1st * ratio
            if (peak  > amp_max_th_2nd)
            {
            working->result.detect_hz = AFD_MODE_OFF;
            working->result.detect_status = AFD_DETECT_STATUS_PEAK_2ND_OVER_TH;
            return TRUE;
            }

            // peak decay or small enough
            if (peak < peak_decay || peak < amp_max_th_low)
            {
                if (tuning->peak_decay_count - 1 == peak_decay_cnt)
                {
                    working->result.detect_status = AFD_DETECT_STATUS_OK;
                    return FALSE;
                }
                peak_decay = fft[index - 1];
                peak_decay_cnt++;
            }
            else
            {
                peak_decay = 0;
            }
        }
    }
    //printf("2.5 current freq :%d",working->result.detect_hz);
    working->result.detect_status = AFD_DETECT_STATUS_OK;
    return FALSE;
}

void    _flicker_detection(AFD_RUNTIME_DATA *runtime_data)
{
    if (TRUE == _flicker_detection_frame_diff(runtime_data) ||
        TRUE == _flicker_scan_1st_region(runtime_data))
        return;

    if (0 == runtime_data->tuning_data.enable_2nd_region)
        return;

    _flicker_scan_2nd_region(runtime_data);
}

void    _update_stable_queue_report(AFD_RUNTIME_DATA *runtime_data)
{
    UINT16 i = 0;
    UINT16 cnt_50 = 0;
    UINT16 cnt_60 = 0;
    for (i = 0; i < AFD_STABLE_QUEUE_SIZE; i++)
    {
        if (AFD_MODE_50HZ == runtime_data->queue_stable[i].detect_hz)
            cnt_50++;
        if (AFD_MODE_60HZ == runtime_data->queue_stable[i].detect_hz)
            cnt_60++;
    }
    runtime_data->stable_50hz_cnt = cnt_50;
    runtime_data->stable_60hz_cnt = cnt_60;
    //printf("stable_50hz_cnt is %d \n", runtime_data->stable_50hz_cnt);
    //printf("stable_60hz_cnt is %d \n", runtime_data->stable_60hz_cnt);
    LOG_DEBUG("stable_50hz_cnt is %d", runtime_data->stable_50hz_cnt);
    LOG_DEBUG("stable_60hz_cnt is %d", runtime_data->stable_60hz_cnt);

    if (cnt_50 >= runtime_data->conditions.th_stable)
    {
        runtime_data->stable_result.detect_status = AFD_DETECT_STATUS_OK;
        runtime_data->stable_result.detect_hz = AFD_MODE_50HZ;
    }
    else if (cnt_60 >= runtime_data->conditions.th_stable)
    {
        runtime_data->stable_result.detect_status = AFD_DETECT_STATUS_OK;
        runtime_data->stable_result.detect_hz = AFD_MODE_60HZ;
    }
    else
    {
        runtime_data->stable_result.detect_status = AFD_DETECT_STATUS_STABLE_TH;
        runtime_data->stable_result.detect_hz = AFD_MODE_OFF;
    }

    if (AFD_MODE_OFF == runtime_data->stable_result.detect_hz)
    {
        runtime_data->afd_setting.enable = 1;
    }
    else
    {
        runtime_data->afd_setting.mode = runtime_data->stable_result.detect_hz;
        
        if (1 == runtime_data->tuning_data.enable_close_after_change_mode)
        {
            runtime_data->afd_setting.enable = 0;
            memset(&runtime_data->queue_stable, 0, sizeof(AFD_STABLE_DATA) * AFD_STABLE_QUEUE_SIZE);
        }
    }

    runtime_data->output_report.flag_enable = runtime_data->afd_setting.enable;
    runtime_data->output_report.afd_mode_stable = runtime_data->afd_setting.mode;
}

/* public */
UINT32  allib_afd_getlib_runtimebuf_size(void)
{
    return sizeof(AFD_RUNTIME_DATA);
}

void    allib_afd_getlib_version(struct allib_afd_lib_version_t *lib_version)
{
    lib_version->major_version = ALTEK_AFD_VERSION_MAJOR;
    lib_version->minor_version = ALTEK_AFD_VERSION_MINOR;
}

/* init */
UINT32  alAFDLib_init(void **afd_buffer)
{
    AFD_RUNTIME_DATA *runtime_data = &g_afd_runtime_data_array;

    if (NULL == runtime_data)
    {
        return _AL_AFDLIB_INIT_BUFFER_NULL;
    }

    *afd_buffer = runtime_data;   //set working buffer address, outside can know runtime data pointer address

    memset(runtime_data, 0, allib_afd_getlib_runtimebuf_size());
    _load_default_parameters(runtime_data);

    runtime_data->flag_update_range = 1;
    return _AL_AFDLIB_SUCCESS;
}

/* deinit */
UINT32  alAFDLib_deinit(void *afd_buffer)
{
    if (NULL == afd_buffer)
    {
        return _AL_AFDLIB_DEINIT_NULL_OBJ;
    }

    memset(afd_buffer, 0, allib_afd_getlib_runtimebuf_size());

    return _AL_AFDLIB_SUCCESS;
}

/* set */
UINT32  alAFDLib_set_param(struct allib_afd_set_parameter_t *param, void *afd_dat)
{
    AFD_RUNTIME_DATA *runtime_data = (AFD_RUNTIME_DATA*)afd_dat;
    if (NULL == param || NULL == afd_dat)
    {
        return _AL_AFDLIB_SET_PARAM_INPUT_NULL;
    }

    switch (param->type)
    {
    default:
        return _AL_AFDLIB_SET_PARAM_INPUT_TYPE;
    case ALAFD_SET_PARAM_TUNING_DATA:
        break;
    case ALAFD_SET_PARAM_STATS_SIZE:
        if (0 == param->para.stats_size)
            return _AL_AFDLIB_SET_PARAM_STATS_SIZE_EMPTY;
        runtime_data->stats_size = param->para.stats_size;
        break;

    case ALAFD_SET_PARAM_FLICKER_SENSOR_INFO:
        if (param->para.flicker_sensor_info.bin_num == 0)
        {
            runtime_data->flag_flicker_sensor_info = 0;
        }
        else
        {
            memcpy(&runtime_data->flicker_sensor_info, &param->para.flicker_sensor_info, sizeof(struct afd_flicker_sensor_info_t));
            runtime_data->flag_flicker_sensor_info = 1;
        }
        break;

    case ALAFD_SET_PARAM_SENSOR_INFO:

        if (0 == param->para.sensor_info.line_time_ns)
        {
            runtime_data->flag_sensor_info = 0;
            runtime_data->flag_update_range = 1;
            _update_sensor_info_from_tuning_file(runtime_data);
        }
        else
        {
            if (0 == param->para.sensor_info.sensor_width ||
                0 == param->para.sensor_info.sensor_height)
            {
                return _AL_AFDLIB_SET_PARAM_SENSOR_INFO_EMPTY;
            }

            if (0 == param->para.sensor_info.sensor_woi_w || 0 == param->para.sensor_info.sensor_woi_h)
            {
                param->para.sensor_info.sensor_woi_x = 0;
                param->para.sensor_info.sensor_woi_y = 0;
                param->para.sensor_info.sensor_woi_w = 0;
                param->para.sensor_info.sensor_woi_h = 0;
            }
            else
            {
                if (param->para.sensor_info.sensor_woi_x + param->para.sensor_info.sensor_woi_w > param->para.sensor_info.sensor_width ||
                    param->para.sensor_info.sensor_woi_y + param->para.sensor_info.sensor_woi_h > param->para.sensor_info.sensor_height)
                {
                    return _AL_AFDLIB_SET_PARAM_SENSOR_INFO_INVALID_WOI;
                }
            }
            runtime_data->flag_sensor_info = 1;
            runtime_data->flag_update_range = 1;
            memcpy(&runtime_data->sensor_info, &param->para.sensor_info, sizeof(struct afd_sensor_info_t));
        }
        break;

    case ALAFD_SET_PARAM_SETTING:
        if (AFD_MODE_50HZ != param->para.afd_setting.mode && AFD_MODE_60HZ != param->para.afd_setting.mode)
            param->para.afd_setting.mode = runtime_data->afd_setting.mode;

        memcpy(&runtime_data->afd_setting, &param->para.afd_setting, sizeof(struct afd_setting_t));
        break;
    case ALAFD_SET_PARAM_DETECTION:
        runtime_data->flag_detection = 1;
        runtime_data->flag_update_range = 1;
        memcpy(&runtime_data->detection, &param->para.detection, sizeof(struct afd_parameter_detect_t));
        break;
    case ALAFD_SET_PARAM_EXPOSURE_INFO:
        memcpy(&runtime_data->exposure_info, &param->para.exposure_info, sizeof(struct afd_exposure_info_t));
        break;
    case ALAFD_SET_PARAM_POWER_LINE_FREQUENCY_CONTROL:
        break;
    case ALAFD_SET_PARAM_ENABLE:
        runtime_data->afd_setting.enable = param->para.afd_setting.enable;
        runtime_data->afd_setting.fs_enable = param->para.afd_setting.fs_enable;
        runtime_data->afd_setting.enable_close_after_detection = param->para.afd_setting.enable_close_after_detection;
        runtime_data->afd_setting.mode = param->para.afd_setting.mode;
        break;
    case ALAFD_SET_PARAM_TUNING:
        //default:
        runtime_data->sensor_info.line_time_ns = param->para.tuning_setting.line_time_ns;
        runtime_data->sensor_info.sensor_height = param->para.tuning_setting.sensor_height;
        runtime_data->sensor_info.sensor_width = param->para.tuning_setting.sensor_width;

        runtime_data->tuning_data._50Hz_flicker_pos = param->para.tuning_setting._50Hz_flicker_pos;
        runtime_data->tuning_data._60Hz_flicker_pos = param->para.tuning_setting._60Hz_flicker_pos;
        runtime_data->tuning_data.default_hz = param->para.tuning_setting.default_hz;
        runtime_data->tuning_data.level_50 = param->para.tuning_setting.level_50;
        runtime_data->tuning_data.level_60 = param->para.tuning_setting.level_60;
        runtime_data->tuning_data.level_stable_th = param->para.tuning_setting.level_stable_th;
        runtime_data->tuning_data.base = param->para.tuning_setting.base;
        runtime_data->tuning_data.ratio_shift = param->para.tuning_setting.ratio_shift;

        //effect down_sampling
        runtime_data->tuning_data.jump_sample_do_avg_count = param->para.tuning_setting.jump_sample_do_avg_count;
        runtime_data->tuning_data.jump_sample_num = param->para.tuning_setting.jump_sample_num;

        //effect
        runtime_data->tuning_data.enable_2nd_region = param->para.tuning_setting.enable_2nd_region;
        runtime_data->tuning_data.ratio_max_range = param->para.tuning_setting.ratio_max_range;
        runtime_data->tuning_data.points_1st_region = param->para.tuning_setting.points_1st_region;
        runtime_data->tuning_data.points_2nd_region = param->para.tuning_setting.points_2nd_region;
        runtime_data->tuning_data.th_amp = param->para.tuning_setting.th_amp;
        runtime_data->tuning_data.th_amp_min = param->para.tuning_setting.th_amp_min;
        runtime_data->tuning_data.ratio_peak_dc = param->para.tuning_setting.ratio_peak_dc;
        runtime_data->tuning_data.ratio_peak_2nd = param->para.tuning_setting.ratio_peak_2nd;
        runtime_data->tuning_data.index_range_50_0 = param->para.tuning_setting.index_range_50_0;
        runtime_data->tuning_data.index_range_50_1 = param->para.tuning_setting.index_range_50_1;
        runtime_data->tuning_data.index_range_50_2 = param->para.tuning_setting.index_range_50_2;
        runtime_data->tuning_data.index_range_60_0 = param->para.tuning_setting.index_range_60_0;
        runtime_data->tuning_data.index_range_60_1 = param->para.tuning_setting.index_range_60_1;
        runtime_data->tuning_data.index_range_60_2 = param->para.tuning_setting.index_range_60_2;
        runtime_data->tuning_data.th_symm_min = param->para.tuning_setting.th_symm_min;
        runtime_data->tuning_data.size_symmetric = param->para.tuning_setting.size_symmetric;
        memcpy(runtime_data->tuning_data.ratio_symmetric, param->para.tuning_setting.ratio_symmetric, sizeof(UINT16) * 4);
        runtime_data->tuning_data.th_symm_60hz = param->para.tuning_setting.th_symm_60hz;
        runtime_data->tuning_data.peak_decay_count = param->para.tuning_setting.peak_decay_count;
        runtime_data->tuning_data.ratio_low_amp = param->para.tuning_setting.ratio_low_amp;

        break;

    case ALAFD_SET_PARAM_TUNING_FS:

        runtime_data->tuning_data.fs_th_dc_amp = param->para.fs_tuning_setting.fs_th_dc_amp;
        runtime_data->tuning_data.amp_2nd_peak_ratio = param->para.fs_tuning_setting.amp_2nd_peak_ratio;
        memcpy(&runtime_data->tuning_data.freq_box[0], &param->para.fs_tuning_setting.freq_box[0], sizeof(UINT16)* VOTE_NUM);
        runtime_data->tuning_data.vote_delta_th = param->para.fs_tuning_setting.vote_delta_th;
        runtime_data->tuning_data.fs_th_stable = param->para.fs_tuning_setting.fs_th_stable;
        runtime_data->tuning_data.fs_enable_close_after_change_mode = param->para.fs_tuning_setting.fs_enable_close_after_change_mode;
        break;

    case ALAFD_SET_PARAM_FSRESULT:
        runtime_data->afd_setting.fs_enable = 1;
        memcpy(&runtime_data->fs_result, &param->para.fs_result, sizeof(AFD_FSRESULT));
        break;
    }

    return _AL_AFDLIB_SUCCESS;
}

/* get */
UINT32  alAFDLib_get_param(struct allib_afd_get_parameter_t *param, void *afd_dat)
{
    AFD_RUNTIME_DATA *runtime_data = (AFD_RUNTIME_DATA*)afd_dat;
    
    if (NULL == param || NULL == afd_dat)
    {
        return _AL_AFDLIB_GET_PARAM_INPUT_NULL;
    }

    switch (param->type)
    {
    default:
        return _AL_AFDLIB_GET_PARAM_INPUT_TYPE;

    case ALAFD_GET_PARAM_SENSOR_INFO:
        memcpy(&param->para.sensor_info, &runtime_data->sensor_info, sizeof(struct afd_sensor_info_t));
        break;
    case ALAFD_GET_PARAM_SETTING:
        memcpy(&param->para.afd_setting, &runtime_data->afd_setting, sizeof(struct afd_setting_t));
        break;
    case ALAFD_GET_PARAM_DETECTION:
        memcpy(&param->para.detection, &runtime_data->detection, sizeof(struct afd_parameter_detect_t));
        break;
    case ALAFD_GET_PARAM_POWER_LINE_FREQUENCY_CONTROL:
        break;
    }
    return _AL_AFDLIB_SUCCESS;
}

/* process */
UINT32  alAFDLib_push_stats(void *g_channel_rows_avg_data, void *afd_dat)
{
    UINT32 err = _AL_AFDLIB_SUCCESS;
    AFD_RUNTIME_DATA *runtime_data = (AFD_RUNTIME_DATA*)afd_dat;

    if (NULL == afd_dat)
    {
        return _AL_AFDLIB_PROCESS_MEM_NULL;
    }

    if (NULL == g_channel_rows_avg_data)
    {
        return _AL_AFDLIB_PROCESS_STATS_NULL;
    }

    runtime_data->output_report.flag_enable = runtime_data->afd_setting.enable;

    err = _dispatch_avgG_channel_stats(g_channel_rows_avg_data, runtime_data);

    if (_AL_AFDLIB_SUCCESS != err)
        return err;
    _push_stats(runtime_data);

    //move forward status_queue_index_0 after push stats
    runtime_data->status_queue_index_0 = (runtime_data->status_queue_index_0 + 1) % AFD_STATS_QUEUE_SIZE;

    return _AL_AFDLIB_SUCCESS;
}

UINT32  alAFDLib_kernel_process(void *afd_dat, struct allib_afd_output_data_t *output)
{
    UINT32 err = _AL_AFDLIB_SUCCESS;
    AFD_RUNTIME_DATA *runtime_data = (AFD_RUNTIME_DATA*)afd_dat;

    memset(output, 0, sizeof(struct allib_afd_output_data_t));
    if (NULL == output)
    {
        return _AL_AFDLIB_PROCESS_OUTPUT_NULL;
    }

    if (NULL == afd_dat)
    {
        return _AL_AFDLIB_PROCESS_MEM_NULL;
    }

    if (1 == runtime_data->afd_setting.fs_enable)
    {
        LOG_DEBUG("start fs calc");

        AFDLib_flickerSensor_process(runtime_data, output);

        LOG_DEBUG("finished fs calc");

    }

    LOG_DEBUG("ATF Global calc on or not : %d", runtime_data->afd_setting.enable);
    if (1 == runtime_data->afd_setting.enable)
    {
        runtime_data->output_report.flag_enable = 1;
        
        //move back status_queue_index_0 because push stats API would move forward status_queue_index_0
        runtime_data->status_queue_index_0 = (runtime_data->status_queue_index_0 - 1) < 0 ? AFD_STATS_QUEUE_SIZE - 1 : (runtime_data->status_queue_index_0 - 1);
        //runtime_data->status_queue_index_1 = (runtime_data->status_queue_index_1 - 2) < 0 ? AFD_STATS_QUEUE_SIZE - 1 : (runtime_data->status_queue_index_1 - 2);

        memset(&runtime_data->working_buffer, 0, sizeof(AFD_WORKING_BUFFER));

        switch (runtime_data->detection.afd_level_detect_th)
        {
        default:
        case AFD_LEVEL_STRONG:
            runtime_data->conditions.th_stable = runtime_data->tuning_data.stable_th_0;
            break;
        case AFD_LEVEL_MIDDLE:
            runtime_data->conditions.th_stable = runtime_data->tuning_data.stable_th_1;
            break;
        case AFD_LEVEL_WEAK:
            runtime_data->conditions.th_stable = runtime_data->tuning_data.stable_th_2;
            break;
        }

        if (TRUE == _check_stats_stable(runtime_data))
        {
            runtime_data->working_buffer.result.detect_status = AFD_DETECT_STATUS_STATS_NOT_STABLE;
        }
        else
        {
            if (1 == runtime_data->flag_update_range)
            {
                err = _compute_flicker_range(runtime_data);
                if (_AL_AFDLIB_SUCCESS != err)
                    return err;
                runtime_data->flag_update_range = 0;
            }
            _flicker_detection(runtime_data);
        }

        runtime_data->status_queue_index_0 = (runtime_data->status_queue_index_0 + 1) % AFD_STATS_QUEUE_SIZE;
        runtime_data->status_queue_index_1 = 0;

        runtime_data->queue_stable[runtime_data->stable_queue_index].detect_status = runtime_data->working_buffer.result.detect_status;
        LOG_DEBUG("detect_status: %d ", runtime_data->working_buffer.result.detect_status);
        runtime_data->queue_stable[runtime_data->stable_queue_index].detect_hz = runtime_data->working_buffer.result.detect_hz;
        runtime_data->stable_queue_index = (runtime_data->stable_queue_index + 1) % AFD_STABLE_QUEUE_SIZE;

        //printf("3 current detect freq is %d \n", runtime_data->working_buffer.result.detect_hz);
        LOG_DEBUG("3 current detect freq is %d", runtime_data->working_buffer.result.detect_hz);
        //printf("detect banding : %d \n", runtime_data->working_buffer.is_banding);
        runtime_data->output_report.flag_bending = runtime_data->working_buffer.is_banding;
        runtime_data->output_report.afd_mode = runtime_data->working_buffer.result.detect_hz;
        _update_stable_queue_report(runtime_data);
        //printf("stable frequence mode : %d \n", runtime_data->output_report.afd_mode_stable);
    }

    memcpy(output, &runtime_data->output_report, sizeof(struct allib_afd_output_data_t));
    //printf("detect banding : %d \n", output->flag_bending);
    //printf("stable frequence mode : %d \n", output->afd_mode_stable);

    return _AL_AFDLIB_SUCCESS;
}

UINT32  allib_afd_loadfunc(struct allib_afd_runtime_obj_t *afd_run_obj, UINT32 identityid)
{
    if (NULL == afd_run_obj)   // check input data address validity
    {
        return _AL_AFDLIB_INIT_NULL_OBJ;
    }

    afd_run_obj->identityid = identityid;   // tag identity ID to object, for framework recognition

    afd_run_obj->initial = alAFDLib_init;
    afd_run_obj->deinit = alAFDLib_deinit;
    afd_run_obj->set_param = alAFDLib_set_param;
    afd_run_obj->get_param = alAFDLib_get_param;
    afd_run_obj->push_stats = alAFDLib_push_stats;
    afd_run_obj->kernel_process = alAFDLib_kernel_process;

    return _AL_AFDLIB_SUCCESS;
}