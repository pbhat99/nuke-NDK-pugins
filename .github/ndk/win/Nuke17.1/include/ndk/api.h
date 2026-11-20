// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

// Macros for defining symbol visibility

#ifndef NDK_API_H
#define NDK_API_H

#include "Build/fnBuild.h"

#ifndef NDK_API
  #if defined(FN_NDK_EXPORTS) || defined(nuke_beta_EXPORTS)
    #define NDK_API mFnSharedObjectExport
  #else
    #define NDK_API FN_SHARED_IMPORT
  #endif
#endif

#if defined(_MSC_VER) && !defined(NDK_IGNORE_VISUAL_STUDIO_VERSION_CHECK)
  #if _MSC_VER < 1900
    #error Only plugins compiled with Visual Studio 2015 or later are supported
#endif
#endif

#endif
