// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ViewFrustum_h
#define DDImage_ViewFrustum_h

#include "DDImage/Box.h"
#include "DDImage/DDImage_API.h"
#include "DDImage/DDMath.h"
#include "DDImage/Matrix4.h"
#include "DDImage/Plane.h"
#ifndef FN_NO_LEGACY_3D
#include "DDImage/Primitive.h"
#endif
#include "DDImage/ViewerContext.h"

namespace DD {
  namespace Image {

    // Forward declaration
    class CameraOp;


    //! Frustum planes enumerations
    enum FrustumPlane {
      eNear = 0,
      eFar,
      eLeft,
      eRight,
      eTop,
      eBottom,
      eMax
    };


    //! Frustum visibility result
    enum FrustumVisibility {
      eNotVisible = 0,    //!< the primitive is completely outside the view frustum
      eFullVisible,       //!< the primitive is completely inside the view frustum
      ePartialVisible     //!< part of the primitive is inside the view frustum
    };


    //! View Frustum class. Define the camera view frustum with a list of planes.
    //! A plane is defined in world coordinate. It is used to determine polygon visibility.
    class DDImage_API ViewFrustum
    {
    public:
      //! Default constructor
      ViewFrustum();

      //! Constructor by camera
      ViewFrustum(const CameraOp &camera);

      //! Construct a frustum using the 2D rectangle projected onto the
      //! camera's near and far clipping planes.
      ViewFrustum(ViewerContext* ctx, const Box& rect);

      //! Update frustum plane in world coordinate according to camera data
      void update(const CameraOp& camera);

      //! Update frustum plane according to Matrix4 transformation
      void update(const CameraOp& camera, const Matrix4& m);

      //! Get a frustum plane
      const Plane& getPlane(FrustumPlane plane) const;

      //! Test the visibility of a polygon inside a list of coplanar vertices
#ifndef FN_NO_LEGACY_3D
      FrustumVisibility getVisibility(const PointList* plist) const;
#endif

      //! Test the visibility a primitive face
#ifndef FN_NO_LEGACY_3D
      FrustumVisibility getVisibility(const Vector3* points, const Primitive* prim, unsigned face) const;
#endif

    private:
      //! Build frustum planes according to camera data and matrix transformation
      void build(const CameraOp& camera, const Matrix4& m);

    private:
      // View frustum planes
      Plane _planes[eMax];
    };

  } // namespace Image
}  // namespace DD

#endif
