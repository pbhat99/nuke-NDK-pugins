// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_FRUSTUM_H
#define FDK_MATH_FRUSTUM_H

#include "fdk/math/Vec3.h"
#include "fdk/math/Mat4.h"
#include "fdk/math/Box2.h"
#include "fdk/math/Plane.h"

namespace fdk {

  //! A view frustum class defined by six planes.
  template<class T>
  class Frustum
  {
  public:
    //! Frustum planes
    enum Plane {
      eNear = 0,
      eFar,
      eLeft,
      eRight,
      eTop,
      eBottom,
      eNumPlanes
    };

    enum {
      eNumCorners = 8
    };

    //! Frustum visibility result
    enum class Visibility {
      NotVisible,       //!< The object is completely outside the view frustum
      FullVisible,      //!< The object is completely inside the view frustum
      PartialVisible    //!< Part of the object is inside the view frustum
    };

    //! Default constructor
    Frustum() = default;

    //! Construct a frustum from a projection matrix and viewport
    Frustum(const Mat4<T>& projMatrix, const Box2<T>& rect, const Box2<T>& viewport, T near = 0.0, T far = 1.0);

    //! Construct a frustum from camera parameters
    Frustum(const Mat4<T>& m, T near, T far, T focalLength, T filmWidth, T filmHeight);

    //! Get a frustum plane
    const fdk::Plane<T>& getPlane(Plane plane) const { return _planes[plane]; }

    //! Return the six planes defining the frustum
    const fdk::Plane<T>* planes() const { return _planes; }

    //! Return the eight corners of the frustum
    const Vec3<T>* corners() const { return _corners; }

    //! Test the visibility of a point
    bool containsPoint(const Vec3<T>& point) const;

    //! Test the visibility of a Box
    bool isBoxVisible(const fdk::Box3<T>& b) const;

    //! Test the visibility of a polygon defined by a list of coplanar vertices
    Visibility arePointsVisible(const Vec3<T>* points, size_t count) const;

    //! Transform the plane by matrix \a m
    Frustum transform(const Mat4<T>& m) const;

  private:
    fdk::Plane<T> _planes[eNumPlanes];  //!< The six planes defining the frustum
    fdk::Vec3<T> _corners[eNumCorners]; //!< The eight corners of the frustum
  };


  using Frustumf = Frustum<float>;
  using Frustumd = Frustum<double>;


  // Some utility functions dealing with projections

  //! Project a point from screen space to world space
  template<class T>
  bool project(const fdk::Vec3<T>& world, const fdk::Mat4<T>& m, const fdk::Box2<T>& viewport, fdk::Vec3<T>& window)
  {
    fdk::Vec4<T> out = m * fdk::Vec4<T>(world);
    if (out.w == 0) {
      return false;
    }
    out.wNormalize();
    constexpr T one = static_cast<T>(1);
    constexpr T half = static_cast<T>(0.5);
    window = fdk::Vec3<T>(
      viewport.x() + (one + out.x) * viewport.w() * half,
      viewport.y() + (one + out.y) * viewport.h() * half,
      (one + out.z) * half);
    return true;
  }

  //! Unproject a point from world space to screen space
  template<class T>
  bool unproject(const fdk::Vec3<T>& window, const fdk::Mat4<T>& m, const fdk::Box2<T>& viewport, fdk::Vec3<T>& world)
  {
    constexpr T one = static_cast<T>(1);
    constexpr T two = static_cast<T>(2);
    fdk::Vec4<T> in(two * (window.x - viewport.x()) / viewport.w() - one,
               two * (window.y - viewport.y()) / viewport.h() - one,
               two * window.z - one,
               one);
    const T det = m.getDeterminant();
    if (det == 0) {
      return false;
    }
    world = (m.inverse(det) * in).wNormalized();
    return true;
  }

}

#endif
