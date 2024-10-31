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


#ifndef _ALTEK_AWB_LIB_COMMON_
#define _ALTEK_AWB_LIB_COMMON_

#include "Altek_AWB.h"
#include "allib_awb.h"

/* AWB working buffer reserved */
#define alAWBLib_Private_WorkingBuffer_Reserved     (128)

/* Scale truansorm */
#define alAWBLib_Transform_CalibData(data, base)    ((short)    ( (128 * (data) + (base) / 2) / (base) ))

/* awb stats setting */
#pragma pack(push)
#pragma pack(4)
typedef struct
{
	unsigned int   		*hw3a_awb_stats;            // altek hw3a states pointer
	unsigned int        hw3a_awb_block_num;         // Altek hw3a block width
	unsigned int        hw3a_awb_bank_num;          // Altek hw3a block height
	unsigned int		hw3a_awb_stats_size;		// hw3a stats data total size
}   alAWBLib_stats_setting_t;
#pragma pack(pop)

/* Runtime Buffer */
#pragma pack(push)
#pragma pack(4)
typedef struct
{
	unsigned char                                   handler;
	//. AWB algorithm related
	alAWBLib_stats_setting_t                		awb_stats_setting;             // stats setting
	struct awb_calibration_data_t           		awb_calib_data;                // ISO calibration gain
	short                                  			iso_r_128;                     // Normalized ISO R gain
	short                                 			iso_b_128;                     // Normalized ISO B gain
	struct allib_awb_ae_param_setting_t     		awb_ae_param_setting;          // ae data, from AE
	void                                    		*awb_buffer;                   // working buffer pointer
	void                                   			*tuning_addr;                  // input tuning file addr
	struct allib_awb_output_data    		        awb_output_backup;             // back up last frame info for get param.
} alAWBLib_runtime_data_t;
#pragma pack(pop)

#pragma pack(push,4)
typedef struct  
{
	alAWBLib_runtime_data_t        awb_runtime_data;
	AWB_MEM_ALGORITHM              awb_mem_algorithm;
    char                           working_buffer_reserved[alAWBLib_Private_WorkingBuffer_Reserved];
} AWB_WORKING_BUFFER;
#pragma pack(pop)


#endif

