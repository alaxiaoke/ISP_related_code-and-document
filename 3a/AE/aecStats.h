#include <iostream>

using namespace std;

// block size
#define BLOCK_W  64
#define BLOCK_H  64
// img size
#define IMG_W	2328
#define IMG_H	1748
//block number
#define BLOCKNUM_W  36
#define BLOCKNUM_H  27

typedef struct AEC_stats_in {
	//to do
	// bayer pattern  rggb bggr gbrg
	int bayerPattern;
	// raw size info
	int imgHeight;
	int imgWeight;
	// block info  size & num
	int blockHeight;
	int blockWeight;
	int horBlockNum;
	int verBlockNum;
	// sat pixel value
	uint16_t satValue;  // = 1024 * 0.98;   可调参数
};

typedef struct AEC_stats
{
	uint32_t sum[4][BLOCKNUM_W * BLOCKNUM_H];
	uint16_t non_satNum[4][BLOCKNUM_W * BLOCKNUM_H];
	float histogram[BLOCKNUM_W * BLOCKNUM_H];
};

class AECStats {
public:
	AECStats();
	~AECStats();

	int AECStatsProcess(int16_t* p);

	AEC_stats_in	statsInfo;
	AEC_stats		stats;

};
