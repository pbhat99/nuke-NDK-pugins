// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

/** \file BlinkC.h
 * \brief C functions used by the functions in Blink.h. NOTE: using these functions directly is not recommended, as they might change in future.
*/

#ifndef Blink_Library_BlinkC_h
#define Blink_Library_BlinkC_h
#include "BlinkBuild.h"
#include "BlinkTypes.h"
#ifdef __cplusplus
extern "C" {
#endif

#define BLINK_SUCCESS 0
#define BLINK_ERROR_ERROR 1
#define BLINK_ERROR_OUT_OF_MEMORY 2
#define BLINK_ERROR_RUNNING_KERNEL 3
#define BLINK_ERROR_PARSING_KERNEL 4
#define BLINK_ERROR_COMPILING_KERNEL 5
#define BLINK_ERROR_INVALID_IMAGE_SPEC 6
#define BLINK_ERROR_DEVICE 7
#define BLINK_ERROR_PARAMETER 8
#define BLINK_ERROR_INCOMPATIBLE_IMAGES 9
#define BLINK_ERROR_INVALID_PROPERTY 10

#define BLINK_PROPERTY_NAME 0


BLINK_SHARED_API BlinkRef Blink_Retain(BlinkRef ref);
BLINK_SHARED_API void Blink_Release(BlinkRef ref);

BLINK_SHARED_API const char* Blink_Error_GetUserMessage(BlinkErrorRef errorRef);
BLINK_SHARED_API BlinkErrorCode Blink_Error_GetStringProperty(BlinkErrorRef errorRef, BlinkErrorProperty property, const char** string);
BLINK_SHARED_API BlinkErrorCode Blink_Error_GetIntProperty(BlinkErrorRef errorRef, BlinkErrorProperty property, int* value);

BLINK_SHARED_API BlinkErrorCode Blink_Device_CurrentCPUDevice(BlinkRef* pRef, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Device_CurrentGPUDevice(BlinkRef* pRef, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Device_DeviceWithID(BlinkRef* pRef, BlinkComputeDeviceID id, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Device_Available(BlinkRef device, char* value, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Device_Bind(BlinkRef* pRef, BlinkRef device, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Device_GetProperty(BlinkRef device, BlinkPropertyType type, void* data, int* size, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Image_CreateEmptyImage(BlinkRef* pRef, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Image_CreateImage(BlinkRef* pRef, const struct BlinkImageInfo* imageInfo, BlinkRef device, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Image_ImageForDeviceMemory(BlinkRef* pRef, const struct BlinkDeviceMemory* deviceMemory, const struct BlinkImageInfo* imageInfo, const struct BlinkBufferDesc* layout, BlinkRef device, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Image_CopyRef(BlinkRef* pRef, BlinkRef src, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Image_DistributeTo(BlinkRef* pRef, BlinkRef src, BlinkRef device, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Image_MakeLike(BlinkRef* pRef, BlinkRef src, BlinkRef device, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Image_Info(BlinkRef image, struct BlinkImageInfo* info, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Image_Clear(BlinkRef image, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Image_IsEmpty(BlinkRef image, char* result, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Image_CopyFrom(BlinkRef dst, BlinkRef src, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Image_CopyFromBuffer(BlinkRef dst, const void* src, const struct BlinkBufferDesc* bufferDesc, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Image_CopyToBuffer(BlinkRef src, void* dst, const struct BlinkBufferDesc* bufferDesc, BlinkErrorRef* optionalErrorDesc);


BLINK_SHARED_API BlinkErrorCode Blink_IKernel_Make(BlinkRef* pRef, BlinkRef programSource, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_IKernel_GetKernelInfo(BlinkRef ref, struct BlinkKernelInfo* kInfo, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_IKernel_GetNumImages(BlinkRef ref, unsigned* nImgs, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_IKernel_GetNumParameters(BlinkRef ref, unsigned* nParams, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_IKernel_GetImageInfo(BlinkRef ref, unsigned index, struct BlinkKernelImageInfo* imgInfo, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_IKernel_GetParameterInfo(BlinkRef ref, unsigned index, struct BlinkKernelParameterInfo* paramInfo, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Kernel_Make(BlinkRef* pRef, BlinkRef programSource, BlinkRef dev, BlinkRef* images, int nImages, BlinkCodegenFlags flags, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Kernel_SetIntParam(BlinkRef ref, const char* name, const int* values, int nValues, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Kernel_SetFloatParam(BlinkRef ref, const char* name, const float* values, int nValues, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Kernel_SetBoolParam(BlinkRef ref, const char* name, const char* values, int nValues, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Kernel_GetIntParam(BlinkRef ref, const char* name, int* values, int nValues, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Kernel_GetFloatParam(BlinkRef ref, const char* name, float* values, int nValues, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Kernel_GetBoolParam(BlinkRef ref, const char* name, char* values, int nValues, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Kernel_GetNumInputRanges(BlinkRef ref, unsigned int* count, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Kernel_GetInputRanges(BlinkRef ref, struct BlinkImageAccessDesc* accessDescs, unsigned int maxDescs, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_Kernel_Iterate(BlinkRef ref, BlinkErrorRef* optionalErrorDesc);
BLINK_SHARED_API BlinkErrorCode Blink_Kernel_IterateWithController(BlinkRef ref, const struct BlinkKernelIterationController* controller, BlinkErrorRef* optionalErrorDesc);

BLINK_SHARED_API BlinkErrorCode Blink_ProgramSource_Make(BlinkRef* pRef, const char* source, BlinkErrorRef* optionalErrorDesc);


//This function may change before release.
BLINK_SHARED_API BlinkErrorCode Blink_Add_Precompiled_Kernel(const char* key,
                                                             const void* targetCodeData,
                                                             int targetCodeCompressedSize,
                                                             int targetCodeUncompressedSize,
                                                             const void* sharedLibData,
                                                             int sharedLibCompressedSize,
                                                             int sharedLibUncompressedSize,
                                                             const void* execInfoData,
                                                             int execInfoSize);


#ifdef __cplusplus
}
#endif
#endif
