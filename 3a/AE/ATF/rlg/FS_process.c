#include "FS_process.h"
#include "alAFDLib_Error_Code.h"


// peak amp threshold
BOOL _flicker_sensor_amp_peak_scan(AFD_RUNTIME_DATA* runtime_data)
{
    AFD_FSRESULT fs = runtime_data->fs_result;
    FS_WORKING_BUFFER* working = &runtime_data->fs_working_buffer;

    UINT32 amp_max = fs.freq1.amp;
    UINT16 th_amp = runtime_data->tuning_data.fs_th_dc_amp;

    LOG_DEBUG("FS peak amp : %d", amp_max);

    if (amp_max < (UINT32)(th_amp * th_amp))
    {
        working->result.detect_status = FS_DETECT_STATUS_PEAK_NOT_ENOUGH;
        LOG_DEBUG("FS peak amp not enough, detect_status: %d", working->result.detect_status);
        return FALSE;
    }

    LOG_DEBUG("FS peak amp enough");
    return TRUE;

}

//peak to peak ratio
BOOL _flicker_sensor_amp_pp_ration(AFD_RUNTIME_DATA* runtime_data)
{
    AFD_FSRESULT fs = runtime_data->fs_result;
    FS_WORKING_BUFFER* working = &runtime_data->fs_working_buffer;

    UINT32 amp_max = fs.freq1.amp;
    UINT32 amp_max2 = fs.freq2.amp;
    UINT32 ratio_peak_2nd = runtime_data->tuning_data.amp_2nd_peak_ratio; //1024*0.6
    UINT32 amp_th = amp_max * ratio_peak_2nd >> 10;

    LOG_DEBUG("FS peak amp_max : %d", amp_max);
    LOG_DEBUG("FS peak amp2 : %d", amp_max2);

    // 2nd peak over 1st * ratio
    if (amp_max2 > amp_th )
    {
        working->result.detect_status = FS_DETECT_STATUS_PEAK_2ND_OVER_TH;
        LOG_DEBUG("FS 2nd peak detect_status: %d", working->result.detect_status);
        return FALSE;
    }

    LOG_DEBUG("FS 2nd peak amp not over 1st ratio_th");

    return TRUE;

}

BOOL _flicker_sensor_vote_box(AFD_RUNTIME_DATA* runtime_data)
{
    AFD_FSRESULT fs = runtime_data->fs_result;
    FS_WORKING_BUFFER* working = &runtime_data->fs_working_buffer;
    //MUST BANDING BUT EIGHTER IN VOTE BOX OR NOT
    working->is_banding = TRUE;

    UINT8 multi[7] = {1,1,1,1,1,1,1};  //base freq * multi
    float freq = fs.freq1.freq;
    UINT8 index = 255;
    INT32 delta = 0;
    LOG_DEBUG("freq max is %d", freq);

    UINT16 delta_min = runtime_data->tuning_data.vote_delta_th;

    // judge freq in box
    for (int i = 0; i < VOTE_NUM; i++)
    {
        multi[i] = freq / BOX[i];

        INT32 diff_0 = freq - multi[i] * BOX[i];
        INT32 diff_1 = (multi[i] + 1) * BOX[i] - freq;
        delta = MIN(ABS(diff_0), ABS(diff_1)); //±¶ÆµÖ±½Ócover

        if (delta < delta_min)
        {
            index = i;
            delta_min = delta;
        }
    }

    if (index == 255)
    {
        working->result.detect_status = FS_DETECT_STATUS_STABLE_NOT_IN_BOX;
        working->result.detect_hz = AFD_MODE_OFF;
        LOG_DEBUG("FS box vote failed, stastus: %d, index : %d", working->result.detect_status, index);
        return FALSE;
    }
    LOG_DEBUG("freq in vote box,index is %d", index + 1);

    working->result.detect_status = FS_DETECT_STATUS_OK;
    //working->result.detect_hz = BOX[index] * multi[index];
    working->result.detect_hz = index + 1;

    return TRUE;

}


void _update_fs_stable_queue_report(AFD_RUNTIME_DATA* runtime_data)
{
    UINT16 i = 0;
    UINT32 freq_in = runtime_data->fs_queue_stable[runtime_data->fs_stable_queue_index].detect_hz;

    LOG_DEBUG("fs_stable_queue_index is %d", runtime_data->fs_stable_queue_index);

    runtime_data->fs_stable_queue_index = (runtime_data->fs_stable_queue_index + 1) % AFD_STABLE_QUEUE_SIZE;  //next index
    UINT32 freq_out = runtime_data->fs_queue_stable[runtime_data->fs_stable_queue_index].detect_hz;

    LOG_DEBUG("fs freq in is %d", freq_in);
    LOG_DEBUG("fs freq out is %d", freq_out);

    for (i = 0; i < AFD_STABLE_QUEUE_SIZE; i++)
    {
        if (freq_in == runtime_data->fs_queue_stable[i].detect_hz)
            CNT[i]++;
        if (freq_out != 0 && freq_out == runtime_data->fs_queue_stable[i].detect_hz)
            CNT[i]--;
    }

    // find peak
    UINT16 index;
    index = 0;
    UINT16 cnt_max = CNT[0];

    for (i = 1; i < AFD_STABLE_QUEUE_SIZE; i++)
    {
        if (cnt_max < CNT[i])
        {
            index = i;
            cnt_max = CNT[i];
        }
    }

    // NO peak
    //if (65535 == index)
    //{
    //    runtime_data->fs_stable_result.detect_status = FS_DETECT_STATUS_PEAK_NOT_ENOUGH;
    //    runtime_data->fs_stable_result.detect_hz = AFD_MODE_OFF;
    //}
    //else
    {
        runtime_data->tuning_data.fs_th_stable = 5;

        LOG_DEBUG("fs peak freq cnt: %d", CNT[index]);
        LOG_DEBUG("fs peak th cnt: %d", runtime_data->tuning_data.fs_th_stable);

        if (CNT[index] >= runtime_data->tuning_data.fs_th_stable)
        {
            runtime_data->fs_stable_result.detect_status = runtime_data->fs_queue_stable[index].detect_status;
            runtime_data->fs_stable_result.detect_hz = runtime_data->fs_queue_stable[index].detect_hz;
            LOG_DEBUG("fs peak freq is %d", runtime_data->fs_queue_stable[index].detect_hz);

        }
        else
        {
            runtime_data->fs_stable_result.detect_status = FS_DETECT_STATUS_STABLE_CNT_NOT_ENOUGH;
            runtime_data->fs_stable_result.detect_hz = AFD_MODE_OFF;
            LOG_DEBUG("fs peak cnt is not enough");
        }
    }

    if (AFD_MODE_OFF == runtime_data->fs_stable_result.detect_hz)
    {
        runtime_data->afd_setting.enable = 1;
    }
    else
    {
        //fs freq is varified right, bypass stats calculate of global 50/60
        runtime_data->afd_setting.enable = 0;

        runtime_data->afd_setting.mode = runtime_data->fs_stable_result.detect_hz;

        if (1 == runtime_data->tuning_data.enable_close_after_change_mode)
        {
            runtime_data->afd_setting.fs_enable = 0;
            memset(&runtime_data->fs_queue_stable, 0, sizeof(AFD_STABLE_DATA) * AFD_STABLE_QUEUE_SIZE);
        }
    }

    runtime_data->output_report.flag_enable = runtime_data->afd_setting.enable;
    runtime_data->output_report.afd_mode_stable = runtime_data->afd_setting.mode;
}

// freq vote
void _flicker_sensor_freq_box(AFD_RUNTIME_DATA* runtime_data)
{
    AFD_FSRESULT fs = runtime_data->fs_result;

    // vote for freq mode  -- fs.freq1.freq --> runtime_data->working_buffer.detect_hz

    if (_flicker_sensor_vote_box(runtime_data))
    {
        //stable result
        runtime_data->fs_queue_stable[runtime_data->fs_stable_queue_index].detect_status = runtime_data->fs_working_buffer.result.detect_status;
        //LOG_DEBUG("detect_status: %d ", runtime_data->working_buffer.result.detect_status);
        runtime_data->fs_queue_stable[runtime_data->fs_stable_queue_index].detect_hz = runtime_data->fs_working_buffer.result.detect_hz;
        LOG_DEBUG("detect_hz: %d ", runtime_data->fs_working_buffer.result.detect_hz);
        _update_fs_stable_queue_report(runtime_data);
    }
    else
    {
        //isp global calc ON
        runtime_data->afd_setting.enable = TRUE;
    }

    //runtime_data->afd_setting.fs_enable = 0;

    //comfire banding is existing
    runtime_data->output_report.flag_bending = runtime_data->fs_working_buffer.is_banding;
    //runtime_data->output_report.afd_mode = runtime_data->fs_working_buffer.result.detect_hz;
}

UINT32 AFDLib_flickerSensor_process(void* afd_dat, struct allib_afd_output_data_t* output)
{
    UINT32 err = _AL_AFDLIB_SUCCESS;
    AFD_RUNTIME_DATA* runtime_data = (AFD_RUNTIME_DATA*)afd_dat;

    if (NULL == output)
    {
        return _AL_AFDLIB_PROCESS_OUTPUT_NULL;
    }

    if (NULL == afd_dat)
    {
        return _AL_AFDLIB_PROCESS_MEM_NULL;
    }

    // fs out freq result process
    if (_flicker_sensor_amp_peak_scan(runtime_data) && _flicker_sensor_amp_pp_ration(runtime_data))
    {
        _flicker_sensor_freq_box(runtime_data);
    }
    else
    {
        //isp global calc ON
        runtime_data->afd_setting.enable = TRUE;
    }

    memcpy(output, &runtime_data->output_report, sizeof(struct allib_afd_output_data_t));
    //printf("detect banding : %d \n", output->flag_bending);
    //printf("stable frequence mode : %d \n", output->afd_mode_stable);
    return _AL_AFDLIB_SUCCESS;
}