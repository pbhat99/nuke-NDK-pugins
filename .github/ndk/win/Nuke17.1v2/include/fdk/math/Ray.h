// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_RAY_H
#define FDK_MATH_RAY_H

#include "fdk/math/Vec3.h"
#include "fdk/math/Mat4.h"

#include <limits>

namespace fdk {

  //! A ray cast from a point in a direction.
  template<class T>
  class Ray {
  public:
    Vec3<T> origin; //!< The origin of the ray
    Vec3<T> direction; //! The direction of the ray

    T minT; //!< The minimum distance for the ray
    T maxT; //!< The maximum distance for the ray

    T time; //!< The current time for the ray

    uint32_t typeMask;  //!< Ray-type mask

    //! Default constructor leaves everything uninitialized
    Ray();

    //! Construct a Ray from start point, direction and start/end distances
    Ray(const Vec3<T>& origin, const Vec3<T>& direction, T start = 0, T end = std::numeric_limits<T>::max());

    //! Assign the Ray's spatial and temporal values.
    void set(const Vec3<T>& origin, const Vec3<T>& direction, T time,
             T start = 0, T end = std::numeric_limits<T>::max());

    //! Return the ray position along the direction vector at distance \a t
    Vec3<T> operator() (T t) const;

    //! Return the position at the start distance
    Vec3<T> start() const;

    //! Return the position at the end distance
    Vec3<T> end() const;

    //! Transform the ray by matrix \a m
    Ray transform(const Mat4<T>& m) const;

    //! Transform the ray by two matrices, interpolated by \a t
    Ray transform(const Mat4<T>& m0, const Mat4<T>& m1, float t) const;
  };


  template<class T>
  inline
  Ray<T>::Ray()
  {
  }

  template<class T>
  inline Ray<T>::Ray(const Vec3<T>& src, const Vec3<T>& dir, T start, T end)
    : origin(src)
    , direction(dir)
    , minT(start)
    , maxT(end)
    , time(0)
    , typeMask(0x01/*camera-ray*/)
  {
  }

  template<class T>
  inline void Ray<T>::set(const Vec3<T>& o, const Vec3<T>& d, T t, T start, T end)
  {
    origin = o;
    direction = d;
    time = t;
    minT = start;
    maxT = end;
  }

  template<class T>
  inline Vec3<T> Ray<T>::operator() (T t) const
  {
    return origin + direction * t;
  }

  template<class T>
  inline Vec3<T> Ray<T>::start() const
  {
    return (*this)(minT);
  }

  template<class T>
  inline Vec3<T> Ray<T>::end() const
  {
    return (*this)(maxT);
  }

  template<class T>
  inline Ray<T> Ray<T>::transform(const Mat4<T>& m) const
  {
    return Ray<T>(m.transform(origin), m.vecTransform(direction), minT, maxT);
  }

  template<class T>
  inline Ray<T> Ray<T>::transform(const Mat4<T>& m0, const Mat4<T>& m1, float t) const
  {
    return Ray<T>(m0.transform(origin).interpolateTo(m1.transform(origin), t),
                  m0.vecTransform(direction).interpolateTo(m1.vecTransform(direction), t).normalized(),
                  minT, maxT);
  }

  using Rayf = Ray<float>;
  using Rayd = Ray<double>;

}

#endif
