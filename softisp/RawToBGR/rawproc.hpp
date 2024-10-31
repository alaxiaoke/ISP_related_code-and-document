#pragma once
#include <opencv2/core.hpp>

using namespace cv;

namespace gxr
{
enum RawPatternTypes
{
	Raw_RGGB,
	Raw_BGGR,
	Raw_GRBG,
	Raw_GBRG,
	Raw_Patt_Impl = 4,
};

enum RawPackTypes
{
	Raw_Unpack,
	Raw_Pack_Impl,
};

static unsigned constexpr RawFilter[16] =
{
	0x16161616U,
	0x94949494U,
	0x49494949U,
	0x61616161U,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

inline int constexpr fcol(int r, int c, unsigned pat)
{
	return 3 & static_cast<int>(
		pat >> ((((r << 1) & 14) + (c & 1)) << 1));
}
}