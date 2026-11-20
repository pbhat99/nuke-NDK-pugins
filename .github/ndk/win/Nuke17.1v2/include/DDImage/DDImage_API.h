/* Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved. */

/* Windows compatibility.
   When compiling the library DDImage_EXPORTS is defined with -D
   When compiling programs DDImage_EXPORTS is undefined
 */

#ifndef DDImage_API_H
#define DDImage_API_H

#include "Build/fnBuild.h"

#ifndef DDImage_API
  #if defined(DDImage_EXPORTS) || defined(DDImage_beta_EXPORTS)
    #define DDImage_API mFnSharedObjectExport
  #else
    #define DDImage_API FN_SHARED_IMPORT
  #endif
#endif

#if defined(_MSC_VER) && !defined(NDK_IGNORE_VISUAL_STUDIO_VERSION_CHECK)
  #if _MSC_VER < 1900
    #error Only plugins compiled with Visual Studio 2015 or later are supported
#endif
#endif


/* Emulate newer GCC's symbol for endianess. */
#if !defined(__BIG_ENDIAN__) && defined(__sgi)
  #define __BIG_ENDIAN__ 1
#endif

/*! A type useable for storing 64 bits. */
#ifdef _WIN32
typedef unsigned __int64 U64;
#else
typedef unsigned long long U64;
#endif

/*! A type commonly found in image files. */
typedef unsigned int U32;

/*! A type commonly found in image files. */
typedef unsigned short U16;

/*! A type commonly found in image files. */
typedef unsigned char uchar;

#endif

/* Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved. */
