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

/******************************************************************************
 * File name: allib_awb.h
 * Create Date:
 *
 * Comment:
 * Describe common difinition of alAWBLib algo type definition / interface
 *****************************************************************************/
#ifndef _ALTEK_AWB_LIB_
#define _ALTEK_AWB_LIB_

#ifdef __cplusplus
extern "C"
{
#endif
 
//#include "MyLog.h"


#define colorRegionsCount  8
#define misColorRegionsCount  8
#define CCTLevel  7
#define LightLevel  5
#define lightSourceNum  7
#define ccmNum  5
#define shiftLevel  13


struct parseParam
{
    int lightSourceTable[lightSourceNum][2];
    int cctWeightTable[LightLevel][CCTLevel];
    int lvWeight[5];
    int greyRegions[lightSourceNum][8];
    int colorRegions[colorRegionsCount][8];
    int misColorRegions[misColorRegionsCount][8];
    int colorRegionsParams[colorRegionsCount][3];
    int ccmParams[ccmNum][9];
    int shiftRG[LightLevel][shiftLevel][shiftLevel];
    int shiftBG[LightLevel][shiftLevel][shiftLevel];
};

enum altek_awb_error_code {
    _AL_AWBLIB_SUCCESS               = (0x0000),

	_AL_AWBLIB_GLOBAL_ERR_OFFSET     = (0xB000),

    _AL_AWBLIB_INVALID_INPUT   = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0100)),
    _AL_AWBLIB_INVALID_TABLE_SIZE,
    _AL_AWBLIB_INVALID_CALIB_OTP,
	_AL_AWB_ERROR_INVALID_STABLE_SIZE,
	_AL_AWB_ERROR_INVALID_STABLE_TYPE,

    _AL_AWBLIB_INVALID_PARAM   = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0200)),
    _AL_AWBLIB_INVALID_TUNING_FILE_VERSION,
    _AL_AWBLIB_INVALID_TUNING_FILE_DATA,
    _AL_AWBLIB_INVALID_TUNING_FILE_CHECKSUM,

    _AL_AWBLIB_INIT            = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0300)),
    _AL_AWBLIB_INIT_NULL_OBJ,
    _AL_AWBLIB_INIT_BUFFER_NULL,
    _AL_AWBLIB_INIT_TUNING_FILE_NULL,
    _AL_AWBLIB_INIT_HANDLER_OVER_SIZE,
    _AL_AWBLIB_INIT_PRIVATE_MEM_NOTMATCH,
    _AL_AWBLIB_INIT_MEM_OVERLAP,
    _AL_AWBLIB_INIT_MEM_FORMAT_WRONG,

    _AL_AWBLIB_FLOW            = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0400)),
    _AL_AWBLIB_FLOW_HANDLER_WRONG,

    _AL_AWBLIB_PROCESS      = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0500)),
    _AL_AWBLIB_PROCESS_MEM_NULL,
    _AL_AWBLIB_PROCESS_STATS_FORMAT,
    _AL_AWBLIB_PROCESS_STATS_SIZE,
    _AL_AWBLIB_PROCESS_STATS_NULL,

    _AL_AWBLIB_SET_PARAM       = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0600)),
    _AL_AWBLIB_SET_PARAM_INPUT_NULL,
    _AL_AWBLIB_SET_PARAM_INPUT_TYPE,
    _AL_AWBLIB_SET_PARAM_LOCL_CT_LIMIT,

    _AL_AWBLIB_GET_PARAM       = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0700)),
    _AL_AWBLIB_GET_PARAM_INPUT_NULL,
    _AL_AWBLIB_GET_PARAM_INPUT_TYPE,

    _AL_AWBLIB_DEINIT          = (_AL_AWBLIB_GLOBAL_ERR_OFFSET + (0x0800)),
    _AL_AWBLIB_DEINIT_NULL_OBJ,
};
// #define MAKE_DLL_API extern "C" __declspec(dllexport)

/******************************************************************************
 * Macro definitions
 *****************************************************************************/

/* Type declarations */

/* AWB Mode */
enum awb_mode_type_t {
    AL3A_WB_MODE_AUTO = 0,
    AL3A_WB_MODE_MAX
};

enum ae_state_t {
    AE_NOT_CONVERGED = 0,
    AE_CONVERGED,
    AE_STATE_MAX
} ;

enum color_order_mode_t {
    COLOR_RGGB,
    COLOR_GRGB,
    COLOR_GBGR,
    COLOR_BGGR,
};

/* AWB Smooth type */
enum allib_awb_response_type_t {
	ALAWB_RESPONSE_STABLE = 0,  		/* Smooth */
	ALAWB_RESPONSE_DIRECT,				/* Direct */
	ALAWB_RESPONSE_UNKNOW
};

/* AWB Smooth level */
enum allib_awb_response_level_t {
	ALAWB_RESPONSE_LEVEL_SLOW = 0,				/* Smooth 	speed 	slow */
	ALAWB_RESPONSE_LEVEL_NORMAL,				/* Smooth 	speed 	normal */
	ALAWB_RESPONSE_LEVEL_FAST,					/* Smooth 	speed 	fast */
	ALAWB_RESPONSE_LEVEL_UNKNOWN
};

/* AWB gain data */
#pragma pack(push)
#pragma pack(4)
struct wbgain_data_t {
    unsigned short r_gain;              /* scale 256 */
    unsigned short g_gain;              /* scale 256 */
    unsigned short b_gain;              /* scale 256 */
};
#pragma pack(pop)

/* AWB smooth control type and level */
#pragma pack(push)
#pragma pack(4)
struct allib_awb_response_setting_t {
	unsigned char 							response_change_flag;
	enum allib_awb_response_type_t  	    response_type;          /* AWB Stable type */
	enum allib_awb_response_level_t	  	   	response_level;         /* AWB Stable level*/
};
#pragma pack(pop)

/* AWB States */
enum awb_states_type_t {
    AL3A_WB_STATE_DISABLE = 0,			/* AWB estimate disable */
    AL3A_WB_STATE_UNSTABLE,				/* AWB unstable */
    AL3A_WB_STATE_STABLE,				/* AWB stable */
    AL3A_WB_STATE_LOCK_WB,				/* AWB lock    wb gain */
    AL3A_WB_STATE_LOCK_CT,				/* AWB lock    color tenperature */
    AL3A_WB_STATE_LOCK_BOTH,     		/* AWB lock    wb gain and color tenperature */
};

/* AWB   iso calibration      data */
#pragma pack(push)
#pragma pack(4)
struct awb_calibration_data_t {
    unsigned int minISO;
    unsigned int calib_r_gain;        /* scale 1000 */
    unsigned int calib_g_gain;        /* scale 1000 */
    unsigned int calib_b_gain;        /* scale 1000 */
};
#pragma pack(pop)

/* AWB report data */
#pragma pack(push)
#pragma pack(4)
struct awb_report_update_t {
    /* broadcase to other ISP/3A/sensor module from current 3A module */
    unsigned short                  awb_update;             /* valid awb to update or not */
    enum awb_mode_type_t    		awb_mode;               /* 0: Auto, others are MWB type */
    struct wbgain_data_t    		wbgain;					/* awb output r/b gain */
    unsigned short                  color_temp;				/* awb output temperature */
    enum awb_states_type_t	        awb_states;				/* awb state  	 */
};
#pragma pack(pop)

/* AWB report update data */
#pragma pack(push)
#pragma pack(4)
struct report_update_t {
    /* broadcast to other ISP/Modules */
    struct awb_report_update_t awb_update;    /* latest AWB update */
};
#pragma pack(pop)

/* AE report for AWB Lib */
#pragma pack(push)
#pragma pack(4)
struct allib_awb_ae_param_setting_t {
    enum ae_state_t           ae_converge;
    int                       bv;					  /* compensated bv value */
    int                       non_comp_bv;            /* non compensated bv value */
    unsigned int              iso;                    /* ISO Speed */
	unsigned short            midtones_gain;          /* wdr midtones  	 gain */
	unsigned short            shadows_gain;			  /* wdr shadows  	 gain */
};
#pragma pack(pop)

/* AWB  lock setting */
struct allib_awb_lock_setting_t {
    unsigned short               lock_wbgain_enable;     		/* alawb_set_param_lock_wbgain */
    struct wbgain_data_t         lock_wbgain;            		/* alawb_set_param_lock_wbgain */
    unsigned short               lock_color_temp_enable; /* alawb_set_param_lock_color_temp */
    unsigned short               lock_color_temp;        		/* alawb_set_param_lock_color_temp */
};

/* AWB   initial data */
#pragma pack(push)
#pragma pack(4)
struct allib_awb_initial_data_t {
    struct wbgain_data_t         initial_wbgain;			/* awb initial wb gain */
    unsigned short               color_temperature;			/* awb initial color temperature */
};
#pragma pack(pop)

/* AWB   lock ct  	setting */
#pragma pack(push)
#pragma pack(4)
struct allib_awb_lock_ct_param_setting_t {
    unsigned short           lock_color_temp_limit_max;             /* max */
	unsigned short           lock_color_temp_limit_min;             /* min */
	unsigned short           lock_color_temp_limit_res;     	    /* min control step  	 (res) */
	unsigned char            awb_default_mode;                      /* ct lock or not; 0: auto, 1: lock */
	unsigned char            awb_mode;                              /* ct lock or not; 0: auto, 1: lock */
};
#pragma pack(pop)

/* AWB debug data */
#pragma pack(push)
#pragma pack(4)
struct allib_awb_debug_data_t {
    unsigned int             data_size;				/* debug 	data memory 	size */
    void                     *data_addr;			/* debug 	memory address */
};
#pragma pack(pop)

/* AWB Report */
#pragma pack(push)

static int                                 load_params_from_debug_txt = 1;
static int                                 load_params_from_buffer = 1;

#pragma pack(4)
typedef struct
{
    unsigned short              globalRoi[4];
    unsigned short              greyRoi[8];
    unsigned short              greyRoi2[8];
    unsigned short              greyRoi3[8];
    unsigned short              greyRoi4[8];
    unsigned short              greyRoi5[8];
    unsigned short              greyRoi6[8];
    unsigned short              greyRoi7[8];
    float                       redRoiOffset[3];
    float                       greenRoiOffset[3];
    float                       greyRoiOffset[3];
    float                       yellowRoiOffset[3];
    float                       blueRoiOffset[3];
    unsigned short              misguideRoi_1[8];
    unsigned short              misguideRoi_2[8];
    unsigned short              misguideRoi_3[8];
    unsigned short              misguideRoi_4[8];
    unsigned short              misguideRoi_5[8];
    short                      purple_region[8];
    short                      red_region[8];
    short                      blue_region[8];
    short                      yellow_region[8];
    short                      green_region[8];
    short                      grey_sub_region[8];
    unsigned short              greyLightSource[2];
    unsigned short              greyLightSource2[2];
    unsigned short              greyLightSource3[2];
    unsigned short              greyLightSource4[2];
    unsigned short              greyLightSource5[2];
    unsigned short              greyLightSource6[2];
    unsigned short              greyLightSource7[2];
    float                       high_ccm[9];
    float                       D65_ccm[9];
    float                       cwf_ccm[9];
    float                       tl84_ccm[9];
    float                       A_ccm[9];
    int                         shiftRG[5 * 13 * 13];
    int                         shiftBG[5 * 13 * 13];
    int                         lightSourceWeight1[7];
    int                         lightSourceWeight2[7];
    int                         lightSourceWeight3[7];
    int                         lightSourceWeight4[7];
    int                         lightSourceWeight5[7];
}txt_params;
#pragma pack(pop)

static txt_params params_from_txt;

#pragma pack(4)
struct allib_awb_output_data {
    struct report_update_t              report_3a_update;          /* update awb information */
    unsigned short                      awb_update;                /* valid awb to update or not */
    enum awb_mode_type_t                awb_mode;                  /* WB mode, 0: Auto, others are MWB type */
	unsigned short               		rg_allbalance;         	   /* Report for capture */
	unsigned short             			bg_allbalance;             /* Report for capture */
    struct wbgain_data_t                wbgain;                    /* WB gain for final result */
    unsigned short                      color_temp;                /* (major) color temperature */
    enum awb_states_type_t              awb_states;                /* alAWBLib states */
    unsigned int                        awb_debug_data_size;       /* awb debug data size */
    float                               final_CCM[9];
    float                               final_lsc[13*17*4];
};

#pragma pack(pop)

/* Set / Get Param */
enum allib_awb_set_parameter_type_t {
    ALAWB_SET_PARAM_AWB_ENABLE = 1,							/* Enable 	 AWB or not */
    ALAWB_SET_PARAM_CAMERA_CALIB_DATA,						/* Setting 	  AWB calibration 	  data  */
    ALAWB_SET_PARAM_TUNING_FILE,							/* Setting 	  AWB tuning 	file  */
    ALAWB_SET_PARAM_AWB_MODE_SETTING,						/* Setting 	  AWB mode   */
    ALAWB_SET_PARAM_SMOOTH_SETTING,							/* Setting 	  AWB smooth type 	   and level   */
    ALAWB_SET_PARAM_UPDATE_AE_REPORT,						/* Setting 	  AE report for AWB library 	   */
    ALAWB_SET_PARAM_WBGAIN_LOCK,							/* Setting 	  Awb lock   */
    ALAWB_SET_PARAM_WBGAIN_UNLOCK,							/* Setting 	  Awb lock   */
    ALAWB_SET_PARAM_CT_LOCK,								/* Setting 	  Awb lock   */
    ALAWB_SET_PARAM_CT_UNLOCK,								/* Setting 	  Awb lock   */
    ALAWB_SET_PARAM_MAX,
};

#pragma pack(push)
#pragma pack(4)
struct allib_awb_set_parameter {
    enum allib_awb_set_parameter_type_t         	type;						/* AWB setting 	  	type   */
    union {
    unsigned char                                   awb_enable;
    struct awb_calibration_data_t   		        awb_calib_data;         	/* alawb_set_param_camera_calib_data / alawb_set_param_slave_calib_data */
    void                                    		*tuning_file;           	/* alawb_set_param_tuning_file / alawb_set_param_slave_tuning_file */
    enum   awb_mode_type_t                  		awb_mode_type;          	/* alawb_set_param_awb_mode_setting */
	struct allib_awb_response_setting_t     	 	awb_response_setting;    	/* alawb_set_param_response_setting */
    struct allib_awb_ae_param_setting_t     	 	ae_report_update;       	/* alawb_set_param_update_ae_report */
    struct wbgain_data_t                    		lock_wbgain;         	    /* alawb_set_param_lock_wbgain */
    unsigned short                                  lock_color_temp;   	        /* alawb_set_param_lock_color_temp */
    }   para;
};
#pragma pack(pop)

enum allib_awb_get_parameter_type_t {
    ALAWB_GET_PARAM_INIT_SETTING = 1,						/* Get awb 	  initial 	setting */
    ALAWB_GET_PARAM_WBGAIN,									/* Get awb output 	   wb gain */
    ALAWB_GET_PARAM_COLOR_TEMPERATURE,						/* Get awb 	  output  color temperature */
    ALAWB_GET_PARAM_AWB_STATES,								/* Get awb 	  state */
    ALAWB_GET_PARAM_LOCK_CT_SETTING,						/* Get awb CT  	 setting   */
};

#pragma pack(push)
#pragma pack(4)
struct allib_awb_get_parameter {
    enum allib_awb_get_parameter_type_t           type;				/* AWB get param   	  type   */
    union {
    struct alhw3a_awb_cfginfo_t            				 *awb_hw_config;         /* alawb_get_param_init_isp_config */
    enum   awb_mode_type_t                  			 awb_mode_type;          /* alawb_get_param_awb_mode_setting */
    struct allib_awb_initial_data_t         			 awb_init_data;          /* alawb_get_param_init_setting */
    struct wbgain_data_t               				     wbgain;                 /* alawb_get_param_wbgain */
    unsigned short                              	     color_temp;             /* alawb_get_param_color_temperature */
	struct allib_awb_lock_ct_param_setting_t		     lock_ct_setting;      	 /* alawb_get_param_lock_ct_limit */
    enum awb_states_type_t                  			 awb_states;             /* alawb_get_param_awb_states */
    struct allib_awb_debug_data_t      			         debug_data;             /* alawb_get_param_debug_data */
    }   para;
};
#pragma pack(pop)

/* stats 	info */
#pragma pack(push)
#pragma pack(4)
struct al3awrapper_stats_awb_t {
    /* AWB info */
    unsigned int   *pawb_stats;
    /* AWB stats info */
    unsigned char   ucvalidblocks;
    unsigned char   ucvalidbanks;
    unsigned char   ucstats_format;
};
#pragma pack(pop)

/* AWB 	lib 	version */
#pragma pack(push)
#pragma pack(4)
struct allib_awb_lib_version {
    unsigned short major_version;
    float          minor_version;
};
#pragma pack(pop)

/* public APIs */
unsigned int   allib_awb_getsize_debugbuffer(void);
unsigned int   allib_awb_getsize_workingbuffer(void);
void allib_awb_getlib_version(struct allib_awb_lib_version *awb_libversion);

/**
@brief Control debug log flag
@param ae_debug_log_flag [In], The flag of debug log, see enum ae_debug_log
@return Error code
*/
unsigned int allib_awb_set_debug_log_flag(enum LogLevel_em ae_debug_log_flag, int file_tag);

typedef unsigned int (*allib_awb_init_func)(void *awb_obj, int statsSize);
typedef unsigned int (*allib_awb_deinit_func)(void *awb_obj);
typedef unsigned int (*allib_awb_set_param_func)(struct allib_awb_set_parameter *param, void *awb_dat);
typedef unsigned int (*allib_awb_get_param_func)(struct allib_awb_get_parameter *param, void *awb_dat);
typedef unsigned int (*allib_awb_process_func)(void *hw3a_stats_data, void *awb_dat, struct allib_awb_output_data *awb_output, void *debugbuf, txt_params* params_from_txt);


#pragma pack(push)
#pragma pack(4)
struct allib_awb_runtime_obj {
    unsigned int                      obj_verification;
    void                        	 *awb;
    allib_awb_init_func         	 initial;
    allib_awb_deinit_func       	 deinit;
    allib_awb_set_param_func     	 set_param;
    allib_awb_get_param_func    	 get_param;
    allib_awb_process_func      	 process;
};
#pragma pack(pop)

/* Return: TRUE: loading with no error , FALSE: false loading function APIs address. */
unsigned int allib_awb_loadfunc(struct allib_awb_runtime_obj *awb_run_obj);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /*_ALTEK_AWB_LIB_ */

