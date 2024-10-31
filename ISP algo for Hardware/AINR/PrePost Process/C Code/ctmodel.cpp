#include <stdio.h>
#include <stdlib.h>
#include "ctmodel.h"
#include "preprocess.h"
#include "postprocess.h"

FILE *fp = NULL;


bool CtModel::unit_sequence(unsigned short int* rawbuf,
	unsigned short int* post_out,
	isp_statistic_t* isp_message,
	bool if_end, bool if_start) {
	pre_statistic_t pre_message;
	pre_message.RegImageWidth = isp_message->RegImageWidth;
	pre_message.RegReal_size = isp_message->RegReal_size;
	pre_message.RegOverlap_pad = isp_message->RegOverlap_pad;
	//pre_message.RegNumber_row_remainder= isp_message->RegImageHeight / 2 -isp_message->RegRow_startloc[isp_message->RegNumber_row-1];
	pre_message.RegNumber_row_remainder = 106;
	post_statistic_t post_message;
	post_message.RegImageWidth = isp_message->RegImageWidth;
	post_message.RegReal_size = isp_message->RegReal_size;
	post_message.RegOverlap_pad = isp_message->RegOverlap_pad;
	post_message.RegNumber_row_remainder = pre_message.RegNumber_row_remainder;

	unsigned short int* pre_out;
	pre_out = (unsigned short int*)malloc(sizeof(unsigned short int) * (pre_message.RegReal_size + 2 * pre_message.RegOverlap_pad) * pre_message.RegImageWidth * 2);
	PreProcess preprocess;
	preprocess.unit_preprocess(rawbuf, pre_out, &pre_message, if_end);

	//*******************npu******************//
	//*******************npu******************//
	PostProcess postprocess;
	postprocess.unit_postprocess(pre_out, post_out, &post_message, if_start, if_end);

	return true;
}

bool CtModel::get_row(unsigned short int* input, unsigned short int* output, unsigned int start, unsigned int stop, unsigned short ImageWidth) {
	for (int i = start; i < stop; i++) {
		for (int j = 0; j < ImageWidth; j++) {
			output[(i - start) * ImageWidth + j] = input[i * ImageWidth + j];
		}
	}
	return true;
}

bool CtModel::handle_img(unsigned short int* input,
	unsigned short int* fout,
	isp_statistic_t isp_message) {
	unsigned short int* rawbuf = NULL;

	long size;
	PostProcess postprocess;
	bool if_end,if_start;
	for (int i = 0; i < isp_message.RegNumber_row; i++) {
		fseek(fp, isp_message.RegRow_startloc[i]*2* isp_message.RegImageWidth*2, SEEK_SET);
		if (i == isp_message.RegNumber_row - 1) {
			if_end = true;
			size = (isp_message.RegImageHeight - isp_message.RegRow_startloc[i] * 2)* isp_message.RegImageWidth;
		}
		else {
			if_end = false;
			size = (isp_message.RegReal_size + isp_message.RegOverlap_pad * 2)*2 * isp_message.RegImageWidth;
		}
		if (i==0) {
			if_start = true;
		}
		else {
			if_start = false;
		}
		rawbuf = (unsigned short int*) malloc(size * 2);

		this->get_row(input,rawbuf, isp_message.RegRow_startloc[i] * 2, isp_message.RegRow_startloc[i] * 2+ size / isp_message.RegImageWidth, isp_message.RegImageWidth);

		printf("%d\n", size/ isp_message.RegImageWidth);
		if (rawbuf != NULL) {
			unsigned short int* post_out = NULL;
			post_out = (unsigned short int*)malloc(sizeof(unsigned short int) *size);
			this->unit_sequence(rawbuf, post_out, &isp_message,if_end,if_start);
			const char* name[12] = { "00","01","02",
"10","11","12",
"20","21","22",
"30","31","32", };
			char* buffer = (char*)malloc(100);
			strcpy(buffer, "G:/test_data/c_test/raw/");
			strcat(buffer, name[i]);
			strcat(buffer, ".raw");
			printf("%s\n", buffer);
			FILE* buf2raw = NULL;
			buf2raw = fopen(buffer, "wb");
			fwrite(post_out, 2, size, buf2raw);
			for (int j = isp_message.RegRow_startloc[i] * 2; j < isp_message.RegRow_startloc[i] * 2+size / isp_message.RegImageWidth; j++) {

				if (if_start) {
					for (int k = 0; k < isp_message.RegImageWidth; k++) {
						fout[j * isp_message.RegImageWidth + k] = post_out[(j - isp_message.RegRow_startloc[i] * 2) * isp_message.RegImageWidth + k];
					}
				}
				else {
					if (j - isp_message.RegRow_startloc[i] * 2 < isp_message.RegOverlap_pad << 2) {
						for (int k = 0; k < isp_message.RegImageWidth; k++) {
								fout[j * isp_message.RegImageWidth + k] = fout[j * isp_message.RegImageWidth + k]+post_out[(j - isp_message.RegRow_startloc[i] * 2) * isp_message.RegImageWidth + k];
						}
					}
					else {
						for (int k = 0; k < isp_message.RegImageWidth; k++) {
							fout[j * isp_message.RegImageWidth + k] = post_out[(j - isp_message.RegRow_startloc[i] * 2) * isp_message.RegImageWidth + k];
						}
					}
				}
			}
		}
	}
	return true;
}

int main(int argc, char *argv[]) {
	isp_statistic_t isp_message;
	isp_message.RegImageWidth = 2328;
	isp_message.RegImageHeight = 1748;
	isp_message.RegOverlap_pad = 8;
	isp_message.RegReal_size = 256;
	isp_message.RegRow_startloc[0]=0;
	isp_message.RegRow_startloc[1] = 256;
	isp_message.RegRow_startloc[2] = 512;
	isp_message.RegRow_startloc[3] = 768;
	isp_message.RegNumber_row = 4;
	/*isp_message.Regtitle_min = 64;
	isp_message.Regtitle_level = 48;
	unsigned short suggest_Overlap_size= isp_message.Regtitle_min;
	unsigned short Number_row;
	unsigned short Number_col;
	long cost;
	long cost_min;
	for (int i = 0; i < isp_message.Regtitle_level; i++) {
		Number_row = (isp_message.RegImageHeight / 2- isp_message.RegOverlap_pad) % (isp_message.Regtitle_min + 8 * i) == 0 ? (isp_message.RegImageHeight / 2 - isp_message.RegOverlap_pad) / (isp_message.Regtitle_min + 8 * i)
			: (isp_message.RegImageHeight / 2 - isp_message.RegOverlap_pad) / (isp_message.Regtitle_min + 8 * i) + 1;
		Number_col = (isp_message.RegImageWidth / 2 - isp_message.RegOverlap_pad) % (isp_message.Regtitle_min + 8 * i) == 0 ? (isp_message.RegImageWidth / 2 - isp_message.RegOverlap_pad) / (isp_message.Regtitle_min + 8 * i)
			: (isp_message.RegImageWidth / 2 - isp_message.RegOverlap_pad) / (isp_message.Regtitle_min + 8 * i) + 1;
		cost = Number_row * Number_col * ((isp_message.Regtitle_min + 8 * i) + 2 * isp_message.RegOverlap_pad) * ((isp_message.Regtitle_min + 8 * i) + 2 * isp_message.RegOverlap_pad);
		if (i==0) {
			cost_min = cost;
			suggest_Overlap_size = isp_message.Regtitle_min + 8 * i;
		}
		else if (cost < cost_min){
			suggest_Overlap_size = isp_message.Regtitle_min + 8 * i;
			cost_min = cost;
		}
	}
	printf("suggest_Overlap_size is %d \n", suggest_Overlap_size);*/

	//isp_message.RegRow_startloc[0] = 0;
	//for (int i = 1; i < 28; i++) {
	//	if (i < isp_message.RegNumber_row) {
	//		isp_message.RegRow_startloc[i] = isp_message.RegReal_size * i;
	//		printf(" %d\n", isp_message.RegRow_startloc[i]);
	//	}
	//	else {
	//		isp_message.RegRow_startloc[i] = 0;
	//	}
	//}
	//printf(" %d\n", isp_message.RegNumber_row_remainder);
	//printf(" %d\n", isp_message.RegNumber_row);

	CtModel ctmodel;
	fp = fopen(argv[1], "rb");
	if (fp == NULL)
	{
		printf("open raw file %s error\n", argv[1]);
		return -1;
	}
	printf("open raw file %s success\n", argv[1]);
	fseek(fp, 0, SEEK_END);
	printf("raw file size: %d\n", ftell(fp));
	fseek(fp, 0, SEEK_SET);
	unsigned short int* input = NULL;
	input = (unsigned short int*) malloc(isp_message.RegImageWidth* isp_message.RegImageHeight * 2);
	fread(input, 2, isp_message.RegImageWidth * isp_message.RegImageHeight , fp);
	unsigned short int *fout = NULL;
	fout = (unsigned short int*)malloc(sizeof(unsigned short int) * isp_message.RegImageWidth * isp_message.RegImageHeight);
	ctmodel.handle_img(input,fout ,isp_message );

	if (fp != NULL)
		fclose(fp);
	if (fout != NULL)
		free(fout);
	printf("over");
	return 0;
}
