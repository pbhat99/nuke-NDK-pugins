// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// Macros for defining symbol visibility

#ifndef USG_API_H
#define USG_API_H

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

#ifndef USG_API
  #if defined(FN_USG_EXPORTS) || defined(nuke_beta_EXPORTS)
    #define USG_API mFnSharedObjectExport
  #else
    #define USG_API FN_SHARED_IMPORT
  #endif
#endif

#if defined(_MSC_VER) && !defined(NDK_IGNORE_VISUAL_STUDIO_VERSION_CHECK)
  #if _MSC_VER < 1900
    #error Only plugins compiled with Visual Studio 2015 or later are supported
#endif
#endif


namespace usg
{

  // Version of underlying geometry USD API to cast geom impl() objects
  USG_API int usdAPIMajorVersion();
  USG_API int usdAPIMinorVersion();
  USG_API int usdAPIPathVersion();

  USG_API int usdAPIVersion();

  //! For plugins to test at runtime.
  inline bool usdVersionMatchesUsg(int version) { return (version == usdAPIVersion()); }

}

#endif
