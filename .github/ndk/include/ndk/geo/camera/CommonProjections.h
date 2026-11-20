// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_CAMERA_COMMONPROJECTIONS_H
#define NDK_GEO_CAMERA_COMMONPROJECTIONS_H

#include "ndk/geo/camera/LensProjection.h"

namespace ndk {

  /*! Linear lens projection with no distortions, describable via a 4x4 matrix.
      Perspective or orthographic.
  */
  class NDK_API LinearProjection : public LensProjectionFunction
  {
    bool ortho = false;   //!< Projection is orthographic so all rays are parallel

  public:
    LinearProjection(bool _ortho = true);

    const char* identifier() const override { return (!ortho) ? "Perspective" : "Orthographic"; }
    LensProjection lensProjectionMode() const override { return (!ortho) ? LensProjection::Perspective : LensProjection::Orthographic; }
    bool isLinear() const override { return true; }
    bool canBeExtendedHorizontally() const override { return true; }
    bool canBeExtendedVertically() const override { return true; }

    void  _validateProjection() override;

    //! Builds an orthographic or perspective projection matrix.
    fdk::Mat4d projectionMatrix(const Camera::Sample& cameraContext) const override;

    fdk::Mat4d ndcTransformMatrix(const Camera::Sample& cameraContext) const override;

    float distanceToLinearZ(const Camera::Sample& cameraContext,
                            const fdk::Vec2f&     screenWindowSt,
                            float                 distance) const override;

    float linearZToDistance(const Camera::Sample& cameraContext,
                            const fdk::Vec2f&     screenWindowSt,
                            float                 linearZ) const override;

    void project(const Camera::Sample&         cameraContext,
                 const ProjectionImageContext& imageContext,
                 uint32_t                      nVerts,
                 ProjectCoord*                 vertArray) const override;

    void generateRayGeometry(uint32_t                      nRays,
                             const ProjectCoord*           screenCoords,
                             const Camera::Sample*         cameraContexts,
                             const ProjectionImageContext& imageContext,
                             RayContext**                  raysOut) const override;
  };


  //----------------------------------------------------------------------


  /*! Spherical projection camera.
  */
  class NDK_API SphericalProjection : public LensProjectionFunction
  {
  public:
    SphericalProjection();

    const char* identifier() const override { return "Spherical"; }
    LensProjection lensProjectionMode() const override { return LensProjection::Spherical; }
    bool isLinear() const override { return false; }

    void  _validateProjection() override;

    void project(const Camera::Sample&         cameraContext,
                 const ProjectionImageContext& imageContext,
                 uint32_t                      nVerts,
                 ProjectCoord*                 vertArray) const override;

    void generateRayGeometry(uint32_t                      nRays,
                             const ProjectCoord*           screenCoords,
                             const Camera::Sample*         cameraContexts,
                             const ProjectionImageContext& imageContext,
                             RayContext**                  raysOut) const override;
  };


  //----------------------------------------------------------------------


  /*! Cylindrical projection camera.
  */
  class NDK_API CylindricalProjection : public LensProjectionFunction
  {
  public:
    CylindricalProjection();

    const char* identifier() const override { return "Cylindrical"; }
    LensProjection lensProjectionMode() const override { return LensProjection::Cylindrical; }
    bool isLinear() const override { return false; }

    void  _validateProjection() override;

    void project(const Camera::Sample&         cameraContext,
                 const ProjectionImageContext& imageContext,
                 uint32_t                      nVerts,
                 ProjectCoord*                 vertArray) const override;

    void generateRayGeometry(uint32_t                      nRays,
                             const ProjectCoord*           screenCoords,
                             const Camera::Sample*         cameraContexts,
                             const ProjectionImageContext& imageContext,
                             RayContext**                  raysOut) const override;
  };

} // namespace ndk

#endif
