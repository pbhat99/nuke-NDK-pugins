// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef Build_fnAssert_h
#define Build_fnAssert_h

#pragma once

// This file is intended to be a place to define assert macros and
// utilities that are not OS/Compiler specific.
// This header must compile in isolating without any cpp depedancies as 
// it will be shipped with the Nuke NDK

// NOTE: If FN_USE_ASSERT_LOGGING is on it requires that Core/Build be built as
//       a library and linked in
#if FN_USE_ASSERT_LOGGING
void internalFnAssertHandler( const char* const expr, const int line, const char* const file, const char* const msg);

#  define mFnAssertMsg(EXP, MSG)                                  \
    do{                                                           \
      if(!(mFnHintExpectFalse(EXP))) {                            \
        internalFnAssertHandler( #EXP, __LINE__, __FILE__, MSG ); \
        FN_USE_ASSERT_LOGGING(EXP);                               \
      }                                                           \
    }while(0)                                                     

#  define mFnAssert(EXP) mFnAssertMsg(EXP, "")

   // No implementation required as the above mFnAssert and mFnAssertMsg reference variables in the expression
   // in release builds.
#  define mFnOnlyUsedInAssert(VARIABLE)

   // The following macro should be used rare and only in performance critical code
#  define mFnAssertFast(EXP) assert(EXP)

#else // FN_USE_ASSERT_LOGGING

#ifdef NDEBUG
#  define mFnAssert(EXPR) ((void)0)
#else

// mFnAssert/mFnAssertMsg - A soft-assert i.e. continuable.
// Expects the condition to be false for performance reasons (asserts are
// checking that the condition is false) and sets a hardware-breakpoint if it
// isn't
#  define mFnAssert(EXPR)                       \
      do {                                      \
        if(mFnHintExpectFalse(!(EXPR))) {       \
          mFnHardwareBreakpoint();              \
        }                                       \
      } while(0)
#endif

#  define mFnAssertMsg(EXP, MSG) mFnAssert(EXP)
#  define mFnAssertFast(EXP) mFnAssert(EXP)

   // Pass in local variables that are only referenced inside mFnAssert() to avoid unreferenced compiler warnings in release builds.
   // The use of a macro makes the intention in the user code clearer (and the 'fix' easily changeable if necessary).
#  ifdef FN_DEBUG
#    define mFnOnlyUsedInAssert(VARIABLE)
#  else
#    define mFnOnlyUsedInAssert(VARIABLE) (void)VARIABLE
#  endif

#endif // FN_USE_ASSERT_LOGGING


#if defined(FN_OS_WINDOWS)

   // Hardware breakpoint on WINDOWS - useful for continuable asserts
   // use MSVC's inbuilt, and very useful, __debugbreak() to set a hardware
   // breakpoint avoiding the need for avoiding the need for the equivalent
   // asm("int $3"); and the need for the system debug tools to be installed
#  define mFnHardwareBreakpoint() do { __debugbreak(); }while(0)

   // intended to help the compiler make better branch-predictions in
   // situations where we know the more common branch will be the `else` branch
   // We could/should be using __assume() here but it's dangerous and has side
   // effects, see the docs for __assume()
#  define mFnHintExpectFalse(EXPR) ( EXPR )

#  include <intrin.h> // for the __debugbreak intrinsic

#elif defined(FN_OS_LINUX) || defined(FN_OS_MAC)

   // Hardware breakpoint on LINUX-like systems - useful for continuable
   // asserts
   // raise a sigtrap on linux and mac, avoiding the need for asm("int $3");
   // which requires extra compiler options to enable ASM, -fasm-blocks, at
   // least on NUKE's build
#  define mFnHardwareBreakpoint() do { ::raise(SIGTRAP); }while(0)

   // help the compiler make better branch-predictions in situations where we
   // know the more common branch will be the `else` branch
   // NOTE: see the differences in the WINDOWS version before using this
#  define mFnHintExpectFalse(EXPR) ( __builtin_expect((EXPR), false) )

   // signal.h required for SIGTRAP
#  include <signal.h>

#else // defined(FN_OS_WINDOWS)

// all other OSs
#  define mFnHardwareBreakpoint() do { assert(false); }while(0)
#  define mFnHintExpectFalse(EXPR) ( EXPR )

#endif // defined(FN_OS_WINDOWS)

#endif // Build_fnAssert_h

