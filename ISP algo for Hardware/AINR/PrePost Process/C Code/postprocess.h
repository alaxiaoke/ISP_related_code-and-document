#pragma once
//
// Created by DELL on 2022/9/6.
//

#ifndef CMODEL_POSTPROCESS_H
#define CMODEL_POSTPROCESS_H

typedef enum unit_result {
	UNIT_RESULT_OK = 0,
	UNIT_RESULT_FAIL,
} unit_result_t;

typedef struct post_statistic {
	unsigned short RegImageWidth;//12bit
	unsigned short RegReal_size;//9bit
	unsigned short RegOverlap_pad;//<0.5*overlap_size-7bit
	unsigned short RegNumber_row_remainder;
	unsigned short Reg_weight[32] = { 0,8,16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136,144,152,160,168,176,184,192,200,208,216,224,232,240,248};
} post_statistic_t;

class PostProcess {
private:
	long val_18;
public:
	int Q108_avg(
		unsigned long val,
		short int w
		);
	bool unit_postprocess(
		unsigned short int *input,
		unsigned short int *output,
		post_statistic_t *post_message, 
		bool if_start,
		bool if_end
	);

	PostProcess() {

	}

	~PostProcess() {

	}
};




#endif //CMODEL_POSTPROCESS_H
