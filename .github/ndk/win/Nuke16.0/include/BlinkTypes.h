// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

/** \file BlinkTypes.h
 * \brief Defines some types used by the Blink API.
*/

#ifndef Blink_Library_BlinkTypes_h
#define Blink_Library_BlinkTypes_h
#ifdef __cplusplus
extern "C" {
#endif

typedef long long BlinkFlagType;

typedef BlinkFlagType BlinkCodegenFlags;
typedef BlinkFlagType BlinkDataType;
typedef BlinkFlagType BlinkPropertyType;
typedef BlinkFlagType BlinkErrorProperty;

// Kernel type (Iteration, Rolling, etc...)
typedef BlinkFlagType BlinkKernelType;

typedef BlinkFlagType BlinkKernelGranularity;

// Type of image access (Read or write)
typedef BlinkFlagType BlinkImageAccessType;

// Pattern (Point, Ranged1D, etc...)
typedef BlinkFlagType BlinkImagePatternType;
typedef BlinkFlagType BlinkAxisType;

// Edge (Constant, Clamped, ...)
typedef BlinkFlagType BlinkImageEdgeType;

struct BlinkObj;
typedef struct BlinkObj* BlinkRef;
typedef struct BlinkObj* BlinkErrorRef;

typedef int BlinkErrorCode;

typedef void* BlinkComputeDeviceID;

typedef int BlinkMemoryType;

/// \brief Rectangle
///
/// Rectangle covering range  (x1,y1) to (x2,y2)
/// x range is x1 to x2-1
/// y range is y1 to y2-1
struct BlinkRect
{
  int x1;
  int y1;
  int x2;
  int y2;
};

/// Pixel information
struct BlinkPixelInfo
{
  /// Number of components
  int nComponents;

  /// Data type
  BlinkDataType dataType;
};

/// Image description
struct BlinkImageInfo
{
  /// Bounds of image
  struct BlinkRect bounds;

  /// Pixel format
  struct BlinkPixelInfo pixelInfo;
};

/// Image iteration description
struct BlinkKernelIterationController
{

  /// Bounds of iteration
  struct BlinkRect bounds;

  //TODO component range? Step size?
};

struct BlinkBufferDesc
{
  unsigned long long pixelStepBytes;
  unsigned long long rowStepBytes;
  unsigned long long componentStepBytes;
};

struct BlinkDeviceMemory
{
  //Type of memory
  BlinkMemoryType type;

  //Pointer to the memory base object. For host memory this would be a pointer to the memory pointer i.e. a void**
  void* memoryBasePtr;
  // Size of memory base object
  unsigned long long memoryBaseSize;

  // Offset into memory - currently this must be zero
  unsigned long long offset;
};

/// General information about a kernel
struct BlinkKernelInfo
{
  // Name of the kernel
  const char* kernelName;

  // Type of the kernel (ImageComputation, Rolling, etc...)
  BlinkKernelType kernelType;

  BlinkKernelGranularity kernelGranularity;
};

/// General information about a image in a kernel (it only
/// contains the information that can be gathered by parsing
/// the kernel source (i.e. no information about size or pixel type).
struct BlinkKernelImageInfo
{
  // Name of the image
  const char* name;

  // Access type (eRead or eWrite)
  BlinkImageAccessType accessType;

  // Pattern type (ePoint, eRanged1D, etc...)
  BlinkImagePatternType patternType;

  // Edge Type (eEdgeNone, eEdgeClamp, etc...)
  BlinkImageEdgeType edgeType;
};

struct BlinkKernelParameterInfo
{
  const char* name;

  BlinkDataType type;

  unsigned nComponents;

  unsigned nElements;
};

struct BlinkImageAccessDesc
{
  BlinkImagePatternType patternType;
  BlinkAxisType axis;
  struct BlinkRect range;
};

#ifdef __cplusplus
}
#endif
#endif
