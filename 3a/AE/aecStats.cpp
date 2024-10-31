#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>

using namespace std;

#include "aecStats.h"

int AECStats::AECStatsProcess(int16_t* p)
{
	memset(&stats, 0, sizeof(AEC_stats));
	uint16_t satValue = statsInfo.satValue;
	if (p == NULL) {
		std::cout << "source file open is null";
		return 1;
	}
	int clip_w = (IMG_W - (BLOCKNUM_W * BLOCK_W)) / 2;
	int clip_h = (IMG_H - (BLOCKNUM_H * BLOCK_H)) / 2;
	if (clip_w < 0) {
		std::cout << "block width size set error , out of range";
		return 1;
	}
	if (clip_h < 0) {
		std::cout << "sblock height size set error , out of range";
		return 1;
	}
	for (int y = clip_h; y + clip_h < IMG_H; y++) {
		int r = (y - clip_h) / BLOCK_H;
		int index_add = y * IMG_W;
		for (int x = clip_w; x + clip_w < IMG_W; x++) {
			int c = (x - clip_w) / BLOCK_W;
			int blockNum = r * BLOCKNUM_W + c;
			uint16_t pixel_index = index_add + x;
			uint16_t value = p[pixel_index];
			if (value <= satValue) {
				stats.sum[(x % 2) * 2 + y % 2][blockNum] += value;    //r,gr,gb/b
				stats.non_satNum[(x % 2) * 2 + y % 2][blockNum] += 1;
			}
		}
	}
	// histogram value save with block bin 
	for (int i = 0; i < BLOCKNUM_H * BLOCKNUM_W; i++) {
		float avg_r = (float)stats.sum[0][i] / stats.non_satNum[0][i];
		float avg_gr = (float)stats.sum[1][i] / stats.non_satNum[1][i];
		float avg_gb = (float)stats.sum[2][i] / stats.non_satNum[2][i];
		float avg_b = (float)stats.sum[3][i] / stats.non_satNum[3][i];
		//std::cout << i / BLOCKNUM_W << "," << i % BLOCKNUM_W << ":" << "avg_r gr gb b : " << avg_r << "  " << avg_gr << "  " << avg_gb << "  " << avg_b << endl;
		stats.histogram[i] = avg_r * RGB2Y[0] + (avg_gr + (avg_gb - avg_gr) / 2) * RGB2Y[1] + avg_b * RGB2Y[2];
		//std::cout << i / BLOCKNUM_W << "," << i % BLOCKNUM_W << ":" << "avg_y:"<< stats.histogram[i] << endl;
	}
	return 0;
}