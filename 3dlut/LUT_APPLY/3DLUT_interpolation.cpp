#include "3DLUT_interpolation.h"

struct ColorTriplet
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
};

// YUV420 to RGB conversion function
//YUV转RGB系数：
//R = Y + c0 * (Cr - 128)
//G = Y + c1 * (Cb - 128) + c2 * (Cr - 128)
//B = Y + c3 (Cb - 128)

void CLut3D::YUV420toRGB(uint8_t* yuv, uint8_t* rgb, int width, int height) {

	int y, u, v;
	int r, g, b;
	int yIndex, uvIndex;
	int index;

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			yIndex = j * width + i;
			uvIndex = (j >> 1) * (width >> 1) + (i >> 1);

			y = yuv[yIndex];
			u = yuv[height * width + uvIndex] - lutParam.YUV2RGB_coef[4];
			v = yuv[height * width + ((height * width) >> 2) + uvIndex] - lutParam.YUV2RGB_coef[4];

			// Convert YUV to RGB
			r = (int)(y + ((lutParam.YUV2RGB_coef[0] * v + (1 << 11)) >> 12));
			g = (int)(y + ((lutParam.YUV2RGB_coef[1] * u + lutParam.YUV2RGB_coef[2] * v + (1 << 11)) >> 12));
			b = (int)(y + ((lutParam.YUV2RGB_coef[3] * u + (1 << 11)) >> 12));

			// Clamp values to [0, 255]
			if (r > 255) r = 255;
			if (r < 0) r = 0;
			if (g > 255) g = 255;
			if (g < 0) g = 0;
			if (b > 255) b = 255;
			if (b < 0) b = 0;

			// Store the RGB values
			index = yIndex * 3;
			rgb[index] = b;
			rgb[index + 1] = g;
			rgb[index + 2] = r;
		}
	}
}

// RGB to YUV conversion function 假设输入的RGB数据是连续存储的，每个像素占用3个字节（R、G、B）
//RGB转YUV系数：
//Y = c0 * R + c1 * G + c2 * B
//U = c3 * R + c4 * G + c5 * B + 128
//V = c6 * R + c7 * G + c8 * B + 128
void CLut3D::RGBtoYUV420(const uint8_t* rgb, uint8_t* yuv, int width, int height) {

	const int frameSize = width * height;
	const int chromaSize = frameSize >> 2;
	uint8_t r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;

	for (int i = 0, yIdx = 0, uIdx = frameSize, vIdx = frameSize + chromaSize; i < height; ++i) {
		for (int j = 0; j < width; j += 2) {
			// Read RGB values
			b1 = rgb[(i * width + j) * 3];
			g1 = rgb[(i * width + j) * 3 + 1];
			r1 = rgb[(i * width + j) * 3 + 2];

			b2 = rgb[(i * width + j + 1) * 3];
			g2 = rgb[(i * width + j + 1) * 3 + 1];
			r2 = rgb[(i * width + j + 1) * 3 + 2];


			// Convert to YUV
			yuv[yIdx++] = (uint8_t)((lutParam.RGB2YUV_coef[0] * r1 + lutParam.RGB2YUV_coef[1] * g1 + lutParam.RGB2YUV_coef[2] * b1 + (1 << 11)) >> 12);
			yuv[yIdx++] = (uint8_t)((lutParam.RGB2YUV_coef[0] * r2 + lutParam.RGB2YUV_coef[1] * g2 + lutParam.RGB2YUV_coef[2] * b2 + (1 << 11)) >> 12);

			if (i % 2 == 0) { // Only calculate U and V for every other row

				b3 = rgb[((i + 1) * width + j) * 3];
				g3 = rgb[((i + 1) * width + j) * 3 + 1];
				r3 = rgb[((i + 1) * width + j) * 3 + 2];

				b4 = rgb[((i + 1) * width + j + 1) * 3];
				g4 = rgb[((i + 1) * width + j + 1) * 3 + 1];
				r4 = rgb[((i + 1) * width + j + 1) * 3 + 2];

				yuv[uIdx] = (uint8_t)(((lutParam.RGB2YUV_coef[3] * r1 + lutParam.RGB2YUV_coef[4] * g1 + lutParam.RGB2YUV_coef[5] * b1 + 
					(lutParam.RGB2YUV_coef[3] * r2 + lutParam.RGB2YUV_coef[4] * g2 + lutParam.RGB2YUV_coef[5] * b2) + 
					(lutParam.RGB2YUV_coef[3] * r3 + lutParam.RGB2YUV_coef[4] * g3 + lutParam.RGB2YUV_coef[5] * b3) +
					(lutParam.RGB2YUV_coef[3] * r4 + lutParam.RGB2YUV_coef[4] * g4 + lutParam.RGB2YUV_coef[5] * b4) +
					(1 << 11)) >> 14) + lutParam.RGB2YUV_coef[9]);

				yuv[vIdx] = (uint8_t)(((lutParam.RGB2YUV_coef[6] * r1 + lutParam.RGB2YUV_coef[7] * g1 + lutParam.RGB2YUV_coef[8] * b1 +
					(lutParam.RGB2YUV_coef[6] * r2 + lutParam.RGB2YUV_coef[7] * g2 + lutParam.RGB2YUV_coef[8] * b2) +
					(lutParam.RGB2YUV_coef[6] * r3 + lutParam.RGB2YUV_coef[7] * g3 + lutParam.RGB2YUV_coef[8] * b3) +
					(lutParam.RGB2YUV_coef[6] * r4 + lutParam.RGB2YUV_coef[7] * g4 + lutParam.RGB2YUV_coef[8] * b4) +
					(1 << 11)) >> 14) + lutParam.RGB2YUV_coef[9]);
				uIdx++;
				vIdx++;
			}
		}
	}
}

int CLut3D::cubeIndex(int r, int g, int b)
{
	return (int(r + g * LUT_SIZE + b * LUT_SIZE * LUT_SIZE));
}



int CLut3D::ProcessImagePCTest(uint8_t* rgb, uint8_t* LUTD3, uint8_t* rgbout)
{

	int width = lutParam.RegImgWidth;
	int height = lutParam.RegImgHeight;


	unsigned char rH, gH, bH, rL, gL, bL;
	int index000, index001, index010, index011, index100, index101, index110, index111;
	int32_t c1 = 0, c2 = 0, c3 = 0;
	int rout = 0, gout = 0, bout = 0;

	uint32_t len = LUT_SIZE;
	int32_t r_rem = 0, g_rem = 0, b_rem = 0;

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int b = (rgb[i * height * 3 + j * 3]);
			int g = (rgb[i * height * 3 + j * 3 + 1]);
			int r = (rgb[i * height * 3 + j * 3 + 2]);

			int r255 = r << SHIFT;
			int g255 = g << SHIFT;
			int b255 = b << SHIFT;
			// 
			int r255_2 = (r255 >> 8) * (LUT_SIZE - 1);			//直接右移的方法得到ceil和floor
			int g255_2 = (g255 >> 8) * (LUT_SIZE - 1);
			int b255_2 = (b255 >> 8) * (LUT_SIZE - 1);

			//printf("r255_3:%.6d\n", r255_3);

			rL = r255_2 >> SHIFT;
			gL = g255_2 >> SHIFT;
			bL = b255_2 >> SHIFT;

			rH = rL + 1;
			gH = gL + 1;
			bH = bL + 1;

			index000 = cubeIndex(rL, gL, bL);
			index001 = cubeIndex(rL, gL, bH);
			index010 = cubeIndex(rL, gH, bL);
			index011 = cubeIndex(rL, gH, bH);
			index100 = cubeIndex(rH, gL, bL);
			index101 = cubeIndex(rH, gL, bH);
			index110 = cubeIndex(rH, gH, bL);
			index111 = cubeIndex(rH, gH, bH);

			ColorTriplet c000 = { LUTD3[index000 * 3], LUTD3[index000 * 3 + 1], LUTD3[index000 * 3 + 2] };
			ColorTriplet c001 = { LUTD3[index001 * 3], LUTD3[index001 * 3 + 1], LUTD3[index001 * 3 + 2] };
			ColorTriplet c010 = { LUTD3[index010 * 3], LUTD3[index010 * 3 + 1], LUTD3[index010 * 3 + 2] };
			ColorTriplet c011 = { LUTD3[index011 * 3], LUTD3[index011 * 3 + 1], LUTD3[index011 * 3 + 2] };
			ColorTriplet c100 = { LUTD3[index100 * 3], LUTD3[index100 * 3 + 1], LUTD3[index100 * 3 + 2] };
			ColorTriplet c101 = { LUTD3[index101 * 3], LUTD3[index101 * 3 + 1], LUTD3[index101 * 3 + 2] };
			ColorTriplet c110 = { LUTD3[index110 * 3], LUTD3[index110 * 3 + 1], LUTD3[index110 * 3 + 2] };
			ColorTriplet c111 = { LUTD3[index111 * 3], LUTD3[index111 * 3 + 1], LUTD3[index111 * 3 + 2] };

			r_rem = r255_2 & ((1 << SHIFT) - 1);
			g_rem = g255_2 & ((1 << SHIFT) - 1);
			b_rem = b255_2 & ((1 << SHIFT) - 1);

			uint32_t casev = 0;
			if (r_rem > g_rem && g_rem >= b_rem) casev = 1;
			else if (r_rem >= b_rem && b_rem > g_rem) casev = 2;
			else if (b_rem >= r_rem && r_rem >= g_rem) casev = 3;
			else if (g_rem >= r_rem && r_rem > b_rem) casev = 4;
			else if (g_rem > b_rem && b_rem >= r_rem) casev = 5;
			else casev = 6;

			switch (casev)
			{
			case 1:
				rout = c000.R + ((((c100.R - c000.R) * r_rem + (c110.R - c100.R) * g_rem + (c111.R - c110.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c100.G - c000.G) * r_rem + (c110.G - c100.G) * g_rem + (c111.G - c110.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c100.B - c000.B) * r_rem + (c110.B - c100.B) * g_rem + (c111.B - c110.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			case 2:
				rout = c000.R + ((((c100.R - c000.R) * r_rem + (c111.R - c101.R) * g_rem + (c101.R - c100.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c100.G - c000.G) * r_rem + (c111.G - c101.G) * g_rem + (c101.G - c100.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c100.B - c000.B) * r_rem + (c111.B - c101.B) * g_rem + (c101.B - c100.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			case 3:
				rout = c000.R + ((((c101.R - c001.R) * r_rem + (c111.R - c101.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c101.G - c001.G) * r_rem + (c111.G - c101.G) * g_rem + (c001.G - c000.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c101.B - c001.B) * r_rem + (c111.B - c101.B) * g_rem + (c001.B - c000.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			case 4:
				rout = c000.R + ((((c110.R - c010.R) * r_rem + (c010.R - c000.R) * g_rem + (c111.R - c110.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c110.G - c010.G) * r_rem + (c010.G - c000.G) * g_rem + (c111.G - c110.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c110.B - c010.B) * r_rem + (c010.B - c000.B) * g_rem + (c111.B - c110.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			case 5:
				rout = c000.R + ((((c111.R - c011.R) * r_rem + (c011.R - c001.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c111.G - c011.G) * r_rem + (c010.G - c000.G) * g_rem + (c011.G - c010.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c111.B - c011.B) * r_rem + (c010.B - c000.B) * g_rem + (c011.B - c010.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			case 6:
				rout = c000.R + ((((c111.R - c011.R) * r_rem + (c011.R - c001.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c111.G - c011.G) * r_rem + (c011.G - c001.G) * g_rem + (c001.G - c000.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c111.B - c011.B) * r_rem + (c011.B - c001.B) * g_rem + (c001.B - c000.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			default:
				break;
			}

			bout = bout < 0 ? 0 : bout;
			gout = gout < 0 ? 0 : gout;
			rout = rout < 0 ? 0 : rout;

			bout = bout > 255 ? 255 : bout;
			gout = gout > 255 ? 255 : gout;
			rout = rout > 255 ? 255 : rout;

			rgbout[i * height * 3 + j * 3] = bout;
			rgbout[i * height * 3 + j * 3 + 1] = gout;
			rgbout[i * height * 3 + j * 3 + 2] = rout;

		}
	}

	return 0;
}

int CLut3D::ProcessImage(uint8_t* yuv, uint8_t* LUTD3, uint8_t* yuvout)
{
	uint8_t* rgb; uint8_t* rgbout;
	rgb = new uint8_t[3280 * 3 * 2448];
	rgbout = new uint8_t[3280 * 3 * 2448];
	YUV420toRGB(yuv, rgb, lutParam.RegImgWidth, lutParam.RegImgHeight);

	int width = lutParam.RegImgWidth;
	int height = lutParam.RegImgHeight;

	if(lutParam.RegEnable == 0){
		memcpy(yuvout, yuv, width * height * 3 / 2 * sizeof(uint8_t));
		return 0;
	}

	unsigned char rH, gH, bH, rL, gL, bL;
	int index000, index001, index010, index011, index100, index101, index110, index111;
	int32_t c1 = 0, c2 = 0, c3 = 0;
	int rout = 0, gout = 0, bout = 0;

	uint32_t len = LUT_SIZE;
	int32_t r_rem = 0, g_rem = 0, b_rem = 0;

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int r = (rgb[i * height * 3 + j * 3]);
			int g = (rgb[i * height * 3 + j * 3 + 1]);
			int b = (rgb[i * height * 3 + j * 3 + 2]);

			int b255 = r << SHIFT;
			int g255 = g << SHIFT;
			int r255 = b << SHIFT;
			// 
			int r255_2 = (r255 >> 8) * (LUT_SIZE - 1);			//直接右移的方法得到ceil和floor
			int g255_2 = (g255 >> 8) * (LUT_SIZE - 1);
			int b255_2 = (b255 >> 8) * (LUT_SIZE - 1);

			//printf("r255_3:%.6d\n", r255_3);

			rL = r255_2 >> SHIFT;
			gL = g255_2 >> SHIFT;
			bL = b255_2 >> SHIFT;

			rH = rL + 1;
			gH = gL + 1;
			bH = bL + 1;

			index000 = cubeIndex(rL, gL, bL);
			index001 = cubeIndex(rL, gL, bH);
			index010 = cubeIndex(rL, gH, bL);
			index011 = cubeIndex(rL, gH, bH);
			index100 = cubeIndex(rH, gL, bL);
			index101 = cubeIndex(rH, gL, bH);
			index110 = cubeIndex(rH, gH, bL);
			index111 = cubeIndex(rH, gH, bH);

			ColorTriplet c000 = { LUTD3[index000 * 3], LUTD3[index000 * 3 + 1], LUTD3[index000 * 3 + 2] };
			ColorTriplet c001 = { LUTD3[index001 * 3], LUTD3[index001 * 3 + 1], LUTD3[index001 * 3 + 2] };
			ColorTriplet c010 = { LUTD3[index010 * 3], LUTD3[index010 * 3 + 1], LUTD3[index010 * 3 + 2] };
			ColorTriplet c011 = { LUTD3[index011 * 3], LUTD3[index011 * 3 + 1], LUTD3[index011 * 3 + 2] };
			ColorTriplet c100 = { LUTD3[index100 * 3], LUTD3[index100 * 3 + 1], LUTD3[index100 * 3 + 2] };
			ColorTriplet c101 = { LUTD3[index101 * 3], LUTD3[index101 * 3 + 1], LUTD3[index101 * 3 + 2] };
			ColorTriplet c110 = { LUTD3[index110 * 3], LUTD3[index110 * 3 + 1], LUTD3[index110 * 3 + 2] };
			ColorTriplet c111 = { LUTD3[index111 * 3], LUTD3[index111 * 3 + 1], LUTD3[index111 * 3 + 2] };

			r_rem = r255_2 & ((1 << SHIFT) - 1);
			g_rem = g255_2 & ((1 << SHIFT) - 1);
			b_rem = b255_2 & ((1 << SHIFT) - 1);

			uint32_t casev = 0;
			if (r_rem > g_rem && g_rem >= b_rem) casev = 1;
			else if (r_rem >= b_rem && b_rem > g_rem) casev = 2;
			else if (b_rem >= r_rem && r_rem >= g_rem) casev = 3;
			else if (g_rem >= r_rem && r_rem > b_rem) casev = 4;
			else if (g_rem > b_rem && b_rem >= r_rem) casev = 5;
			else casev = 6;

			switch (casev)
			{
			case 1:
				rout = c000.R + ((((c100.R - c000.R) * r_rem + (c110.R - c100.R) * g_rem + (c111.R - c110.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c100.G - c000.G) * r_rem + (c110.G - c100.G) * g_rem + (c111.G - c110.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c100.B - c000.B) * r_rem + (c110.B - c100.B) * g_rem + (c111.B - c110.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			case 2:
				rout = c000.R + ((((c100.R - c000.R) * r_rem + (c111.R - c101.R) * g_rem + (c101.R - c100.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c100.G - c000.G) * r_rem + (c111.G - c101.G) * g_rem + (c101.G - c100.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c100.B - c000.B) * r_rem + (c111.B - c101.B) * g_rem + (c101.B - c100.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			case 3:
				rout = c000.R + ((((c101.R - c001.R) * r_rem + (c111.R - c101.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c101.G - c001.G) * r_rem + (c111.G - c101.G) * g_rem + (c001.G - c000.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c101.B - c001.B) * r_rem + (c111.B - c101.B) * g_rem + (c001.B - c000.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			case 4:
				rout = c000.R + ((((c110.R - c010.R) * r_rem + (c010.R - c000.R) * g_rem + (c111.R - c110.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c110.G - c010.G) * r_rem + (c010.G - c000.G) * g_rem + (c111.G - c110.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c110.B - c010.B) * r_rem + (c010.B - c000.B) * g_rem + (c111.B - c110.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			case 5:
				rout = c000.R + ((((c111.R - c011.R) * r_rem + (c011.R - c001.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c111.G - c011.G) * r_rem + (c010.G - c000.G) * g_rem + (c011.G - c010.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c111.B - c011.B) * r_rem + (c010.B - c000.B) * g_rem + (c011.B - c010.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			case 6:
				rout = c000.R + ((((c111.R - c011.R) * r_rem + (c011.R - c001.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				gout = c000.G + ((((c111.G - c011.G) * r_rem + (c011.G - c001.G) * g_rem + (c001.G - c000.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
				bout = c000.B + ((((c111.B - c011.B) * r_rem + (c011.B - c001.B) * g_rem + (c001.B - c000.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);

				break;
			default:
				break;
			}

			bout = bout < 0 ? 0 : bout;
			gout = gout < 0 ? 0 : gout;
			rout = rout < 0 ? 0 : rout;

			bout = bout > 255 ? 255 : bout;
			gout = gout > 255 ? 255 : gout;
			rout = rout > 255 ? 255 : rout;

			rgbout[i * height * 3 + j * 3] = bout;
			rgbout[i * height * 3 + j * 3 + 1] = gout;
			rgbout[i * height * 3 + j * 3 + 2] = rout;

		}
	}

	RGBtoYUV420(rgbout, yuvout, lutParam.RegImgWidth, lutParam.RegImgHeight);

	delete[] rgb;
	rgb = nullptr;
	delete[] rgbout;
	rgbout = nullptr;

	return 0;
}

