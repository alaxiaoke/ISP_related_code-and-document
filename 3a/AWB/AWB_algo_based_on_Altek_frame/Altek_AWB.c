#include "Altek_AWB.h"
#include "alAWBLib_AWB_Common.h"
#include "alwrapper.h"
#include "IQ_AWB.h"
#include "BU_AwbPara.h"



AWB_MEM_ALGORITHM* mem_algorithm = (AWB_MEM_ALGORITHM*)0;

/* awb algorithm parameter */
int                             g_tuningFile_para = 0;
static AWB_LIGHTSOURCE                 g_tuningFile_LightSources[AWB_LIGHTSOURCE_NUM_MAX] = AWB_TUNING_FILE_LIGHTSOURCE_ARRAY_PARA0;
static TUNING_FILE_LS_WEIGHT_PARA      g_tuningFile_LSWeighting[AWB_LS_WEIGHTING_TABLE_MAX_BV_NUM] = TUNING_FILE_LS_WEIGHTING_PARA0;
static short                    g_tuningFile_stylish_bv_table[AWB_STYLISH_TABLE_MAX_BV_NUM] = AWB_LS_SHIFT_TABLE_BV_PARA0;
static short                           g_tuningFile_stylish_ct_table[AWB_LIGHTSOURCE_NUM_MAX] = AWB_LS_SHIFT_TABLE_CT_PARA0;
static short                           g_tuningFile_stylish_rg_shift[AWB_LS_SHIFT_RG_TABLE_NUM_PARA0][AWB_LIGHTSOURCE_NUM_MAX] = AWB_LS_SHIFT_RG_TABLE_PARA0;
//short                           g_tuningFile_stylish_bg_shift[AWB_STYLISH_TABLE_MAX_BV_NUM][AWB_LIGHTSOURCE_NUM_MAX] = AWB_LS_SHIFT_BG_TABLE_PARA0;
static short                           g_tuningFile_stylish_bg_shift[AWB_LS_SHIFT_RG_TABLE_NUM_PARA0][AWB_LIGHTSOURCE_NUM_MAX] = AWB_LS_SHIFT_BG_TABLE_PARA0;
static short                           g_tuningFile_global_roi[4] = AWB_GLOBAL_REGION;
static short                           g_tuningFile_grey_roi[8] = AWB_GREY_REGION;
static short                           g_tuningFile_grey_roi2[8] = AWB_GREY_REGION2;
static short                           g_tuningFile_grey_roi3[8] = AWB_GREY_REGION3;
static short                           g_tuningFile_grey_roi4[8] = AWB_GREY_REGION4;
static short                           g_tuningFile_grey_roi5[8] = AWB_GREY_REGION5;
static short                           g_tuningFile_grey_roi6[8] = AWB_GREY_REGION6;
static short                           g_tuningFile_grey_roi7[8] = AWB_GREY_REGION7;
static short                           grey_sub_region[AWB_GREY_REGION_SUB_NUM][8] = AWB_GREY_REGION_SUB;
static float                           g_tuningFile_grey_offset[3] = AWB_GREY_OFFSET;
static float                           g_tuningFile_red_offset[3] = AWB_RED_OFFSET;
static float                           g_tuningFile_green_offset[3] = AWB_GREEN_OFFSET;
static float                           g_tuningFile_yellow_offset[3] = AWB_YELLOW_OFFSET;
static float                           g_tuningFile_blue_offset[3] = AWB_BLUE_OFFSET;
static short                           g_tuningFile_misguide_roi_1[8] = MISGUIDE_REGION_1;
static short                           g_tuningFile_misguide_roi_2[8] = MISGUIDE_REGION_2;
static short                           g_tuningFile_misguide_roi_3[8] = MISGUIDE_REGION_3;
static short                           g_tuningFile_misguide_roi_4[8] = MISGUIDE_REGION_4;
static short                           g_tuningFile_misguide_roi_5[8] = MISGUIDE_REGION_5;
static short                           purple_region[8] = AWB_PURPLE_REGION1;
static short                           red_region[8] = AWB_RED_REGION1;
static short                           blue_region[8] = AWB_BLUE_REGION1;
static short                           yellow_region[8] = AWB_YELLOW_REGION1;
static short                           green_region[8] = AWB_GREEN_REGION1;
static short                           g_tuningFile_gain_adj[AWB_TUNING_FILE_GAIN_ADJUST_NUM][5] = AWB_TUNING_FILE_GAIN_ADJUST;

/* For HW3A Temp buffer, 2D format */
static unsigned int  hw3A_64_48[AWB_HANDLE_NUM_MAX][AWB_HW3A_PATCH_SIZE];



//
//================================================================================================
/**
*@brief                 Function: sqrt
*@param[In]   input     32 bits value
*@return                16 bits sqrt result
*/
unsigned short sqrt_16(unsigned int input)
{
    unsigned int output, i;
    unsigned long tmp, ttp;
    if (input == 0)
        return 0;
    output = 0;
    tmp = (input >> 30);
    input <<= 2;
    if (tmp > 1)
    {
        output++;
        tmp -= output;
    }
    for (i = 15; i > 0; i--)
    {
        output <<= 1;
        tmp <<= 2;
        tmp += (input >> 30);
        ttp = output;
        ttp = (ttp << 1) + 1;
        input <<= 2;
        if (tmp >= ttp)
        {
            tmp -= ttp;
            output++;
        }
    }
    return output;
}

//==================================================================================================
/**
*@brief                    Caculate stats patch
*@param[In]   *imageIn     per stats patch
*@param[In]   *inputPara    Input param
*@param[In]   iso_r_128     iso r gain
*@param[In]   iso_b_128     iso b gain
*@param[OUT]  *imageOut     Output patch r/g , b/g
*@return                   none
*/
static void AWB_PatchRaw_CalculateOneISP1(
    unsigned int* imageIn,
    IQAWB_INPUT_PARA* inputPara,
    short               iso_r_128,
    short               iso_b_128,
    unsigned int* imageOut)
{
    //. From Hw3A setting.
    // Set AWB statistic position, (a+b+c = sub_x; d+e+f = sub_y)
    // b/2 = ucDsX(DownSample_X); e/2 = ucDsY(DownSampe_Y)
    unsigned char   ucDsX = 1;
    unsigned char   ucDsY = 1;
    unsigned int* lineAddr;
    unsigned short  uwSubXCnt, uwSubYCnt;
    unsigned int  RValue, GValue, BValue;
    unsigned int  RSum, GrSum, GbSum, BSum, Count = 0;
    unsigned int  saturation;
    unsigned short   Y, RG, BG;

    //. Stats comp bypass
    //if (tuning_file->tuningFile_check_gain_comp == FALSE

    saturation = 0x3FF;
    RSum = GrSum = GbSum = BSum = 0;

    lineAddr = (unsigned int*)imageIn;
    for (uwSubYCnt = 0; uwSubYCnt < ucDsY; uwSubYCnt++)
    {
        for (uwSubXCnt = 0; uwSubXCnt < ucDsX; uwSubXCnt++)
        {
            //. set start position, must be RGGB
            // Subtract black offset.
            RSum = (*lineAddr);
            GrSum = (*(lineAddr += 1));
            GbSum = (*(lineAddr += 1));
            BSum = (*(lineAddr += 1));
            Count = *(lineAddr += 1);
        }//end of for ucDsX
    }//end of for ucDsY
    //if (Count != 0)
    //{
    //	RSum /= Count;
    //	GrSum /= Count;
    //	GbSum /= Count;
    //	BSum /= Count;
    //}
    //else
    //{
    //	RSum  =0;
    //	GrSum =0;
    //	GbSum =0;
    //	BSum  =0;

    //}
	//此处HULK和仿真代码需要不同
    RValue = (unsigned int)(RSum);
    GValue = (unsigned int)((GrSum + GbSum) / 2);
    BValue = (unsigned int)(BSum);

    //. Clamping
    if (RValue > saturation) RValue = saturation;
    if (GValue > saturation) GValue = saturation;
    if (BValue > saturation) BValue = saturation;

    if (GValue > 0)
    {
        AWB_RawRGB_To_HW3A(&Y, &RG, &BG, RValue, GValue, BValue, iso_r_128, iso_b_128);
        if ((RG > (unsigned short)AWB_HW3A_RATIO_MAX) || (BG > (unsigned short)AWB_HW3A_RATIO_MAX))
        {
            RG = 0;
            BG = 0;
        }
    }
    else
    {
        Y = 0;
        RG = 0;
        BG = 0;
    }

    *imageOut = ((Y << 20) + (RG << 10) + BG);
}

//=================================================================================================
/**
*@brief                     Get memory size to initial algorithm
*@return                    memory size of private working buffer used;
*/
unsigned int  IQAWB_Init_GetMemSize()
{
    return sizeof(AWB_MEM_ALGORITHM);
}

//=================================================================================================
/**
*@brief                     Initial AWB parameters
*@param[In]   *alInit_para  Customer input
*@param[OUT]  *outputPara   Fill up AWB parameters
*@return                    Error code
*/
enum altek_awb_error_code IQAWB_Init(IQAWB_INPUT_PARA* inputPara, IQAWB_OUTPUT_PARA* outputPara)
{
    AWB_RGB rgb_gain;
    enum altek_awb_error_code Error = _AL_AWBLIB_SUCCESS;

    Error = AWB_InitialSetting(inputPara, outputPara);
    if (_AL_AWBLIB_SUCCESS != Error)
        return Error;

    for (int i = 0; i < AWB_HANDLE_NUM_MAX; i++) {
        HANDLE_RELATED_PARAM* ptr;
        ptr = &handle_param_list[i];
        ptr->frameNum = 0;
        ptr->seriesLengthEnoughLabel = 0;
    }

    AWB_Calculate_rgbGain(outputPara->rg_allbalance, outputPara->bg_allbalance, &rgb_gain);

    outputPara->valid_report = 1;
    outputPara->r_gain_final = rgb_gain.R;
    outputPara->g_gain_final = rgb_gain.G;
    outputPara->b_gain_final = rgb_gain.B;
    outputPara->rg_allbalance = outputPara->rg_allbalance;
    outputPara->bg_allbalance = outputPara->bg_allbalance;
    outputPara->color_temperature = 5500;
    return Error;
}

//=================================================================================================
/**
*@brief                      Uninitial AWB parameters
*@param[In]   handler   Uninit module num
*@return                   Error code
//. Note that make sure AP would re-init after Uninit when changing mode.
//. Only if change sensor mode , then you need to call Uninit / init to set HW config again.
*/
enum altek_awb_error_code IQAWB_Deinit(void)
{
    if (0 != mem_algorithm)
        mem_algorithm = (AWB_MEM_ALGORITHM*)0;

    return _AL_AWBLIB_SUCCESS;
}

//=================================================================================================
/**
*@brief                    AWB Algorithm Estimation
*@param[In]   *inputPara   Input AWB Algorithm Data
*@param[OUT]  *outputPara  Output Estimation report
*@return                   Error code
*/
enum altek_awb_error_code IQAWB_Estimation(unsigned short cameraID, IQAWB_INPUT_PARA* inputPara, IQAWB_OUTPUT_PARA* outputPara, void* debugbuf)
{
    short   ct, ct_capture;
    unsigned short  rg_capture, bg_capture;
    AWB_RGB rgb_gain;
    AWB_MEM_ALGORITHM* algorithm_mem;
    IQAWB_ALGORITHM_PARA* algorithm_Para;
    IQAWB_ALGORITHM_REPORT* algorithm_Report;

    short   BV = inputPara->bv;

    enum altek_awb_error_code    Error = _AL_AWBLIB_SUCCESS;

    algorithm_mem = (AWB_MEM_ALGORITHM*)inputPara->awb_mem_address;
    algorithm_Para = &(algorithm_mem->awb_algorithm_para);
    algorithm_Report = &(algorithm_mem->awb_algorithm_report);

    //.     Set Para. 
    Error = AWB_SetAlgorithmPara(inputPara);
    if (_AL_AWBLIB_SUCCESS != Error)
    {
        outputPara->rg_allbalance = 0;
        outputPara->bg_allbalance = 0;

        return Error;
    }

    Error = AWB_ComputePatch(algorithm_Para, algorithm_Report);
    if (_AL_AWBLIB_SUCCESS != Error)
        return Error;

    //.     MultiVoting, report with 4x digits.
    AWB_MultiVoting(algorithm_Para, algorithm_Report, cameraID);      //算法主要内容在这里面，由得到的统计数据得到rgain bgain  主要是这里面 AWB_MultiWin_WeightingResult AWBPV_MultiWin_ScanBlockData两个函数做计算
    rg_capture = algorithm_Report->vote_Report.RG_Capture;
    bg_capture = algorithm_Report->vote_Report.BG_Capture;
    ct = ct_capture = algorithm_Report->vote_Report.ColorTemperature;

    //.     Stylish
    if ((1 == inputPara->flag_enable_stylish) && (1 == outputPara->valid_report))
    {
        unsigned short sty_Rgain = rg_capture, sty_Bgain = bg_capture;
        AWB_Stylish_Gain(BV, ct, &sty_Rgain, &sty_Bgain, &(algorithm_Para->stylish_Para), &(algorithm_Report->stylish_Report));
        rg_capture = sty_Rgain;
        bg_capture = sty_Bgain;
    }
    else
    {
        //. for debug
        algorithm_Report->stylish_Report.R_Gain = rg_capture;
        algorithm_Report->stylish_Report.B_Gain = bg_capture;
        algorithm_Report->stylish_Report.comp_RG = 0;
        algorithm_Report->stylish_Report.comp_BG = 0;
        algorithm_Report->stylish_Report.enable_stylish = inputPara->flag_enable_stylish;
    }

    //.     Prepare Output data
    if (0 == ct)
    {
        outputPara->valid_report = 0;
        ct = algorithm_Report->last_result.CT_last;// Set default 5000K to avoid CCM interpolation error.
        rg_capture = algorithm_Report->last_result.ALL_RG_last;
        bg_capture = algorithm_Report->last_result.ALL_BG_last;
        rgb_gain.R = algorithm_Report->last_result.R_Gain_last;
        rgb_gain.G = algorithm_Report->last_result.G_Gain_last;
        rgb_gain.B = algorithm_Report->last_result.B_Gain_last;

    }
    else
    {
        outputPara->valid_report = 1;
        //.     Turn to Real Color
        AWB_Calculate_rgbGain(rg_capture, bg_capture, &rgb_gain);         // 相当于求1/rg  (256/rg*1000)
    }
    outputPara->rg_allbalance = algorithm_Report->vote_Report.R_Gain_no_shift;
    outputPara->bg_allbalance = algorithm_Report->vote_Report.B_Gain_no_shift;

    //. 	record new result
    algorithm_Report->last_result.ALL_RG_last = rgb_gain.R;
    algorithm_Report->last_result.ALL_BG_last = rgb_gain.B;

    if (!(rgb_gain.R == 256 && rgb_gain.G == 256 && rgb_gain.B == 256)
        && algorithm_Report->vote_Report.maxGreyCount * 100.0f / algorithm_Report->vote_Report.grey_count > algorithm_Report->module_Para.gain_adj[2]
        && algorithm_Report->vote_Report.maxGreyCountIdx * 100 >= algorithm_Report->module_Para.gain_adj[0]
        && algorithm_Report->vote_Report.maxGreyCountIdx * 100 < algorithm_Report->module_Para.gain_adj[1])
    {
        rgb_gain.R = rgb_gain.R * algorithm_Report->module_Para.gain_adj[3] / 100.0f;//AWB_RGAIN_ADJ;
        rgb_gain.B = rgb_gain.B * algorithm_Report->module_Para.gain_adj[4] / 100.0f;//AWB_BGAIN_ADJ;
    }

    //.     Final WB gain
    outputPara->r_gain_final = rgb_gain.R;
    outputPara->g_gain_final = rgb_gain.G;
    outputPara->b_gain_final = rgb_gain.B;
    outputPara->color_temperature = ct;

    //.		record new result
    algorithm_Report->last_result.CT_last = ct;// Set default 5000K to avoid CCM interpolation error.
    algorithm_Report->last_result.R_Gain_last = rgb_gain.R;
    algorithm_Report->last_result.G_Gain_last = rgb_gain.G;
    algorithm_Report->last_result.B_Gain_last = rgb_gain.B;

    //LOG_WARN("[WhiteBox] maxGreyCount: %3d\r\n", (int)algorithm_Report->vote_Report.maxGreyCountIdx);
    //LOG_DEBUG("final rgb gain %u %u %u", outputPara->r_gain_final, outputPara->g_gain_final, outputPara->b_gain_final);
    return Error;
}

//=================================================================================================
/**
*@brief                     Get memory size for initial AWB parameters
*@return                    awb private memory size
*/
unsigned int  ALTEK_Init_GetMemSize(void)
{
    return IQAWB_Init_GetMemSize();
}

//=================================================================================================
/**
*@brief                     Check Input parameters
*@return                    Error code
*/
enum altek_awb_error_code    AWB_Check_InputPara(unsigned char booting, IQAWB_INPUT_PARA* inputPara)
{
    if ((inputPara->iso_r_128 <= 0) ||
        (inputPara->iso_b_128 <= 0) ||
        (inputPara->iso_r_128 >= 512) ||
        (inputPara->iso_b_128 >= 512))
    {
        return _AL_AWBLIB_INVALID_CALIB_OTP;
    }

    if (0 == inputPara->awb_mem_address)
        return _AL_AWBLIB_PROCESS_MEM_NULL;


    if (0 == booting)
    {

        if ((0 == inputPara->width) || (0 == inputPara->height) || (0 == inputPara->totalsize))
        {
            return _AL_AWBLIB_PROCESS_STATS_SIZE;
        }

    }
    return _AL_AWBLIB_SUCCESS;
}

//=================================================================================================
/**
*@brief                     Initial AWB parameters
*@param[In]   *inputPara    Input AWB Algorithm runtime data
*@param[OUT]  *outputPara   Fill up AWB first initialed data
*@return                    Error code
*/
enum altek_awb_error_code  ALTEK_AWB_Init(IQAWB_INPUT_PARA* inputPara, IQAWB_OUTPUT_PARA* outputPara)
{
    enum altek_awb_error_code  Error = _AL_AWBLIB_SUCCESS;

    Error = AWB_Check_InputPara(1, inputPara);
    if (0 != Error)
        return Error;

    return IQAWB_Init(inputPara, outputPara);
}

//=================================================================================================
/**
*@brief                     Initial AWB parameters
*@param[In]   module handler
*@return                    Error code
*/
enum altek_awb_error_code    ALTEK_AWB_Deinit(void)
{
    return IQAWB_Deinit();
}
//}
//=================================================================================================
/**
*@brief                     Altek AWB process
*@param[In]   *inputPara    Input AWB Algorithm runtime data
*@param[OUT]  *outputPara   Output Estimation report
*@return                    Error code
*/
//IQAWB_OUTPUT_PARA *outputPara_temp;
enum altek_awb_error_code    ALTEK_AWB_Estimation(unsigned short cameraID, IQAWB_INPUT_PARA* inputPara, IQAWB_OUTPUT_PARA* outputPara, void* debugbuf)
{
    enum altek_awb_error_code  Error = _AL_AWBLIB_SUCCESS;

    // Error = AWB_Check_InputPara(0, inputPara);
    // if( _AL_AWBLIB_SUCCESS != Error )
    // {
    //     return Error;
    // }

    altek_AWB_ComputePatch_ISP1(cameraID, inputPara);             //这里跑完已经得到了R/G   B/G了，用的就是纯纯的输入stats的r g b。输出R/G、B/G的时候乘了100

    Error = IQAWB_Estimation(cameraID, inputPara, outputPara, debugbuf);


    return Error;
}

//struct Point ADD(const struct Point a, const struct Point b)
//{
//    struct Point ADD;
//    ADD.x = a.x + b.x;
//    ADD.y = a.y + b.y;
//    return ADD;
//}
//struct Point MINUS(const struct Point a, const struct Point b)
//{
//    struct Point MINUS;
//    MINUS.x = a.x - b.x;
//    MINUS.y = a.y - b.y;
//    return MINUS;
//}
//short dot(const struct Point a, const struct Point b)// *
//{
//    return a.x * b.x + a.y * b.y;
//}
//short cross(const struct Point a, const struct Point b)// ^
//{
//    return a.x * b.y - a.y * b.x;
//}
//int dcmp(short x)
//{
//    return (x < 0) ? -1 : 1;
//}


//算法最主要的耗时来自这个函数！！！占到80%
int AWB_GREY_ROI_PROCESS(struct Point ROI[5], unsigned int rg, unsigned int bg)
{
    struct Point statsP;
    statsP.x = rg;
    statsP.y = bg;
    int flag = 0;

    for (int i = 0; i < 4; i++)
    {
        if (((ROI[i].x-statsP.x)* (ROI[i+1].y - statsP.y)- (ROI[i].y - statsP.y) * (ROI[i + 1].x - statsP.x)) == 0
            && ((ROI[i].x - statsP.x) * (ROI[i + 1].x + statsP.x) + (ROI[i].y - statsP.y) * (ROI[i + 1].y - statsP.y)) <= 0)
        {
            return 1;
        }
        if (((ROI[i].y - statsP.y) > 0 != (ROI[i + 1].y - statsP.y) > 0)
            && (statsP.x - (statsP.y - ROI[i].y) * (ROI[i].x - ROI[i + 1].x) / (ROI[i].y - ROI[i + 1].y) - ROI[i].x) < 0)
        {
            flag = (flag == 1) ? 0 : 1;
        }
    }

    return flag;
}

//int AWB_GREY_ROI_PROCESSold(struct Point ROI[5], unsigned int rg, unsigned int bg)
//{
//    struct Point statsP;
//    statsP.x = rg;
//    statsP.y = bg;
//    int flag = 0;
//
//    for (int i = 0; i < 4; i++)
//    {
//        if (cross(MINUS(ROI[i], statsP), MINUS(ROI[i + 1], statsP)) == 0
//            && dot(MINUS(ROI[i], statsP), MINUS(ROI[i + 1], statsP)) <= 0)
//        {
//            return 1;
//        }
//        if (((ROI[i].y - statsP.y) > 0 != (ROI[i + 1].y - statsP.y) > 0)
//            && (statsP.x - (statsP.y - ROI[i].y) * (ROI[i].x - ROI[i + 1].x) / (ROI[i].y - ROI[i + 1].y) - ROI[i].x) < 0)
//        {
//            flag = (flag == 1) ? 0 : 1;
//        }
//    }
//
//    return flag;
//}

void AWB_COLOR_ROI_PROCESS(struct Point ROI[5], unsigned int rg, unsigned int bg, unsigned int white_point_rg, unsigned int white_point_bg,
    unsigned int* white_point_rg_sum, unsigned int* white_point_bg_sum, unsigned int* count)
{
    struct Point statsP;
    statsP.x = rg;
    statsP.y = bg;

    for (int i = 0; i < 4; i++)
    {
        if (((ROI[i].y - statsP.y) > 0 != (ROI[i + 1].y - statsP.y) > 0)
            && (statsP.x - (statsP.y - ROI[i].y) * (ROI[i].x - ROI[i + 1].x) / (ROI[i].y - ROI[i + 1].y) - ROI[i].x) < 0)
        {
            *white_point_rg_sum += white_point_rg;
            *white_point_bg_sum += white_point_bg;
            *count += 1;
        }
    }
}

void cpyROI2Point(unsigned short greyROI[8], struct Point* greyPoint)
{
    int idx = 0;
    for (int i = 0; i < 8; i = i + 2)
    {
        greyPoint[idx].x = greyROI[i];
        greyPoint[idx].y = greyROI[i + 1];
        idx++;
    }
    greyPoint[idx].x = greyROI[0];
    greyPoint[idx].y = greyROI[1];
}


unsigned short MISGUIDE_ROI_PROCESS(AWB_MODULE_PARA* module_Para, unsigned int rg, unsigned int bg)
{
    unsigned short withdraw = 0;

    short BV = module_Para->BV;

    struct Point misguideRoi_1[5];
    struct Point misguideRoi_2[5];
    struct Point misguideRoi_3[5];
    struct Point misguideRoi_4[5];
    struct Point misguideRoi_5[5];

    cpyROI2Point(&module_Para->misguideRoi_1[0], misguideRoi_1);
    cpyROI2Point(&module_Para->misguideRoi_2[0], misguideRoi_2);
    cpyROI2Point(&module_Para->misguideRoi_3[0], misguideRoi_3);

    //亮度与误导颜色区1组成的抠除统计区间1
    if (BV < 3000 && BV > 1500) {
        if (AWB_GREY_ROI_PROCESS(misguideRoi_1, rg, bg))
        {
            module_Para->misguideRoi_1[8] += 1;
            withdraw = 1;
        }

    }
    //亮度与误导颜色区2组成的抠除统计区间2
    if (BV < 3000 && BV > 1500) {
        if (AWB_GREY_ROI_PROCESS(misguideRoi_2, rg, bg))
        {
            module_Para->misguideRoi_2[8] += 1;
            withdraw = 1;
        }
    }
    //亮度与误导颜色区3组成的抠除统计区间3
    if (BV < 3000 && BV > 1500) {
        if (AWB_GREY_ROI_PROCESS(misguideRoi_3, rg, bg))
        {
            module_Para->misguideRoi_3[8] += 1;
            withdraw = 1;
        }
    }
    return withdraw;
}


//=================================================================================================
/**
*@brief                    Scan for Gray Part, calculate rg/bg for each LightSource
*@param[In]   *patchData   Input patch data
*@param[In]   *module_Para  Input module parameters for voting process
*@param[OUT]  *report      Output light source voting result
*@return                   none
*/
void    AWBPV_MultiWin_ScanBlockData(
    AWB_INPUT_PATCH* patch_Para,
    AWB_MODULE_PARA* module_Para,
    AWB_VOTING_REPORT* report,
    unsigned short cameraID
)
{
    unsigned char   ucLSIndex;
    short   blockIndex, blockSize;
    //unsigned char   Y;
    unsigned short       RG, BG;
    unsigned short* lsWeightLUT;
    unsigned short  ucMinIndex, ucMinIndex2;
    unsigned short       udDist, udDistMin, udDistMin2;
    unsigned int         RG_diff, BG_diff;
    unsigned long long   RGSum[AWB_LIGHTSOURCE_NUM_MAX];
    unsigned long long   BGSum[AWB_LIGHTSOURCE_NUM_MAX];
    unsigned short       vote_NSum[AWB_LIGHTSOURCE_NUM_MAX];
    unsigned int* blockPtr;
    AWB_LIGHTSOURCE* lightsource;
    AWB_LIGHTSOURCE* cctCurve;
    AWB_VOTING_LIGHTSOURCE* lightsourceReport;

    unsigned short* globalRoi;
    struct Point purpleRoi[5];
    struct Point redRoi[5];
    struct Point greenRoi[5];
    struct Point yellowRoi[5];
    struct Point blueRoi[5];

    float* redOffset;
    float* greenOffset;
    float* greyOffset;
    float* yellowOffset;
    float* blueOffset;

    unsigned int white_point_rg;
    unsigned int white_point_bg;
    unsigned int cct, cct1, cct2;
    unsigned int grey_white_point_rg_sum = 0;
    unsigned int grey_white_point_bg_sum = 0;
    unsigned int grey_white_point_rg_sum_no_shift = 0;
    unsigned int grey_white_point_bg_sum_no_shift = 0;
    unsigned int global_count = 0;
    unsigned int grey_count = 0;
    unsigned int grey_count_real = 0;

    unsigned int red_white_point_rg_sum = 0;
    unsigned int red_white_point_bg_sum = 0;
    unsigned int green_white_point_rg_sum = 0;
    unsigned int green_white_point_bg_sum = 0;
    unsigned int yellow_white_point_rg_sum = 0;
    unsigned int yellow_white_point_bg_sum = 0;
    unsigned int blue_white_point_rg_sum = 0;
    unsigned int blue_white_point_bg_sum = 0;
    unsigned int green_count = 0;
    unsigned int red_count = 0;
    unsigned int yellow_count = 0;
    unsigned int blue_count = 0;
    unsigned int greyCountList[7] = { 0 };
    unsigned int greyValueSumRgList[7] = { 0 };
    unsigned int greyValueSumBgList[7] = { 0 };
    unsigned int maxGreyCountIdx = 0;
    unsigned int maxGreyCount = 0;

    struct Point greyRoi[5];
    struct Point greyRoi2[5];
    struct Point greyRoi3[5];
    struct Point greyRoi4[5];
    struct Point greyRoi5[5];
    struct Point greyRoi6[5];
    struct Point greyRoi7[5];
    struct Point greySubRoi[3][5];

    memset(RGSum, 0, sizeof(RGSum));
    memset(BGSum, 0, sizeof(BGSum));
    memset(vote_NSum, 0, sizeof(vote_NSum));

    blockPtr = patch_Para->addr;
    blockSize = patch_Para->width * patch_Para->height;

    lightsource = &(module_Para->tLightSourceArray.source[0]);
    lsWeightLUT = &(module_Para->auwLightSourceWeightTable[0]);                 //这个auwLightSourceWeightTable里面是按照亮度插值之后的各个标定光源的权重

    globalRoi = &(module_Para->globalRoi[0]);
    cpyROI2Point(module_Para->greyRoi, greyRoi);
    cpyROI2Point(module_Para->greyRoi2, greyRoi2);
    cpyROI2Point(module_Para->greyRoi3, greyRoi3);
    cpyROI2Point(module_Para->greyRoi4, greyRoi4);
    cpyROI2Point(module_Para->greyRoi5, greyRoi5);
    cpyROI2Point(module_Para->greyRoi6, greyRoi6);
    cpyROI2Point(module_Para->greyRoi7, greyRoi7);

    cpyROI2Point(module_Para->purple_region, purpleRoi);
    cpyROI2Point(module_Para->red_region, redRoi);
    cpyROI2Point(module_Para->green_region, greenRoi);
    cpyROI2Point(module_Para->blue_region, blueRoi);
    cpyROI2Point(module_Para->yellow_region, yellowRoi);

    for (int i = 0; i < 3; i++)
    {
        cpyROI2Point(&module_Para->grey_sub_region[i * 8], greySubRoi[i]);
    }

    redOffset = &(module_Para->redRoiOffset[0]);
    greenOffset = &(module_Para->greenRoiOffset[0]);
    greyOffset = &(module_Para->greyRoiOffset[0]);
    yellowOffset = &(module_Para->yellowRoiOffset[0]);
    blueOffset = &(module_Para->blueRoiOffset[0]);

    float grey_gain[2] = { 0 };
    float yellow_gain[2] = { 0 };
    float blue_gain[2] = { 0 };
    float red_gain[2] = { 0 };
    float green_gain[2] = { 0 };

    lightsourceReport = &(report->awb_LightSource[0]);
    short BV = module_Para->BV;
    int BV_index_high = -1, BV_index_low = -1;
    //根据亮度获取最近的两个亮度等级，若在边界亮度等级之外，则两个亮度等级都取边界
    short BV_level[AWB_LS_SHIFT_TABLE_BV_NUM_PARA0] = AWB_LS_SHIFT_TABLE_BV_PARA0;
    for (int i = 0; i < AWB_LS_SHIFT_TABLE_BV_NUM_PARA0; i++)
    {
        if (BV_level[i] >= BV)
        {
            break;
        }
        BV_index_low = i;
    }
    if (BV_index_low == -1) {
        BV_index_high = 0;
        BV_index_low = 0;
    }
    else if (BV_index_low == AWB_LS_SHIFT_TABLE_BV_NUM_PARA0 - 1) {
        BV_index_high = BV_index_low;
    }
    else
    {
        BV_index_high = BV_index_low + 1;
    }
    for (blockIndex = 0; blockIndex < blockSize; blockIndex++, blockPtr++)
    {
        RG = (unsigned short)(((*blockPtr) >> 10) & 0x3FF);                         //佳云提供的RG 和 BG可能是反的，所以这里也反着用了！！！
        BG = (unsigned short)(((*blockPtr) >> 0) & 0x3FF);

        //LOG_DEBUG("blockIndex = %d, RG = %d, BG = %d,", blockIndex, RG, BG);

        if ((RG < AWB_STATS_RG_SAT) || (BG < AWB_STATS_BG_SAT))
            continue;
        if ((RG < globalRoi[0]) || (RG > globalRoi[1]) || (BG < globalRoi[2]) || (BG > globalRoi[3]))
            continue;
        if (MISGUIDE_ROI_PROCESS(module_Para, RG, BG))
            continue;

        global_count += 1;
        ucMinIndex = 0;
        ucMinIndex2 = 0;
        udDistMin = 0xFFFF;
        udDistMin2 = 0xFFFF;
        for (ucLSIndex = 0; ucLSIndex < module_Para->ls_num; ucLSIndex++)
        {
            BG_diff = (unsigned int)ABS(RG - lightsource[ucLSIndex].rg);                //标定的rg bg的值乘了100 ， RG 和 BG也是乘了100， rg bg是原始的Bu_AwbPara.h里的值
            RG_diff = (unsigned int)ABS(BG - lightsource[ucLSIndex].bg);
            udDist = sqrt_16(RG_diff * RG_diff + BG_diff * BG_diff);
            // if(udDist > lightsource[ucLSIndex].range)
            // 	continue;			
            if (udDist < udDistMin2)
            {
                udDistMin2 = udDist;
                ucMinIndex2 = ucLSIndex;
            }if (udDist < udDistMin)
            {
                udDistMin2 = udDistMin;
                ucMinIndex2 = ucMinIndex;
                udDistMin = udDist;
                ucMinIndex = ucLSIndex;
            }
        }

        if (udDistMin == 0xFFFF)
            continue;
        if (0 == lsWeightLUT[ucMinIndex])
            continue;
        white_point_rg = RG;
        white_point_bg = BG;

        int isGrey = 0;
        if (AWB_GREY_ROI_PROCESS(greyRoi7, RG, BG))
        {
            //grey_white_point_rg_sum += (float)(white_point_rg ) * lightWeight / (float)(10 + udDistMin);
            //grey_white_point_bg_sum += (float)(white_point_bg ) * lightWeight / (float)(10 + udDistMin);
            
            grey_white_point_rg_sum += (white_point_rg + lightsource[6].shift_rg) * lsWeightLUT[6] / (10 + udDistMin);
            grey_white_point_bg_sum += (white_point_bg + lightsource[6].shift_bg) * lsWeightLUT[6] / (10 + udDistMin);
            grey_count += lsWeightLUT[6] / (10 + udDistMin);
            isGrey = 1;
            greyCountList[6]++;
            greyValueSumRgList[6] += white_point_rg;
            greyValueSumBgList[6] += white_point_bg;
        }
        if (AWB_GREY_ROI_PROCESS(greyRoi6, RG, BG))
        {
            grey_white_point_rg_sum += (white_point_rg + lightsource[5].shift_rg) * lsWeightLUT[5] / (10 + udDistMin);
            grey_white_point_bg_sum += (white_point_bg + lightsource[5].shift_bg) * lsWeightLUT[5] / (10 + udDistMin);
            /*grey_count += 1.0 * lightWeight / (float)(10 + udDistMin);*/
            grey_count += lsWeightLUT[5] / (10 + udDistMin);
            isGrey = 1;
            greyCountList[5]++;
            greyValueSumRgList[5] += white_point_rg;
            greyValueSumBgList[5] += white_point_bg;
        }
        if (AWB_GREY_ROI_PROCESS(greyRoi5, RG, BG))        //d50
        {
            grey_white_point_rg_sum += (white_point_rg + lightsource[4].shift_rg) * lsWeightLUT[4] / (10 + udDistMin);
            grey_white_point_bg_sum += (white_point_bg + lightsource[4].shift_bg) * lsWeightLUT[4] / (10 + udDistMin);
            grey_count += lsWeightLUT[4] / (10 + udDistMin);
            isGrey = 1;
            greyCountList[4]++;
            greyValueSumRgList[4] += white_point_rg;
            greyValueSumBgList[4] += white_point_bg;
        }
        if (AWB_GREY_ROI_PROCESS(greyRoi4, RG, BG))        //cwf
        {
            grey_white_point_rg_sum += (white_point_rg + lightsource[3].shift_rg) * lsWeightLUT[3] / (10 + udDistMin);
            grey_white_point_bg_sum += (white_point_bg + lightsource[3].shift_bg) * lsWeightLUT[3] / (10 + udDistMin);
            grey_count += lsWeightLUT[3] / (10 + udDistMin);
            isGrey = 1;
            greyCountList[3]++;
            greyValueSumRgList[3] += white_point_rg;
            greyValueSumBgList[3] += white_point_bg;
        }
        if (AWB_GREY_ROI_PROCESS(greyRoi3, RG, BG))        //tl84
        {
            grey_white_point_rg_sum += (white_point_rg + lightsource[2].shift_rg) * lsWeightLUT[2] /(10 + udDistMin);
            grey_white_point_bg_sum += (white_point_bg + lightsource[2].shift_bg) * lsWeightLUT[2] /(10 + udDistMin);
            grey_count += lsWeightLUT[2] / (10 + udDistMin);
            isGrey = 1;
            greyCountList[2]++;
            greyValueSumRgList[2] += white_point_rg;
            greyValueSumBgList[2] += white_point_bg;
        }
        if (AWB_GREY_ROI_PROCESS(greyRoi2, RG, BG))        //A
        {
            grey_white_point_rg_sum += (white_point_rg + lightsource[1].shift_rg) * lsWeightLUT[1] /(10 + udDistMin);
            grey_white_point_bg_sum += (white_point_bg + lightsource[1].shift_bg) * lsWeightLUT[1] /(10 + udDistMin);
            grey_count += lsWeightLUT[1] / (10 + udDistMin);
            isGrey = 1;
            greyCountList[1]++;
            greyValueSumRgList[1] += white_point_rg;
            greyValueSumBgList[1] += white_point_bg;
        }
        if (AWB_GREY_ROI_PROCESS(greyRoi, RG, BG))            //H
        {
            grey_white_point_rg_sum += (white_point_rg + lightsource[0].shift_rg) * lsWeightLUT[0] / (10 + udDistMin);
            grey_white_point_bg_sum += (white_point_bg + lightsource[0].shift_bg) * lsWeightLUT[0] / (10 + udDistMin);
            grey_count += lsWeightLUT[0] / (10 + udDistMin);
            isGrey = 1;
            greyCountList[0]++;
            greyValueSumRgList[0] += white_point_rg;
            greyValueSumBgList[0] += white_point_bg;
        }
        if (isGrey == 1)
        {
            grey_white_point_rg_sum_no_shift += RG;
            grey_white_point_bg_sum_no_shift += BG;
            grey_count_real++;
        }
        //AWB_ROI_PROCESS(greyRoi, RG, BG, white_point_rg, white_point_bg, &grey_white_point_rg_sum, &grey_white_point_bg_sum, &grey_count);

        AWB_COLOR_ROI_PROCESS(redRoi, RG, BG, white_point_rg, white_point_bg, &red_white_point_rg_sum, &red_white_point_bg_sum, &red_count);
        AWB_COLOR_ROI_PROCESS(greenRoi, RG, BG, white_point_rg, white_point_bg, &green_white_point_rg_sum, &green_white_point_bg_sum, &green_count);
        AWB_COLOR_ROI_PROCESS(yellowRoi, RG, BG, white_point_rg, white_point_bg, &yellow_white_point_rg_sum, &yellow_white_point_bg_sum, &yellow_count);
        AWB_COLOR_ROI_PROCESS(blueRoi, RG, BG, white_point_rg, white_point_bg, &blue_white_point_rg_sum, &blue_white_point_bg_sum, &blue_count);

        RGSum[ucMinIndex] += (unsigned long long)RG;
        BGSum[ucMinIndex] += (unsigned long long)BG;
        vote_NSum[ucMinIndex]++;
    }

    for (int i = 0; i < 7; i++)
    {
        if (greyCountList[i] > maxGreyCount)
        {
            maxGreyCount = greyCountList[i];
            maxGreyCountIdx = i;
        }
    }

    LOG_DEBUG("\n[WhiteBox] maxGreyCount grey d75 %u d65 %u d50 %u cwf %u tl84 %u A %u H-actual %u \n",
        greyCountList[6], greyCountList[5], greyCountList[4], greyCountList[3], greyCountList[2], greyCountList[1],
        greyCountList[0]);

    int total_count = grey_count_real + red_count + green_count + yellow_count + blue_count;

    if (grey_count_real < (int)64) {
        report->RG = 440;               //统计点数量不足64个时，用D65的默认参数 
        report->BG = 630;
        cct = 6500;
        report->ColorTemperature = (unsigned short)cct;
        report->totalVotes = 0;
        return;
    }
    if (grey_count != 0) {
        grey_gain[0] = grey_white_point_rg_sum / grey_count;
        grey_gain[1] = grey_white_point_bg_sum / grey_count;
    }
    if (red_count != 0) {
        red_gain[0] = red_white_point_rg_sum / red_count;
        red_gain[1] = red_white_point_bg_sum / red_count;
    }
    if (green_count != 0) {
        green_gain[0] = green_white_point_rg_sum / green_count;
        green_gain[1] = green_white_point_bg_sum / green_count;
    }
    if (yellow_count != 0) {
        yellow_gain[0] = yellow_white_point_rg_sum / yellow_count;
        yellow_gain[1] = yellow_white_point_bg_sum / yellow_count;
    }
    if (blue_count != 0 && blue_count != 0) {
        blue_gain[0] = blue_white_point_rg_sum / blue_count;
        blue_gain[1] = blue_white_point_bg_sum / blue_count;
    }
    grey_gain[0] = (greyOffset[0] + grey_gain[0]) * ((float)grey_count_real / total_count) * (1 + greyOffset[2]);
    grey_gain[1] = (greyOffset[1] + grey_gain[1]) * ((float)grey_count_real / total_count) * (1 + greyOffset[2]);
    red_gain[0] = (redOffset[0] + red_gain[0]) * ((float)red_count / total_count) * (1 + redOffset[2]);
    red_gain[1] = (redOffset[1] + red_gain[1]) * ((float)red_count / total_count) * (1 + redOffset[2]);
    green_gain[0] = (greenOffset[0] + green_gain[0]) * ((float)green_count / total_count) * (1 + greenOffset[2]);
    green_gain[1] = (greenOffset[1] + green_gain[1]) * ((float)green_count / total_count) * (1 + greenOffset[2]);
    yellow_gain[0] = (yellowOffset[0] + yellow_gain[0]) * ((float)yellow_count / total_count) * (1 + yellowOffset[2]);
    yellow_gain[1] = (yellowOffset[1] + yellow_gain[1]) * ((float)yellow_count / total_count) * (1 + yellowOffset[2]);
    blue_gain[0] = (blueOffset[0] + blue_gain[0]) * ((float)blue_count / total_count) * (1 + blueOffset[2]);
    blue_gain[1] = (blueOffset[1] + blue_gain[1]) * ((float)blue_count / total_count) * (1 + blueOffset[2]);

    unsigned short RGLow = (unsigned short)(grey_gain[0] + red_gain[0] + green_gain[0] + yellow_gain[0] + blue_gain[0]);            //这就是最终倒数为gain的rg 和 bg了
    unsigned short BGLow = (unsigned short)(grey_gain[1] + red_gain[1] + green_gain[1] + yellow_gain[1] + blue_gain[1]);

    float ratio_high = 0, ratio_low = 0;
    if (BV_index_low == BV_index_high) {
        ratio_high = 1;
        ratio_low = 1 - ratio_high;
    }
    else
    {
        ratio_high = (BV - BV_level[BV_index_low]) / (float)(BV_level[BV_index_high] - BV_level[BV_index_low]);
        ratio_low = 1 - ratio_high;
    }
    //udDistMin = 0xFFFF;
    grey_white_point_rg_sum_no_shift = grey_white_point_rg_sum_no_shift / grey_count_real;
    grey_white_point_bg_sum_no_shift = grey_white_point_bg_sum_no_shift / grey_count_real;
    report->R_Gain_no_shift = grey_white_point_rg_sum_no_shift;
    report->B_Gain_no_shift = grey_white_point_bg_sum_no_shift;

    report->RG = (unsigned short)(RGLow * 10);                                 //这就是最终倒数为gain的rg 和 bg了
    report->BG = (unsigned short)(BGLow * 10);
    if (report->RG <= 0 || report->RG >= 1500) report->RG = lightsource[3].rg;
    if (report->BG <= 0 || report->BG >= 1500) report->BG = lightsource[3].bg;
    //cctCurve[15].ct; 这个值经过了上面两行代码就会变化，为什么？？？

    //**********************************时序算法部分，离线仿真的时候可以考虑关闭**************************************
    short stableLabel = stableCalculate(module_Para, report->RG, report->BG, report->R_Gain_no_shift, report->B_Gain_no_shift, &greyCountList, cameraID);
    if (stableLabel == 1) {
        report->RG = handle_param_list[cameraID].stableRG;
        report->BG = handle_param_list[cameraID].stableBG;
        report->R_Gain_no_shift = handle_param_list[cameraID].stableNoShiftRG;
        report->B_Gain_no_shift = handle_param_list[cameraID].stableNoShiftBG;
    }
    //**********************************时序算法部分，离线仿真的时候可以考虑关闭**************************************

    report->ColorTemperature = (unsigned short)6500;
    // First find out max weighting for exterior color.
    report->totalVotes += vote_NSum[ucLSIndex];
    report->grey_count = grey_count;
    report->maxGreyCount = maxGreyCount;
    report->maxGreyCountIdx = maxGreyCountIdx;
    report->udDistMin = udDistMin;
    //LOG_DEBUG("rg = %d, bg = %d", report->RG, report->BG);
}

int abs_(int num) {
    if (num < 0) {
        return -num;
    }
    else {
        return num;
    }
}

short stableCalculate(AWB_MODULE_PARA* module_para, unsigned short RG, unsigned short BG, unsigned short noShiftRG, unsigned short noShiftBG, unsigned int* greyCountList, unsigned short cameraID)
{

    unsigned short currentTop3Region[3];
    //手遮挡的情况下是否出stable  满足条件则不出，其他情况都出
    if (handle_param_list[cameraID].stableLabel == 1) {
        if ((abs_(RG - handle_param_list[cameraID].stableRG) + abs_(BG - handle_param_list[cameraID].stableBG)) > 20) {
            rankingTop3(greyCountList, &currentTop3Region);
            //若手遮挡同时色温变高，则出stable
            int history_high_cct_num = 0, this_frame_high_cct_num = 0;
            for (int m = 6; m > handle_param_list[cameraID].topCountRegion[0]; m--) {
                history_high_cct_num += handle_param_list[cameraID].RegionCountHistory[m];
                this_frame_high_cct_num += greyCountList[m];
            }
            if (this_frame_high_cct_num > history_high_cct_num * 1.1) {
                if (this_frame_high_cct_num > 50) {
                    return handle_param_list[cameraID].stableLabel = 0;
                }
            }
            if (currentTop3Region[0] <= handle_param_list[cameraID].topCountRegion[0]) {
                if (currentTop3Region[0] + currentTop3Region[1] <= handle_param_list[cameraID].topCountRegion[0] + handle_param_list[cameraID].topCountRegion[1]) {
                    if (abs_(currentTop3Region[1] - handle_param_list[cameraID].topCountRegion[1]) + abs_(currentTop3Region[0] - handle_param_list[cameraID].topCountRegion[0]) <= 5) {
                        if (handle_param_list[cameraID].RegionCountHistory[handle_param_list[cameraID].topCountRegion[0]] >= greyCountList[handle_param_list[cameraID].topCountRegion[0]]) {
                            float tmp1 = handle_param_list[cameraID].RegionCountHistory[handle_param_list[cameraID].topCountRegion[1]] + handle_param_list[cameraID].RegionCountHistory[handle_param_list[cameraID].topCountRegion[0]];
                            float tmp2 = greyCountList[handle_param_list[cameraID].topCountRegion[1]] + greyCountList[handle_param_list[cameraID].topCountRegion[0]];
                            if (tmp2 / tmp1 > 0.2) {
                                return handle_param_list[cameraID].stableLabel = 1;
                            }
                        }
                    }
                }
            }
        }
        else if ((abs_(RG - handle_param_list[cameraID].stableRG) + abs_(BG - handle_param_list[cameraID].stableBG)) <= 20) {
            return handle_param_list[cameraID].stableLabel = 1;
        }
        handle_param_list[cameraID].seriesLengthEnoughLabel = 0;
        return handle_param_list[cameraID].stableLabel = 0;
    }

    if (handle_param_list[cameraID].stableLabel != 1) {
        if (handle_param_list[cameraID].frameNum % 6 == 0) {          //每隔6帧存一次gain，一直存10次  
            if (handle_param_list[cameraID].seriesLengthEnoughLabel == 0) {
                handle_param_list[cameraID].RGainSeries[handle_param_list[cameraID].frameNum / 6] = RG;
                handle_param_list[cameraID].BGainSeries[handle_param_list[cameraID].frameNum / 6] = BG;
                handle_param_list[cameraID].noShiftRGainSeries[handle_param_list[cameraID].frameNum / 6] = noShiftRG;
                handle_param_list[cameraID].noShiftBGainSeries[handle_param_list[cameraID].frameNum / 6] = noShiftBG;
                handle_param_list[cameraID].frameNum += 1;
                return 0;
            }
            if (handle_param_list[cameraID].seriesLengthEnoughLabel == 1) {
                for (int j = 0; j < 9; j++) {
                    handle_param_list[cameraID].RGainSeries[j] = handle_param_list[cameraID].RGainSeries[j + 1];
                    handle_param_list[cameraID].BGainSeries[j] = handle_param_list[cameraID].BGainSeries[j + 1];
                    handle_param_list[cameraID].noShiftRGainSeries[j] = handle_param_list[cameraID].noShiftRGainSeries[j + 1];
                    handle_param_list[cameraID].noShiftBGainSeries[j] = handle_param_list[cameraID].noShiftBGainSeries[j + 1];
                }
                handle_param_list[cameraID].RGainSeries[9] = RG;
                handle_param_list[cameraID].BGainSeries[9] = BG;
                handle_param_list[cameraID].noShiftRGainSeries[9] = noShiftRG;
                handle_param_list[cameraID].noShiftBGainSeries[9] = noShiftBG;
            }
        }

        handle_param_list[cameraID].frameNum += 1;
        if (handle_param_list[cameraID].frameNum > 54) {      //每隔6帧存一次gain，存够6*(10-1)次就够  
            handle_param_list[cameraID].frameNum = 0;
            if (handle_param_list[cameraID].seriesLengthEnoughLabel == 0) {
                handle_param_list[cameraID].seriesLengthEnoughLabel = 1;
            }
        }

        if (handle_param_list[cameraID].seriesLengthEnoughLabel == 1) {
            int sum_rg = 0, sum_bg = 0, avg_rg = 0, avg_bg = 0, delta_rg = 0, delta_bg = 0, sum_no_shift_rg = 0, sum_no_shift_bg = 0,
                avg_no_shift_rg = 0, avg_no_shift_bg = 0;
            for (int i = 0; i < 9; i++) {          //i<19:只用最近10帧之前的数据计算稳定状态
                sum_rg += handle_param_list[cameraID].RGainSeries[i];
                sum_bg += handle_param_list[cameraID].BGainSeries[i];
                sum_no_shift_rg += handle_param_list[cameraID].noShiftRGainSeries[i];
                sum_no_shift_bg += handle_param_list[cameraID].noShiftBGainSeries[i];
            }
            avg_rg = sum_rg / 9.0f;
            avg_bg = sum_bg / 9.0f;
            avg_no_shift_rg = sum_no_shift_rg / 9.0f;
            avg_no_shift_bg = sum_no_shift_bg / 9.0f;
            for (int i = 0; i < 9; i++) {          //i<19:只用最近10帧之前的数据计算稳定状态
                delta_rg = abs_(handle_param_list[cameraID].RGainSeries[i] - avg_rg);
                delta_bg = abs_(handle_param_list[cameraID].BGainSeries[i] - avg_bg);
                if ((delta_rg + delta_bg) >= 25) {
                    return handle_param_list[cameraID].stableLabel = 0;
                }
            }
            rankingTop3(greyCountList, &handle_param_list[cameraID].topCountRegion);
            for (int j = 0; j < 7; j++) {
                handle_param_list[cameraID].RegionCountHistory[j] = greyCountList[j];
            }
            handle_param_list[cameraID].stableLabel = 1;
            handle_param_list[cameraID].stableRG = avg_rg;
            handle_param_list[cameraID].stableBG = avg_bg;
            handle_param_list[cameraID].stableNoShiftRG = avg_no_shift_rg;
            handle_param_list[cameraID].stableNoShiftBG = avg_no_shift_bg;
        }
    }
    return handle_param_list[cameraID].stableLabel;
}

void rankingTop3(unsigned int* greyCountList, unsigned short* top3Region) {
    int i;
    int min = 0, min2 = 0, min3 = 0;
    int min_index = 0, min2_index = 0, min3_index = 0;
    //排序主体
    for (i = 0; i < 7; i++) {
        if (greyCountList[i] > min2)
        {
            min2 = greyCountList[i];
            min2_index = i;
            if (greyCountList[i] > min) {
                min2 = min;
                min = greyCountList[i];
                min2_index = min_index;
                min_index = i;
            }
        }
    }
    for (i = 0; i < 7; i++) {
        if (greyCountList[i] < min2)
        {
            if (greyCountList[i] > min3) {
                min3 = greyCountList[i];
                min3_index = i;
            }
        }
    }
    top3Region[0] = min_index;
    top3Region[1] = min2_index;
    top3Region[2] = min3_index;
}


//=================================================================================================
/**
*@brief                Weighting each light source volting result
*@param[In]   *report  Input light source volting result
*@param[Out]  *report  Output weighted white point
*@return               None
*/
void AWB_MultiWin_WeightingResult(AWB_VOTING_REPORT* report)
{
    report->RG_Capture = report->RG;
    report->BG_Capture = report->BG;
    report->ColorTemperature_Capture = report->ColorTemperature;
}

//=================================================================================================
/**
*@brief                        Multi window voting
*@param[In]   *algorithm_Para  Input Patch data and module parameters
*@param[OUT]  *algorithm_Para  Output voting result
*@return                       none
*/
void AWB_MultiVoting(IQAWB_ALGORITHM_PARA* algorithm_Para, IQAWB_ALGORITHM_REPORT* algorithm_Report, unsigned short cameraID)
{
    AWB_INPUT_PATCH* input_Patch = &algorithm_Para->input_Patch;       // Input data pointer	
    AWB_MODULE_PARA* module_Para = &(algorithm_Report->module_Para);
    AWB_VOTING_REPORT* vote_Report = &(algorithm_Report->vote_Report);

    vote_Report->BV = module_Para->BV;
    vote_Report->ls_num = module_Para->ls_num;
    // Scan Gray Zone, calculate RG BG for each LightSource
    AWBPV_MultiWin_ScanBlockData(input_Patch, module_Para, vote_Report, cameraID);        //光源标定数据经过计算后读入到vote_Report->awb_LightSource里面，并不是直接读入

    // Weighting Result from different Light Source and find the max weighting result index
    AWB_MultiWin_WeightingResult(vote_Report);     

}

//=================================================================================================
/**
*@brief                        Calculate patch data
*@param[In]   *algorithm_Para  Input Patch information
*@param[OUT]  *algorithm_Para  Output patch data with sumup ( to spee up
*@return                       none
*/
enum altek_awb_error_code   AWB_ComputePatch(IQAWB_ALGORITHM_PARA* algorithm_Para, IQAWB_ALGORITHM_REPORT* algorithm_Report)
{
    AWB_INPUT_PATCH* input_Patch = &algorithm_Para->input_Patch;       // Input data pointer
    AWB_PATCH_REPORT* patch_Report = &algorithm_Report->patch_Report;    // Output data pointer
    //AWB_PATCH_INFO      *patch = patch_Para->patch_Addr;

    // Avoid patching width / height div 0
    if ((0 == input_Patch->width) || (0 == input_Patch->height))
    {
        return _AL_AWBLIB_PROCESS_STATS_SIZE;
    }

    patch_Report->patch_Width = input_Patch->width;
    patch_Report->patch_Height = input_Patch->height;
    patch_Report->iso_r_128 = input_Patch->iso_r_128;
    patch_Report->iso_b_128 = input_Patch->iso_b_128;

    return _AL_AWBLIB_SUCCESS;
}


void altek_AWB_ComputePatch_ISP1(unsigned short cameraID, IQAWB_INPUT_PARA* inputPara)
{
    short   patch_W = inputPara->width;
    short   patch_H = inputPara->height;
    unsigned short  uwXCnt, uwYCnt;
    unsigned int* imageAddr;
    unsigned int* imageIn;
    unsigned int* imageOut;
    unsigned int* imageOutLine;

    //. set start position, must be RGGB
    imageIn = (unsigned int*)inputPara->addr;

    //for (int i = 0; i < 1000; i++) {
    //    printf("aaaa INIT stats %d\r\n", imageIn[i]);
    //}
    //. prepare first HW3A 64x48 data
    imageOutLine = hw3A_64_48[cameraID];
    imageAddr = imageIn;// +(unsigned int)image_W * uwYCnt;
    for (uwYCnt = 0; uwYCnt < patch_H; uwYCnt++)
    {
        imageOut = imageOutLine;

        for (uwXCnt = 0; uwXCnt < patch_W; uwXCnt++)
        {
            // Calculate one bank's R sum, G sum and B sum.
            AWB_PatchRaw_CalculateOneISP1(
                imageAddr,
                inputPara,
                inputPara->iso_r_128,
                inputPara->iso_b_128,
                imageOut
            );
            // imageAddr += 10;
            imageAddr += 5;
            imageOut += 1;
        }
        imageOutLine += patch_W;
    }

    inputPara->addr = hw3A_64_48[cameraID];
}


//      Transform APIs
//==================================================================================================
static int AWB_MultiWin_InterpInt(int a_dX1, int a_dY1, int a_dX2, int a_dY2, int a_dX)
{
    if (a_dX <= a_dX1)
        return a_dY1;
    else if (a_dX >= a_dX2)
        return a_dY2;
    else
        return a_dY1 + (a_dY2 - a_dY1) * (a_dX - a_dX1) / (a_dX2 - a_dX1);
}

//==================================================================================================
/**
*@brief                     Calculate AWB Gain(256 base)
*@param[In]                 RG
*@param[In]                 BG
*@param[In]     *iso_gain   ISO Gain
*@param[Out]    *rgb        AWB Gain
*@return                    none
*/
void AWB_Calculate_rgbGain(unsigned short rg, unsigned short bg, AWB_RGB* rgb)
{
    unsigned int   min_gain;
    unsigned int   min_gain_2;
    unsigned int  R, G, B;

    R = (unsigned int)(256000 / (unsigned int)rg);              // 约等于256 / rg * 1000   相当于求1/rg
    G = 256;
    B = (unsigned int)(256000 / (unsigned int)bg);

    min_gain = G;
    min_gain_2 = min_gain >> 1;

    if (0 != min_gain)
    {
        rgb->R = (unsigned short)(((unsigned int)R * 256 + min_gain_2) / min_gain);                      //基本上不会怎么改变RGB的值
        rgb->G = (unsigned short)(((unsigned int)G * 256 + min_gain_2) / min_gain);
        rgb->B = (unsigned short)(((unsigned int)B * 256 + min_gain_2) / min_gain);
    }
    else
    {
        rgb->R = 256;
        rgb->G = 256;
        rgb->B = 256;
    }
}

//==================================================================================================
/**
*@brief         Transform RGB(9 bit) to HW3A YCC
*@param[Out]    *output_Y           hw3a Y
*@param[Out]    *output_RG          hw3a RG
*@param[Out]    *output_RG          hw3a BG
*@param[In]     RValue              R value @ 9 bit
*@param[In]     GValue              G value @ 9 bit
*@param[In]     BValue              B value @ 9 bit
*@param[In]     iso_r_128           iso r calibraion
*@param[In]     iso_b_128           iso b calibraion
*@param[In]     hw_bbr              BBR Line setting in HW_Config
*@return                 none
*/

void    AWB_RawRGB_To_HW3A(
    unsigned short* output_Y,
    unsigned short* output_RG,
    unsigned short* output_BG,
    unsigned int  RValue,
    unsigned int  GValue,
    unsigned int  BValue,
    short   iso_r_128,
    short   iso_b_128)
{
    unsigned short   RG, BG;
    unsigned int  R_Linear, G_Linear, B_Linear, Y_Linear;

    //Calculate Linear Y
    R_Linear = ((RValue * iso_r_128) >> 4);            //*128之后右移4位，等于是左移3位
    G_Linear = ((GValue) << 3);
    B_Linear = ((BValue * iso_b_128) >> 4);

    if (R_Linear > 8191)         R_Linear = 8191;
    if (B_Linear > 8191)         B_Linear = 8191;

    Y_Linear = ((int)(306 * R_Linear) + (int)(601 * G_Linear) + (int)(117 * B_Linear) + 16384) >> 15;
    if (Y_Linear > 255)        Y_Linear = 255;

    RG = (unsigned short)(100 * RValue / GValue);           //乘了100！！！
    BG = (unsigned short)(100 * BValue / GValue);

    *output_Y = (unsigned short)Y_Linear;
    *output_RG = RG;
    *output_BG = BG;

}


//==================================================================================================
/**
*@brief                    Set Gray Ballot light source parameters
*@param[In]   *inputPara   Input parameter index
*@param[In]   *tuning_file   Input tuning file
*@param[OUT]  *ptLightSourceArray  Fill up Gray Ballot light source parameters
*@return                   none
*/
void    AWB_MultiWin_SetGrayLightSourceArray(IQAWB_INPUT_PARA* inputPara, unsigned short light_num, void * lightsource_data, AWB_LIGHTSOURCE_ARRAY* ptLightSourceArray)
{
    unsigned char   lsIndex = 0;
    unsigned short  lightsourceNum = light_num;
    short   iso_r_128, iso_b_128;
    AWB_LIGHTSOURCE* lightsource = (AWB_LIGHTSOURCE*)lightsource_data;

    iso_r_128 = inputPara->iso_r_128;
    iso_b_128 = inputPara->iso_b_128;
    for (lsIndex = 0; lsIndex < lightsourceNum; lsIndex++)
    {
        ptLightSourceArray->source[lsIndex].rg = ((lightsource[lsIndex].rg) << 7) / iso_r_128;         //128即为2的7次方
        ptLightSourceArray->source[lsIndex].bg = ((lightsource[lsIndex].bg) << 7) / iso_b_128;
        ptLightSourceArray->source[lsIndex].ct = lightsource[lsIndex].ct;
        ptLightSourceArray->source[lsIndex].shift_rg = lightsource[lsIndex].shift_rg;
        ptLightSourceArray->source[lsIndex].shift_bg = lightsource[lsIndex].shift_bg;
    }
}


//==================================================================================================
/**
*@brief                    According BV to set Gray Ballot light source weighting table
*@param[In]   *inputPara   Input BV and parameter index
*@param[In]   *tuning_file   Input tuning file
*@param[OUT]  *a_pauwLightSourceWeightTable  Fill up Gray Ballot light source weighting table
*@return                   none
*/
static void AWB_MultiWin_SetGrayLightSourceWeightTable(IQAWB_INPUT_PARA* inputPara, unsigned short BV_levels, void* lightsource_data, unsigned short* a_pauwLightSourceWeightTable)   //根据亮度挡位插值出一个光源的权重
{
    unsigned char   ucLSIdx, ucBvTabIdx;
    short   dBV = inputPara->bv;
    unsigned short  number = BV_levels;
    TUNING_FILE_LS_WEIGHT_PARA* para_curr, * para_next;

    para_curr = para_next = (TUNING_FILE_LS_WEIGHT_PARA*)lightsource_data;            //根据亮度挡位插值出一个光源的权重
    for (ucBvTabIdx = 0; ucBvTabIdx < number; ucBvTabIdx++)
    {
        if (ucBvTabIdx == 0 && dBV <= para_curr->bv)
        {
            memcpy(a_pauwLightSourceWeightTable,
                &(para_curr->ls_weight_tab),
                sizeof(unsigned short) * (para_curr->ls_weight_lightsource_num));
        }
        else if (dBV >= para_curr->bv && dBV <= para_next->bv)
        {
            for (ucLSIdx = 0; ucLSIdx < para_curr->ls_weight_lightsource_num; ucLSIdx++)
            {
                a_pauwLightSourceWeightTable[ucLSIdx] = (unsigned short)AWB_MultiWin_InterpInt(
                    para_curr->bv,
                    para_curr->ls_weight_tab[ucLSIdx],
                    para_next->bv,
                    para_next->ls_weight_tab[ucLSIdx],
                    dBV);
            }
        }
        else if (ucBvTabIdx == number - 1 && dBV >= para_next->bv)
        {
            memcpy(a_pauwLightSourceWeightTable,
                &(para_next->ls_weight_tab),
                sizeof(unsigned short) * (para_next->ls_weight_lightsource_num));
        }

        para_curr = para_next;
        para_next++;
    }
}

//==================================================================================================
/**
*@brief                       Set shift table parameters
*@param[In]   *inputPara      Input shift table index
*@param[OUT]  *stylish_Para  Fill up shift table parameters
*@return                      none
*/
void    AWB_SetStylishPara(AWB_STYLISH_PARA* stylish_Para)
{
    stylish_Para->Num_Bv = (unsigned char)AWB_LS_SHIFT_TABLE_BV_NUM_PARA0;
    stylish_Para->Tbl_Bv = (short*)g_tuningFile_stylish_bv_table;

    stylish_Para->Num_CT = (unsigned char)AWB_LS_SHIFT_TABLE_CT_NUM_PARA0;
    stylish_Para->Tbl_CT = (short*)g_tuningFile_stylish_ct_table;

    stylish_Para->tAwbTab_RG = (short*)g_tuningFile_stylish_rg_shift;
    stylish_Para->tAwbTab_BG = (short*)g_tuningFile_stylish_bg_shift;
}
//==================================================================================================

void AWB_SetModulePara_New(IQAWB_INPUT_PARA* inputPara, AWB_MODULE_PARA* module_Para)            //AWB的配置参数都在这个函数里面初始化的, 从tuningFile_addr读到module_Para
{
    //memset(module_Para, 0, sizeof(AWB_MODULE_PARA));

    module_Para->ls_num = AWB_TUNING_FILE_LIGHTSOURCE_NUM_PARA0;
    module_Para->BV = inputPara->bv;

    memcpy(module_Para->globalRoi, g_tuningFile_global_roi, sizeof(g_tuningFile_global_roi));

    memcpy(module_Para->greyRoi, g_tuningFile_grey_roi, sizeof(g_tuningFile_grey_roi));
    memcpy(module_Para->greyRoi2, g_tuningFile_grey_roi2, sizeof(g_tuningFile_grey_roi2));
    memcpy(module_Para->greyRoi3, g_tuningFile_grey_roi3, sizeof(g_tuningFile_grey_roi3));
    memcpy(module_Para->greyRoi4, g_tuningFile_grey_roi4, sizeof(g_tuningFile_grey_roi4));
    memcpy(module_Para->greyRoi5, g_tuningFile_grey_roi5, sizeof(g_tuningFile_grey_roi5));
    memcpy(module_Para->greyRoi6, g_tuningFile_grey_roi6, sizeof(g_tuningFile_grey_roi6));
    memcpy(module_Para->greyRoi7, g_tuningFile_grey_roi7, sizeof(g_tuningFile_grey_roi7));

    memcpy(module_Para->redRoiOffset, g_tuningFile_red_offset, sizeof(g_tuningFile_red_offset));
    memcpy(module_Para->greenRoiOffset, g_tuningFile_green_offset, sizeof(g_tuningFile_green_offset));
    memcpy(module_Para->greyRoiOffset, g_tuningFile_grey_offset, sizeof(g_tuningFile_grey_offset));
    memcpy(module_Para->yellowRoiOffset, g_tuningFile_yellow_offset, sizeof(g_tuningFile_yellow_offset));
    memcpy(module_Para->blueRoiOffset, g_tuningFile_blue_offset, sizeof(g_tuningFile_blue_offset));

    memcpy(module_Para->misguideRoi_1, g_tuningFile_misguide_roi_1, sizeof(g_tuningFile_misguide_roi_1));
    memcpy(module_Para->misguideRoi_2, g_tuningFile_misguide_roi_2, sizeof(g_tuningFile_misguide_roi_2));
    memcpy(module_Para->misguideRoi_3, g_tuningFile_misguide_roi_3, sizeof(g_tuningFile_misguide_roi_3));

    module_Para->purple_region = &purple_region;
    module_Para->red_region = &red_region;
    module_Para->blue_region = &blue_region;
    module_Para->yellow_region = &yellow_region;
    module_Para->green_region = &green_region;
    module_Para->grey_sub_region = &grey_sub_region;

    AWB_MultiWin_SetGrayLightSourceArray(inputPara, AWB_TUNING_FILE_LIGHTSOURCE_NUM_PARA0, g_tuningFile_LightSources, &(module_Para->tLightSourceArray));
    //. Weighting  
    AWB_MultiWin_SetGrayLightSourceWeightTable(inputPara, AWB_LS_WEIGHTING_TABLE_BV_NUM_PARA0, g_tuningFile_LSWeighting, (module_Para->auwLightSourceWeightTable));

    //.***************************************输入参数范围验证***************************************


    // ***************************************输入参数范围验证***************************************
    // 
    //如果没有读到param.txt,这里不能判断为真。只在init里面跑
    if (inputPara->load_params_from_buffer == 1) {


        module_Para->purple_region = &inputPara->params_from_txt.purple_region;
        module_Para->red_region = &inputPara->params_from_txt.red_region;
        module_Para->blue_region = &inputPara->params_from_txt.blue_region;
        module_Para->yellow_region = &inputPara->params_from_txt.yellow_region;
        module_Para->green_region = &inputPara->params_from_txt.green_region;

        for (int lsIndex = 0; lsIndex < lightSourceNum; lsIndex++)
        {
            module_Para->tLightSourceArray.source[lsIndex].shift_rg = inputPara->params_from_txt.LightSourceShift[2 * lsIndex];
            module_Para->tLightSourceArray.source[lsIndex].shift_bg = inputPara->params_from_txt.LightSourceShift[2 * lsIndex + 1];
        }

        memcpy(module_Para->greyRoi, (unsigned short*)(inputPara->params_from_txt.greyRoi), sizeof(g_tuningFile_grey_roi));
        memcpy(module_Para->greyRoi2, (unsigned short*)(inputPara->params_from_txt.greyRoi2), sizeof(g_tuningFile_grey_roi2));
        memcpy(module_Para->greyRoi3, (unsigned short*)(inputPara->params_from_txt.greyRoi3), sizeof(g_tuningFile_grey_roi3));
        memcpy(module_Para->greyRoi4, (unsigned short*)(inputPara->params_from_txt.greyRoi4), sizeof(g_tuningFile_grey_roi4));
        memcpy(module_Para->greyRoi5, (unsigned short*)(inputPara->params_from_txt.greyRoi5), sizeof(g_tuningFile_grey_roi5));
        memcpy(module_Para->greyRoi6, (unsigned short*)(inputPara->params_from_txt.greyRoi6), sizeof(g_tuningFile_grey_roi6));
        memcpy(module_Para->greyRoi7, (unsigned short*)(inputPara->params_from_txt.greyRoi7), sizeof(g_tuningFile_grey_roi7));

        memcpy(module_Para->redRoiOffset, (float*)(inputPara->params_from_txt.redRoiOffset), sizeof(g_tuningFile_red_offset));
        memcpy(module_Para->greenRoiOffset, (float*)(inputPara->params_from_txt.greenRoiOffset), sizeof(g_tuningFile_green_offset));
        memcpy(module_Para->yellowRoiOffset, (float*)(inputPara->params_from_txt.yellowRoiOffset), sizeof(g_tuningFile_yellow_offset));
        memcpy(module_Para->blueRoiOffset, (float*)(inputPara->params_from_txt.blueRoiOffset), sizeof(g_tuningFile_blue_offset));

        memcpy(module_Para->misguideRoi_1, (unsigned short*)(inputPara->params_from_txt.misguideRoi_1), sizeof(g_tuningFile_misguide_roi_1));
        memcpy(module_Para->misguideRoi_2, (unsigned short*)(inputPara->params_from_txt.misguideRoi_2), sizeof(g_tuningFile_misguide_roi_2));
        memcpy(module_Para->misguideRoi_3, (unsigned short*)(inputPara->params_from_txt.misguideRoi_3), sizeof(g_tuningFile_misguide_roi_3));
        //inputPara->load_params_from_buffer == 0;
    }

    module_Para->gain_adj = &g_tuningFile_gain_adj;
    //. Light Source and cct curve

}


//==================================================================================================
/**
*@brief                     		Load algorithm parameter
*@param[In]    *inputPara           input AWB parameters
*@param[Out]   *ouputPara            Output para
*@return                             none
*/
enum altek_awb_error_code    AWB_LoadTuningFileAddr(IQAWB_INPUT_PARA* inputPara)
{
    enum altek_awb_error_code   Error = _AL_AWBLIB_SUCCESS;

    if (inputPara->load_params_from_debug_txt == 1 && inputPara->enterFromProcess == 1) {
        int index = 0;
        for (int i = 0; i < 65; i++) {
            for (int j = 0; j < 13; j++) {
                g_tuningFile_stylish_rg_shift[i][j] = inputPara->params_from_txt.shiftRG[index];
                g_tuningFile_stylish_bg_shift[i][j] = inputPara->params_from_txt.shiftBG[index];
                index++;
            }
        }
        g_tuningFile_LightSources[0].rg = inputPara->params_from_txt.greyLightSource[0];
        g_tuningFile_LightSources[0].bg = inputPara->params_from_txt.greyLightSource[1];
        g_tuningFile_LightSources[1].rg = inputPara->params_from_txt.greyLightSource2[0];
        g_tuningFile_LightSources[1].bg = inputPara->params_from_txt.greyLightSource2[1];
        g_tuningFile_LightSources[2].rg = inputPara->params_from_txt.greyLightSource3[0];
        g_tuningFile_LightSources[2].bg = inputPara->params_from_txt.greyLightSource3[1];
        g_tuningFile_LightSources[3].rg = inputPara->params_from_txt.greyLightSource4[0];
        g_tuningFile_LightSources[3].bg = inputPara->params_from_txt.greyLightSource4[1];
        g_tuningFile_LightSources[4].rg = inputPara->params_from_txt.greyLightSource5[0];
        g_tuningFile_LightSources[4].bg = inputPara->params_from_txt.greyLightSource5[1];
        g_tuningFile_LightSources[5].rg = inputPara->params_from_txt.greyLightSource6[0];
        g_tuningFile_LightSources[5].bg = inputPara->params_from_txt.greyLightSource6[1];
        g_tuningFile_LightSources[6].rg = inputPara->params_from_txt.greyLightSource7[0];
        g_tuningFile_LightSources[6].bg = inputPara->params_from_txt.greyLightSource7[1];


        for (int i = 0; i < 7; i++) {
            g_tuningFile_LSWeighting[0].ls_weight_tab[i] = inputPara->params_from_txt.lightSourceWeight1[i];
            g_tuningFile_LSWeighting[1].ls_weight_tab[i] = inputPara->params_from_txt.lightSourceWeight2[i];
            g_tuningFile_LSWeighting[2].ls_weight_tab[i] = inputPara->params_from_txt.lightSourceWeight3[i];
            g_tuningFile_LSWeighting[3].ls_weight_tab[i] = inputPara->params_from_txt.lightSourceWeight4[i];
            g_tuningFile_LSWeighting[4].ls_weight_tab[i] = inputPara->params_from_txt.lightSourceWeight5[i];
        }

        //inputPara->load_params_from_debug_txt == 0;
    }




    return Error;
}
//==================================================================================================
/**
*@brief                              Initial AWB default Setting
*@param[In]    *inputPara           input AWB parameters
*@param[Out]   *ouputPara               Output para
*@return                             none
*/
enum altek_awb_error_code AWB_InitialSetting(IQAWB_INPUT_PARA* inputPara, IQAWB_OUTPUT_PARA* outputPara)
{
    enum altek_awb_error_code   Error = _AL_AWBLIB_SUCCESS;

    outputPara->rg_allbalance = (unsigned short)(128000 / (unsigned int)inputPara->iso_r_128);
    outputPara->bg_allbalance = (unsigned short)(128000 / (unsigned int)inputPara->iso_b_128);

    return Error;
}
//=================================================================================================
/**
*@brief                         Set input patch parameters for altek format
*@param[In]    format           Format of altek stats
*@param[In]    *inputPara       Input AWB Algorithm Data
*@param[Out]   *inputPatch      Fill up patch parameters
*@return                        Error code
*/
enum altek_awb_error_code AWB_SetPatchInfo_Altek(IQAWB_INPUT_PARA* inputPara, AWB_MEM_ALGORITHM* mem_algorithm)
{
    short               available_W, available_H;
    AWB_INPUT_PATCH* input_Patch = &(mem_algorithm->awb_algorithm_para.input_Patch);

    //. Check input para size
    if (inputPara->width > AWB_HW3A_PATCH_W || inputPara->height > AWB_HW3A_PATCH_H)
    {
        return _AL_AWBLIB_PROCESS_STATS_SIZE;
    }

    available_W = inputPara->width;
    available_H = inputPara->height;

    input_Patch->addr = (unsigned int*)inputPara->addr;

    //. be even since dzoom map.
    available_W = (available_W / AWB_BLOCK_PATCH_W_SCALE) & 0xFE;
    available_H = (available_H / AWB_BLOCK_PATCH_H_SCALE) & 0xFE;
    if (available_W > AWB_BLOCK_XCNT || available_H > AWB_BLOCK_YCNT)
    {
        return _AL_AWBLIB_PROCESS_STATS_SIZE;
    }

    input_Patch->width = available_W;
    input_Patch->height = available_H;
    input_Patch->iso_r_128 = inputPara->iso_r_128;
    input_Patch->iso_b_128 = inputPara->iso_b_128;


    return _AL_AWBLIB_SUCCESS;
}

//=================================================================================================
/**
*@brief                         Set input patch parameters
*@param[In]    *inputPara       Input AWB Algorithm Data
*@param[Out]   *inputPatch      Fill up patch parameters
*@return                        Error code
*/
enum altek_awb_error_code AWB_SetPatchInfo(IQAWB_INPUT_PARA* inputPara, AWB_MEM_ALGORITHM* mem_algorithm)
{
    return AWB_SetPatchInfo_Altek(inputPara, mem_algorithm);

}

//=================================================================================================
/**
*@brief                    Algorithm parameters setting
*@param[In]   *inputPara   Input AWB Algorithm Data
*@param[OUT]  *mem_addr    The memory pointer of algorithm parameters
*@return                   Error code
*/
enum altek_awb_error_code AWB_SetAlgorithmPara(IQAWB_INPUT_PARA* inputPara)
{
    AWB_MEM_ALGORITHM* algorithmMem;
    IQAWB_ALGORITHM_PARA* algorithm_Para;
    IQAWB_ALGORITHM_REPORT* algorithm_Report;
    enum altek_awb_error_code   Error = _AL_AWBLIB_SUCCESS;

    algorithmMem = (AWB_MEM_ALGORITHM*)(inputPara->awb_mem_address);
    algorithm_Para = &(algorithmMem->awb_algorithm_para);
    algorithm_Report = &(algorithmMem->awb_algorithm_report);

    //. Load CamInfo
    Error = AWB_LoadTuningFileAddr(inputPara);
    if (_AL_AWBLIB_SUCCESS != Error)
        return Error;

    //. Patch Module
    Error = AWB_SetPatchInfo(inputPara, algorithmMem);
    if (_AL_AWBLIB_SUCCESS != Error)
        return Error;

    //. Voting Module
    memset(&algorithm_Report->vote_Report, 0, sizeof(AWB_VOTING_REPORT));
    AWB_SetModulePara_New(inputPara, &(algorithm_Report->module_Para));

    //. Shift Table module
    AWB_SetStylishPara( &(algorithm_Para->stylish_Para));

    return Error;
}

void AWB_Stylish_Interpolation2D_New(
    short BV,
    unsigned short R_Gain,
    unsigned short B_Gain,
    short* a_pfPrun1,
    short* a_uc2DTable1,      //128-based 2D Table, size = (a_wTableW-2)x(a_wTableH-2)
    short* a_wXNodesPos,      //length = a_wTableW
    short* a_wYNodesPos,      //length = a_wTableH
    short       a_wTableW,
    short       a_wTableH)
{
    short   i;
    short   BVIndex_Low, BVIndex_High;
    short   RGIndex, BGIndex;
    int   aucFourCorner[8] = { 0,0,0,0,0,0,0,0 }; //For table1 
    int   wXRem;
    int   wXRange;
    int   wYRem_RG, wYRem_BG;
    int   wYRange_RG, wYRange_BG;
    float   fIntValue1_bg1, fIntValue1_bg2;
    float   fIntValue1_rg1, fIntValue1_rg2;
    //*R_Gain = 313;
    //*B_Gain = 722;
    *a_pfPrun1 = 0;
    if (BV < a_wXNodesPos[0])
        BV = a_wXNodesPos[0];

    if (BV >= a_wXNodesPos[a_wTableW - 1])
        BV = a_wXNodesPos[a_wTableW - 1];

    if (R_Gain < a_wYNodesPos[0])
        R_Gain = a_wYNodesPos[0];

    if (R_Gain >= a_wYNodesPos[a_wTableH - 1])
        R_Gain = a_wYNodesPos[a_wTableH - 1];

    if (B_Gain < a_wYNodesPos[0])
        B_Gain = a_wYNodesPos[0];

    if (B_Gain >= a_wYNodesPos[a_wTableH - 1])
        B_Gain = a_wYNodesPos[a_wTableH - 1];

    BVIndex_Low = 0;
    BVIndex_High = 0;
    for (i = 1; i < a_wTableW; i++)
    {
        if (a_wXNodesPos[i] >= BV)
        {
            BVIndex_Low = i - 1;
            break;
        }
    }
    BVIndex_High = BVIndex_Low + 1;
    wXRem = (int)(BV - a_wXNodesPos[BVIndex_Low]);
    wXRange = (int)(a_wXNodesPos[BVIndex_Low + 1] - a_wXNodesPos[BVIndex_Low]);

    RGIndex = 0;
    for (i = 1; i < a_wTableH; i++)
    {
        if (a_wYNodesPos[i] >= R_Gain)
        {
            RGIndex = i - 1;
            break;
        }
    }
    wYRem_RG = (int)(R_Gain - a_wYNodesPos[RGIndex]);
    wYRange_RG = (int)(a_wYNodesPos[RGIndex + 1] - a_wYNodesPos[RGIndex]);

    BGIndex = 0;
    for (i = 1; i < a_wTableH; i++)
    {
        if (a_wYNodesPos[i] >= B_Gain)
        {
            BGIndex = i - 1;
            break;
        }
    }
    wYRem_BG = (int)(B_Gain - a_wYNodesPos[BGIndex]);
    wYRange_BG = (int)(a_wYNodesPos[BGIndex + 1] - a_wYNodesPos[BGIndex]);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //Table boundary check. The first line of table top, bottom, left, and right are default value (1.0).
    aucFourCorner[0] = (int)a_uc2DTable1[((RGIndex + AWB_LS_SHIFT_TABLE_CT_NUM_PARA0 * BVIndex_Low) * AWB_LS_SHIFT_TABLE_CT_NUM_PARA0) + BGIndex];

    aucFourCorner[1] = (int)a_uc2DTable1[((RGIndex + 1 + AWB_LS_SHIFT_TABLE_CT_NUM_PARA0 * BVIndex_Low) * AWB_LS_SHIFT_TABLE_CT_NUM_PARA0) + BGIndex];

    aucFourCorner[2] = (int)a_uc2DTable1[((RGIndex + AWB_LS_SHIFT_TABLE_CT_NUM_PARA0 * BVIndex_Low) * AWB_LS_SHIFT_TABLE_CT_NUM_PARA0) + BGIndex + 1];

    aucFourCorner[3] = (int)a_uc2DTable1[((RGIndex + 1 + AWB_LS_SHIFT_TABLE_CT_NUM_PARA0 * BVIndex_Low) * AWB_LS_SHIFT_TABLE_CT_NUM_PARA0) + BGIndex + 1];

    fIntValue1_bg1 = (float)(aucFourCorner[0] * (wYRange_BG - wYRem_BG) + aucFourCorner[2] * (wYRem_BG)) / (float)wYRange_BG;
    fIntValue1_bg2 = (float)(aucFourCorner[1] * (wYRange_BG - wYRem_BG) + aucFourCorner[3] * (wYRem_BG)) / (float)wYRange_BG;
    fIntValue1_rg1 = (fIntValue1_bg1 * (wYRange_RG - wYRem_RG) + fIntValue1_bg2 * (wYRem_RG)) / wYRange_RG;
    aucFourCorner[4] = (int)a_uc2DTable1[((RGIndex + AWB_LS_SHIFT_TABLE_CT_NUM_PARA0 * BVIndex_High) * AWB_LS_SHIFT_TABLE_CT_NUM_PARA0) + BGIndex];

    aucFourCorner[5] = (int)a_uc2DTable1[((RGIndex + 1 + AWB_LS_SHIFT_TABLE_CT_NUM_PARA0 * BVIndex_High) * AWB_LS_SHIFT_TABLE_CT_NUM_PARA0) + BGIndex];

    aucFourCorner[6] = (int)a_uc2DTable1[((RGIndex + AWB_LS_SHIFT_TABLE_CT_NUM_PARA0 * BVIndex_High) * AWB_LS_SHIFT_TABLE_CT_NUM_PARA0) + BGIndex + 1];

    aucFourCorner[7] = (int)a_uc2DTable1[((RGIndex + 1 + AWB_LS_SHIFT_TABLE_CT_NUM_PARA0 * BVIndex_High) * AWB_LS_SHIFT_TABLE_CT_NUM_PARA0) + BGIndex + 1];

    fIntValue1_bg1 = (float)(aucFourCorner[4] * (wYRange_BG - wYRem_BG) + aucFourCorner[5] * (wYRem_BG)) / (float)wYRange_BG;
    fIntValue1_bg2 = (float)(aucFourCorner[6] * (wYRange_BG - wYRem_BG) + aucFourCorner[7] * (wYRem_BG)) / (float)wYRange_BG;
    fIntValue1_rg2 = (float)(fIntValue1_bg1 * (wYRange_RG - wYRem_RG) + fIntValue1_bg2 * (wYRem_RG)) / (float)wYRange_RG;

    *a_pfPrun1 = (short)((fIntValue1_rg1 * (wXRange - wXRem) + fIntValue1_rg2 * wXRem) / (float)wXRange);
}

//=================================================================================================
/**
*@brief                      2-D Interpolation
*@param[Out] *a_pfPrun1     The result of interpolation
*@param[In]   *a_uc2DTable1  2-D shift table pointer
*@param[In]   *a_wXNodesPos  The width node value in the shift table
*@param[In]   *a_wYNodesPos  The height node value in the shift table
*@param[In]   a_wTableW      shift table width( The number of RG nodes )
*@param[In]   a_wTableH      shift table Height( The number of BG nodes )
*@param[In]   a_wProjX       white point rg
*@param[In]   a_wProjY       white point bg
*@return                     none
*/
void AWB_Stylish_Interpolation2D(
    short* a_pfPrun1,
    short* a_uc2DTable1,      //128-based 2D Table, size = (a_wTableW-2)x(a_wTableH-2)
    short* a_wXNodesPos,      //length = a_wTableW
    short* a_wYNodesPos,      //length = a_wTableH
    short       a_wTableW,
    short       a_wTableH,
    short       a_wProjX,
    short       a_wProjY)
{
    short   i;
    short   wXIndex;
    short   wYIndex;
    int   aucFourCorner[4] = { 0,0,0,0 }; //For table1 
    int   wXRem;
    int   wXRange;
    int   wYRem;
    int   wYRange;
    int   fIntValue1_L;
    int   fIntValue1_H;

    *a_pfPrun1 = 0;
    if (a_wProjX < a_wXNodesPos[0])
        a_wProjX = a_wXNodesPos[0];

    if (a_wProjX >= a_wXNodesPos[a_wTableW - 1])
        a_wProjX = a_wXNodesPos[a_wTableW - 1];

    if (a_wProjY < a_wYNodesPos[0])
        a_wProjY = a_wYNodesPos[0];

    if (a_wProjY >= a_wYNodesPos[a_wTableH - 1])
        a_wProjY = a_wYNodesPos[a_wTableH - 1];

    wXIndex = 0;
    for (i = 1; i < a_wTableW; i++)
    {
        if (a_wXNodesPos[i] >= a_wProjX)
        {
            wXIndex = i - 1;
            break;
        }
    }
    wXRem = (int)(a_wProjX - a_wXNodesPos[wXIndex]);
    wXRange = (int)(a_wXNodesPos[wXIndex + 1] - a_wXNodesPos[wXIndex]);

    wYIndex = 0;
    for (i = 1; i < a_wTableH; i++)
    {
        if (a_wYNodesPos[i] >= a_wProjY)
        {
            wYIndex = i - 1;
            break;
        }
    }
    wYRem = (int)(a_wProjY - a_wYNodesPos[wYIndex]);
    wYRange = (int)(a_wYNodesPos[wYIndex + 1] - a_wYNodesPos[wYIndex]);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //Table boundary check. The first line of table top, bottom, left, and right are default value (1.0).
    if ((wYIndex) >= 0 && (wYIndex) < a_wTableH && (wXIndex) >= 0 && (wXIndex) < a_wTableW)
        aucFourCorner[0] = (int)a_uc2DTable1[(wYIndex)+(a_wTableH) * (wXIndex)];

    if ((wYIndex) >= 0 && (wYIndex) < a_wTableH && (wXIndex + 1) >= 0 && (wXIndex + 1) < a_wTableW)
        aucFourCorner[1] = (int)a_uc2DTable1[(wYIndex)+(a_wTableH) * (wXIndex + 1)];

    if ((wYIndex + 1) >= 0 && (wYIndex) < a_wTableH && (wXIndex) >= 0 && (wXIndex) < a_wTableW)
        aucFourCorner[2] = (int)a_uc2DTable1[(wYIndex + 1) + (a_wTableH) * (wXIndex)];

    if ((wYIndex + 1) >= 0 && (wYIndex) < a_wTableH && (wXIndex + 1) >= 0 && (wXIndex + 1) < a_wTableW)
        aucFourCorner[3] = (int)a_uc2DTable1[(wYIndex + 1) + (a_wTableH) * (wXIndex + 1)];


    fIntValue1_L = (aucFourCorner[0] * (wXRange - wXRem) + aucFourCorner[1] * (wXRem)) / wXRange;
    fIntValue1_H = (aucFourCorner[2] * (wXRange - wXRem) + aucFourCorner[3] * (wXRem)) / wXRange;

    *a_pfPrun1 = (short)((fIntValue1_L * (wYRange - wYRem) + fIntValue1_H * wYRem) / wYRange);

}
//=================================================================================================
void    AWB_Stylish_Shift(
    unsigned short* R_Gain,
    unsigned short* B_Gain,
    short              comp_RG,
    short              comp_BG)
{
    short R = 0;
    short B = 0;

    R = (short)(*R_Gain);
    B = (short)(*B_Gain);

    R += comp_RG;
    B += comp_BG;

    if (R < 0)
    {
        *R_Gain = 0;
    }
    else
    {
        *R_Gain = (unsigned short)R;
    }
    if (B < 0)
    {
        *B_Gain = 0;
    }
    else
    {
        *B_Gain = (unsigned short)B;
    }

}
//=================================================================================================
/**
*@brief              AWB Stylish Tuning
*@param[In]   BV     BV
*@param[In]   *RG    RG
*@param[In]   *BG    BG
*@param[In]   *para  shift table
*@param[Out]  *RG    RG after shift table
*@param[Out]  *BG    BG after shift table
*@return             none
*/
void    AWB_Stylish_Gain(
    short               BV,
    short				CT,
    unsigned short* R_Gain,
    unsigned short* B_Gain,
    AWB_STYLISH_PARA* para,
    AWB_STYLISH_REPORT* data)
{
    short       num_Bv, num_RG_level;
    short* tbl_Bv, * tbl_RG_level;
    short* RG_Table, * BG_Table;
    short       rg_target = 0, bg_target = 0;

    if (0 == para->Num_Bv || 0 == para->Num_CT)
        return;

    num_Bv = para->Num_Bv;
    num_RG_level = para->Num_CT;

    tbl_Bv = para->Tbl_Bv;
    tbl_RG_level = para->Tbl_CT;

    RG_Table = para->tAwbTab_RG;
    BG_Table = para->tAwbTab_BG;

    AWB_Stylish_Interpolation2D_New(BV, *R_Gain, *B_Gain, &rg_target, RG_Table, tbl_Bv, tbl_RG_level, num_Bv, num_RG_level);
    AWB_Stylish_Interpolation2D_New(BV, *R_Gain, *B_Gain, &bg_target, BG_Table, tbl_Bv, tbl_RG_level, num_Bv, num_RG_level);
    //AWB_Stylish_Interpolation2D(&rg_com2, para_RG, tbl_Bv, tbl_CT, num_Bv, num_CT, BV, CT);
    //AWB_Stylish_Interpolation2D(&bg_com2, para_BG, tbl_Bv, tbl_CT, num_Bv, num_CT, BV, CT);
    AWB_Stylish_Shift(R_Gain, B_Gain, rg_target, bg_target);
    //LOG_DEBUG("2Dlut stylish target rg %u bg %u  bv %d", *R_Gain, *B_Gain, BV);
    if (0 != data)
    {
        data->R_Gain = *R_Gain;
        data->B_Gain = *B_Gain;
        data->comp_RG = rg_target;
        data->comp_BG = bg_target;
        data->enable_stylish = 1;
    }
}


