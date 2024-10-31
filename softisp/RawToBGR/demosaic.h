#pragma once
#include <cstdlib>
#include <vector>

typedef unsigned short ushort;
typedef unsigned char uchar;

/* Src 输入图Bayer
   Dst 结果图BGR pack排列
   rows 输入图高度
   cols 输入图宽度
   rpat RGGB 0, GRBG 1 GBRG 2 BGGR 3
   sat (1 << bitdepth) - 1 */
void demosaic(ushort* Src, ushort* Dst, int rows, int cols, int rpat, int sat);




