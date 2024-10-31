#pragma once
#ifndef DEFINATION_H
#define DEFINATION_H
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef Q
#define Q 15
#endif



typedef struct isp_statistic {
//输入参数
	unsigned short RegImageWidth;//12bit
	unsigned short RegImageHeight;//12bit
	unsigned short RegReal_size;//9bit
	unsigned short RegOverlap_pad;//<0.5*overlap_size-7bit
//计算参数
	unsigned char RegNumber_row;//4-8bit
	unsigned short RegRow_startloc[28]={0};//11bit
} isp_statistic_t;

class CtModel {
private:
	
public:
	bool unit_sequence(unsigned short int *rawbuf, unsigned short int *out_isp, isp_statistic_t *isp_message, bool if_end, bool if_start);
	bool handle_img(unsigned short int* input,unsigned short int* output,isp_statistic_t isp_message);
	bool get_row(unsigned short int* input,unsigned short int* output, unsigned int start, unsigned int stop, unsigned short ImageWidth);
};


#endif
