#ifndef _FS_PROCESS_H_
#define _FS_PROCESS_H_

#include "alAFDLib_Core.h"

//FS FREQ VOTE

#define VOTE_NUM (7)

static UINT16 BOX[VOTE_NUM] = { 100,120,90,144,240,320,360 };  //µ×²ãfreq

//static UINT32 CNT[AFD_STABLE_QUEUE_SIZE] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};  //stable_freq cnt
static UINT32 CNT[AFD_STABLE_QUEUE_SIZE] = { 0 };  //stable_freq cnt

UINT32 AFDLib_flickerSensor_process(void* afd_dat, struct allib_afd_output_data_t* output);

#endif // !_FS_PROCESS_H_
