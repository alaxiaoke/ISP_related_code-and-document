#pragma once
//
// Created by DELL on 2022/8/31.
//

#ifndef CMODEL_PREPROCESS_H
#define CMODEL_PREPROCESS_H

//Ô¤´¦Àí¼Ä´æÆ÷
typedef struct pre_statistic {
	unsigned short RegImageWidth;//12bit
	unsigned short RegReal_size;//9bit
	unsigned short RegOverlap_pad;//7bit
	unsigned short RegNumber_row_remainder;
} pre_statistic_t;


class PreProcess {
private:

public:
	bool unit_preprocess(
		unsigned short int* input,
		unsigned short int *output,
		pre_statistic_t *pre_message,
		bool if_end
	);
	PreProcess() {

	}

	~PreProcess() {

	}

};

#endif //CMODEL_PREPROCESS_H

