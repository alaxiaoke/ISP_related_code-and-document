
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <string>


#include "../Include/allib_ae_errcode.h"
#include "../Include/allib_ae.h"
//#include "../aeHandle.h"


#define _SIMULATE_LSC_RUN                     (20)
#define _SIMULATE_AFD_RUN                     (50)
#define SCANNER_HEIGHT                        (1088)
#define STATS_DATA_SIZE                       (1072)

typedef struct gxr_handle{

    void*                               ae_stats;
    void*                               yhis_stats;
    struct ae_set_param_t               ae_set_para;
    struct ae_get_param_t               ae_get_para;
    struct ae_output_data_t             ae_output;

}gxr_aehd;

al4ahd          al4a;
gxr_aehd        gxr_ae;
unsigned int    g_udsofidx = 0;
unsigned char   g_ucIsICCAbort = FALSE;
void* ae_stats = NULL;
void* yhis_stats = NULL;
unsigned int    adgain_current_valid = 0;
unsigned int    exposure_time_current_valid = 0;
unsigned char   ae_stats_buffer[8320];   // 16 * 16 * 32 + 128
unsigned int    yhis_stats_buffer[1152];  //256*4+128



//int statsProc(unsigned int* p, int* pStats, int rows, int cols, int imgH, int imgW) {
//    if (p == NULL) {
//        printf( "source file open is null");
//        return -1;
//    }
//    memset(pStats, 0, sizeof(int) * rows * cols);
//
//    int blockW = imgW / cols;
//    int blockH = imgH / rows;
//    int clip_w = (imgW - (cols * blockW)) / 2;
//    int clip_h = (imgH - (rows * blockH)) / 2;
//    clip_w += (clip_w % 2);
//    clip_h += (clip_h % 2);
//    if (clip_w < 0) {
//        prinff("block width size set error , out of range");
//        return -1;
//    }
//    if (clip_h < 0) {
//        printf("sblock height size set error , out of range");
//        return -1;
//    }
//    unsigned int satValue = 1024 * 0.97;   //可调参数
//    for (int y = clip_h; y + clip_h - (clip_h % 2) < imgH; y++) {
//        int r = (y - clip_h) / blockH;
//        unsigned int index_add = y * imgW;
//        for (int x = clip_w; x + clip_w - (clip_w % 2) < imgW; x++) {
//            int c = (x - clip_w) / blockW;
//            unsigned int blockNum = r * cols + c;
//            unsigned int pixel_index = index_add + x;
//            unsigned int value = p[pixel_index] < 64 ? 0 : p[pixel_index] - 64;  // blc ?
//            value *= (1023.0 / 959);
//            //cout << value << "	;";
//            if (value <= satValue) {
//                pStats[(y % 2) * 2 + x % 2 + blockNum * 8] += value;   //r/gr/gb/b  max is 2*20  
//                pStats[(y % 2) * 2 + x % 2 + blockNum * 8 + 4] += 1;
//            }
//        }
//    }
//    return 0;
//}
//
//int histProc(unsigned int* ptr, int* pHist, int rows, int cols, int clip_h, int clip_w)
//{
//    float RGB2Y[3] = { 0.299, 0.587, 0.114 };
//    if (pHist == NULL) {
//        printf("Hist ptr is null");
//        return -1;
//    }
//    memset(pHist, 0, sizeof(int) * 256);
//    //创建三维数组
//    int*** rggb;  // R:0, Gr:1, Gb:2, B:3
//    int i, j, k;
//    // 自己分配一个三维的内存
//    rggb = (int***)malloc(cols / 2 * sizeof(int**));    //第一维
//    memset(rggb, 0, cols / 2 * sizeof(int**));
//    //分配二维的
//    for (i = 0; i < cols / 2; i++)
//    {
//        *(rggb + i) = (int**)malloc(rows / 2 * sizeof(int*));
//        memset(*(rggb + i), 0, rows / 2 * sizeof(int*));
//        //分配三维
//        for (j = 0; j < rows / 2; j++)
//        {
//            *(*(rggb + i) + j) = (int*)malloc(4 * sizeof(int));
//            memset(*(*(rggb + i) + j), 0, 4 * sizeof(int));
//        }
//    }
//
//    // get rggb value
//    for (int y = clip_h; y + clip_h < rows; y++)
//    {
//        for (int x = clip_w; x + clip_w < cols; x++)
//        {
//            unsigned int value = ptr[y * cols + x] < 64 ? 0 : ptr[y * cols + x] - 64;
//            value *= (1023.0 / 959);
//            rggb[x / 2][y / 2][(y % 2) * 2 + x % 2] = value;
//        }
//    }
//    //get piexl level luma --> resize()-->1/4
//    //pHist[0] -= ((clip_h * cols + clip_w * rows - clip_w * clip_h) * 2 >> 2);  // or x、y起始roi修改
//    //减去clip掉的像素,rggb初始值  
//    for (int y = clip_h / 2; y + clip_h / 2 < rows / 2; y++)
//    {
//        for (int x = clip_w / 2; x + clip_w / 2 < cols / 2; x++)
//        {
//            int avgY = (int)(rggb[x][y][0] * RGB2Y[0] + (rggb[x][y][1] + (rggb[x][y][2] - rggb[x][y][1]) / 2) * RGB2Y[1] + rggb[x][y][3] * RGB2Y[2]) >> 2;
//            pHist[avgY] += 1;
//        }
//    }
//    //pHist[0] -= clip_black;
//    //释放内存  3->2->1  与创建相反
//    for (i = 0; i < cols / 2; i++)
//    {
//        for (j = 0; j < rows / 2; j++)
//        {
//            free((rggb[i][j]));
//        }
//        free(rggb[i]);
//    }
//    free(rggb);
//    return 0;
//}
//
//void ReadRawFromFile(const char* filePath, unsigned int* bTemp)
//{
//    FILE* fp;
//    fp = fopen(filePath, "rb");
//    if (fp == NULL)
//    {
//        printf( "打开文件失败");
//        return;
//    }
//    fpos_t startpos, endpos;
//    fseek(fp, 0, SEEK_END);
//    fgetpos(fp, &endpos);
//    fseek(fp, 0, SEEK_SET);
//    fgetpos(fp, &startpos);
//    long file1en = (long)(endpos - startpos);
//    char data[4096*2304*2];
//    if (bTemp == NULL)
//    {
//        fclose(fp);
//        return;
//    }
//    //memset(bTemp, 0, file1en);
//    fread(data, sizeof(char), file1en, fp);
//    fclose(fp);
//
//    // 遍历每个数据项，并调用readInt10函数将其解析为10位数据  
//    for (int i = 0; i < file1en; i += 2) {
//        unsigned int value = unpack(&data[i]);
//        *(bTemp + i / 2) = value;
//        //std::cout << "Value at offset " << i << ": " << value << std::endl;
//    }
//
//    free(data);
//    data[0] = NULL;
//    return;
//}


int main(void)
{
    unsigned int err = 0;


    /*  ===========================================
    *     Initial flow
    *   =========================================== */

    // set calibration data for AE initial
    // min ISO gain, WB gain r/g/b, afd mode
    //al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.min_iso = al4a.lsc_get_para.para.default_ISO.min_iso;
    //al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_b_gain = al4a.lsc_get_para.para.default_ISO.gain_b;
    //al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain = al4a.lsc_get_para.para.default_ISO.gain_g;
    //al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_r_gain = al4a.lsc_get_para.para.default_ISO.gain_r;
    //al4a.ae_set_para.set_param.ae_initial_setting.afd_flicker_mode = (enum ae_antiflicker_mode_t)al4a.afd_get_para.para.afd_setting.mode;

    al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.min_iso = 100;   // scale 100
    al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_b_gain = 1000;   // scale 1000
    al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_g_gain = 1000;
    al4a.ae_set_para.set_param.ae_initial_setting.ae_calib_wb_gain.calib_r_gain = 1000;
    al4a.ae_set_para.set_param.ae_initial_setting.afd_flicker_mode = (enum ae_antiflicker_mode_t)0;
    

    //构造input data   block   16*16 algo
    for (int i = 0; i < 256; i++)
    {
        ae_stats_buffer[i * 8] = 127;
        ae_stats_buffer[i * 8 + 4] = 127;
        ae_stats_buffer[i * 8 + 8] = 127;
        ae_stats_buffer[i * 8 + 12] = 127;
        ae_stats_buffer[i * 8 + 16] = 100;
        ae_stats_buffer[i * 8 + 20] = 1024;
        ae_stats_buffer[i * 8 + 24] = 1024;
        ae_stats_buffer[i * 8 + 28] = 1024;
        yhis_stats_buffer[i] = 100;
    }

    //char file[512];
    //char* file_name;
    //char* savePath;
    //char* pathName;
    //char* fileName;
    ////// get statistic from isp
    ////STATS
    //int rows = 16;
    //int cols = 16;
    //int imgH = 1744;
    //int imgW = 2328;
    //int blknum = rows * cols;
    //int* pStats = (int*)malloc(sizeof(int) * blknum * 8);
    //memset(pStats, 0, sizeof(int) * blknum * 8);
    ////HIST
    //int* pHist = (int*)malloc(sizeof(int) * 256);
    //memset(pHist, 0, sizeof(int) * 256);
    ////内接矩形计算直方图 or clip black-->FOV Histogram
    //int clip_h = 0;
    //int clip_w = 0;
    ////int clip_black = 120186;

    ////// read raw for sim
    //int pixels = imgH * imgW;
    //unsigned int* p = (unsigned int*)malloc(sizeof(unsigned int) * pixels);
    //if (p == NULL) {
    //    printf( "p 指针内存分配失败");
    //    return 1;
    //}

    //fileName = "D:\\work\\AE\\data\\4l\\convergence\\7ms_8x_globalMotion\\L\\CAM1_[377]_[2328_1744]_[152114_0627]_4199306.raw";
    ////strcpy(file, fileName.c_str());
    ////file_name = file;
    //memset(p, 0, pixels);
    //ReadRawFromFile(file_name, p);

    //if (statsProc(p, pStats, rows, cols, imgH, imgW) == 0)
    //{
    //    ae_stats_buffer[0] = (char*)pStats;
    //}

    ////内接矩形计算直方图-->fov Histogram
    //if (histProc(p, pHist, imgH, imgW, clip_h, clip_w) == 0)
    //{
    //    //int h = 300;
    //    //int w = 256;
    //    //showHist(h, w, pHist);
    //    yhis_stats_buffer[0] = (char*)pHist;
    //}

    err = ae_initial(&al4a);
    //printf("[AE initial] iso %d, ExpT %ld(us), Gain %d, bv_val %ld\n", al4a.ae_get_para.para.ae_get_init_expo_param.iso, \
    //    al4a.ae_get_para.para.ae_get_init_expo_param.exp_time, al4a.ae_get_para.para.ae_get_init_expo_param.ad_gain, \
    //    al4a.ae_get_para.para.ae_get_init_expo_param.bv_val);


    /* the following is a example to describe the timing of ISP initial after 4A*/
    // ISPQ_INPUT *input_initial;
    //input_initial->uwGain_Sensor_RGB = ae_get_para.para.ae_get_init_expo_param.ad_gain;
    //input_initial->uwDGain_IR = 256;
    //input_initial->uwWB_Gain_R = awb_get_parapara.awb_init_data.initial_wbgain.r_gain;
    //input_initial->uwWB_Gain_GR = awb_get_parapara.awb_init_data.initial_wbgain.g_gain;
    //input_initial->uwWB_Gain_GB = awb_get_parapara.awb_init_data.initial_wbgain.g_gain;
    //input_initial->uwWB_Gain_B = awb_get_parapara.awb_init_data.initial_wbgain.b_gain;
    //input_initial->uwWB_CT = awb_get_parapara.awb_init_data.color_temperature;
    //int err;
    //err = ISPQ_Scenario_Case_2_1_Initial(qmerge_obj, &input_initial, &ispq_output);

    /*  ===========================================
    *       process flow
    *       sample iteranion run (g_udsofidx)
    *   =========================================== */
    do
    {
        unsigned int err = 0;

        /* TO DO:
        * Wait SOF event and get the effective exposure time and adgain of the current statistics frame from sensor module
        */

        err = ae_run(&al4a, g_udsofidx);
        if (err != _AL_3ALIB_SUCCESS) {
            printf("AE run error: %#x\r\n", err);
            return err;
        }

        printf("[alAELib] Process[%d]: Ymean %d, Lux %ld ExpT %ld(us), Gain %ld, converge %d\n", \
            g_udsofidx, al4a.ae_output.ymean100_8bit, al4a.ae_output.lux_result, al4a.ae_output.udexposure_time, \
            al4a.ae_output.udsensor_ad_gain, al4a.ae_output.ae_converged);


        /*  ===========================================
        *     ISP update sample
        *     the following is an example to describe the timing of update ISP after 4A
        *   ===========================================
        */ //the update parameter from RGB AE / RGB AWB library
        //input_initial.uwGain_Sensor_RGB = ae_output.udsensor_ad_gain;
        //input_initial.uwGain_Midtones_RGB = ae_output->midtones_gain;
        //input_initial.uwGain_Shadows_RGB = ae_output->shadows_gain;
        //input_initial.uwWB_Gain_R = awb_output->wbgain.r_gain;
        //input_update.uwWB_Gain_GR = awb_output->wbgain.g_gain;
        //input_initial.uwWB_Gain_GB = awb_output->wbgain.g_gain;
        //input_initial.uwWB_Gain_B = awb_output->wbgain.b_gain;
        //input_initial.uwWB_CT = awb_output->color_temp;
        //ISPQ_Scenario_Case_2_1_Update(&input_initial, &ispq_output);


        // So far, we finished 4A process once
        // from the example we simply run the estimation iteration by SOF index 
        // in real case, please make sure to send the SOF notify with new valid and effective exp-time/adgain 
        g_udsofidx++;

    } while (g_udsofidx <= 2);

    /*  ===========================================
    *     De-init flow
    *   =========================================== */

    err = al4a.ae_func_obj.deinit(al4a.ae_obj);
    if (err != _AL_3ALIB_SUCCESS) {
        printf("ae deinit error\r\n");
        return err;
    }

    //system("PAUSE");
    return 0;

}

unsigned int ae_run(al4ahd* pal4a, unsigned int sofidx)
{
    unsigned int ae_err = _AL_3ALIB_SUCCESS;

    void** ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj* ae_func_obj = &(pal4a->ae_func_obj);
    struct ae_set_param_t* ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t* ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t* ae_output = &(pal4a->ae_output);
    struct alhw3a_ae_proc_data_t* ae_proc_data = &(pal4a->ae_proc_data);
    enum ae_antiflicker_mode_t afdmode;

    if (sofidx < 2) {
        afdmode = 0;
    }
    else {
        afdmode = 1;
    }

    /* set effective exposure time and effective adgain which getting from sensor
    *  for the sample example,  we assume effective exposure and effective gain
    *  are the same as AE output report from the previous estimation
    */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_SOF_NOTIFY;
    if (sofidx == 0)
    {
        //ae_set_para->set_param.sof_notify_param.exp_adgain = ae_get_para->para.ae_get_init_expo_param.ad_gain;
        //ae_set_para->set_param.sof_notify_param.exp_time = ae_get_para->para.ae_get_init_expo_param.exp_time;
        //ae_set_para->set_param.sof_notify_param.midtones_gain = ae_get_para->para.ae_get_init_expo_param.midtones_gain;

        ae_set_para->set_param.sof_notify_param.exp_adgain = 800;
        ae_set_para->set_param.sof_notify_param.exp_time = 7000;
        ae_set_para->set_param.sof_notify_param.midtones_gain = 100;
    }
    else
    {
        ae_set_para->set_param.sof_notify_param.exp_adgain = ae_output->udsensor_ad_gain;
        ae_set_para->set_param.sof_notify_param.exp_time = ae_output->udexposure_time;
        ae_set_para->set_param.sof_notify_param.midtones_gain = ae_output->midtones_gain;
    }
    ae_set_para->set_param.sof_notify_param.sys_sof_index = sofidx;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /* set anti-flicker mode from AFD output results */
    ae_set_para->ae_set_param_type = AE_SET_PARAM_ANTIFLICKER_MODE;
    ae_set_para->set_param.afd_flicker_mode = afdmode;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    /* TO DO:
    * Prepare HW stats data and wait frame done event
    */
    ae_proc_data->udsys_sof_idx = sofidx;

    /* Get AE stats from hw */
    // The address of HW stats data, here is pseudo sample
    ae_stats = (void*)ae_stats_buffer;
    // The address of HW Yhis stats data, here is pseudo sample
    yhis_stats = (void*)yhis_stats_buffer;

    ae_err = ae_func_obj->process(ae_stats, yhis_stats, *ae_proc_data, *ae_obj, ae_output);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        printf("3\r\n");
        return ae_err;
    }

    return ae_err;
}

unsigned int load_lib(al4ahd* pal4a)
{
    unsigned int ae_err = _AL_3ALIB_SUCCESS;

    ae_err = allib_ae_loadfunc(&(pal4a->ae_func_obj), 0);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        printf("ERROR %x\r\n", (unsigned int)ae_err);
        return ae_err;
    }

    return 0;
}

unsigned int ae_initial(al4ahd* pal4a)
{
    /*  ===========================================
    *     Get function version.
    *   =========================================== */
    allib_ae_getlib_version(&(al4a.ae_ver));
    printf("AE ver: %d.%d\n", al4a.ae_ver.major_version, al4a.ae_ver.minor_version);

    /*  ===========================================
    *     Load tuning file and psudo stats.
    *   =========================================== */
    //err = load_tuning_bin_and_stats();
    //if (err) {
    //    return err;
    //}

    /*  ===========================================
    *     Load lib.
    *   =========================================== */
    int err = load_lib(&al4a);
    if (err) {
        return err;
    }
    unsigned int ae_err = _AL_3ALIB_SUCCESS;

    void** ae_obj = &(pal4a->ae_obj);
    struct alaeruntimeobj* ae_func_obj = &(pal4a->ae_func_obj);
    struct ae_set_param_t* ae_set_para = &(pal4a->ae_set_para);
    struct ae_get_param_t* ae_get_para = &(pal4a->ae_get_para);
    struct ae_output_data_t* ae_output = &(pal4a->ae_output);

    ae_err = ae_func_obj->initial(ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ae_set_para->ae_set_param_type = AE_SET_PARAM_INIT_SETTING;
    ae_set_para->set_param.ae_initial_setting.ae_enable = 1;
    ae_err = ae_func_obj->set_param(ae_set_para, ae_output, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    ae_get_para->ae_get_param_type = AE_GET_INIT_EXPOSURE_PARAM;
    ae_err = ae_func_obj->get_param(ae_get_para, *ae_obj);
    if (ae_err != _AL_3ALIB_SUCCESS) {
        return ae_err;
    }

    return ae_err;
}
