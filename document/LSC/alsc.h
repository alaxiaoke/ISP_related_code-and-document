#pragma once
#ifndef _ISP_ALSC_FW_H_
#define _ISP_ALSC_FW_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef clip
#define clip(x, a, b) (min(max(x, a), b))
#endif

struct isp_bayer_alsc_stat_fw {
	int32_t state_w;
	int32_t state_h;
	int32_t state_rbuffer[64 * 48];
	int32_t state_grbuffer[64 * 48];
	int32_t state_gbbuffer[64 * 48];
	int32_t state_bbuffer[64 * 48];
	int32_t state_wbuffer[64 * 48];
};

struct isp_bayer_alsc_storecrb_fw {
	int32_t tab_cr[64 * 48];              //4.12
	int32_t tab_cb[64 * 48];              //4.12
};

struct isp_bayer_alsc_lsctabawb_fw {
	int32_t image_w;
	int32_t image_h;

	int32_t awbgain_r;                 //3.12
	int32_t awbgain_b;                 //3.12
	int32_t corlort;                   //
	int32_t white_indx[64 * 48];       // not use now

	//int32_t grad_th_r;                 // .12
	//int32_t grad_th_b;                 // .12
	//int32_t maxgain_r;                 // not use  need fix
	//int32_t maxgain_b;                 // not use  need fix
	//int32_t center_x;                  // not use now   FIX zero
	//int32_t center_y;                  // not use now	FIX zero
};

struct isp_bayer_alsc_caltab_fw {
	unsigned short block_w;
	unsigned short block_h;
	unsigned short colort_list[7];
	float tab1_r[64 * 48];             //
	float tab1_gr[64 * 48];
	float tab1_gb[64 * 48];
	float tab1_b[64 * 48];
	float tab2_r[64 * 48];
	float tab2_gr[64 * 48];
	float tab2_gb[64 * 48];
	float tab2_b[64 * 48];
	float tab3_r[64 * 48];
	float tab3_gr[64 * 48];
	float tab3_gb[64 * 48];
	float tab3_b[64 * 48];
	float tab4_r[64 * 48];
	float tab4_gr[64 * 48];
	float tab4_gb[64 * 48];
	float tab4_b[64 * 48];
	float tab5_r[64 * 48];
	float tab5_gr[64 * 48];
	float tab5_gb[64 * 48];
	float tab5_b[64 * 48];
	float tab6_r[64 * 48];
	float tab6_gr[64 * 48];
	float tab6_gb[64 * 48];
	float tab6_b[64 * 48];
	float tab7_r[64 * 48];
	float tab7_gr[64 * 48];
	float tab7_gb[64 * 48];
	float tab7_b[64 * 48];
};

struct isp_bayer_alsc_tune_fw {
	int32_t enable;
	int32_t alsc_enable;
	int32_t r_enable;
	int32_t b_enable;
	int32_t alsc_mode;                 //0, 1, 2, 3
	int32_t statr_inv;                 //  inv statr
	int32_t statb_inv;                 //  inv starb
	
	int32_t manul_mode;
	int32_t mgrad_th_r;                //.12 or .8
	int32_t mgrad_th_b;
	int32_t mmaxgain_r;
	int32_t mmaxgain_b;
	int32_t mcenter_x;                  // not use now   FIX zero
	int32_t mcenter_y;                  // not use now	FIX zero

	int32_t indx_ratio;                //1.8
	int32_t th_pixnum;
	int32_t th_saunum;
	int32_t th_darnum;

	int32_t white_mode;	
	int32_t pad_mode;
	int32_t pad_w;
	int32_t pad_h;
	int32_t work_mode;

	int32_t div_th;
	int32_t multi_mode;            // not use now
	int32_t overlap_w;             // not use now
	int32_t overlap_h;             // not use now
};

struct isp_bayer_alsc_out_fw {
	int32_t block_w;
	int32_t block_h;
	int32_t lsctab_r[64 * 48];             //4.12
	int32_t lsctab_gr[64 * 48];            //4.12
	int32_t lsctab_gb[64 * 48];            //4.12
	int32_t lsctab_b[64 * 48];             //4.12
};

struct isp_bayer_alsc_inter_fw {
	int32_t hue_r[64 * 48];               //.12
	int32_t hue_b[64 * 48];               //.12
	int32_t index[64 * 48];
	int32_t tab_r[64 * 48];               //4.12
	int32_t tab_g[64 * 48];               //4.12
	int32_t tab_b[64 * 48];               //4.12

	int32_t tab_cr[64 * 48];              //4.12
	int32_t tab_cb[64 * 48];              //4.12
	int32_t tab_sinx[64 * 48];
	int32_t tab_siny[64 * 48];
	int32_t tab_cosx[64 * 48];
	int32_t tab_cosy[64 * 48];

	int32_t grad_thr;
	int32_t grad_thb;
	int32_t block_w;
	int32_t block_h;

	//for debug
	int32_t debug_validx[64 * 48];
	int32_t debug_validy[64 * 48];
	int32_t debug_gradx[64 * 48];
	int32_t debug_grady[64 * 48];
};

int isp_bayer_alsc_stat_inv(struct isp_bayer_alsc_stat_fw* alsc_state, struct isp_bayer_alsc_storecrb_fw* alsc_tabcrb, struct isp_bayer_alsc_tune_fw* alsc_tune);
int isp_bayer_alsc_init(struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, struct isp_bayer_alsc_tune_fw* alsc_tune, struct isp_bayer_alsc_inter_fw* alsc_inter, struct isp_bayer_alsc_out_fw* alsc_out, struct isp_bayer_alsc_caltab_fw* alsc_calibtab);
int isp_bayer_alsc_dlsc(struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, struct isp_bayer_alsc_caltab_fw* alsc_calibtab, struct isp_bayer_alsc_inter_fw* alsc_inter, struct isp_bayer_alsc_out_fw* alsc_out);
int isp_bayer_alsc_calc_hue(struct isp_bayer_alsc_stat_fw* alsc_state, struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, struct isp_bayer_alsc_inter_fw* alsc_inter);
int isp_bayer_alsc_calc_ind(struct isp_bayer_alsc_stat_fw* alsc_state, int imagew, int imageh, struct isp_bayer_alsc_tune_fw* alsc_tune, struct isp_bayer_alsc_inter_fw* alsc_inter);

int isp_bayer_alsc_calc_tab(struct isp_bayer_alsc_stat_fw* alsc_state,
	struct isp_bayer_alsc_inter_fw* alsc_inter,
	struct isp_bayer_alsc_lsctabawb_fw* alsc_calib,
	struct isp_bayer_alsc_tune_fw* alsc_tune,
	int32_t gradr_th, int32_t* huerb, int32_t* tab_crb);

int isp_bayer_alsc_calc_wbpad(struct isp_bayer_alsc_stat_fw* alsc_state, struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, struct isp_bayer_alsc_tune_fw* alsc_tune, int* tab_hue, int* tab_crb);
int isp_bayer_alsc_updatestore(int state_w, int state_h, int32_t* tabcrb, int32_t* storecrb);
int isp_bayer_alsc_update(struct isp_bayer_alsc_stat_fw* alsc_state, struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, struct isp_bayer_alsc_inter_fw* alsc_inter, int* tabg, int* tabcrb, int* tabcrb_out);
int isp_bayer_alsc_process(struct isp_bayer_alsc_stat_fw* alsc_state, struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, struct isp_bayer_alsc_tune_fw* alsc_tune, struct isp_bayer_alsc_caltab_fw* alsc_calibtab, struct isp_bayer_alsc_storecrb_fw* alsc_storecrb,float* lsctabtoisp);

bool SaveAlscTab(const char* savefilename, int32_t* lsctabtoisp, int Height, int Width, int mode_int);
#endif







