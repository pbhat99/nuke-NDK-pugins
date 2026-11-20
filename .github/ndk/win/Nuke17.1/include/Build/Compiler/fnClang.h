// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef __Build_fnClang_h__
#define __Build_fnClang_h__

#ifdef __GNUC__
  #include <float.h>
#endif
#ifdef __GNUG__
  #include <cfloat>
#endif

#define mFnAlign(ALIGNMENT) __attribute__((aligned(16)))

#ifndef FN_SHARED_EXPORT
#define FN_SHARED_EXPORT __attribute__ ((visibility("default")))
#endif

#ifndef mFnSharedObjectExport
#define mFnSharedObjectExport FN_SHARED_EXPORT
#endif

#ifndef FN_SHARED_IMPORT
// FIXME With GCC this macro expands to nothing however with clang that causes
// problems with dynamic_casts and exceptions. The only way I've been to fix those
// is to use the same visibility for import and export, as described in
// https://gcc.gnu.org/wiki/Visibility. I couldn't find any clang specific documentation
// about this.
#define FN_SHARED_IMPORT mFnSharedObjectExport
#endif

#ifndef FN_NOINLINE
// Force non-inline functions
#define FN_NOINLINE __attribute__ ((noinline))
#endif

/* Function Deprecation macro
 * The message parameter should be a string detailing how to replace the function.
 * The attribute is set in this order, despite the docs saying to do otherwise,
 * as it allows functions defined in the following ways to be marked deprecated :
 * be marked as deprecated : 
 *    func() {} - dec & body
 *    func();   - dec only 
 *
 * Tested against :
 *    OSX gcc 4.2.1
 *    OSX clang 4.2 (LLVM 3.2)
 *    Linux gcc 4.1.2
 */
#if __GNUC__ > 4 || \
    (__GNUC__ == 4 && (__GNUC_MINOR__ > 4 ) )
// Test for GCC >= 4.5 which supports messages being passed to the deprecated attribute
# define mFnDeprecatedFunc(message, func) __attribute__ ((deprecated(message))) func
#else
// messages in deprecated warning not supported before gcc-4.5
# define mFnDeprecatedFunc(message, func) __attribute__ ((deprecated)) func
#endif // > gcc-4/5

#endif // __Build_fnClang_h__

