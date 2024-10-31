/****************************************************************************
*                                                                           *
*       This software and proprietary is confidential and may be used       *
*        only as expressly authorized by a licensing agreement from         *
*                                                                           *
*                            G Semiconductor                            *
*                                                                           *
*                   (C) COPYRIGHT 2022 G SEMICONDUCTOR                  *
*                            ALL RIGHTS RESERVED                            *
*                                                                           *
*                 The entire notice above must be reproduced                *
*                  on all copies and should not be removed.                 *
*                                                                           *
*****************************************************************************/
/**
@File name mtype.h
@Author: HubertHuang
@date 2014/9/9
*/

#ifndef _AL_MTYPE_H_
#define _AL_MTYPE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Global constant */
#ifndef TRUE
#define TRUE                    1
#endif

#ifndef FALSE
#define FALSE                   0
#endif

#ifndef NULL
#define NULL                    ((void *)0)
#endif

#ifndef UNUSED
#define UNUSED(x) (void)x
#endif

/* Global data type */

typedef float                   FLOAT32;
typedef double                  FLOAT64;

typedef char                    CHAR;
typedef signed char             SINT8;
#ifdef _ARC
typedef char                    INT8;
#else
typedef signed char             INT8;
#endif

typedef unsigned char           UINT8;
typedef short                   INT16;
typedef unsigned short          UINT16;
typedef int						INT32;
typedef unsigned int			UINT32;

#ifdef WIN32
#include <stdint.h>
typedef int64_t                 INT64;
typedef uint64_t                UINT64;
#else
typedef long long               INT64;
typedef unsigned long long      UINT64;
#endif

typedef int                   BOOL;
typedef UINT32                  ERRCODE;


/* For Android platform */
typedef unsigned char           uint8;
typedef unsigned short          uint16;
typedef unsigned int            uint32;
typedef unsigned long long      uint64;
#ifdef _ARC
typedef char                    int8;
#else
typedef signed char             int8;
#endif
typedef short                   int16;
typedef int                     int32;
typedef long long               int64;

typedef signed char             sint8;

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _AL_MTYPE_H_ */
