// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef __fnVC_h__
#define __fnVC_h__

#include <stdlib.h>

#ifndef FN_SHARED_EXPORT
#define FN_SHARED_EXPORT __declspec(dllexport)
#endif

#ifndef mFnSharedObjectExport
#define mFnSharedObjectExport FN_SHARED_EXPORT
#endif

#ifndef FN_SHARED_IMPORT
#define FN_SHARED_IMPORT __declspec(dllimport)
#endif

#ifndef FN_NOINLINE
// Force non-inline functions
#define FN_NOINLINE __declspec(noinline)
#endif

#ifndef NAME_MAX
  #define NAME_MAX ( _MAX_FNAME + _MAX_EXT )
#endif

#ifndef PATH_MAX
  #define PATH_MAX ( 1024 )
#endif

#ifndef _MT
  #error TURN ON THE MULTITHREADED FLAG
#endif

#ifdef __cplusplus
  #ifndef _CPPUNWIND
    #error TURN ON EXCEPTION HANDLING
  #endif // _CPPUNWIND
#endif // __cplusplus

/* force w2k api to be used: */
#if WINVER < 0x0500
  #define WINVER 0x500
#endif
#if _WIN32_WINNT < 0x500
  #define _WIN32_WINNT 0x500
#endif

//VC++ warn you when an overload is resolved by argument type, i.e.
//void f(int a) {a++;}
//void f(float a) {a++;}
//and you call f(int(0)), you'll get a warning. AS long as it's the right thing, who cares?
#pragma warning( disable : 4675 )

#ifndef NOMINMAX
#error NOMINMAX must be added as a compile definition!
#endif

#ifndef _USE_MATH_DEFINES
#error _USE_MATH_DEFINES must be added as a compile definition!
#endif

#ifndef mFnAlign
#define mFnAlign(ALIGNMENT) __declspec(align(ALIGNMENT))
#endif

/* Function Deprecation macro
 * The message parameter should be a string detailing how to replace the function.
 */
#define mFnDeprecatedFunc(message, func) __declspec(deprecated(message)) func

#endif // __fnVC_h__

