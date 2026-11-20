// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_MATH_H
#define FDK_MATH_MATH_H

#include "fdk/api.h"

#include "fdk/math/half.h" // for Vec2,Vec3,etc templates

#include <algorithm>
#include <cmath> // For M_PI_2, etc
#include <type_traits>
#include <vector>

#ifdef _WIN32

  #ifndef M_PI_2
    #define M_PI_2    1.57079632679489661923
  #endif
  #ifndef M_2_PI
    #define M_2_PI    0.63661977236758134308
  #endif

#endif

namespace fdk {

  //! Convert degrees to radians.
  template <typename T>
  inline T     radians(T degrees) { return degrees * static_cast<T>(M_PI_2 / 90); }

  //! Convert radians to degrees.
  template <typename T>
  inline T     degrees(T radians) { return radians * static_cast<T>(90 * M_2_PI); }

  //! Linear-interpolate between two scalars at t, where t=0..1, clamping
  //! between \a s0 and \a s1.
  template<typename S, typename T>
  S lerpClamped(const S& s0, const S& s1, T t)
  {
    if (t < std::numeric_limits<S>::epsilon()) {
      return s0; // before or at first
    }
    else if (t > (static_cast<S>(1) - std::numeric_limits<S>::epsilon())) {
      return s1; // at or after last
    }
    return static_cast<S>(s0 * (1 - t) + s1 * t);
  }

  //! Linear-interpolate between two scalars at t, where t=0..1, clamping
  //! between \a s0 and \a s1.
  template<typename S, typename T>
  S lerpUnclamped(const S& s0, const S& s1, T t)
  {
    return static_cast<S>(s0 * (1 - t) + s1 * t);
  }

  //! Inverts a scalar. The same as 1 - \a v.
  template <typename T>
  inline T invert(T v) { return (static_cast<T>(1) - v); }

  //! Clamps \a v so that it is within the \a min to \a max range.
  template <typename T, typename S>
  inline T clamp(T v, S min, S max) { return std::clamp<T>(v, min, max); }

  //! Clamps \a v so that it is within 0 and 1.
  template <typename T>
  inline T clamp(T v) { return std::clamp<T>(v, 0, 1); }

  //! Returns true if val1 is nearly val2, within the threshold range.
  template <typename T>
  inline bool  isNearly(T val1, T val2, T threshold =std::numeric_limits<T>::epsilon()) { return (::fabs(val1 - val2) <= threshold); }

  //! Returns true if value is nearly 0, within the threshold range.
  template <typename T>
  inline bool  isNearlyZero(T v, T threshold =std::numeric_limits<T>::epsilon()) { return (::fabs(v) <= threshold); }

  //! Returns true if value is nearly 1, within the threshold range.
  template <typename T>
  inline bool  isNearlyOne(T v, T threshold =std::numeric_limits<T>::epsilon()) { return (::fabs(static_cast<T>(1) - v) <= threshold); }

  template <typename T>
  constexpr bool is_arithmetic = std::is_arithmetic<T>::value || std::is_same<fdk::half, typename std::remove_cv<T>::type>::value;

  template <typename T>
  constexpr bool is_floating_point = std::is_floating_point<T>::value || std::is_same<fdk::half, typename std::remove_cv<T>::type>::value;

}

#endif
