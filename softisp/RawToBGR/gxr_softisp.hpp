#ifndef GXR_SOFTISP_HPP_FILE
#define GXR_SOFTISP_HPP_FILE

#ifndef __cplusplus
#  error C++, not C
#endif

#include <cstdint>
#include <cstdlib>
#include <cstring>

# ifdef GXR_SOFTISP_Build
# define GXR_SOFTISP_Export __declspec(dllexport)
# else
# define GXR_SOFTISP_Export __declspec(dllimport)
# endif

enum
{
	GXR_Raw_RGGB,
	GXR_Raw_BGGR,
	GXR_Raw_GRBG,
	GXR_Raw_GBRG,
	GXR_Raw_Patt_Impl = 4,
};


struct GXR_SOFTISP_VERSION
{
	const char* Version;
};


class GXR_SOFTISP_Export GXR_Softisp
{
	GXR_Softisp(GXR_Softisp const&) = delete;
	GXR_Softisp& operator =(GXR_Softisp const&) = delete;
	GXR_Softisp(GXR_Softisp&&) = delete;
	GXR_Softisp& operator =(GXR_Softisp&&) = delete;

public:
	struct SizeParam
	{
		int block_height, block_width;
		int lsc_height, lsc_width;
		int org_height, org_width; // origin size
		int roi_height, roi_width; // group size
	};

	struct Config
	{
		int verbose;
		int nthread; // -1 则默认 num(CPU) - 1
		int wben, expen;
		int aesat; // 12 位意义上
		unsigned short* gmtablex, * gmtabley; // 257
		float ynoise[5 * 8], cnoise[5 * 8]; // 去噪
		float sharp[5 * 8]; // 锐化
		float bdlarge[8];
	};

	/* 长度为 4 的数组按 RGGB 顺序 */
	struct Input
	{
		int rpat, depth;
		int blacklevel[4]; // 10 位意义上，与 RAW 数据的位数无关
		float wbgain[4];
		float ccm[9];
		float* lsctable[4];
		float sensorgain, ispgain, adrcgain;
		float shutter; // 单位为秒
		float luxindex;
		int top, left; // 凝视区左上顶点
		unsigned short* data;

		struct Stats
		{
			int* awb_blockstats; // SumB SumG SumR 12 位意义上，G是2通道加起来的，外面分配, awb
			int* ae_blockstats; // SumR SumGr SumGb SumB NonSatCntR NonSatCntSumGr NonSatCntSumGb NonSatCntSumB 10 位意义上，外面分配, ae
			int* histgram; // Y 256，外面分配
		};
		Stats stats;
	};

	struct Output
	{
		unsigned char* data;
	};

private:
	void* impl;
	void* handle;

public:
	// construction
	int create(SizeParam& p);

	int config(Config& c);

	int prestats(Input& in);

	int poststats(Output& out);

	int release();

	GXR_Softisp() : impl(0), handle(0) {}
	~GXR_Softisp() { release(); }
	static GXR_SOFTISP_VERSION get_version();
	void get_blocknum(int& blocknum_x, int& blocknum_y);
};

#undef GXR_SOFTISP_Export

#endif