// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// Macros for defining symbol visibility

#ifndef USG_ENGINE_API_H
#define USG_ENGINE_API_H

// Mac and Linux use the same export
#ifndef mFnSharedObjectExport
    #if defined(_WIN32)
        #define mFnSharedObjectExport __declspec(dllexport)
    #else
        #define mFnSharedObjectExport __attribute__ ((visibility("default")))
    #endif
#endif

#ifndef FN_SHARED_IMPORT
    #if defined(_WIN32)
        #define FN_SHARED_IMPORT __declspec(dllimport)
    #elif defined(__APPLE__)
        #define FN_SHARED_IMPORT mFnSharedObjectExport
    #else
        #define FN_SHARED_IMPORT
    #endif
#endif

#ifndef USG_ENGINE_API
  #if defined(FN_USG_ENGINE_EXPORTS) || defined(nuke_beta_EXPORTS)
    #define USG_ENGINE_API mFnSharedObjectExport
  #else
    #define USG_ENGINE_API FN_SHARED_IMPORT
  #endif
#endif

#if defined(_MSC_VER) && !defined(NDK_IGNORE_VISUAL_STUDIO_VERSION_CHECK)
  #if _MSC_VER < 1900
    #error Only plugins compiled with Visual Studio 2015 or later are supported
#endif
#endif

#endif
