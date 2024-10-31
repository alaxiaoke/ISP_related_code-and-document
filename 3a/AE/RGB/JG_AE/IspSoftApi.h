#ifndef __ISP_SOFT_API_H__
#define __ISP_SOFT_API_H__


#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C"
{
#endif


    #ifndef ISP_SOFT_CALL_TYPE
    #define ISP_SOFT_CALL_TYPE                    __stdcall
    #endif


    #ifdef ISP_SOFT_EXPORTS
        #define ISP_SOFT_API(returnType)        __declspec(dllexport) returnType ISP_SOFT_CALL_TYPE
    #elif ISP_SOFT_DIRECT_CALL
        #define ISP_SOFT_API(returnType)        returnType
    #else
        #define ISP_SOFT_API(returnType)        __declspec(dllimport) returnType ISP_SOFT_CALL_TYPE
    #endif


    typedef void *IspSoftHandle;


    typedef int32_t (*IspSoftPostMessageFunc)(void *owner, uint32_t postType, void *data);


    typedef IspSoftHandle (*IspSoftInitFunc)(void *owner, IspSoftPostMessageFunc postMessage, void *data);


    typedef int32_t (*IspSoftProcessFunc)(IspSoftHandle handle, uint32_t processType, void *data);


    typedef int32_t (*IspSoftDeinitFunc)(IspSoftHandle handle);


    typedef struct IspSoftModule
    {
        IspSoftInitFunc Init;
        IspSoftProcessFunc Process;
        IspSoftDeinitFunc Deinit;
    } IspSoftModule;


    typedef struct IspSoftBuffer
    {
        int32_t Length;
        uint8_t *Data;
    } IspSoftBuffer;


    typedef enum IspSoftImageFormat
    {
        ISP_SOFT_IMAGE_RAW8 = 0,
        ISP_SOFT_IMAGE_RAW10_PACK,
        ISP_SOFT_IMAGE_RAW10_UNPACK,
        ISP_SOFT_IMAGE_YUV420,
        ISP_SOFT_IMAGE_RGB888
    } IspSoftImageFormat;


    typedef struct IspSoftImageSize
    {
        int32_t Width;
        int32_t Height;
    } IspSoftImageSize;


    typedef struct IspSoftImageInfo
    {
        IspSoftImageFormat Format;
        IspSoftImageSize Size;
    } IspSoftImageInfo;


#ifdef __cplusplus
}
#endif


#endif