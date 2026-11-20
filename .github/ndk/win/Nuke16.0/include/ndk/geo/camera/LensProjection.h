// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_CAMERA_LENSPROJECTION_H
#define NDK_GEO_CAMERA_LENSPROJECTION_H

#include "ndk/api.h"
#include "ndk/geo/camera/Camera.h"
#include "fdk/base/Time.h"
#include "fdk/base/StringTypes.h" // for fdk::StringList

#include "fdk/math/Box2.h"
#include "fdk/math/Vec2.h"
#include "fdk/math/Mat4.h"

namespace ndk {
  class RayContext;
  class LensProjectionFunction;


  /*! Predefined lens projection modes intended for cameras and renderers.

      Use LensProjectionClassName(mode) to retrieve the class name for the
      built-in projection function corresponding to the predefined modes.
      This class name is used to retrieve named lens shader plugins.
  */
  enum class LensProjection : int32_t
  {
    // Linear projections:
    Perspective = 0,              //!< Standard linear perspective projection
    Orthographic,                 //!< Standard linear orthographic projection
    // Non-linear projections:
    Spherical,                    //!< 360x180 world spherically projected around camera
    Cylindrical,                  //!< 360 world cylindrically projected around camera
    // Custom projections start here:
    Custom                        //!< Custom projections start here.
  };

  //! Retrieve the class name identifier of the projection function associated with the
  //! predefined LensProjection modes.
  //! Undefined mode indices >= Custom will return an empty string.
  //! 
  //! Note that the class name is without any plugin prefix or suffix and will usually be
  //! what the user will sees in a UI - ie 'Perspective', 'Orthographic', etc.
  NDK_API const std::string&  LensProjectionClassName(LensProjection mode);

  //! Allocate and return an instance of one of the standard projection functions
  NDK_API LensProjectionFunction*  CreateProjectionFunc(LensProjection mode);

  //! List of predefined lens projection mode names. These correspond to named projection
  //! function plugin classes.
  NDK_API const fdk::StringList&  LensProjectionClassNameList();

  //! Returns true if the LensProjection can be described linearly with a 4x4 matrix.
  //! Note this is only true for base perspective and orthographic modes when no lens
  //! distortion is being applied.
  inline bool  IsProjectionModeLinear(LensProjection mode) { return (mode <= LensProjection::Orthographic); }

  //! Build a standard linear projection matrix.
  NDK_API fdk::Mat4d  BuildProjectionMatrix(double     lensMagnification,
                                            double     clipNearPlane,
                                            double     clipFarPlane,
                                            fdk::Vec2d apertureTranslate,
                                            fdk::Vec2d apertureScale,
                                            double     apertureRotateInDegrees,
                                            bool       isPerspective = true);

  //! Clamp an st value to -1..+1 range.
  inline float ClampToNdc(float v) { return fdk::clamp(v, -1.0f, 1.0f); }

  //! Clamp an st coordinate to -1..+1 range in both s & t.
  inline fdk::Vec2f ClampToNdc(const fdk::Vec2f& st) { return fdk::Vec2f(ClampToNdc(st.x), ClampToNdc(st.y)); }

  //----------------------------------------------------------------------


  /*! Data passed to LensProjectionFunction methods used for both mapping into and out of.
      For example when generating rays the screen pixel location is provided in \a screenWindowST,
      the \a P,w values are ignored and the result is a separate array of RayContexts.
      When unprojecting \a screenWindowST is still the source coord but \a P,w is the result.
      When projecting \a P,w is the source coord while \a screenWindowSt becomes the result.

      ScreenWindow coordinates are assumed to be 'square' and fitted in X, so the -X edge is at
      -1 and the +X edge is at +1. The Y coordinates are scaled to match the aspect ratio of the
      screen rectangle. For example an HD 16:9 (1.77) image with resolution 1980x1080 will map x=0 to -1,
      x=1979 to +1, y=0 to -0.5625, and y=1079 to 0.5625.

      For more info see the ProjectionImageContext::pixelXToScreenWindowX() and
      ProjectionImageContext::pixelXToScreenWindowY() methods for the math used to convert a pixel
      coord to a screen-window coord.
  */
  struct ProjectCoord
  {
    fdk::Vec2f  screenWindowSt;       //!< Screen window (NDC) coord in range -1.0...+1.0, -aspect...+aspect.
    fdk::Vec2f  screenWindowDsDt;     //!< Optional radius of screen coord (offsets in x & y.) Set to 0,0 if not needed.
    float       screenWindowStDepth;  //!< Optional distance value (usually 'Z') of screenWindow coord. Set to 0 if not needed.
    //
    fdk::Vec3f  P;    //!< 3D point location in ProjectionFunction's input space (usually eye, or camera-space)
    float       w;    //!< Optional homogeneous w coordinate. Set to 1 if not needed.


    //! Default ctor leaves vars intentionally uninitialized for performance reasons.
    ProjectCoord() = default;
    //! Initialize with a 2D screen st coord.
    ProjectCoord(const fdk::Vec2f& st, float stDepth = 0.0f);
    //! Initialize with a 3D point.
    ProjectCoord(const fdk::Vec3f& point, float pointW = 1.0f);
  };

  using ProjectCoordList = std::vector<ProjectCoord>;


  //----------------------------------------------------------------------


  //! Contains image context information for projection functions.
  //! Not all these values are needed depending on the projection function.
  struct ProjectionImageContext
  {
    //! Image width in pixels, mapped to screen-window width -1...+1
    float              imageWidth() const { return _imageWidth; }

    //! Image height in pixels, mapped to screen-window height -apertureAspect...+apertureAspect
    float              imageHeight() const { return _imageHeight; }

    //! Image format projection region - endpoint-exclusive pixel rectangle
    const fdk::Box2f&  apertureBbox() const { return _apertureBbox; }

    //! Image rectangle aspect -vertical- ratio, includes any pixel-aspect ratio (aspect = h/w/pa)
    float              apertureAspect() const { return _apertureAspect; }

    //-----------------------------------------------------------------

    //! Assign the image width and height. Use \a setAperture() to configure pixel aspect ratio.
    void  setImageSize(int32_t width, int32_t height);

    //! Assigns the projection aperture bounding box and calculates its overall aspect
    //! taking into account the provided pixel aspect ratio.
    //! \a apertureAspect is calculated by \a bbox H / \a bbox W / \a pixelAspectRatio.
    void  setAperture(const fdk::Box2i& bbox,
                      double            pixelAspectRatio = 1.0);

    //-----------------------------------------------------------------

    //! Convert a screen-window (NDC) S coordinate to pixel-space X range.
    float  screenWindowSToPixelX(float screenWindowS) const;

    //! Convert a screen-window (NDC) T coordinate to pixel-space Y range.
    float  screenWindowTToPixelY(float screenWindowT) const;

    //! Convert a screen-window (NDC) coordinate to pixel-space XY range.
    fdk::Vec2f  screenWindowStToPixelXY(const fdk::Vec2f& screenWindowSt) const;

    //-----------------------------------------------------------------

    //! Convert a pixel-space X coordinate into -1...+1 screen-window (NDC) range.
    float  pixelXToScreenWindowS(float pixelX) const;

    //! Convert a pixel-space Y coordinate into -apertureAspect...+apertureAspect screen-window (NDC) range.
    float  pixelYToScreenWindowT(float pixelY) const;

    //! Convert a pixel-space XY coordinate into -1...+1, -apertureAspect...+apertureAspect screen-window (NDC) range.
    fdk::Vec2f  pixelXYToScreenWindowSt(const fdk::Vec2f& pixelXY) const;

    //! Convert a pixel-space XY coordinate into -1...+1, -apertureAspect...+apertureAspect screen-window (NDC) range.
    void  pixelXYToScreenWindowSt(const fdk::Vec2f& pixelXY,
                                  fdk::Vec2f&       screenWindowSt) const;

    //-----------------------------------------------------------------

    //! For debugging projection calculations
    const fdk::Vec2i&  pixelXY() const { return _pixelXY; }
    const fdk::Vec2i&  debugPixelXY() const { return _debugPixel; }

    bool  isDebugPixel(int32_t px, int32_t py) const { return (_debugPixel.x == px && _debugPixel.y == py); }
    bool  isDebugPixel(const fdk::Vec2f& spXY) const { return isDebugPixel(std::floor(spXY.x), std::floor(spXY.y)); }
    bool  isDebugPixelX(int32_t px) const { return (_debugPixel.x == px); }
    bool  isDebugPixelY(int32_t py) const { return (_debugPixel.y == py); }

    bool  isDebugPixel() const { return isDebugPixel(_pixelXY.x, _pixelXY.y); }

    //-----------------------------------------------------------------
    // Derived vars intentionally left public for ease of assignment:
    float       _imageWidth = 0.0f;
    float       _imageHeight = 0.0f;
    fdk::Box2f  _apertureBbox = {0.0f, 0.0f};
    float       _apScaleW = 1.0f;
    float       _apInvScaleW = 1.0f;
    float       _apScaleH = 1.0f;
    float       _apInvScaleH = 1.0f;
    float       _apertureAspect = 1.0f;
    //
    fdk::Vec2i  _pixelXY = {-1,-1};     //!< For debugging projection calculations
    fdk::Vec2i  _debugPixel = {-1,-1};  //!< For debugging projection calculations
  };


  //----------------------------------------------------------------------


  /*! \class ndk::LensProjectionFunction

      Abstract base class for projection functions.
  */
  class NDK_API LensProjectionFunction
  {
  protected:
    //!
    LensProjectionFunction();

    //! Subclass implementation of validateProjection(). Default implementation does nothing.
    virtual void  _validateProjection() {}

  public:
    //!
    virtual ~LensProjectionFunction() {}

    //! Calls subclass _validateProjection().
    void validateProjection();

    //! Return the object this is attached to cast to LensProjectionFunction interface.
    //! Subclasses should return 'this'.
    virtual LensProjectionFunction* asLensProjectionFunction() { return nullptr; }

    //-----------------------------------------------------------------

    //! The identifier name of the function. For example 'Perspective' or 'Orthographic'.
    //! Must implement.
    virtual const char* identifier() const = 0;

    //! Which predefined camera LensProjection mode this func represents.
    //! Default implementation returns Custom.
    virtual LensProjection lensProjectionMode() const { return LensProjection::Custom; }

    //! Returns true if the projection function can be described linearly with a 4x4 matrix.
    //! Note this is only true for standard perspective and orthographic projections when
    //! no lens distortion is being applied.
    //! Must implement.
    virtual bool isLinear() const = 0;

    //! Correct vector distance to linear-projected 'Z', typically for converting
    //! camera ray distance to surface position into the linearly-projected equivalent
    //! you would get when transforming the surface position through a projection
    //! matrix.
    //! Usually this is only required when the projection is perspective.
    //! \a screenWindowSt provides the distance from lens center.
    //! Default implementation returns the distance value unchanged.
    virtual float distanceToLinearZ(const Camera::Sample& cameraContext,
                                    const fdk::Vec2f&     screenWindowSt,
                                    float                 distance) const { return distance; }

    //-----------------------------------------------------------------

    //! Lens magnification (zoom) is usually film-width / focal-length for a perspective
    //! projection and 1.0 for a non-linear one.
    //! Default implementation returns 1.0.
    virtual double lensMagnification(const Camera::Sample& cameraContext) const;

    //! Return a linear matrix for the projection function, if possible.
    //!
    //! This method should be used in conjunction with isLinear() to determine
    //! how to use the returned matrix, as many projection functions cannot be
    //! supported by a linear matrix. For example a spherical projection or a
    //! linear perspective projection which becomes non linear if lens distortion
    //! is being applied.
    //! 
    //! In non-linear cases the matrix returned can be considered the 'base'
    //! projection for the camera based on its current lens settings.
    //! In other words although the projection mode may be linear perspective and
    //! the matrix returned is a valid perspective projection, if lens distortion
    //! is used then the final projection is a combination of both the projection
    //! matrix and the distortion.
    //!
    //! Default implementation returns a perspective projection from
    //! Camera::BuildLinearProjection(cameraContext) and includes ndcTransformMatrix().
    virtual fdk::Mat4d projectionMatrix(const Camera::Sample& cameraContext) const;

    //! Return a matrix suitable for transforming NDC coordinates.
    //!
    //! Typically this is used to rotate or shift the sensor relative to the projection
    //! and its inverse is often incorporated into the projection matrix if the projection
    //! is linear.
    //!
    //! Default implementation returns Camera::BuildNDCTransform(cameraContext).
    virtual fdk::Mat4d ndcTransformMatrix(const Camera::Sample& cameraContext) const;

    //! Can the projection region be extended horizontally in a meaningful way?
    //! Note this is -NOT- the scaling of the projection, this is expanding outside
    //! the -1..+1 screenWindow ndc region.
    //!
    //! This function allows the projection function to communicate to a caller
    //! whether extended values are handled so they could be pre-clamped or a UI can
    //! communicate this info.
    //!
    //! Some projection geometries like spherical or cylindrical which wrap around
    //! or have a singularity cannot be meanfully extended. Others like perspective
    //! and orthographic, especially with non-linear distortion, can be.
    virtual bool canBeExtendedHorizontally() const { return false; }

    //! Same as canProjectionBeExpandedHorizontally() but in T.
    virtual bool canBeExtendedVertically() const { return false; }

    //-----------------------------------------------------------------

    //! Generate RayContext ray geometry.
    //! \a screenCoords is an array of ProjectCoord struct \a nRays in size.
    //! \a cameraContexts is an array of ProjectionCameraContexts \a nRays in size.
    //! \a raysOut must be a pre-allocated array large enough to hold \a nRays worth
    //! of RayContexts.
    //!
    //! The projection function should not assume the type of RayContext being
    //! configured and should only set ray geometry values origin, direction, minT,
    //! maxT, dxDir, and dyDir. The caller has the option to preset the other values
    //! before calling this method at the risk a subclass implementation may change
    //! things it's not "supposed to", so it's safer for the caller to set other
    //! RayContext attributes after calling this method.
    //!
    //! Output ray origin is assumed to be in camera-local space and is normally
    //! 0,0,0 by default unless there's a DOF or stereo offset to apply.
    //! Output ray direction is assumed to be in camera-local space where a typical
    //! perspective projection will be pointing down the -Z axis at screen center.
    //!
    //! Note the ProjectionImageContext is only provided for reference, usually for
    //! pixel aspect ratio and pixel debug info.
    virtual void generateRayGeometry(uint32_t                      nRays,
                                     const ProjectCoord*           screenCoords,
                                     const Camera::Sample*         cameraContexts,
                                     const ProjectionImageContext& imageContext,
                                     ndk::RayContext*              raysOut) const {}

    //! Project 3D coordinates through the function to 2D screen coords (but NOT
    //! pixel coords!)
    //!
    //! Note the ProjectionImageContext is only provided for reference, usually for
    //! pixel aspect ratio and pixel debug info.
    //!
    //! Must implement.
    virtual void project(const Camera::Sample&         cameraContext,
                         const ProjectionImageContext& imageContext,
                         uint32_t                      nVerts,
                         ProjectCoord*                 vertArray) const = 0;

  };



//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------


inline ProjectCoord::ProjectCoord(const fdk::Vec2f& st, float stDepth)
  : screenWindowSt(st)
  , screenWindowDsDt(0.0f)
  , screenWindowStDepth(stDepth)
  , P(0.0f)
  , w(1.0f)
{
  //
}

inline ProjectCoord::ProjectCoord(const fdk::Vec3f& point, float pointW)
  : screenWindowSt(0.0f)
  , screenWindowDsDt(0.0f)
  , screenWindowStDepth(0.0f)
  , P(point)
  , w(pointW)
{
  //
}

inline void ProjectionImageContext::setImageSize(int32_t width, int32_t height)
{
  _imageWidth = static_cast<float>(std::abs(width));
  _imageHeight = static_cast<float>(std::abs(height));
}

inline void ProjectionImageContext::setAperture(const fdk::Box2i& bbox,
                                                double            pixelAspectRatio)
{
  _apertureBbox = bbox.asBox2f();
  _apScaleW = _apertureBbox.w() / 2.0f;
  _apScaleH = _apertureBbox.h() / 2.0f;
  _apInvScaleW = 1.0f / _apScaleW;
  _apInvScaleH = 1.0f / _apScaleH;
  _apertureAspect = _apertureBbox.h() / _apertureBbox.w() / ((pixelAspectRatio > 0.0) ? pixelAspectRatio : 1.0);
}

inline float ProjectionImageContext::screenWindowSToPixelX(float screenWindowS) const
{
  return (screenWindowS + 1.0f) * _apScaleW + _apertureBbox.min.x;
}

inline float ProjectionImageContext::screenWindowTToPixelY(float screenWindowT) const
{
  return (screenWindowT / _apertureAspect + 1.0f) * _apScaleH + _apertureBbox.min.y;
}

inline fdk::Vec2f ProjectionImageContext::screenWindowStToPixelXY(const fdk::Vec2f& screenWindowSt) const
{
  return fdk::Vec2f(screenWindowSToPixelX(screenWindowSt.x),
                    screenWindowTToPixelY(screenWindowSt.y));
}

inline float ProjectionImageContext::pixelXToScreenWindowS(float pixelX) const
{
  return (pixelX - _apertureBbox.min.x) * _apInvScaleW - 1.0f;
}

inline float ProjectionImageContext::pixelYToScreenWindowT(float pixelY) const
{
  return ((pixelY - _apertureBbox.min.y) * _apInvScaleH - 1.0f) * _apertureAspect;
}

inline fdk::Vec2f ProjectionImageContext::pixelXYToScreenWindowSt(const fdk::Vec2f& pixelXY) const
{
  return fdk::Vec2f(pixelXToScreenWindowS(pixelXY.x),
                    pixelYToScreenWindowT(pixelXY.y));
}

inline void ProjectionImageContext::pixelXYToScreenWindowSt(const fdk::Vec2f& pixelXY,
                                                            fdk::Vec2f&       screenWindowSt) const
{
  screenWindowSt.x = pixelXToScreenWindowS(pixelXY.x);
  screenWindowSt.y = pixelYToScreenWindowT(pixelXY.y);
}

} // namespace ndk

#endif
