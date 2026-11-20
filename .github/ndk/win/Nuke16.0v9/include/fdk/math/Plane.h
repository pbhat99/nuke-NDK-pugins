// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_PLANE_H
#define FDK_MATH_PLANE_H

#include "fdk/math/Vec3.h"

namespace fdk {

  //! A 3D plane in normalized form
  template<class T>
  class Plane {
  public:
    Vec3<T> normal; //!< The normal to the plane
    T d;            //!< The distance from the plane to the origin

    Plane()
      : normal(Vec3<T>(0.0f, 0.0f, 1.0f))
      , d(0.0f)
    {
    }

    //! Define a plane from a normal and a point on the plane
    Plane(const Vec3<T>& n, const Vec3<T>& p)
    {
      normal = n;
      d = -normal.dot(p);
    }

    //! Construct a plane from three points on the plane
    Plane(const Vec3<T>& p0, const Vec3<T>& p1, const Vec3<T>& p2)
    {
      Vec3<T> v1 = p0 - p1;
      Vec3<T> v2 = p2 - p0;

      normal = v1.cross(v2);
      normal.normalize();
      d = -normal.dot(p0);
    }

    //! Return the signed distance between a point and the plane
    T distance(const Vec3<T>& p) const
    {
      return normal.dot(p) + d;
    }

    //! Transform the plane by matrix \a m
    Plane transform(const Mat4<T>& m) const;

  };


  template<class T>
  inline Plane<T> Plane<T>::transform(const Mat4<T>& m) const
  {
    auto newPoint = m.transform(d * normal);
    auto newNormal = m.normalTransform(normal);
    return Plane(newNormal, newPoint.dot(newNormal));
  }


  using Planef = Plane<float>;
  using Planed = Plane<double>;

}

#endif
