// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef __Build_fnMac_h__
#define __Build_fnMac_h__

// We need to check for clang first since it also defines __GNUC__.
#ifdef __clang__
  #include "Build/Compiler/fnClang.h"
#elif defined(__GNUC__)
  #include "Build/Compiler/fnGPlusPlus.h"
#endif // __clang__

#if defined(FN_SHARED)
  #define FoundryAPI __attribute__ ((visibility("default")))
#else
  #define FoundryAPI
#endif


#ifdef __INTEL_COMPILER
//This looks a little weird doesn't it? Well, as Intel is a fly in replacement for g++,
//I'm assuming for now that this file is actually OK.
//Feel free to add an fnIntel.h if that seems appropriate. PRP.
  #include "Build/Compiler/fnGPlusPlus.h"
#endif

#define FN_CTRLKEYNAME "Cmd"

#endif
