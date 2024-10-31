#define GXR_CPU_REFERENCE 0

#if GXR_CPU_REFERENCE
#include <opencv2/core.hpp>
#include "../RawToBGR/utility.hpp"
#include "clprogram.hpp"
using cv::min;
using cv::max;
using gxr::gxr_clamp;
#define __kernel
#define __global
#else
#define gxr_clamp(x, a, b) clamp(x, a, b)
#endif

#define GXR_OPENCL_DEBUG 1

static inline short TuneDiff(short center, short cur, short th_l, uchar th_h)
{
	short wDiff = cur - center;
	return (gxr_clamp(th_l - abs(wDiff), 0, 1 << th_h) * wDiff) >> th_h;
}

static void calBayerSpatialFilter(short res[4], short Win[100], short Th_L[4], uchar Th_H[4], int cfa_order)
{
	short Th_L0 = Th_L[0];
	short Th_L1 = Th_L[1];
	short Th_L2 = Th_L[2];
	short Th_L3 = Th_L[3];

	short Th_H0 = Th_H[0];
	short Th_H1 = Th_H[1];
	short Th_H2 = Th_H[2];
	short Th_H3 = Th_H[3];

	short diff, cur;
	int dsum;
	if (cfa_order == 0 || cfa_order == 3)
	{
		dsum = 0;
		cur = Win[44];
		dsum += TuneDiff(cur, Win[0], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[2], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[4], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[6], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[8], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[20], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[22], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[24], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[26], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[28], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[40], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[42], Th_L0, Th_H0);
		//dsum += TuneDiff(cur, cur, Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[46], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[48], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[60], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[62], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[64], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[66], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[68], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[80], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[82], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[84], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[86], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[88], Th_L0, Th_H0);
		if (dsum < 0)
			dsum = -((dsum * -43 + 512) >> 10);
		else
			dsum = ((dsum * 43 + 512) >> 10);
		res[0] = gxr_clamp(cur + dsum, 0, 1023);


		dsum = 0;
		cur = Win[55];
		dsum += TuneDiff(cur, Win[11], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[13], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[15], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[17], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[19], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[31], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[33], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[35], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[37], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[39], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[51], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[53], Th_L3, Th_H3);
		//dsum += TuneDiff(cur, cur, Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[57], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[59], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[71], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[73], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[75], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[77], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[79], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[91], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[93], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[95], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[97], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[99], Th_L3, Th_H3);
		if (dsum < 0)
			dsum = -((dsum * -43 + 512) >> 10);
		else
			dsum = ((dsum * 43 + 512) >> 10);
		res[3] = gxr_clamp(cur + dsum, 0, 1023);



		dsum = 0;
		cur = Win[45];
		dsum += TuneDiff(cur, Win[1], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[3], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[5], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[7], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[9], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[21], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[23], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[25], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[27], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[29], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[41], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[43], Th_L1, Th_H1);
		//dsum += TuneDiff(cur, cur, Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[47], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[49], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[61], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[63], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[65], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[67], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[69], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[81], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[83], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[85], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[87], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[89], Th_L1, Th_H1);

		dsum += TuneDiff(cur, Win[10], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[12], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[14], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[16], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[18], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[30], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[32], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[34], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[36], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[38], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[50], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[52], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[54], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[56], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[58], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[70], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[72], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[74], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[76], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[78], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[90], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[92], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[94], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[96], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[98], Th_L1, Th_H1);
		if (dsum < 0)
			dsum = -((dsum * -21 + 512) >> 10);
		else
			dsum = ((dsum * 21 + 512) >> 10);
		res[1] = gxr_clamp(cur + dsum, 0, 1023);



		dsum = 0;
		cur = Win[54];
		dsum += TuneDiff(cur, Win[1], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[3], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[5], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[7], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[9], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[21], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[23], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[25], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[27], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[29], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[41], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[43], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[45], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[47], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[49], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[61], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[63], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[65], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[67], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[69], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[81], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[83], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[85], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[87], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[89], Th_L2, Th_H2);

		dsum += TuneDiff(cur, Win[10], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[12], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[14], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[16], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[18], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[30], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[32], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[34], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[36], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[38], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[50], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[52], Th_L2, Th_H2);
		//dsum += TuneDiff(cur, cur, Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[56], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[58], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[70], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[72], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[74], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[76], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[78], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[90], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[92], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[94], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[96], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[98], Th_L2, Th_H2);
		if (dsum < 0)
			dsum = -((dsum * -21 + 512) >> 10);
		else
			dsum = ((dsum * 21 + 512) >> 10);
		res[2] = gxr_clamp(cur + dsum, 0, 1023);
	}
	else
	{
		dsum = 0;
		cur = Win[44];
		dsum += TuneDiff(cur, Win[0], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[2], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[4], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[6], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[8], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[20], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[22], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[24], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[26], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[28], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[40], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[42], Th_L0, Th_H0);
		//dsum += TuneDiff(cur, cur, Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[46], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[48], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[60], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[62], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[64], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[66], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[68], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[80], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[82], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[84], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[86], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[88], Th_L0, Th_H0);

		dsum += TuneDiff(cur, Win[11], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[13], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[15], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[17], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[19], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[31], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[33], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[35], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[37], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[39], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[51], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[53], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[55], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[57], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[59], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[71], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[73], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[75], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[77], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[79], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[91], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[93], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[95], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[97], Th_L0, Th_H0);
		dsum += TuneDiff(cur, Win[99], Th_L0, Th_H0);
		if (dsum < 0)
			dsum = -((dsum * -21 + 512) >> 10);
		else
			dsum = ((dsum * 21 + 512) >> 10);
		res[0] = gxr_clamp(cur + dsum, 0, 1023);


		dsum = 0;
		cur = Win[55];
		dsum += TuneDiff(cur, Win[0], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[2], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[4], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[6], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[8], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[20], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[22], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[24], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[26], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[28], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[40], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[42], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[44], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[46], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[48], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[60], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[62], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[64], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[66], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[68], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[80], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[82], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[84], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[86], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[88], Th_L3, Th_H3);

		dsum += TuneDiff(cur, Win[11], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[13], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[15], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[17], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[19], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[31], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[33], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[35], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[37], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[39], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[51], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[53], Th_L3, Th_H3);
		//dsum += TuneDiff(cur, cur, Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[57], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[59], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[71], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[73], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[75], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[77], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[79], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[91], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[93], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[95], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[97], Th_L3, Th_H3);
		dsum += TuneDiff(cur, Win[99], Th_L3, Th_H3);
		if (dsum < 0)
			dsum = -((dsum * -21 + 512) >> 10);
		else
			dsum = ((dsum * 21 + 512) >> 10);
		res[3] = gxr_clamp(cur + dsum, 0, 1023);



		dsum = 0;
		cur = Win[45];
		dsum += TuneDiff(cur, Win[1], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[3], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[5], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[7], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[9], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[21], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[23], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[25], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[27], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[29], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[41], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[43], Th_L1, Th_H1);
		//dsum += TuneDiff(cur, cur, Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[47], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[49], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[61], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[63], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[65], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[67], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[69], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[81], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[83], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[85], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[87], Th_L1, Th_H1);
		dsum += TuneDiff(cur, Win[89], Th_L1, Th_H1);
		if (dsum < 0)
			dsum = -((dsum * -43 + 512) >> 10);
		else
			dsum = ((dsum * 43 + 512) >> 10);
		res[1] = gxr_clamp(cur + dsum, 0, 1023);



		dsum = 0;
		cur = Win[54];
		dsum += TuneDiff(cur, Win[10], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[12], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[14], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[16], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[18], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[30], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[32], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[34], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[36], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[38], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[50], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[52], Th_L2, Th_H2);
		//dsum += TuneDiff(cur, cur, Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[56], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[58], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[70], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[72], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[74], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[76], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[78], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[90], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[92], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[94], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[96], Th_L2, Th_H2);
		dsum += TuneDiff(cur, Win[98], Th_L2, Th_H2);
		if (dsum < 0)
			dsum = -((dsum * -43 + 512) >> 10);
		else
			dsum = ((dsum * 43 + 512) >> 10);
		res[2] = gxr_clamp(cur + dsum, 0, 1023);
	}
}

__kernel void spafilter(__global short* src, __global short* dst, int rows, int cols, 
__global uchar const* filter_th_0, __global uchar const* filter_th_1, __global uchar const* filter_th_2, __global uchar const* filter_th_3, 
__global uchar const* filter_th_power, __global uchar const* mae_preserve_lut, int blending_weight, int cfa_order)
{
#if GXR_CPU_REFERENCE
	for (int h_half = 0; h_half < rows / 2; h_half++)
	{
		for (int w_half = 0; w_half < cols / 2; w_half++)
		{
#else
	int w_half = get_global_id(0);
	int h_half = get_global_id(1);
#endif
	int step = cols;
	int blending_weight_cmp = 128 - blending_weight;
	__global short* S[10];
	short Y[25];
	short Win[100];
	short y, ymean, mae, mean[4], weight;
	short Th_L[4];
	uchar Th_H[4];
	int h = h_half << 1;
	int w = w_half << 1;
	if (h >= 4 && h <= rows - 6 && w >= 4 && w <= cols - 6)
	{
		for (int i = 0; i < 10; i++)
			S[i] = src + (h + i - 4) * step;
		__global short* D = dst + h * cols;
		__global short* Line[2];
		int index;
		ymean = 0;
		mae = 0;
		mean[0] = mean[1] = mean[2] = mean[3] = 0;
		for (int i = 0; i < 10; i += 2)
		{
			Line[0] = S[i] + w - 4;
			Line[1] = Line[0] + step;
			index = (i >> 1) * 5;
			for (int j = 0; j < 10; j += 2)
			{
				Win[i * 10 + j] = Line[0][j];
				Win[i * 10 + j + 1] = Line[0][j + 1];
				Win[(i + 1) * 10 + j] = Line[1][j];
				Win[(i + 1) * 10 + j + 1] = Line[1][j + 1];

				y = (Win[i * 10 + j] + Win[i * 10 + j + 1] +
					Win[(i + 1) * 10 + j] + Win[(i + 1) * 10 + j + 1]) >> 2;
				Y[index + (j >> 1)] = y;
				ymean += y;

				mean[0] += Win[i * 10 + j];
				mean[1] += Win[i * 10 + j + 1];
				mean[2] += Win[(i + 1) * 10 + j];
				mean[3] += Win[(i + 1) * 10 + j + 1];
			}
		}
		ymean = (ymean * 41) >> 10;
		for (int i = 0; i < 25; i++)
			mae += abs(Y[i] - ymean);
		mae = gxr_clamp((mae * 41) >> 10, 0, 63);

		mean[0] = (mean[0] * 41) >> 10;
		mean[1] = (mean[1] * 41) >> 10;
		mean[2] = (mean[2] * 41) >> 10;
		mean[3] = (mean[3] * 41) >> 10;

		mean[0] = (blending_weight * Win[44] + blending_weight_cmp * mean[0] + 64) >> 7;
		mean[1] = (blending_weight * Win[45] + blending_weight_cmp * mean[1] + 64) >> 7;
		mean[2] = (blending_weight * Win[54] + blending_weight_cmp * mean[2] + 64) >> 7;
		mean[3] = (blending_weight * Win[55] + blending_weight_cmp * mean[3] + 64) >> 7;

		weight = mae_preserve_lut[mae];
		Th_H[0] = gxr_clamp((int)filter_th_power[mean[0] >> 8], 0, 8);
		Th_H[1] = gxr_clamp((int)filter_th_power[mean[1] >> 8], 0, 8);
		Th_H[2] = gxr_clamp((int)filter_th_power[mean[2] >> 8], 0, 8);
		Th_H[3] = gxr_clamp((int)filter_th_power[mean[3] >> 8], 0, 8);
		Th_L[0] = gxr_clamp((filter_th_0[mean[0]] * weight) >> 4, 0, 127) + (1 << Th_H[0]);
		Th_L[1] = gxr_clamp((filter_th_1[mean[1]] * weight) >> 4, 0, 127) + (1 << Th_H[1]);
		Th_L[2] = gxr_clamp((filter_th_2[mean[2]] * weight) >> 4, 0, 127) + (1 << Th_H[2]);
		Th_L[3] = gxr_clamp((filter_th_3[mean[3]] * weight) >> 4, 0, 127) + (1 << Th_H[3]);
		calBayerSpatialFilter(mean, Win, Th_L, Th_H, cfa_order);

		D[w] = mean[0];
		D[w + 1] = mean[1];
		D[w + cols] = mean[2];
		D[w + 1 + cols] = mean[3];
	}
#if GXR_CPU_REFERENCE
		}
	}
#endif
}

static inline uchar Discount(int var, int var_calib, __global uchar const* discount_lut)
{
	uchar discount;
	int gap = (int)(512 * log2((var + 0.5f) / (var_calib + 0.5f)));
	if (gap > 4095)
		discount = discount_lut[15];
	else if (gap > 3584)
		discount = discount_lut[14];
	else if (gap > 3702)
		discount = discount_lut[13];
	else if (gap > 2560)
		discount = discount_lut[12];
	else if (gap > 2048)
		discount = discount_lut[11];
	else if (gap > 1536)
		discount = discount_lut[10];
	else if (gap > 1024)
		discount = discount_lut[9];
	else if (gap > 512)
		discount = discount_lut[8];
	else if (gap > 0)
		discount = discount_lut[7];
	else if (gap > -512)
		discount = discount_lut[6];
	else if (gap > -(1 << 10))
		discount = discount_lut[5];
	else if (gap > -1536)
		discount = discount_lut[4];
	else if (gap > -2047)
		discount = discount_lut[3];
	else if (gap > -2560)
		discount = discount_lut[2];
	else if (gap > -3072)
		discount = discount_lut[1];
	else
		discount = discount_lut[0];
	return discount;
}

static inline uchar Anti_Ghost(int diff, int var, __global uchar const* anti_ghost_lut)
{
	int res, gap;
	if (diff == 0)
		res = 127;
	else
	{
		gap = (int)(512 * log2((diff * diff + 0.5f) / (var + 0.5f)));
		if (gap <= -3403)
			res = anti_ghost_lut[0];
		else if (gap <= -1701)
			res = anti_ghost_lut[1];
		else if (gap <= -1189)
			res = anti_ghost_lut[2];
		else if (gap <= -889)
			res = anti_ghost_lut[3];
		else if (gap <= -677)
			res = anti_ghost_lut[4];
		else if (gap <= -512)
			res = anti_ghost_lut[5];
		else if (gap <= -377)
			res = anti_ghost_lut[6];
		else if (gap <= -263)
			res = anti_ghost_lut[7];
		else if (gap <= -165)
			res = anti_ghost_lut[8];
		else if (gap <= -78)
			res = anti_ghost_lut[9];
		else if (gap <= -0)
			res = anti_ghost_lut[10];
		else if (gap <= 70)
			res = anti_ghost_lut[11];
		else if (gap <= 135)
			res = anti_ghost_lut[12];
		else if (gap <= 194)
			res = anti_ghost_lut[13];
		else if (gap <= 249)
			res = anti_ghost_lut[14];
		else if (gap <= 300)
			res = anti_ghost_lut[15];
		else if (gap <= 347)
			res = anti_ghost_lut[16];
		else if (gap <= 392)
			res = anti_ghost_lut[17];
		else if (gap <= 434)
			res = anti_ghost_lut[18];
		else if (gap <= 474)
			res = anti_ghost_lut[19];
		else if (gap <= 512)
			res = anti_ghost_lut[20];
		else if (gap <= 548)
			res = anti_ghost_lut[21];
		else if (gap <= 582)
			res = anti_ghost_lut[22];
		else if (gap <= 615)
			res = anti_ghost_lut[23];
		else if (gap <= 647)
			res = anti_ghost_lut[24];
		else if (gap <= 677)
			res = anti_ghost_lut[25];
		else
			res = anti_ghost_lut[26];
	}
	return res;
}

__kernel void tnr_pre(__global short* src, __global short* pre, __global short* spares, __global uchar* confidence_map, int rows, int cols,
	__global uchar const* filter_th_0, __global uchar const* filter_th_1, __global uchar const* filter_th_2, __global uchar const* filter_th_3,
	__global uchar const* filter_th_power, __global uchar const* mae_preserve_lut, int blending_weight, int cfa_order,
	__global ushort const* var_r, __global ushort const* var_g, __global uchar const* discount_lut, __global uchar const* anti_ghost_lut, int use_calivar_only)
{
#if GXR_CPU_REFERENCE
	for (int h_half = 0; h_half < rows / 2; h_half++)
	{
		for (int w_half = 0; w_half < cols / 2; w_half++)
		{
#else
	int w_half = get_global_id(0);
	int h_half = get_global_id(1);
#endif
	int step = cols;
	int step_r = cols;
	int blending_weight_cmp = 128 - blending_weight;
	__global short* S[10];
	__global short* R[10];
	__global short* D[2];
	short Y[25];
	short Win[100];
	short y, ymean, mae, mean[4], spa_res[4], weight;
	short Th_L[4];
	uchar Th_H[4];

	short mean_src[4], mean_ref[4];
	int var[3], var_calib[3];
	uchar confidence;

	int h = h_half << 1;
	int w = w_half << 1;
	if (h >= 4 && h <= rows - 6 && w >= 4 && w <= cols - 6)
	{
		for (int i = 0; i < 10; i++)
			S[i] = src + (h + i - 4) * step;
		for (int i = 0; i < 10; i++)
			R[i] = pre + (h + i - 4) * step_r;
		D[0] = spares + h * cols;
		D[1] = D[0] + cols;
		__global short* Line[2];
		__global short* Line_R[2];
		int index;
		ymean = 0;
		mae = 0;
		mean[0] = mean[1] = mean[2] = mean[3] = 0;

		mean_src[0] = mean_src[1] = mean_src[2] = mean_src[3] = 0;
		mean_ref[0] = mean_ref[1] = mean_ref[2] = mean_ref[3] = 0;
		var[0] = var[1] = var[2] = 0;

		for (int i = 0; i < 10; i += 2)
		{
			Line[0] = S[i] + w - 4;
			Line[1] = Line[0] + step;

			Line_R[0] = R[i] + w - 4;
			Line_R[1] = Line_R[0] + step_r;

			index = (i >> 1) * 5;

			for (int j = 0; j < 10; j += 2)
			{
				spa_res[0] = Line[0][j];
				spa_res[1] = Line[0][j + 1];
				spa_res[2] = Line[1][j];
				spa_res[3] = Line[1][j + 1];

				Win[i * 10 + j] = spa_res[0];
				Win[i * 10 + j + 1] = spa_res[1];
				Win[(i + 1) * 10 + j] = spa_res[2];
				Win[(i + 1) * 10 + j + 1] = spa_res[3];

				y = (spa_res[0] + spa_res[1] +
					spa_res[2] + spa_res[3]) >> 2;
				Y[index + (j >> 1)] = y;
				ymean += y;

				mean[0] += spa_res[0];
				mean[1] += spa_res[1];
				mean[2] += spa_res[2];
				mean[3] += spa_res[3];

				if (i >= 2 && i <= 6 && j >= 2 && j <= 6)
				{
					mean_src[0] += spa_res[0];
					mean_src[1] += spa_res[1];
					mean_src[2] += spa_res[2];
					mean_src[3] += spa_res[3];

					mean_ref[0] += Line_R[0][j];
					mean_ref[1] += Line_R[0][j + 1];
					mean_ref[2] += Line_R[1][j];
					mean_ref[3] += Line_R[1][j + 1];
				}
			}
		}

		ymean = (ymean * 41) >> 10;
		for (int i = 0; i < 25; i++)
			mae += abs(Y[i] - ymean);
		mae = gxr_clamp((mae * 41) >> 10, 0, 63);

		mean[0] = (mean[0] * 41) >> 10;
		mean[1] = (mean[1] * 41) >> 10;
		mean[2] = (mean[2] * 41) >> 10;
		mean[3] = (mean[3] * 41) >> 10;

		mean[0] = (blending_weight * Win[44] + blending_weight_cmp * mean[0] + 64) >> 7;
		mean[1] = (blending_weight * Win[45] + blending_weight_cmp * mean[1] + 64) >> 7;
		mean[2] = (blending_weight * Win[54] + blending_weight_cmp * mean[2] + 64) >> 7;
		mean[3] = (blending_weight * Win[55] + blending_weight_cmp * mean[3] + 64) >> 7;

		weight = mae_preserve_lut[mae];
		Th_H[0] = gxr_clamp((int)filter_th_power[mean[0] >> 8], 0, 8);
		Th_H[1] = gxr_clamp((int)filter_th_power[mean[1] >> 8], 0, 8);
		Th_H[2] = gxr_clamp((int)filter_th_power[mean[2] >> 8], 0, 8);
		Th_H[3] = gxr_clamp((int)filter_th_power[mean[3] >> 8], 0, 8);
		Th_L[0] = gxr_clamp((filter_th_0[mean[0]] * weight) >> 4, 0, 127) + (1 << Th_H[0]);
		Th_L[1] = gxr_clamp((filter_th_1[mean[1]] * weight) >> 4, 0, 127) + (1 << Th_H[1]);
		Th_L[2] = gxr_clamp((filter_th_2[mean[2]] * weight) >> 4, 0, 127) + (1 << Th_H[2]);
		Th_L[3] = gxr_clamp((filter_th_3[mean[3]] * weight) >> 4, 0, 127) + (1 << Th_H[3]);
		calBayerSpatialFilter(spa_res, Win, Th_L, Th_H, cfa_order);
		D[0][w] = spa_res[0];
		D[0][w + 1] = spa_res[1];
		D[1][w] = spa_res[2];
		D[1][w + 1] = spa_res[3];



		mean_src[0] = (mean_src[0] * 114) >> 10;
		mean_src[1] = (mean_src[1] * 114) >> 10;
		mean_src[2] = (mean_src[2] * 114) >> 10;
		mean_src[3] = (mean_src[3] * 114) >> 10;

		mean_ref[0] = (mean_ref[0] * 114) >> 10;
		mean_ref[1] = (mean_ref[1] * 114) >> 10;
		mean_ref[2] = (mean_ref[2] * 114) >> 10;
		mean_ref[3] = (mean_ref[3] * 114) >> 10;

		if (cfa_order == 0 || cfa_order == 3)
		{
			short m, d;
			uchar discount;

			m = mean_src[0];
			d = m - Win[22]; var[0] += d * d;
			d = m - Win[24]; var[0] += d * d;
			d = m - Win[26]; var[0] += d * d;

			d = m - Win[42]; var[0] += d * d;
			d = m - Win[44]; var[0] += d * d;
			d = m - Win[46]; var[0] += d * d;

			d = m - Win[62]; var[0] += d * d;
			d = m - Win[64]; var[0] += d * d;
			d = m - Win[66]; var[0] += d * d;

			var[0] = (var[0] * 114) >> 10;
			var_calib[0] = var_r[m >> 1];

			discount = Discount(var[0], var_calib[0], discount_lut);
			var_calib[0] = use_calivar_only ? var_calib[0] : max(var[0], var_calib[0]);
			var[0] = gxr_clamp((int)((abs(mean_src[0] - mean_ref[0]) * discount) >> 6), 0, 1023);
			confidence = Anti_Ghost(var[0], var_calib[0], anti_ghost_lut);



			m = mean_src[1];
			d = m - Win[23]; var[1] += d * d;
			d = m - Win[25]; var[1] += d * d;
			d = m - Win[27]; var[1] += d * d;

			d = m - Win[43]; var[1] += d * d;
			d = m - Win[45]; var[1] += d * d;
			d = m - Win[47]; var[1] += d * d;

			d = m - Win[63]; var[1] += d * d;
			d = m - Win[65]; var[1] += d * d;
			d = m - Win[67]; var[1] += d * d;

			var[1] = (var[1] * 114) >> 10;
			var_calib[1] = var_g[m >> 1];

			discount = Discount(var[1], var_calib[1], discount_lut);
			var_calib[1] = use_calivar_only ? var_calib[1] : max(var[1], var_calib[1]);
			var[1] = gxr_clamp((int)((abs(mean_src[1] - mean_ref[1]) * discount) >> 6), 0, 1023);
			confidence = min(Anti_Ghost(var[1], var_calib[1], anti_ghost_lut), confidence);



			m = mean_src[3];
			d = m - Win[33]; var[2] += d * d;
			d = m - Win[35]; var[2] += d * d;
			d = m - Win[37]; var[2] += d * d;

			d = m - Win[53]; var[2] += d * d;
			d = m - Win[55]; var[2] += d * d;
			d = m - Win[57]; var[2] += d * d;

			d = m - Win[73]; var[2] += d * d;
			d = m - Win[75]; var[2] += d * d;
			d = m - Win[77]; var[2] += d * d;

			var[2] = (var[2] * 114) >> 10;
			var_calib[2] = var_r[m >> 1];

			discount = Discount(var[2], var_calib[2], discount_lut);
			var_calib[2] = use_calivar_only ? var_calib[2] : max(var[2], var_calib[2]);
			var[2] = gxr_clamp((int)((abs(mean_src[3] - mean_ref[3]) * discount) >> 6), 0, 1023);
			confidence = min(Anti_Ghost(var[2], var_calib[2], anti_ghost_lut), confidence);
		}
		else
		{
			short m, d;
			uchar discount;

			m = mean_src[0];
			d = m - Win[22]; var[0] += d * d;
			d = m - Win[24]; var[0] += d * d;
			d = m - Win[26]; var[0] += d * d;

			d = m - Win[42]; var[0] += d * d;
			d = m - Win[44]; var[0] += d * d;
			d = m - Win[46]; var[0] += d * d;

			d = m - Win[62]; var[0] += d * d;
			d = m - Win[64]; var[0] += d * d;
			d = m - Win[66]; var[0] += d * d;

			var[0] = (var[0] * 114) >> 10;
			var_calib[0] = var_g[m >> 1];

			discount = Discount(var[0], var_calib[0], discount_lut);
			var_calib[0] = use_calivar_only ? var_calib[0] : max(var[0], var_calib[0]);
			var[0] = gxr_clamp((int)((abs(mean_src[0] - mean_ref[0]) * discount) >> 6), 0, 1023);
			confidence = Anti_Ghost(var[0], var_calib[0], anti_ghost_lut);



			m = mean_src[1];
			d = m - Win[23]; var[1] += d * d;
			d = m - Win[25]; var[1] += d * d;
			d = m - Win[27]; var[1] += d * d;

			d = m - Win[43]; var[1] += d * d;
			d = m - Win[45]; var[1] += d * d;
			d = m - Win[47]; var[1] += d * d;

			d = m - Win[63]; var[1] += d * d;
			d = m - Win[65]; var[1] += d * d;
			d = m - Win[67]; var[1] += d * d;

			var[1] = (var[1] * 114) >> 10;
			var_calib[1] = var_r[m >> 1];

			discount = Discount(var[1], var_calib[1], discount_lut);
			var_calib[1] = use_calivar_only ? var_calib[1] : max(var[1], var_calib[1]);
			var[1] = gxr_clamp((int)((abs(mean_src[1] - mean_ref[1]) * discount) >> 6), 0, 1023);
			confidence = min(Anti_Ghost(var[1], var_calib[1], anti_ghost_lut), confidence);



			m = mean_src[2];
			d = m - Win[32]; var[2] += d * d;
			d = m - Win[34]; var[2] += d * d;
			d = m - Win[36]; var[2] += d * d;

			d = m - Win[52]; var[2] += d * d;
			d = m - Win[54]; var[2] += d * d;
			d = m - Win[56]; var[2] += d * d;

			d = m - Win[72]; var[2] += d * d;
			d = m - Win[74]; var[2] += d * d;
			d = m - Win[76]; var[2] += d * d;

			var[2] = (var[2] * 114) >> 10;
			var_calib[2] = var_r[m >> 1];

			discount = Discount(var[2], var_calib[2], discount_lut);
			var_calib[2] = use_calivar_only ? var_calib[2] : max(var[2], var_calib[2]);
			var[2] = gxr_clamp((int)((abs(mean_src[3] - mean_ref[3]) * discount) >> 6), 0, 1023);
			confidence = min(Anti_Ghost(var[2], var_calib[2], anti_ghost_lut), confidence);
		}
		confidence_map[((h * cols) >> 2) + w_half] = confidence;
	}
#if GXR_CPU_REFERENCE
		}
	}
#endif
}

__kernel void tnr_post(__global short* src, __global short* pre, __global short* spares, __global uchar* confidence_map, int rows, int cols, int tw, __global uchar* confidence_debug)
{
#if GXR_CPU_REFERENCE
	for (int h_half = 0; h_half < rows / 2; h_half++)
	{
		for (int w_half = 0; w_half < cols / 2; w_half++)
		{
#else
	int w_half = get_global_id(0);
	int h_half = get_global_id(1);
#endif

	__global short* S[2];
	__global short* R[2];
	__global short* D[2];
	__global uchar* C[5];

	int h = h_half << 1;
	int w = w_half << 1;
	int hcols = cols >> 1;

	uchar M[25], mini, confidence;
	ushort mean;
	short s[4], d[4];
	short t;

	if (h >= 4 && h <= rows - 6 && w >= 4 && w <= cols - 6)
	{
		S[0] = src + h * cols;
		S[1] = S[0] + cols;

		R[0] = pre + h * cols;
		R[1] = R[0] + cols;

		D[0] = spares + h * cols;
		D[1] = D[0] + cols;

		C[2] = confidence_map + h_half * hcols + w_half;
		C[1] = C[2] - hcols;
		C[0] = C[1] - hcols;
		C[3] = C[2] + hcols;
		C[4] = C[3] + hcols;
		
		M[0] = C[0][-2]; M[1] = C[0][-1]; M[2] = C[0][0]; M[3] = C[0][1]; M[4] = C[0][2];
		M[5] = C[1][-2]; M[6] = C[1][-1]; M[7] = C[1][0]; M[8] = C[1][1]; M[9] = C[1][2];
		M[10] = C[2][-2]; M[11] = C[2][-1]; M[12] = C[2][0]; M[13] = C[2][1]; M[14] = C[2][2];
		M[15] = C[3][-2]; M[16] = C[3][-1]; M[17] = C[3][0]; M[18] = C[3][1]; M[19] = C[3][2];
		M[20] = C[4][-2]; M[21] = C[4][-1]; M[22] = C[4][0]; M[23] = C[4][1]; M[24] = C[4][2];

		mean = 0;
		mini = 127;

		for (int i = 0; i < 25; i++)
		{
			mean += M[i];
			mini = M[i] < mini ? M[i] : mini;
		}
		mean = (mean * 41) >> 10;

		confidence = ((mini << 7) + (M[12] - mini) * mean + 64) >> 7;

		s[0] = S[0][w]; s[1] = S[0][w + 1];
		s[2] = S[1][w]; s[3] = S[1][w + 1];

		d[0] = D[0][w]; d[1] = D[0][w + 1];
		d[2] = D[1][w]; d[3] = D[1][w + 1];

		t = ((s[0] << 7) + tw * (R[0][w] - s[0]) + 64) >> 7;
		R[0][w] = ((d[0] << 7) + (t - d[0]) * confidence + 64) >> 7;
		t = ((s[1] << 7) + tw * (R[0][w + 1] - s[1]) + 64) >> 7;
		R[0][w + 1] = ((d[1] << 7) + (t - d[1]) * confidence + 64) >> 7;
		t = ((s[2] << 7) + tw * (R[1][w] - s[2]) + 64) >> 7;
		R[1][w] = ((d[2] << 7) + (t - d[2]) * confidence + 64) >> 7;
		t = ((s[3] << 7) + tw * (R[1][w + 1] - s[3]) + 64) >> 7;
		R[1][w + 1] = ((d[3] << 7) + (t - d[3]) * confidence + 64) >> 7;

#if GXR_OPENCL_DEBUG
		confidence_debug[h * cols + w] = confidence;
		confidence_debug[h * cols + w + 1] = confidence;
		confidence_debug[(h + 1) * cols + w] = confidence;
		confidence_debug[(h + 1) * cols + w + 1] = confidence;
#endif
	}
#if GXR_CPU_REFERENCE
		}
	}
#endif
}

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void ydenoise(__read_only image2d_t src, __global uchar* dst, int rows, int cols, __global int const* Th_L, __global int const* Th_H)
{
	int w = get_global_id(0);
	int h = get_global_id(1);
	if (w < cols && h < rows)
	{
		int2 pos = {w, h};
		int center = read_imageui(src, sampler, pos).x;
		int mean = 0;
		for (int i = -2; i <= 2; i++)
		{
			pos.y = h + i;
			for (int j = -2; j <= 2; j++)
			{
				pos.x = w + j;
				int cur = read_imageui(src, sampler, pos).x;
				mean += cur;
			}
		}
		mean = (mean * 41) >> 10;

		int th_h = Th_H[mean >> 6];
		int th = 1 << th_h;
		int th_l = Th_L[mean >> 5] + th;
		int dsum = 0;
		for (int i = -2; i <= 2; i++)
		{
			pos.y = h + i;
			for (int j = -2; j <= 2; j++)
			{
				pos.x = w + j;
				int cur = read_imageui(src, sampler, pos).x;
				int diff = cur - center;
				dsum += ((gxr_clamp(th_l - (int)abs(diff), 0, th) * diff) >> th_h);
			}
		}
		int minus = dsum < 0 ? 1 : 0;
		dsum = ((dsum * (minus ? -43 : 43) + 512) >> 10);
		dst[h * cols + w] = gxr_clamp(center + (minus ? -dsum : dsum), 0, 255);
	}
}

__kernel void ydown(__read_only image2d_t src, __global uchar* dst, int drows, int dcols)
{
	int w = get_global_id(0);
	int h = get_global_id(1);
	int const coef[4] = {1, 3, 3, 1};
	if (w < dcols && h < drows)
	{
		int2 pos, corner = { (w << 1) - 1, (h << 1) - 1 };
		int sum = 0;
		for (int i = 0; i < 4; i++)
		{
			pos.y = corner.y + i;
			int rowsum = 0;
			for (int j = 0; j < 4; j++)
			{
				pos.x = corner.x + j;
				int cur = read_imageui(src, sampler, pos).x;
				rowsum += cur * coef[j];
			}
			sum += rowsum * coef[i];
		}
		sum = (sum + 32) >> 6;
		dst[h * dcols + w] = sum;
	}
}

const sampler_t sampler_linear = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

__kernel void yupmerge(__read_only image2d_t hsrc, __read_only image2d_t hdst, __read_only image2d_t src, __global uchar* dst, int rows, int cols)
{
	int w = get_global_id(0);
	int h = get_global_id(1);
	if (w < cols && h < rows)
	{
		int2 pos = { w, h};
		float2 hpos = {0.5f * w + 0.25f, 0.5f * h + 0.25f};
		int s = read_imageui(src, sampler, pos).x;
		int hs = read_imageui(hsrc, sampler_linear, hpos).x;
		int hd = read_imageui(hdst, sampler_linear, hpos).x;
		int d = gxr_clamp(s + hd - hs, 0, 255);
		dst[h * cols + w] = d;
	}
}

__kernel void cdenoise(__read_only image2d_t src, __global uchar* dst, int rows, int cols, __global int const* Th_L, __global int const* Th_H)
{
	int w = get_global_id(0);
	int h = get_global_id(1);
	if (w < cols && h < rows)
	{
		int2 pos = { w, h };
		int2 center = convert_int2(read_imageui(src, sampler, pos).xy);

		//int th_h = Th_H[0];
		//int th = 1 << th_h;
		//int th_l = Th_L[0] + th;

		int th_h = 4;
		int th = 1 << th_h;
		int th_l = 8 + th;

		int2 dsum = 0;
		for (int i = -2; i <= 2; i++)
		{
			pos.y = h + i;
			for (int j = -2; j <= 2; j++)
			{
				pos.x = w + j;
				int2 cur = convert_int2(read_imageui(src, sampler, pos).xy);
				int2 diff = cur - center;
				dsum += ((gxr_clamp(th_l - convert_int2(abs(diff)), 0, th) * diff) >> th_h);
			}
		}

		int minus = dsum.x < 0 ? 1 : 0;
		dsum.x = ((dsum.x * (minus ? -43 : 43) + 512) >> 10);
		int index = (h * cols + w) << 1;
		dst[index] = gxr_clamp(center.x + (minus ? -dsum.x : dsum.x), 0, 255);

		minus = dsum.y < 0 ? 1 : 0;
		dsum.y = ((dsum.y * (minus ? -43 : 43) + 512) >> 10);
		dst[index + 1] = gxr_clamp(center.y + (minus ? -dsum.y : dsum.y), 0, 255);
	}
}

__kernel void cdown(__read_only image2d_t src, __global uchar* dst, int drows, int dcols)
{
	int w = get_global_id(0);
	int h = get_global_id(1);
	int const coef[4] = { 1, 3, 3, 1 };
	if (w < dcols && h < drows)
	{
		int2 pos, corner = { (w << 1) - 1, (h << 1) - 1 };
		int2 sum = 0;
		for (int i = 0; i < 4; i++)
		{
			pos.y = corner.y + i;
			int2 rowsum = 0;
			for (int j = 0; j < 4; j++)
			{
				pos.x = corner.x + j;
				int2 cur = convert_int2(read_imageui(src, sampler, pos).xy);
				rowsum += cur * coef[j];
			}
			sum += rowsum * coef[i];
		}
		sum = (sum + 32) >> 6;

		int index = (h * dcols + w) << 1;
		dst[index] = sum.x;
		dst[index + 1] = sum.y;
	}
}

__kernel void cmerge(__read_only image2d_t hsrc, __read_only image2d_t hdst, __read_only image2d_t src, __global uchar* dst, int rows, int cols)
{
	int w = get_global_id(0);
	int h = get_global_id(1);
	if (w < cols && h < rows)
	{
		int2 pos = { w, h };
		float2 hpos = { 0.5f * w + 0.25f, 0.5f * h + 0.25f };
		int2 s = convert_int2(read_imageui(src, sampler, pos).xy);
		int2 hs = convert_int2(read_imageui(hsrc, sampler_linear, hpos).xy);
		int2 hd = convert_int2(read_imageui(hdst, sampler_linear, hpos).xy);
		int2 d = gxr_clamp(s + hd - hs, 0, 255);

		int index = (h * cols + w) << 1;
		dst[index] = d.x;
		dst[index + 1] = d.y;
	}
}


