// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

/** \file BlinkConstants.h
 * \brief Defines some constants for use with the Blink API.
*/

#ifndef Blink_Library_BlinkConstants_h
#define Blink_Library_BlinkConstants_h

#include "BlinkBuild.h"

#include "BlinkC.h"

#ifdef __cplusplus
extern "C" {
#endif


BLINK_SHARED_API extern BlinkCodegenFlags kBlinkCodegenDefault;
BLINK_SHARED_API extern BlinkCodegenFlags kBlinkCodegenVectorize;


BLINK_SHARED_API extern BlinkDataType kBlinkDataNone;
BLINK_SHARED_API extern BlinkDataType kBlinkDataUByte;
BLINK_SHARED_API extern BlinkDataType kBlinkDataUShort;
BLINK_SHARED_API extern BlinkDataType kBlinkDataUInt;
BLINK_SHARED_API extern BlinkDataType kBlinkDataFloat;

BLINK_SHARED_API extern BlinkDataType kBlinkParamBool;
BLINK_SHARED_API extern BlinkDataType kBlinkParamInt;
BLINK_SHARED_API extern BlinkDataType kBlinkParamFloat;

BLINK_SHARED_API extern BlinkErrorProperty kBlinkErrorPropertyLineNumber;
BLINK_SHARED_API extern BlinkErrorProperty kBlinkErrorPropertyCompilationErrors;


BLINK_SHARED_API extern BlinkMemoryType kBlinkMemoryTypeHostPtr;
BLINK_SHARED_API extern BlinkMemoryType kBlinkMemoryTypeDeviceBuffer;

BLINK_SHARED_API extern BlinkKernelType kBlinkKernelTypeNone;
BLINK_SHARED_API extern BlinkKernelType kBlinkKernelTypeIterate;
BLINK_SHARED_API extern BlinkKernelType kBlinkKernelTypeReduce;
BLINK_SHARED_API extern BlinkKernelType kBlinkKernelTypeRolling;

BLINK_SHARED_API extern BlinkKernelGranularity kBlinkKernelPixelWise;
BLINK_SHARED_API extern BlinkKernelGranularity kBlinkKernelComponentWise;

BLINK_SHARED_API extern BlinkImageAccessType kBlinkImageAccessRead;
BLINK_SHARED_API extern BlinkImageAccessType kBlinkImageAccessWrite;

BLINK_SHARED_API extern BlinkImagePatternType kBlinkImagePatternNone;
BLINK_SHARED_API extern BlinkImagePatternType kBlinkImagePatternPoint;
BLINK_SHARED_API extern BlinkImagePatternType kBlinkImagePatternRanged1D;
BLINK_SHARED_API extern BlinkImagePatternType kBlinkImagePatternRanged2D;
BLINK_SHARED_API extern BlinkImagePatternType kBlinkImagePatternRandom;

BLINK_SHARED_API extern BlinkAxisType kBlinkAxisX;
BLINK_SHARED_API extern BlinkAxisType kBlinkAxisY;

BLINK_SHARED_API extern BlinkImageEdgeType kBlinkImageEdgeNone;
BLINK_SHARED_API extern BlinkImageEdgeType kBlinkImageEdgeNull;
BLINK_SHARED_API extern BlinkImageEdgeType kBlinkImageEdgeClamp;
BLINK_SHARED_API extern BlinkImageEdgeType kBlinkImageEdgeConstant;

#ifdef __cplusplus
}
#endif
#endif
