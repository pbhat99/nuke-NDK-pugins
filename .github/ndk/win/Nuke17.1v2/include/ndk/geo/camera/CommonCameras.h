// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_CAMERA_COMMONCAMERAS_H
#define NDK_GEO_CAMERA_COMMONCAMERAS_H

#include "ndk/geo/camera/Camera.h"

namespace ndk {

  /*! Linear projection camera with no distortions, describable via a 4x4
      projection matrix. Perspective or orthographic projection.
  */
  class NDK_API LinearCamera : public Camera
  {
    Camera::Sample cam;         //!<
    fdk::Mat4d     projection;  //!<
    bool           ortho;       //!<

  public:
    LinearCamera()
      : Camera()
      , projection(1.0)
      , ortho(false)
    {
      //
    }

  };


  //----------------------------------------------------------------------


  /*! Linear projection camera with distortion support, describable via
      a base 4x4 projection matrix and abc polynomial distort parameters.
  */
  class NDK_API WarpedLinearCamera : public LinearCamera
  {
    double  distortA;     //!< Distortion a polynomial
    double  distortB;     //!< Distortion b polynomial
    double  distortC;     //!< Distortion c polynomial

  public:
    WarpedLinearCamera()
      : LinearCamera()
    {
      //
    }

  };


  //----------------------------------------------------------------------


  /*! Spherical projection camera.
  */
  class NDK_API SphericalCamera : public Camera
  {
    Camera::Sample cam;         //!<

  public:
    SphericalCamera()
      : Camera()
    {
      //
    }

  };

  //----------------------------------------------------------------------


  /*! Cylindrical projection camera.
  */
  class NDK_API CylindricalCamera : public Camera
  {
    Camera::Sample cam;         //!<

  public:
    CylindricalCamera()
      : Camera()
    {
      //
    }

  };

} // namespace ndk

#endif
