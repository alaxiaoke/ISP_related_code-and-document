//
// Created by DELL on 2022/9/6.
//
#include "postprocess.h"
#include <stdio.h>
#include <stdlib.h>
int PostProcess::Q108_avg(
	unsigned long val,
	short int w
) {
	val=val*w +(1<<7);
	val = val >> 8;
	return val;
}
bool PostProcess::unit_postprocess(
	unsigned short int *input,
	unsigned short int *output,
	post_statistic_t *post_message,
	bool if_start,
	bool if_end) {
	long Each_block_len = (post_message->RegReal_size + 2 * post_message->RegOverlap_pad) * post_message->RegImageWidth / 2;
	unsigned short stop_row = if_end ? post_message->RegNumber_row_remainder: post_message->RegReal_size + 2 * post_message->RegOverlap_pad;
	long i_left_1, i_left_2, i_right, j_left;
	for (int i = 0; i < stop_row; i++) {
		
		i_left_1 = (i << 1) * post_message->RegImageWidth;
		i_left_2 = i_left_1 + post_message->RegImageWidth;
		i_right = i * post_message->RegImageWidth / 2;
		if (i< post_message->RegOverlap_pad << 1 && !if_start){
			for (int j = 0; j < post_message->RegImageWidth / 2; j++) {
				j_left = j << 1;
				output[i_left_1 + j_left] = this->Q108_avg(input[i_right + j], post_message->Reg_weight[i] * 16/ post_message->RegOverlap_pad );
				output[i_left_1 + j_left + 1] = this->Q108_avg(input[i_right + Each_block_len + j], post_message->Reg_weight[i] * 16 / post_message->RegOverlap_pad );
				output[i_left_2 + j_left] = this->Q108_avg(input[i_right + Each_block_len * 2 + j], post_message->Reg_weight[i] *16/ post_message->RegOverlap_pad );
				output[i_left_2 + j_left + 1] = this->Q108_avg(input[i_right + Each_block_len * 3 + j], post_message->Reg_weight[i] *16 / post_message->RegOverlap_pad );
			}
		}
		else {
			if (i >= post_message->RegReal_size && !if_end) {
				for (int j = 0; j < post_message->RegImageWidth / 2; j++) {
					j_left = j << 1;
					output[i_left_1 + j_left] = this->Q108_avg(input[i_right + j], post_message->Reg_weight[stop_row - i-1]* 16 / post_message->RegOverlap_pad  + 128 / post_message->RegOverlap_pad );
					output[i_left_1 + j_left + 1] = this->Q108_avg(input[i_right + Each_block_len + j], post_message->Reg_weight[stop_row - i - 1] * 16 / post_message->RegOverlap_pad  + 128 / post_message->RegOverlap_pad );
					output[i_left_2 + j_left] = this->Q108_avg(input[i_right + Each_block_len * 2 + j], post_message->Reg_weight[stop_row - i - 1] * 16 / post_message->RegOverlap_pad  + 128 / post_message->RegOverlap_pad );
					output[i_left_2 + j_left + 1] = this->Q108_avg(input[i_right + Each_block_len * 3 + j], post_message->Reg_weight[stop_row - i - 1] * 16 / post_message->RegOverlap_pad  + 128 / post_message->RegOverlap_pad );
				}
			}
			else {
				for (int j = 0; j < post_message->RegImageWidth / 2; j++) {
					j_left = j << 1;
					output[i_left_1 + j_left] = input[i_right + j];
					output[i_left_1 + j_left + 1] = input[i_right + Each_block_len + j];
					output[i_left_2 + j_left] = input[i_right + Each_block_len * 2 + j];
					output[i_left_2 + j_left + 1] = input[i_right + Each_block_len * 3 + j];
				}
			}
		}
	}
	return UNIT_RESULT_OK;
}