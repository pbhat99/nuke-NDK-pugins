// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file DDMath.h

   \brief Use <DDImage/DDMath.h> instead of <math.h> to make portable programs.

   This header file only contains inline functions and thus can be used
   in any code, whether or not it is linked with the DDImage library. It's
   primary purpose is to fix deficiencies in various platforms math header
   files.

   \li Defines symbols and functions that are in the Linux math.h that
     are missing from NT.

   \li Defines 'f' forms of all math functions that take and return a float
     on systems that don't have them.

   \li Defines a number of templates and inline functions to do max, min,
     noise, clamping, and many functions from RenderMan and other shading
     languages.

   See also <noise.h>

 */

#ifndef DDImage_math_h
#define DDImage_math_h

#include <math.h>
#include <float.h>
// We need to include stdlib here since it includes some math-ish
// functions like drand48 which is macro'd below if it doesn't
// exist. It seems that this was the case on DD's machines but not
// in my install of the latest cygwin. Maybe they're on an older
// version? So we'll check here but probably some day in the future
// this could come out along with some of the defines below.
#include <stdlib.h>
#include "Build/fnMacros.h"

// Warning: fast_rint and fast_floor assume int is a 32 bit integer!
static_assert(sizeof(int) == 4, "");

#define rintf(v)    floorf((v) + .5f)

#ifdef _WIN32

  #ifndef M_PI
    #define M_PI        3.14159265358979323846
  #endif
  #ifndef M_PI_2
    #define M_PI_2    1.57079632679489661923
  #endif
  #ifndef M_PI_4
    #define M_PI_4    0.78539816339744830962
  #endif
  #ifndef M_1_PI
    #define M_1_PI    0.31830988618379067154
  #endif
  #ifndef M_2_PI
    #define M_2_PI    0.63661977236758134308
  #endif
  #ifndef M_SQRT2
    #define M_SQRT2    1.41421356237309504880
  #endif
  #ifndef M_SQRT1_2
    #define M_SQRT1_2    0.70710678118654752440
  #endif
  #ifndef M_LOG2E
    #define M_LOG2E  1.4426950408889634074
  #endif

  #ifndef _WIN32
    #ifndef rint
      inline double rint(const double v){ return floor((v) + .5);}
      inline float rint(const float v){ return floorf((v) + .5f);}
    #endif
  #endif

  #ifndef drand48
    #define drand48()    ((double)rand() / RAND_MAX)
  #endif
  #ifndef srand48
    #define srand48(n)    srand((n));
  #endif

#endif


// Float constants
#define M_PI_F      float(M_PI)
#define M_PI_2_F    float(M_PI_2)
#define M_PI_4_F    float(M_PI_4)
#define M_1_PI_F    float(M_1_PI)
#define M_2_PI_F    float(M_2_PI)
#define M_SQRT2_F   float(M_SQRT2)
#define M_SQRT1_2_F float(M_SQRT1_2)


#ifdef __FreeBSD__
extern "C" {
extern double drand48(void);
extern void srand48(long int);
}
  #define asinf(a)    asin(a)
  #define acosf(a)    acos(a)
  #define atanf(a)    atan(a)
#endif

#ifdef __alpha // powf is broken on alphas, at least in gcc
  #define powf(a, b)    (float)pow(a, b)
#endif

#if NO_SINCOS
  #define sincos(angle, s, c) do { *(s) = sin(angle); *(c) = cos(angle); } \
  while (0)
  #define sincosf(angle, s, c) sincos(angle, s, c)
#endif

#if __BIG_ENDIAN__
  #define iman_ 1
#else
  #define iman_ 0
#endif

#ifdef __APPLE__
  #define mFnVolatile volatile
#else
  #define mFnVolatile
#endif

union Double_Long {
  mFnVolatile double v;
  mFnVolatile int word[2];
};

#undef mFnVolatile

/*! Fast version of (int)rint().
   Works for -2147483648.5 .. 2147483647.49975574019,
   Requires IEEE floating point.
 */
inline long fast_rint(double val)
{
  Double_Long v;
  v.v = val + 68719476736.0 * 65536.0 * 1.5;
  return v.word[iman_];
}

/*! Fast version of (int)floor().
   Works for -32728 to 32727.99999236688,
   rounds numbers greater than n.9999923668 to n+1 rather than n.
   Requires IEEE floating point.
 */
inline long fast_floor(double val)
{
  /*! This doesn't work for numbers outside the bounds, but is
    4x faster than standard floor within those bounds. Adding this
    bounds check doesn't seem to affect its performance, so this allows us
    to keep using the function
  */
  if (val > 32728.0 || val < -32728.0)
      return (long)floor(val);

  Double_Long v;
  v.v = val + (68719476736.0 * 1.5);
#if 0
  // This alternative works for -2147483648 to 2147483647.999923688
  return (long)(v.big >> 16);
#else
  return v.word[iman_] >> 16;
#endif
}

#ifndef INFINITY
  #ifdef HUGE_VALF
    #define INFINITY HUGE_VALF
  #else
static const unsigned inf_bits = 0x7f800000;
    #define INFINITY (*(float*)(&inf_bits))
  #endif
#endif

#ifndef NAN
static const unsigned nan_bits = 0x7fc00000;
  #define NAN (*(float*)(&nan_bits))
#endif

/*! Convert degrees to radians. */
inline double radians(double degrees) { return degrees * (M_PI_2 / 90); }

/*! Convert degrees to radians, returns a float. */
inline float radiansf(float degrees) { return degrees * float(M_PI_2 / 90); }

/*! Convert radians to degrees. */
inline double degrees(double radians) { return radians * (90 * M_2_PI); }

/*! Convert radians to degrees, returns a float. */
inline float degreesf(float radians) { return radians * float(90 * M_2_PI); }

#undef MIN
#undef MAX
#undef clamp

/*! Takes the minimum of two things with a '<' operator. Returns \a b
   if either \a a or \a b is NaN. The function name is uppercase to
   avoid collisions with the many vender header files that define a
   macro or other template called min().
 */
template <class Type>
inline Type MIN(const Type& a, const Type& b) { return a < b ? a : b;  }

/*! Takes the maximum of two things with a '<' operator. Returns \a b
   if either \a a or \a b is NaN. The function name is uppercase to
   avoid collisions with the many vender header files that define a
   macro or other template called max().
 */
template <class Type>
inline Type MAX(const Type& a, const Type& b) { return b < a ? a : b;  }

/*! Restricts \a a to the range \a b through \a c for any type with a
   '<' operator. Returns \a b if either \a a or \a b or \a c is NaN. */
template <class Type, class Typeb>
inline Type clamp(Type a, Typeb b, Typeb c)
{
  return (b < a) ? (a < c ? a : (Type)c) : (Type)b;
}

/*! Same as clamp(a,0,1) */
template <class Type>
inline Type clamp(Type a)
{
  return (0 < a) ? (a < 1 ? a : (Type)1) : (Type)0;
}

/*! Returns 0 if \a x is less than \a a, returns 1 otherwise. Matches
   the function in RenderMan and other shading languages.
 */
template <class Type, class Typeb>
inline bool step(Type a, Typeb x) { return a < x; }

/*! Returns 0 if \a x is less than \a a, returns 1 if \a x is greater
   or equal to \a b, returns a smooth cubic interpolation otherwise.
   Matches the function in RenderMan and other shading languages.
 */
template <class Type, class Typeb>
inline Type smoothstep(Type a, Type b, Typeb x)
{
  Type t = (Type(x) - a) / (b - a);
  return t > 0 ? t < 1 ? (3 - 2 * t) * t * t : (Type)1 : (Type)0;
}

/*! Returns a point on the line f(x) where f(0)==a and f(1)==b. Matches
   the function in RenderMan and other shading languages.
 */
template <class Type, class Typeb>
inline Type lerp(Type a, Type b, Typeb x)
{
  return Type(a * (1 - x) + b * x);
}

#if !defined _WIN32 && !defined FN_OS_MAC
  #ifndef signbit
// fix stupid mess in gcc headers so signbit() works again!
using namespace __gnu_cxx;
  #endif
#endif

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
