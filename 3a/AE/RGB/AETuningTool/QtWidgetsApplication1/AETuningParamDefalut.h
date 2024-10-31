#pragma once
struct parseParam defaultParam =
{   //paramVer
	{
	    1.04,   //minorVer
	    24724,//majorVer
    },
	//ParamSingleInt
	1, 8,  //ucAntiFlickerMode
	1, 0,  //manual_isolevel
	1, 0,  //manual_exptime
	1, 0,  //manual_adgain
	1, 0,  //manual_ae_target
	1, 0,  //ae_metering_mode
	1, 0,  //converge_speedlv
	1, 1,  //ae_target_mode
	1, 2,  //ae_expo_mode
	//iaeHistEn
	1,
	//ParamIAEHist
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, //hist_dark_high
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, //hist_dark_low
	200, 200, 200, 200, 200, 200, 200, 200, 200, 200, //hist_sat_high
	200, 200, 200, 200, 200, 200, 200, 200, 200, 200, //hist_sat_low
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, //hist_dark_range_end0
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, //hist_dark_range_end1
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, //hist_dark_range_end2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //hist_dark_range_start0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //hist_dark_range_start1
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //hist_dark_range_start2
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //hist_sat_pushdown_range_end0
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //hist_sat_pushdown_range_end1
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //hist_sat_pushdown_range_end2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //hist_sat_pushdown_range_start0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //hist_sat_pushdown_range_start1
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //hist_sat_pushdown_range_start2
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //hist_sat_pushup_range_end0
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //hist_sat_pushup_range_end1
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //hist_sat_pushup_range_end2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //hist_sat_pushup_range_start0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //hist_sat_pushup_range_start1
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //hist_sat_pushup_range_start2
	100, 100, 100, 100, 100, 100, 100, 100, 100, 100, //max_adjust_ratio
	100, 100, 100, 100, 100, 100, 100, 100, 100, 100, //min_adjust_ratio
	1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, //max_midtones_gain
	200, 200, 200, 150, 150, 150, 150, 150, 150, 150, //max_shadows_gain
	500, 500, 500, 500, 500, 500, 500, 500, 500, 500,//max_isp_midtones_gain
	//iaeBVThEn
	1,
	//wdrEnable
	1,
	//ParamIAEBVTh
	280, 300, 310, 315, 332, 332, 332, 332, 332, 332, 332, 332, 332, 332, 332, 332, 332, 332,
	//paramIAEHistRatio
    116, 2, 4, 30, 2, 50, 0, 400, 110,
	//targetTable
	{
		//targetTable normal sdrtarget
		
		//aeTargetEn
		1,
		//aetargetTable
		0, 4800, 0,    //expindex, target, default0
		24556, 4800, 0,
		29383, 4500, 0,
		31728, 4000, 0,
		33484, 3000, 0,
		35829, 2500, 0,
		38174, 1800, 0,
		39737, 1000, 0,

		//targetTable prview  wdrtarget

		//aeTargetEn
		1,
		//aetargetTable
		0, 3000, 0,    //expindex, target, default0
		24556, 3500, 0,
		29383, 4100, 0,
		31728, 4000, 0,
		33484, 3000, 0,
		35829, 2500, 0,
		38174, 1800, 0,
		39737, 1000, 0,

		//targetTable night

		//aeTargetEn
		1,
		//aetargetTable
		0, 4800, 0,    //expindex, target, default0
		24556, 4800, 0,
		29383, 4500, 0,
		31728, 4000, 0,
		33484, 3000, 0,
		35829, 2500, 0,
		38174, 1800, 0,
		39737, 1000, 0,

		//targetTable face

		//aeTargetEn
		1,
		//aetargetTable
		0, 4800, 0,    //expindex, target, default0
		24556, 4800, 0,
		29383, 4500, 0,
		31728, 4000, 0,
		33484, 3000, 0,
		35829, 2500, 0,
		38174, 1800, 0,
		39737, 1000, 0,

		//targetTable sport

		//aeTargetEn
		1,
		//aetargetTable
		0, 4800, 0,    //expindex, target, default0
		24556, 4800, 0,
		29383, 4500, 0,
		31728, 4000, 0,
		33484, 3000, 0,
		35829, 2500, 0,
		38174, 1800, 0,
		39737, 1000, 0,
	},
	//expousureTable
	{
		//PRECIEW_AUTO
		1, //aeExpoEn
		//aeExpoTable
		0, 7000, 64, 0,    //bv, time, maxgain
        1800, 7000, 32, 0,
        2500, 7000, 16, 0,
        3500, 4166, 16, 0,
        4000, 4166, 16, 0,
        5500, 4166, 12, 0,
        7000, 4166, 8, 0,
        8500, 2000, 4, 0,

		//CAPTURE_AUTO
		1, //aeExpoEn
		//aeExpoTable
		213, 6, 1, 1,         //expindex, time, maxgain, priflag
		24556, 2000, 4, 1,  
		29383, 4166, 8, 0,
		31728, 4166, 16, 0,
		33484, 7000, 16, 0,
		35829, 7000, 32, 0,
		38174, 7000, 64, 0,
		39737, 11111, 64, 0,

		//CAPTURE_NIGHT
		1, //aeExpoEn
		//aeExpoTable
		0, 7000, 64, 0,    //bv, time, maxgain
        1800, 7000, 32, 0,
        2500, 7000, 16, 0,
        3500, 4166, 16, 0,
        4000, 4166, 16, 0,
        5500, 4166, 12, 0,
        7000, 4166, 8, 0,
        8500, 2000, 4, 0,

		//CAPTURE_HDR
		1, //aeExpoEn
		//aeExpoTable
		0, 7000, 64, 0,    //bv, time, maxgain
        1800, 7000, 32, 0,
        2500, 7000, 16, 0,
        3500, 4166, 16, 0,
        4000, 4166, 16, 0,
        5500, 4166, 12, 0,
        7000, 4166, 8, 0,
        8500, 2000, 4, 0,
	},
	//multicamera
	{
		0, //multiCamMode
		-3000,//BVStart
		2000, //BVEnd
		1.0, //slaveWeightStart
		0.0, //slaveWeightEnd
		5,   //varianceStableTh
		200, //ymeanStableTh
		5,   //stableFrame
		3, //cameraMixROI.left.leftstart
		16,//cameraMixROI.left.rightend
		0, //cameraMixROI.left.top
		16,//cameraMixROI.left.bottom
		0, //cameraMixROI.right.leftstart
		13,//cameraMixROI.right.rightend
		0, //cameraMixROI.right.top
		16,//cameraMixROI.right.bottom
	},
	//configInfo
	{
		1, //aeenable
		4, //pipedelay
		2200, //fNumberx1000
		3000, //maxfps
		1500, //minfps
		6400, //maxGain
		100, //minGain
		65535, //maxLineCnt
		1, //minLineCnt
		11111,//swMaxExptime
		100,//swMinExptime
		5633,//exptimePerLineNs
	},
};