#ifndef AEC_HANDLE
#define AEC_HANDLE

#ifdef AEC_HANDLE
#define AEC_API __declspec(dllexport)
#else
#define AEC_API __declspec(dllimport)
#endif

#include "aecAlgo.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

class AEC
{
public:
	AEC_API AEC();
	AEC_API ~AEC();
	AEC_API void init(InitParma* initParma);
	AEC_API void setParam(AECSetParam* setParam);
	AEC_API void getStats(int* p);
	AEC_API void getHist(int* p);
	AEC_API void process();
	AEC_API void getAEOut(AEC_algo_out* out);

	AECAlgo* handle;
};

#ifdef ____cplusplus
}
#endif // __cplusplus
}
#endif // !AEC_HANDLE