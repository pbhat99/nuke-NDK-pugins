// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef Build_fnStaticAssert_h
#define Build_fnStaticAssert_h

#include "Build/fnBuild.h"

#if defined(FN_CXX11)

#define mFnAssertStatic(expression) static_assert(expression, "");

#else

#define mFnConcatenate(arg1, arg2)   mFnConcatenate1(arg1, arg2)
#define mFnConcatenate1(arg1, arg2)  mFnConcatenate2(arg1, arg2)
#define mFnConcatenate2(arg1, arg2)  arg1##arg2

/**
 * Usage:
 *
 * <code>mFnAssertStatic(expression)</code>
 *
 * When the static assertion test fails, a compiler error message that somehow,
 * contains the "STATIC_ASSERTION_FAILED_AT_LINE_xxx" is generated.
 *
 * mFnAssertStatic(true);
 */

#define mFnAssertStatic(expression)                                                                                                                                       \
  struct mFnConcatenate(__static_assertion_at_line_, __LINE__)                                                                                                            \
  {                                                                                                                                                                       \
    Foundry::Asserts::StaticAssert<static_cast<bool>((expression))> mFnConcatenate(mFnConcatenate(STATIC_ASSERTION_FAILED_AT_LINE_, __LINE__), _);                        \
  };                                                                                                                                                                      \
  typedef Foundry::Asserts::StaticAssertTest<sizeof(mFnConcatenate(__static_assertion_at_line_, __LINE__))> mFnConcatenate(__static_assertion_test_at_line_, __LINE__);   

  // note that we wrap the non existing type inside a struct to avoid warning
  // messages about unused variables when static assertions are used at function
  // scope
  // the use of sizeof makes sure the assertion error is not ignored by SFINAE

namespace Foundry {
  namespace Asserts {

  template <bool>
  struct StaticAssert;

  template <>
  struct StaticAssert<true>
  {
  }; // StaticAssert<true>

  template<int i>
  struct StaticAssertTest
  {
  }; // StaticAssertTest<int>

  } // namespace Asserts
} // namespace Foundry

#endif //FN_CXX11

#endif // Build_fnStaticAssert_h

