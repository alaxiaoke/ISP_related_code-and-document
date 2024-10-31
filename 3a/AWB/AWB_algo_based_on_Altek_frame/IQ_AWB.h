#ifndef _IQ_AWB_
#define _IQ_AWB_

#include "allib_awb.h"

#define ATTRIBUTE_VISIBILITY_DEFAULT

typedef enum
{
	AWB_BAYER_RG = 0,               // RGGB
	AWB_BAYER_GR = 1,               // GRBG
	AWB_BAYER_GB = 2,               // GBRB
	AWB_BAYER_BG = 3,               // BGGR
} AWB_BAYER_ORDER;


#pragma pack(push,1)
typedef struct
{
	unsigned short      R;
	unsigned short      G;
	unsigned short      B;
} AWB_RGB;
#pragma pack(pop)



#pragma pack(push,1)
typedef struct
{
	void                *tuning_file;               // camera info addr data, bin file
	short               bv;                         // BV
	short               iso_r_128;                  // calibration data R
	short               iso_b_128;                  // calibration data B
	unsigned int        *addr;                      // input data addr
	short               width;                      // input data width
	short               height;                     // input data height
	unsigned int		totalsize;					// input data buffer total size
	AWB_BAYER_ORDER     color_order;                // input RAW's color order, ref. AWB_BAYER_ORDER
	unsigned char       flag_enable_stylish;        // Enable stylish (AWB is All-balanced or not
	void                *awb_mem_address;           // Working mem address, REF MEM_REQUAREMENT
	int					load_params_from_debug_txt;
	int					load_params_from_buffer;
	int					enterFromProcess;
    txt_params          params_from_txt;

} IQAWB_INPUT_PARA;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	unsigned char   	valid_report;               // AWB status report. Ture for OK. False for NG
	unsigned short      rg_allbalance;              // Report for capture
	unsigned short      bg_allbalance;              // Report for capture
	unsigned short      r_gain_final;               // Final R gain w/ stylish, 8bit
	unsigned short      g_gain_final;               // Final G gain w/ stylish, 8bit
	unsigned short      b_gain_final;               // Final B gain w/ stylish, 8bit
	unsigned short      color_temperature;          // color temperature 
} IQAWB_OUTPUT_PARA;
#pragma pack(pop)

#endif
