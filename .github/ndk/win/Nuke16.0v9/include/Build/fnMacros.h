// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef __fnMacros_h__
#define __fnMacros_h__

// This file is intended to be a place to define useful macros that
// are not OS/Compiler specific.  Anything that *is* compiler specific
// should go into fnBuild.h, as it gives the OS/Compiler specific
// headers a chance to take over. The stuff in here doesn't need
// overriding, and therefore this stuff can be used without including
// fnBuild.h, and therefore <windows.h>, which'll just slow things
// down.

#include "Build/fnAssert.h"

#ifdef __cplusplus
  // The following header uses c++ features and since we might be
  // compiling c code we have to be careful when to include it.
  #include "Build/fnStaticAssert.h"
#endif // __cplusplus

#ifndef mFnUnused
  #define mFnUnused(A)
#endif

#ifndef mFnStringize
  #define mFnStringize2(A) # A
  #define mFnStringize(A) mFnStringize2(A)
#endif

#define mFnSizeofArray(arrayName) (sizeof(arrayName) / sizeof(arrayName[0]))

//! Creates a local buffer and writes a variable argument string to it.  Useful for printf-style functions.
#define mFnWriteVarArgString(bufVarName, bufSize, formatParam)  char bufVarName[bufSize];                               \
                                                                {                                                       \
                                                                  va_list lArgs;                                        \
                                                                  va_start(lArgs, formatParam);                         \
                                                                  vsnprintf(bufVarName, bufSize, formatParam, lArgs);   \
                                                                  va_end(lArgs);                                        \
                                                                }

#define mFnAlignSize(size, alignment) ((size + (alignment - 1)) & ~(alignment - 1))

//! Set bit n only 
#define mFnBIT(n) (1<<(n))

#endif // __fnMacros_h__

