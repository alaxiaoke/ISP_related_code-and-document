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


#ifndef _AWB_
#define _AWB_

#include "IQ_AWB.h"
#include "allib_awb.h"


/* Transform Stats size to AWB_BLOCK_SIZE */
#define AWB_HW3A_BLOCK_INFO_SIZE                    (128)       //byte
#define AWB_HW3A_PER_BLOCK_SIZE                     (20)        //byte  , stats size = block_wcnt * block_hcnt * AWB_PER_BLOCK_SIZE + AWB_BLOCK_INFO_SIZE
#define AWB_BLOCK_XCNT                              (64)		//max size 64
#define AWB_BLOCK_YCNT                              (48)		//max size 48
#define AWB_BLOCK_SIZE                              (AWB_BLOCK_XCNT*AWB_BLOCK_YCNT)

/* HW3A Fixed size */
#define AWB_HW3A_PATCH_W                            (64)		//max size 64
#define AWB_HW3A_PATCH_H                            (48)		//max size 48
#define AWB_HW3A_PATCH_SIZE                         (AWB_HW3A_PATCH_W*AWB_HW3A_PATCH_H)
#define AWB_HW3A_PATCH_W_MAX                        (64)		//max block width (include dummy)

/* Use scale_w x scale_h hw3a data to patching one block into lib */
#define AWB_BLOCK_PATCH_W_SCALE                     (AWB_HW3A_PATCH_W/AWB_BLOCK_XCNT)
#define AWB_BLOCK_PATCH_H_SCALE                     (AWB_HW3A_PATCH_H/AWB_BLOCK_YCNT)

/* HW3A RG/BG(x100) RATIO MAX VALUE */
#define AWB_HW3A_RATIO_MAX			                (511)    

/* awb algorithm table size */
#define AWB_LS_WEIGHTING_TABLE_MAX_BV_NUM            (5)
#define AWB_STYLISH_TABLE_MAX_BV_NUM                 (5)
#define AWB_LIGHTSOURCE_NUM_MAX  	 				 (13)
#define AWB_LIGHTSOURCE_DATA_LENGTH  	 		     (13)
#define AWB_HANDLE_NUM_MAX  	 				     (8)

/* Macro */
#define ABS(a)              ((a)>0? (a) : (-(a)))
#define MAX(a,b)            ((a)>(b)? (a):(b))
#define MIN(a,b)            ((a)>(b)? (b):(a))
#define MAX3(a,b,c)         ((a)>(b)?((a)>(c)?(a):(c)):((b)>(c)?(b):(c)))
#define ROUND(a, b)         ((a)> 0? (((a)+((b)>>1))/(b)) : (((a)-((b)>>1))/(b)))

///====================================
/* light source weighting table info relate */
#pragma pack(push,4)
typedef struct  
{
	unsigned short       rg;
    unsigned short       bg;
    unsigned short       ct;
	short             shift_rg;
    short             shift_bg;
} AWB_LIGHTSOURCE;
#pragma pack(pop)

#pragma pack(push,4)
typedef struct  
{
	AWB_LIGHTSOURCE     source[AWB_LIGHTSOURCE_NUM_MAX];
}  AWB_LIGHTSOURCE_ARRAY;
#pragma pack(pop)



#pragma pack(push,4)
typedef struct  
{
    short                       BV;
    unsigned char               ls_num;
    AWB_LIGHTSOURCE_ARRAY       tLightSourceArray;
    unsigned short              auwLightSourceWeightTable[AWB_LIGHTSOURCE_NUM_MAX];
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
    short*                      purple_region;
    short*                      red_region;
    short*                      blue_region;
    short*                      yellow_region;
    short*                      green_region;
    short*                      grey_sub_region;
    short*                      gain_adj;
} AWB_MODULE_PARA;
#pragma pack(pop)

#pragma pack(push,4)
typedef struct
{
    int                         seriesLengthEnoughLabel;
    int                         frameNum;
    short                       stableLabel;
    short                       stableRG;
    short                       stableBG;
    short                       stableNoShiftRG;
    short                       stableNoShiftBG;
    short                       topCountRegion[3];
    short                       RegionCountHistory[7];
    unsigned short              RGainSeries[10];
    unsigned short              BGainSeries[10];
    unsigned short              noShiftRGainSeries[10];
    unsigned short              noShiftBGainSeries[10];
} HANDLE_RELATED_PARAM;
#pragma pack(pop)

static HANDLE_RELATED_PARAM handle_param_list[AWB_HANDLE_NUM_MAX];


///====================================
/*   Patch Module	*/

#pragma pack(push,4)
typedef struct  
{
	unsigned int        *addr;
    short           	width;
    short         		height;
    short           	iso_r_128;
    short          		iso_b_128;
} AWB_INPUT_PATCH;
#pragma pack(pop)

#pragma pack(push,4)
typedef struct  
{
	short	 patch_Width;
    short    patch_Height;
    short    iso_r_128;
    short    iso_b_128;
} AWB_PATCH_REPORT;
#pragma pack(pop)


///====================================
/*  Algorithm */

typedef enum    
{
	WEIGHT_LIST_BEGIN,
    WEIGHT_LIST_FINAL,
    WEIGHT_LIST_NUM,
} AWB_WEIGHT_LIST;

#pragma pack(push,4)
typedef struct  
{
	unsigned short      RG;
    unsigned short      BG;
    unsigned short      ColorTemperature;
    unsigned short      voteNum;
    unsigned int      	Weighting[WEIGHT_LIST_NUM];
} AWB_VOTING_LIGHTSOURCE;
#pragma pack(pop)

#pragma pack(push,4)
typedef struct  
{
	unsigned short       R_Gain_last;
	unsigned short       G_Gain_last;
    unsigned short       B_Gain_last;
	short       		 CT_last;
    unsigned short     	 ALL_RG_last;
    unsigned short       ALL_BG_last;
} AWB_LAST_RESULT;
#pragma pack(pop)

#pragma pack(push,4)
typedef struct  
{
	short               BV;
    unsigned short      invalidVotes;
    unsigned short      totalVotes;
    unsigned int        totalWeight;
    unsigned short      RG;
    unsigned short      BG;
    unsigned short      ColorTemperature;
    unsigned short      RG_Capture;
    unsigned short      BG_Capture;
    unsigned short      ColorTemperature_Capture;
	unsigned char       ls_num;	
    AWB_VOTING_LIGHTSOURCE	      awb_LightSource[AWB_LIGHTSOURCE_NUM_MAX];
    unsigned int        grey_count;
    unsigned int        maxGreyCount;
    unsigned int        maxGreyCountIdx;
    unsigned short      udDistMin;
    unsigned short       R_Gain_no_shift;
    unsigned short       B_Gain_no_shift;
} AWB_VOTING_REPORT;
#pragma pack(pop)

///====================================
/*  Stylish */

#pragma pack(push,4)
typedef struct  
{
	unsigned char       Num_Bv;
    unsigned char       Num_CT;
    short       		*Tbl_Bv;
    short      		    *Tbl_CT;
    short      		    *tAwbTab_RG;
    short     		    *tAwbTab_BG;
} AWB_STYLISH_PARA;
#pragma pack(pop)

#pragma pack(push,4)
typedef struct  
{
	unsigned short       R_Gain;
    unsigned short       B_Gain;
    short     			 comp_RG;
    short    		     comp_BG;
	unsigned char	     enable_stylish;
} AWB_STYLISH_REPORT;
#pragma pack(pop)

//#pragma pack(push,4)
//typedef struct  
//{ 	
//	unsigned short		statsblk_w;
//	unsigned short		statsblk_h;
//} TUNING_FILE_PARA;
//#pragma pack(pop)

#pragma pack(push,4)
typedef struct
{
	short                   bv;
    short                   ls_weight_lightsource_num;
    unsigned short		    ls_weight_tab[AWB_LIGHTSOURCE_NUM_MAX];
} TUNING_FILE_LS_WEIGHT_PARA;
#pragma pack(pop)


#pragma pack(push,4)
typedef struct  
{
	int                                 tuningFile_para;
    unsigned int                        tuningFile_LightSources_number;
    AWB_LIGHTSOURCE                     tuningFile_LightSources[AWB_LIGHTSOURCE_NUM_MAX];
    unsigned int                        tuningFile_LSWeighting_number;
    TUNING_FILE_LS_WEIGHT_PARA          tuningFile_LSWeighting[AWB_LS_WEIGHTING_TABLE_MAX_BV_NUM];
    unsigned int                        tuningFile_stylish_bv_number;
    short                               tuningFile_stylish_bv_table[AWB_STYLISH_TABLE_MAX_BV_NUM];
    unsigned int                        tuningFile_stylish_ct_number;
    short                               tuningFile_stylish_ct_table[AWB_LIGHTSOURCE_NUM_MAX];
    short                               tuningFile_stylish_rg_shift[AWB_LIGHTSOURCE_NUM_MAX*AWB_STYLISH_TABLE_MAX_BV_NUM];  
    short                               tuningFile_stylish_bg_shift[AWB_LIGHTSOURCE_NUM_MAX*AWB_STYLISH_TABLE_MAX_BV_NUM];         
} AWB_TUNING_FILE;
#pragma pack(pop)


///====================================
/*  MEM */

#pragma pack(push,4)
typedef struct  
{
    //.     Patch
    AWB_INPUT_PATCH             input_Patch;
    //.     Stylish
    AWB_STYLISH_PARA            stylish_Para;
} IQAWB_ALGORITHM_PARA;
#pragma pack(pop)


#pragma pack(push,4)
typedef struct
{
    //.     Modules
    AWB_MODULE_PARA             module_Para;
    //.     Patch
    AWB_PATCH_REPORT            patch_Report;
    //.     Voting
    AWB_VOTING_REPORT           vote_Report;
    //.     Stylish
    AWB_STYLISH_REPORT          stylish_Report;
	//.     last report
	AWB_LAST_RESULT				last_result;
} IQAWB_ALGORITHM_REPORT;
#pragma pack(pop)


#pragma pack(push,4)
typedef struct  
{
    IQAWB_ALGORITHM_PARA          awb_algorithm_para;
    IQAWB_ALGORITHM_REPORT	      awb_algorithm_report;
} AWB_MEM_ALGORITHM;
#pragma pack(pop)


#endif
