#include "./include/table.h"

int findIndex(float lv, int tableLen, int(*table)[3])
{
	float trigger;
	int index = 0;
	for (int i = 0; i < tableLen; i++)
	{
		trigger = table[i][0] / 100.0f;
		if (lv - trigger > 0.0001f)
		{
			index = i;
		}
	}
	return index;
}

int findIndex(float lv, int tableLen, int(*table)[4])
{
	float trigger;
	int index = 0;
	for (int i = 0; i < tableLen; i++)
	{
		trigger = table[i][0] / 100.0f;
		if (lv - trigger > 0.0001f)
		{
			index = i;
		}
	}
	return index;
}

UINT32 Interpolation(float lv, int index, int tableLen, int(*table)[3])
{
	float trigger1 = table[index][0]/100.0f;
	INT32 target1 = table[index][1];

	if (lv - trigger1 < 0.001f || index + 1 == tableLen)
	{
		return target1;
	}

	float trigger2 = table[index + 1][0]/100.0f;
	INT32 target2 = table[index + 1][1];

	UINT32 result = target1 + (lv - trigger1) * (target2 - target1) / (trigger2 - trigger1);

	return result;
}


void   checkExpoTable(float expIndex, int evIndex, int totalIndex, int(*table)[4], UINT32& newExposureTime, UINT32& newExposureGain, void* ae_runtime_dat)
{
	if ((evIndex + 1) >= totalIndex)
	{
	    newExposureTime = table[totalIndex - 1][1];
	    newExposureGain = table[totalIndex - 1][2];
	    return;
    }
	ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ae_runtime_dat;
	float trigger1 = table[evIndex][0] / 100.0f;
	float expTime = table[evIndex][1];
	float gain = table[evIndex][2];

	float ratio = pow(1.03, expIndex - trigger1);

	if (table[evIndex][3] == 0) //0: gain prio
	{
		float tmpGain = gain * ratio;
		if (tmpGain >= table[evIndex + 1][2])
		{
			gain = table[evIndex + 1][2];
			expTime = expTime * tmpGain / gain;
		}
		else
		{
			gain = tmpGain;
		}
	}
	else //default 1: exptime prio
	{
		float tmpExpTime = expTime * ratio;
		if (tmpExpTime >= table[evIndex + 1][1])
		{
			expTime = table[evIndex + 1][1];
			gain = gain * tmpExpTime / expTime;
		}
		else
		{
			expTime = tmpExpTime;
		}
	}

	//check sensor step
	int TlineCount = expTime / (ae_obj->ae_current_set_param.para.preview_sensor_info.exposuretime_per_exp_line_ns / 1000.0f) + 0.5f;
	float expTimeAfterLine = TlineCount * (ae_obj->ae_current_set_param.para.preview_sensor_info.exposuretime_per_exp_line_ns / 1000.0f);
	float gainAfterLine = (expTime * gain) / expTimeAfterLine;

	newExposureTime = expTimeAfterLine;
	newExposureGain = gainAfterLine * 100;
}

void isDynamicScene(void* ae_runtime_dat)
{
	ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ae_runtime_dat;
	UINT32 n_hist = 1;
	UINT32 n_brightYHist = 0;
	UINT32 n_darkYHist = 0;
	INT16 brightYHistTh = ae_obj->ae_tuning_para.iAE_para.iaeHistRatio[0];
	INT16 darkYHistTh = ae_obj->ae_tuning_para.iAE_para.iaeHistRatio[1];
	INT16 brightRatioInTh = ae_obj->ae_tuning_para.iAE_para.iaeHistRatio[2];
	INT16 darkRatioInTh = ae_obj->ae_tuning_para.iAE_para.iaeHistRatio[3];
	INT16 brightRatioOutTh = ae_obj->ae_tuning_para.iAE_para.iaeHistRatio[4];
	INT16 darkRatioOutTh = ae_obj->ae_tuning_para.iAE_para.iaeHistRatio[5];
	LOG_DEBUG("isDynamic scene: wdrmode %d", (int)ae_obj->ae_tuning_para.iAE_para.WDRMode);

	if (ae_obj->ae_tuning_para.iAE_para.WDRMode == WDR_WDRTARGETTABLE_DEPENDHIST)
	{
	    for (int i = 0; i < AL_MAX_HIST_NUM; i++)
	    {
	    	n_hist += ae_obj->ae_runtime_data.udY_hist[i];
	    	if (i >= brightYHistTh)
	    	{
	    		n_brightYHist += ae_obj->ae_runtime_data.udY_hist[i];
	    	}
	    	if (i <= darkYHistTh)
	    	{
	    		n_darkYHist += ae_obj->ae_runtime_data.udY_hist[i];
	    	}
	    }
	    
	    LOG_DEBUG("isDynamic scene: brightYHist %d  darkYHist %d n_hist %d", n_brightYHist, n_darkYHist, n_hist);
	    if (!ae_obj->ae_runtime_data.isDynamic && (n_brightYHist * 100 / n_hist) >= brightRatioInTh && (n_darkYHist * 100 / n_hist) <= darkRatioInTh)
	    {
	    	ae_obj->ae_runtime_data.isDynamic = true;
	    }
	    else if (ae_obj->ae_runtime_data.isDynamic && (n_brightYHist * 100 / n_hist) < brightRatioOutTh || (n_darkYHist * 100 / n_hist) > darkRatioOutTh)
	    {
	    	ae_obj->ae_runtime_data.isDynamic = false;
	    }
	}
	else if (ae_obj->ae_tuning_para.iAE_para.WDRMode == WDR_WDRTARGETTABLE_DEPENDIAE)
	{
		if (!ae_obj->ae_runtime_data.isDynamic && ae_obj->ae_runtime_data.uwmidtones_gain > ae_obj->ae_tuning_para.iAE_para.wdrTargetGainInTh)
		{
			ae_obj->ae_runtime_data.isDynamic = true;
		}
		else if (ae_obj->ae_runtime_data.isDynamic && ae_obj->ae_runtime_data.uwmidtones_gain < ae_obj->ae_tuning_para.iAE_para.wdrTargetGainOutTh)
		{
			ae_obj->ae_runtime_data.isDynamic = false;
		}
	}
}

UINT32 updateTarget(void* ae_runtime_dat)
{
	UINT32 errRet = _AL_3ALIB_SUCCESS;

	ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ae_runtime_dat;

	UINT32 targetBri = 0;   //scale 100

	int lumaModify = 0;   //offset
	int targetIndexMax = g_AE_NormalTarget.totalIndex;
	int mode = g_AE_NormalTarget.eSceneMode;

	if (ae_obj->ae_tuning_para.iAE_para.WDRMode == WDR_WDRTARGETTABLE_DEPENDIAE || ae_obj->ae_tuning_para.iAE_para.WDRMode == WDR_WDRTARGETTABLE_DEPENDHIST)
	{
		isDynamicScene(ae_runtime_dat);
		if (ae_obj->ae_runtime_data.isDynamic)
		{
            mode = PREVIEW;
		}
	}

	float expIndex = ae_obj->ae_runtime_data.uExpIndex;

	int lv_index = findIndex(expIndex, targetIndexMax, ae_obj->ae_shadow_set_param.para.ae_target_table[mode].table);
	targetBri = Interpolation(expIndex, lv_index, targetIndexMax, ae_obj->ae_shadow_set_param.para.ae_target_table[mode].table);

	LOG_DEBUG("mode/expIndex/lv_index: %d/%f/%d", mode, expIndex, lv_index);
	targetBri += lumaModify;
	ae_obj->ae_tuning_para.udTargetmean = targetBri;
	return errRet;
}

UINT32 matchExpoTable(void* ptAEBuffer, void* ae_output, INT16 wBv1000)
{
    UINT32 errRet = _AL_3ALIB_SUCCESS;

    ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ptAEBuffer;
	al_ae_exp_param_t* ae_out = (al_ae_exp_param_t*)ae_output;

	UINT32 newExposureTime = 0;
	UINT32 newExposureGain = 100;

	UINT32 maxExposureTime = ae_obj->ae_shadow_set_param.para.preview_sensor_info.sw_max_exptime;
	UINT32 minExposureTime = ae_obj->ae_shadow_set_param.para.preview_sensor_info.sw_min_exptime;

	UINT32 udFn = ae_obj->curHWSetting.FN_x1000;
	int aemode = g_AE_CaptureAutoTable.eAETableID;
	//int aemode = ae_obj->ae_current_set_param.para.ae_expo_mode;
	int totalIndex = g_AE_CaptureAutoTable.totalIndex;

	//UINT16 lv = (wBv1000 + 1500) > 0 ? wBv1000 + 1500 : 0;
	//int evIndex = findIndex(lv, totalIndex, (int**)ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table);

    
	ae_obj->ae_runtime_data.uExpIndex = log10(1.0f / pow(2, (wBv1000 - ae_obj->ae_runtime_data.minExpBV) / 1000.0f)) / log10(1.03);

	ae_obj->ae_runtime_data.uExpIndex = (ae_obj->ae_runtime_data.uExpIndex < 0.0f) ? 0.0f : ae_obj->ae_runtime_data.uExpIndex;
	LOG_DEBUG("[CheckTable] after expindex %f, curbv %d, minExpBV %d", ae_obj->ae_runtime_data.uExpIndex, wBv1000, ae_obj->ae_runtime_data.minExpBV);

	float expIndex = ae_obj->ae_runtime_data.uExpIndex;
	int evIndex = 0;
	if (expIndex - (ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table[totalIndex - 1][0] / 100.0f) >= 0.001f)
	{
		evIndex = totalIndex;
		newExposureTime = ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table[totalIndex - 1][1];
		newExposureGain = ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table[totalIndex - 1][2] * 100;
	}
	else
	{
        evIndex = findIndex(expIndex, totalIndex, ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table);
		checkExpoTable(expIndex, evIndex, totalIndex, ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table, newExposureTime, newExposureGain, ae_obj);
	}


	ae_out->udexposure_time = newExposureTime;
	ae_out->udISO = newExposureGain;

    return errRet;
}

UINT32 matchExpoTableNotATF(void* ptAEBuffer, void* ae_output, INT16 wBv1000)
{
	UINT32 errRet = _AL_3ALIB_SUCCESS;

	ae_corelib_obj_t* ae_obj = (ae_corelib_obj_t*)ptAEBuffer;
	al_ae_exp_param_t* ae_out = (al_ae_exp_param_t*)ae_output;

	UINT32 newExposureTime = 0;
	UINT32 newExposureGain = 100;

	UINT32 maxExposureTime = ae_obj->ae_shadow_set_param.para.preview_sensor_info.sw_max_exptime;
	UINT32 minExposureTime = ae_obj->ae_shadow_set_param.para.preview_sensor_info.sw_min_exptime;

	UINT32 udFn = ae_obj->curHWSetting.FN_x1000;
	int aemode = AETABLE_PRECIEW_AUTO;//g_AE_CaptureAutoTable.eAETableID;
	//int aemode = ae_obj->ae_current_set_param.para.ae_expo_mode;
	int totalIndex = g_AE_CaptureAutoTable.totalIndex;

	UINT16 lv = (wBv1000 + 1500) > 0 ? wBv1000 + 1500 : 0;
	int evIndex = findIndex(lv, totalIndex, ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table);

	//LOG_DEBUG("ae mode/expoIndex/totalIndex: %d/%d/%d", aemode, evIndex, totalIndex);


	newExposureTime = ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table[evIndex][1];
	newExposureGain = alAELib_CalculateIsoSpeed(udFn, newExposureTime, wBv1000);

	if (newExposureGain <= 100)
	{
		// min fix ios, modify time
		newExposureTime = max(minExposureTime, newExposureGain * newExposureTime / 100);
		newExposureGain = 100;

	}
	else if (newExposureGain >= ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table[evIndex][2] * 100)
	{
		// max fix ios, modify time
		newExposureTime = min(maxExposureTime, newExposureGain * newExposureTime / ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table[evIndex][2] / 100);
		newExposureGain = ae_obj->ae_shadow_set_param.para.ae_expo_table[aemode].table[evIndex][2] * 100;
	}

	ae_out->udexposure_time = newExposureTime;
	ae_out->udISO = newExposureGain;

	return errRet;
}
