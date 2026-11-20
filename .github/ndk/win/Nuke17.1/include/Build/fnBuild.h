// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef __Build_fnBuild_h__
#define __Build_fnBuild_h__

#if !defined(FN_BUILD)
  #if defined(_WIN32)
    #define FN_OS_WINDOWS 1
  #elif defined(__APPLE__)
    #define FN_OS_MAC 1
  #elif defined(__sgi)
    #define FN_OS_IRIX 1
  #elif defined(__linux)
    #define FN_OS_LINUX 1
  #endif
#endif

#if defined(FN_OS_MAC)
  #include "Build/OS/fnMac.h"
#elif defined(FN_OS_LINUX)
  #include "Build/OS/fnLinux.h"
#elif defined(FN_OS_IRIX)
  #include "Build/OS/fnIRIX.h"
#elif defined(FN_OS_WINDOWS)
  #if !defined(__clang__)
    #include "Build/OS/fnWindows.h"
  #endif
#else
    #error "FN_OS_x not defined"
#endif

#ifdef __INTEL_COMPILER
  #define FN_COMPILER_INTEL
#endif

#ifdef __GNUC__
  #define FN_COMPILER_GNU
#endif

#ifdef __clang__
  #define FN_COMPILER_CLANG
#endif

#ifdef __MWERKS__
  #define FN_COMPILER_METROWERKS
#endif

#ifdef FN_OS_IRIX
  #ifndef FN_COMPILER_GNU
    #define FN_COMPILER_SGI
  #endif
#endif

// Detect building with C++11 support
#if __cplusplus >= 201103L
#define FN_CXX11
#endif

//! Display name of the "Control" key for tooltips etc - already defined on Mac
#ifndef FN_CTRLKEYNAME
  #define FN_CTRLKEYNAME "Ctrl"
#endif

#include "Build/fnMacros.h"
//#include <float.h>
//#include <iostream>
//#include <string>
//#include <vector>
//#include <map>

// FN_FUNCTION_NAME is a macro that defines the C++ macro to use for a function name
// this is __FUNCTION__ in MS compilers, and __PRETTY_FUNCTION__ on others.
#ifdef FN_COMPILER_VC
  #define FN_FUNCTION_NAME __FUNCTION__
#else
  #define FN_FUNCTION_NAME __PRETTY_FUNCTION__
#endif

#ifndef FN_OS_WINDOWS
# define mFnUnusedVariable __attribute__((unused))
#else
# define mFnUnusedVariable
#endif

#endif //__fnBuild_h__
