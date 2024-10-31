#pragma once
#include <cstdlib>
#include <vector>

typedef unsigned short ushort;
typedef unsigned char uchar;

/* Src ����ͼBayer
   Dst ���ͼBGR pack����
   rows ����ͼ�߶�
   cols ����ͼ���
   rpat RGGB 0, GRBG 1 GBRG 2 BGGR 3
   sat (1 << bitdepth) - 1 */
void demosaic(ushort* Src, ushort* Dst, int rows, int cols, int rpat, int sat);




