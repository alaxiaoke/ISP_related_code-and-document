//
// Created by DELL on 2022/8/31.
//
#include "preprocess.h"
#include <stdio.h>

bool PreProcess::unit_preprocess(
	unsigned short int *input,
	unsigned short int *output,
	pre_statistic_t *pre_message,
	bool if_end) {
	long Each_block_len = (pre_message->RegReal_size+2* pre_message->RegOverlap_pad)* pre_message->RegImageWidth/2;
	unsigned short stop_row = if_end? pre_message->RegNumber_row_remainder : pre_message->RegReal_size + 2 * pre_message->RegOverlap_pad;
	long i_right_1, i_right_2, i_left, j_right;
	for (int i =0; i < pre_message->RegReal_size + 2 * pre_message->RegOverlap_pad; i++) {
		i_right_1 = (i << 1) * pre_message->RegImageWidth;
		i_right_2 = i_right_1 + pre_message->RegImageWidth;
		i_left = i * pre_message->RegImageWidth / 2;
		if (i < stop_row) {
			for (int j = 0; j < pre_message->RegImageWidth / 2; j++) {
				j_right = j << 1;
				output[i_left + j] = input[i_right_1 + j_right];
				output[i_left + Each_block_len + j] = input[i_right_1 + j_right + 1];
				output[i_left + Each_block_len * 2 + j] = input[i_right_2 + j_right];
				output[i_left + Each_block_len * 3 + j] = input[i_right_2 + j_right + 1];
			}
		}
		else {
			for (int j = 0; j < pre_message->RegImageWidth / 2; j++) {
				j_right = j << 1;
				output[i_left + j] =0;
				output[i_left + Each_block_len + j] = 0;
				output[i_left + Each_block_len * 2 + j] = 0;
				output[i_left + Each_block_len * 3 + j] = 0;
			}
		}
	}
	return true;
}
