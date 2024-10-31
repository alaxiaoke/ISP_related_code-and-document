#pragma once

#define GXR_WINDOW_OUTPUT 0

#ifndef __cplusplus
#error no C support
#endif

#if (defined _MSC_VER && _MSC_VER < 1900) && (__cplusplus < 201103L)
# error compiler must support basic C++11
#endif

#if defined _MSC_VER && defined _WIN321 && (defined _M_IX86 || defined _M_X64)
#  define GXR_Build 1
#endif

#if defined(_M_X64) || defined(__x86_64__) || defined(__aarch64__)
# define GXR_M_X64
#endif

namespace gxr
{
	template<class Tp> inline constexpr
		Tp const& gxr_min(Tp const& x, Tp const& y)
	{
		return y < x ? y : x;
	}

	template<class Tp> inline constexpr
		Tp const& gxr_max(Tp const& x, Tp const& y)
	{
		return y < x ? x : y;
	}

	template<class Tp> inline constexpr
		Tp const& gxr_clamp(Tp const& x, Tp const& lo, Tp const& hi)
	{
		return x < lo ? lo : (hi < x ? hi : x);
	}

	void log_printf(bool segsev, char const* fmt, ...);
}

#define GXR_Printf(...) gxr::log_printf(false, __VA_ARGS__)