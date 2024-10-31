#ifndef _AFD_DFT_
#define _AFD_DFT_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "allib_mtype.h"

#define AFD_DFT_LEN      (2048)

void DFTIndex(FLOAT32* inputData, int inputDataLength, int index, FLOAT32* re, FLOAT32* im);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // !_AFD_DFT_
