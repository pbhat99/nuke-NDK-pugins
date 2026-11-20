// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_CAMERA_MULTIPROJECTIONCAMERA_H
#define NDK_GEO_CAMERA_MULTIPROJECTIONCAMERA_H

#include "ndk/api.h"
#include "ndk/geo/camera/Camera.h"
#include "ndk/geo/camera/LensProjection.h"

namespace ndk {


  /*! Camera class that can change its projection function, managed by the subclass
      implementation.
  */
  class NDK_API MultiProjectionCamera : public Camera
  {
  public:
    //! Default ctor leaves projection function unassigned (null).
    MultiProjectionCamera();

    //! Access the entire camera sample.
    const Camera::Sample& properties() const { return _params; }

    const CameraSensorParameters&        sensorProperties()       const { return _params.sensor; }
    const CameraMountParameters&         mountProperties()        const { return _params.mount; }
    const CameraLensApertureParameters&  lensApertureProperties() const { return _params.lensAperture; }
    const CameraOpticsParameters&        opticsProperties()       const { return _params.optics; }


    //! Set Camera::Sample parameters to the standard defaults for MultiProjectionCamera.
    static void  SetSampleDefaults(Camera::Sample& properties);

    //! Set Camera::Sample to the standard defaults for MultiProjectionCamera; calls
    //! SetSampleDefaults().
    void  setSampleToDefault(Camera::Sample& properties) const override;

    //-----------------------------------------------------------------

    //! Sets \a _horizontalAperture using field-of-view angle **in degrees**.
    //! Focal length remains the same.
    void    setHorizontalFov(double angleInDegrees);

    //! Horizontal/vertical sensor size - in millimeters
    const fdk::Vec2d&  apertureSize() const { return _params.apertureSize(); }

    //! Set the sensor physical size - in millimeters.
    void  setApertureSize(const fdk::Vec2d& sizeInMm) { _params.setApertureSize(sizeInMm); }

    double  horizontalAperture() const { return _params.apertureSize().x; }
    void    setHorizontalAperture(double widthInMm) { _params.sensor.apertureSize.x = widthInMm; }

    double  verticalAperture() const { return _params.apertureSize().y; }
    void    setVerticalAperture(double heightInMm) { _params.sensor.apertureSize.y = heightInMm; }

    //! Returns the sensor offset in millimeters by converting apertureWindowTranslate from
    //! square (1:1) NDC units via scaling with apertureSize.x.
    //! Important - apertureSize.x must already be set for this to work correctly, and note
    //! that the conversion from NDC units must ONLY use apertureSize.x and NEVER
    //! apertureSize.y, since aperture NDC space is always 1:1 square.
    fdk::Vec2d  apertureOffset() const { return _params.apertureOffset(); }

    //! Sets apertureWindowTranslate using sensor aperture offset units (millimeters).
    //! Important - apertureSize must be already configured as \a offsetInMm is normalized
    //! (scaled) to NDC units by apertureSize.x ONLY, keeping the NDC x/y change in
    //! distance ratio 1:1.
    void  setApertureOffset(const fdk::Vec2d& apTxy) { _params.setApertureOffset(apTxy); }
    void  setApertureOffset(double apTx, double apTy) { _params.setApertureOffset({apTx, apTy}); }

    //! Same but aperture width is passed as well This is for convenience when apertureSize is not
    //! set in millimeters (for example in cameras with non-physical sensor sizes like [1.0 2.0]).
    void  setApertureOffset(const fdk::Vec2d& offsetInMm, double apertureWidthInMm) { _params.setApertureOffset(offsetInMm, apertureWidthInMm); }

    //! NDC window translation in square (1:1) NDC ratio. See NDC notes in Camera.h.
    fdk::Vec2d  windowTranslate() const { return _params.apertureWindowTranslate(); }

    //! Set the NDC window translation in square (1:1) NDC ratio. See NDC notes in Camera.h.
    void  setWindowTranslate(const fdk::Vec2d& winTxy) { _params.setApertureWindowTranslate(winTxy); }
    void  setWindowTranslate(double winTx, double winTy) { _params.setApertureWindowTranslate({winTx, winTy}); }

    //! Rotation of sensor window - in degrees
    double  windowRotate() const { return _params.apertureWindowRotate(); }
    double  windowRotateZ() const { return _params.apertureWindowRotate(); }
    void    setWindowRotate(double angleInDegrees) { _params.setApertureWindowRotate(angleInDegrees); }
    void    setWindowRotateZ(double angleInDegrees) { _params.setApertureWindowRotate(angleInDegrees); }

    //! Scaling percentage of sensor window
    const fdk::Vec2d&  windowScale() const { return _params.apertureWindowScale(); }
    void  setWindowScale(const fdk::Vec2d& apSxy) { _params.setApertureWindowScale(apSxy); }
    void  setWindowScale(double apSx, double apSy) { _params.setApertureWindowScale({apSx, apSy}); }

    //-----------------------------------------------------------------

    double  focalLength() const { return _params.optics.focalLength; }
    void    setFocalLength(double f) { _params.optics.focalLength = f; }

    double  fStop() const { return _params.lensAperture.fStop; }
    void    setfStop(double f) { _params.lensAperture.fStop = f; }

    double  nearPlaneDistance() const { return _params.lensAperture.clipNearDistance; }
    void    setNearPlaneDistance(double n) { _params.lensAperture.clipNearDistance = n; }

    double  farPlaneDistance() const { return _params.lensAperture.clipFarDistance; }
    void    setFarPlaneDistance(double f) { _params.lensAperture.clipFarDistance = f; }

    double  focusDistance() const { return _params.optics.focusDistance; }
    void    setFocusDistance(double f) { _params.optics.focusDistance = f; }

    //-----------------------------------------------------------------

    double distortA() const { return _params.optics.opticalDistortionA; }
    void   setDistortA(double v) { _params.optics.opticalDistortionA = v; }

    double distortB() const { return _params.optics.opticalDistortionB; }
    void   setDistortB(double v) { _params.optics.opticalDistortionB = v; }

    double distortC() const { return _params.optics.opticalDistortionC; }
    void   setDistortC(double v) { _params.optics.opticalDistortionC = v; }

    //-----------------------------------------------------------------

    //! Currently-assigned projection function.
    //! Note that this function pointer is assumed to never be null. If there was a failure
    //! setting a custom function then one of the default functions should be assigned.
    LensProjectionFunction*  projectionFunc() const { return _lensFunc; }

    //! Assign the projection function.
    void  setProjectionFunc(LensProjectionFunction* lensFunc) { _lensFunc = lensFunc; }

    //! Currently-assigned projection mode enum value. May be one of the pre-defined modes
    //! or a custom enum. Note - do not assume this value is within the hardcoded
    //! LensProjection set as a subclass may extend or completely replace the enum set!
    //!
    //! Default implementation returns projectionFunc()->lensProjectionMode().
    virtual LensProjection  projectionMode() const;

    //! Return the base identifier name of the currently-assigned projection function.
    //! Default implementation returns projectionFunc()->identifier().
    virtual const char*  projectionIdentifier() const;

    //! Returns true if current camera projection can be described linearly with a
    //! 4x4 matrix, based on the camera's current settings. This is usually only
    //! true for perspective and orthographic modes if no lens distortion is
    //! being done.
    //!
    //! A subclass should override this method if it's implementing custom projection
    //! calculations in projectionMatrix().
    //!
    //! Default implementation returns projectionFunc()->isLinear().
    virtual bool  isBaseProjectionLinear() const;

    //! Does the projection have a non-linear component? This is the case when
    //! isBaseProjectionLinear() is true but the result of that linear projection
    //! is warped in some fashion, typically due to some optical imperfection or
    //! intentional lens non-linearity.
    //! This value should be ignored when isBaseProjectionLinear() is false as
    //! the base projection itself is non-linear, such as Spherical or Cylindrical.
    //!
    //! For these kinds of linear-but-distorted projections it is often beneficial
    //! to preview the projection using the matrix and only apply the more costly
    //! distortion when needed.
    //! Alternatively the distortion function can be inverted separately to
    //! un-distort images mapped onto geometry.
    //!
    //! Default implementation returns projectionFunc()->isDistorted().
    virtual bool  isDistorted() const;

    //! If isBaseProjectionLinear() == true then this is the projection matrix.
    const fdk::Mat4d&  projectionMatrix() const { return _params._projectionMatrix; }


  protected:
    //! Configures the currently-assigned projection function by calling
    //! projectionFunc()->validateProjection().
    void _validateCamera() override;


  protected:
    Camera::Sample          _params;              //!< Camera properties. Not all are exposed by the base class.
    LensProjectionFunction* _lensFunc = nullptr;  //!< Currently assigned function - externally managed.
  };


} // namespace ndk

#endif
