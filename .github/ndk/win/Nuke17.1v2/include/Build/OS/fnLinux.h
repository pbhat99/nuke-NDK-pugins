// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef __Build_fnLinux_h__
#define __Build_fnLinux_h__

#ifdef __GNUC__
  #include "Build/Compiler/fnGPlusPlus.h"
#endif

#ifdef __INTEL_COMPILER
//This looks a little weird doesn't it? Well, as Intel is a fly in replacement for g++,
//I'm assuming for now that this file is actually OK.
//Feel free to add an fnIntel.h if that seems appropriate. PRP.
  #include "Build/Compiler/fnGPlusPlus.h"
#endif

#if defined(FN_SHARED)
  #define FoundryAPI __attribute__ ((visibility("default")))
#else
  #define FoundryAPI
#endif

#include <float.h>
#include <values.h>

#endif
