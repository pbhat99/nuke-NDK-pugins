// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef __Build_fnWindows_h__
#define __Build_fnWindows_h__

#ifndef NTDDI_VERSION
  #define NTDDI_VERSION NTDDI_WIN7
  #define _WIN32_WINNT 0x0502
#endif

#ifdef _MSC_VER
  #define FN_COMPILER_VC
  #if defined(_MSC_VER) && (_MSC_VER < 1400)
    #error Visual Studio 2003 or older are not supported
  #endif

  #if defined(_MSC_VER) && (_MSC_VER >= 1400)
    #define FN_COMPILER_VC_8
  #endif

  #if defined(_MSC_VER) && (_MSC_VER == 1400)
    #define FN_COMPILER_VC_8_0
    #define FN_COMPILER_VERSION 800
  #endif

#endif

#ifdef FN_COMPILER_VC_8
  #include "Build/Compiler/fnVC8.h"
#endif

#ifdef __GNUC__
  #include "Build/Compiler/fnGPlusPlus.h"
  #define FN_COMPILER_GNU
#endif

#ifndef  FN_PROCESSOR_INTEL
  #define FN_PROCESSOR_INTEL
#endif

#ifndef mFnThrowSpec
  #define mFnThrowSpec(...)
#endif

#ifndef FN_STATIC
  #if defined(FN_SHARED)
    #define FoundryAPI
  #else
    #define FoundryAPI
  #endif
#else
  #define FoundryAPI
#endif

#endif
