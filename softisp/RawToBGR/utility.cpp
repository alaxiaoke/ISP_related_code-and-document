#include "utility.hpp"
#include <cstdio>
#include <cstdarg>
#include <csignal>

#ifdef _MSC_VER
# if _MSC_VER >= 1700
// 'main' signature found without threading model
// #  pragma warning(disable: 4447)
// 'sprintf': This function or variable may be unsafe
#  pragma warning(disable: 4996)
# endif
#endif

# include <Windows.h>
# if (_WIN32_WINNT >= 0x0602)
#   include <synchapi.h>
# endif
# undef small
# undef min
# undef max
# undef abs

namespace gxr
{
	void log_printf(bool segsev, char const* fmt, ...)
	{
		char buf[1 << 14];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);
		buf[sizeof(buf) - 1] = 0;
		if (segsev) // line 总为真，这样可以抑制警告
		{
			fputs(buf, stderr);
			fflush(stderr);
#if _HAS_EXCEPTIONS
			throw buf;
#else
			char volatile* p = reinterpret_cast<char*>(0x2b);
			*p = 0x2b; // 引发异常
#endif
		}
		else
		{
#if GXR_WINDOW_OUTPUT
			OutputDebugStringA(buf);
#else
			fputs(buf, stdout);
#endif
		}
	}
}