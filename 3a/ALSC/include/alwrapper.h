/****************************************************************************
*                                                                           *
*       This software and proprietary is confidential and may be used       *
*        only as expressly authorized by a licensing agreement from         *
*                                                                           *
*                            Altek Semiconductor                            *
*                                                                           *
*                   (C) COPYRIGHT 2018 ALTEK SEMICONDUCTOR                  *
*                            ALL RIGHTS RESERVED                            *
*                                                                           *
*                 The entire notice above must be reproduced                *
*                  on all copies and should not be removed.                 *
*                                                                           *
*****************************************************************************/


/**
@defgroup al_WRAPPER
@brief OS abstraction layer
*/

#ifndef _AL_WRAPPER_H_
#define _AL_WRAPPER_H_
#include <string.h>

#ifdef __cplusplus
  extern "C" {
#endif

#define LOG_MAX_PRINT_LENGTH    255

/**
@brief Wrapper for strcmp.
*/
int wrapper_strcmp(const char *str1, const char *str2);

/**
@brief Wrapper for strncpy.
*/
char *wrapper_strncpy(char *destination, const char *source, int num);

/**
@brief Wrapper for strlen.
*/
int wrapper_strlen(const char *str);

/**
@brief Wrapper for memcpy.
*/
void *wrapper_memcpy(void *dst, const void *src, int count);

/**
@brief Wrapper for memset.
*/
void *wrapper_memset(void *dst, int value, int count);

/**
@brief Wrapper for memcmp.
*/
int wrapper_memcmp(const void *src, const void *dst, int size);

/**
@brief Wrapper for malloc.
*/
void *wrapper_malloc(int size);

/**
@brief Wrapper for realloc.
*/
void *wrapper_realloc(void *ptr, int size);

/**
@brief Wrapper for free.
*/
void wrapper_free(void *p);

/**
@brief Log level.
*/
enum {
  AL_LOG_LEVEL_DISABLE = 0,
  AL_LOG_LEVEL_FATAL,
  AL_LOG_LEVEL_ERROR,
  AL_LOG_LEVEL_WARN,
  AL_LOG_LEVEL_INFO,
  AL_LOG_LEVEL_MAX
};


#ifdef __cplusplus
}
#endif

#endif
