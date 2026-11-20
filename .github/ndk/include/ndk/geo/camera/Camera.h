// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_CAMERA_CAMERA_H
#define NDK_GEO_CAMERA_CAMERA_H

#include "ndk/api.h"
//#include "ndk/geo/camera/Shutter.h"
#include "ndk/geo/utils/Draw.h"

#include "fdk/base/Time.h"
#include "fdk/math/Mat4.h"

#include <vector>
#include <iostream>

namespace ndk {
  struct RayContext;
  class LensProjectionFunction;


  //----------------------------------------------------------------------


  struct XformComponents
  {
    fdk::Mat4d inputParentXform;  //!< Parent transform from input
    //
    fdk::Mat4d localParentAxis;   //!< Local parent axis TRS transform
    fdk::Mat4d localParentXform;  //!< localParentAxis + constraints
    //
    fdk::Mat4d localAxis;         //!< Local axis TRS transform
    fdk::Mat4d localXform;        //!< localAxis + constraints


    //! Default ctor leaves junk in values.
    XformComponents() = default;

    //! Initialize to identity matrix with a constant in the diagonal. (pass in 1.0)
    XformComponents(double d) : inputParentXform(d), localParentAxis(d), localParentXform(d), localAxis(d), localXform(d) {}

    //! Set all matrices to identity.
    void reset() { inputParentXform = localParentAxis = localParentXform = localAxis = localXform = fdk::Mat4d::getIdentity(); }
  };


  struct XformParameters
  {
    fdk::Mat4d  parent;   //!< Parent transform
    fdk::Mat4d  local;    //!< Local object transform
    //
    fdk::Mat4d  l2w;      //!< Parent+local to world transform
    fdk::Mat4d  w2l;      //!< World to parent+local transform


    //! Default ctor leaves junk in values.
    XformParameters() = default;

    //! Initialize to identity matrix with a constant in the diagonal. (pass in 1.0)
    XformParameters(double d) : parent(d), local(d), l2w(d), w2l(d) {}

    //! Set all matrices to identity.
    void reset() { parent = local = l2w = w2l = fdk::Mat4d::getIdentity(); }
  };


  class NDK_API Axis
  {
  public:
    struct NDK_API Sample
    {
      fdk::TimeValue  time;   //!< Time value of sample - relative or absolute depending on usage context
      XformParameters xform;
    };

  public:
    Axis() = default;
    virtual ~Axis() {}

    //! Draw the interface icon for the Axis in object-local space. That is, the drawing code should
    //! assume that the world transform has already been applied and all coordinates are in local
    //! space.
    //! Default implementation does nothing.
    virtual void drawAxis(const DisplayOptions& options,
                          const Axis::Sample&   sample) const {}

  };

  using AxisSampleList = std::vector<Axis::Sample>;


  //----------------------------------------------------------------------


  // General Camera Parameter Categories: https://github.com/SMPTE/ris-osvp-metadata-camdkit
  //  Sensor - the planar image sensor array perpendicular to optical path behind the lens ('film back', 'film plane', 'ccd array', etc)
  //  Mount - camera body spatial offsets - for stereo, etc.
  //  Aperture - (lens iris) - the nodal rotation point where the optical light paths cross inside the lens
  //  Lens - (optics)


  //! Camera sensor properties common to most camera bodies.
  //! A 'camera sensor' is the planar photo-sensitive image array perpendicular to optical
  //! path behind the lens. Also termed the 'film back', 'film plane', 'ccd array', etc
  //! depending on the camera technology.
  //!
  //! Note that the use of the word 'aperture' in these vars is not the same as used in
  //! lens terminology where it describes the lens 'iris'. In both cases it describes a
  //! measurable, physical spatial 'portal' which light passes through or onto.
  //!
  //! Note that an accurately defined \a sceneToSensorFactor is important for calculating
  //! COC sizes where a distance from the sensor in scene units converts to an area on the
  //! sensor surface in sensor units.
  //!
  //! Note that the sensor 'window' is defined as a uniform / square / 1:1 NDC space, where
  //! a distance change in X is the same in Y (assuming apertureWindowScale is also uniform).
  //! The NDC range -1..0..+1 is fitted to the width of the aperture so that -1 is at its
  //! left edge, 0 is the center of the aperture, and +1 is at its right edge.
  //! If the aperture's height is smaller than its width then the top and bottom aperture
  //! edges will always be inside the NDC Y -1..+1 range. In other words the NDC Y -1..+1
  //! range is NEVER distorted (fitted) to the aperture height.
  struct CameraSensorParameters
  {
    fdk::Vec2d  apertureSize = {1.0, 1.0};      //!< Horizontal/vertical sensor size - in millimeters
    //
    fdk::Vec2d  apertureWindowTranslate = {0.0, 0.0}; //!< Horizontal/vertical translation of sensor window - in square (1:1) NDC units
    double      apertureWindowRotate = 0.0;           //!< Rotation of sensor window - in degrees
    fdk::Vec2d  apertureWindowScale = {1.0, 1.0};     //!< Scaling percentage of sensor window
    //
    double      sceneToCameraFactor = 0.001;    //!< Scene units to camera sensor/lens units scaling factor - default is meters to mm.
    //
    double      sensorISO = 100.0;              //!< Sensor 'speed' or 'sensitivity' rating, or how fast light accumulates on sensor.
    double      sensorExposureBias = 0.0;       //!< Adjust the exposure of the sensor by an exposure multiplier (2^sensorExposureBias)
    //
    double      shutterOpenOffset = 0.0;        //!< Offset in time for when the sensor is first exposed to the light path
    double      shutterCloseOffset = 0.0;       //!< Offset in time for when the sensor no longer exposed to the light path
    double      shutterBias = 0.0;              //!< Weights the shutter samples towards shutter close with a power function
  };


  //! Camera mount properties common to most camera bodies.
  struct CameraMountParameters
  {
    double  horizontalOffset = 0.0;   //!< Camera body relative X offset - in scene units
    double  verticalOffset = 0.0;     //!< Camera body relative Y offset - in scene units
  };


  //! Lens aperture (iris) properties common to camera lenses.
  struct CameraLensApertureParameters
  {
    double  fStop = 0.0;              //!< Lens aperture diameter - unitless ratio tied to focal-length
    //
    double  clipNearDistance = 0.001; //!< Near clipping plane distance - in scene units
    double  clipFarDistance = 1.0;    //!< Far clipping plane distance - in scene units
  };


  //! Optical properties common to camera lenses.
  //! opticalDistortionA,B,C are inputs to a mapping function where input 'r' is the distance of a
  //! point to lens center: r' = A * r^4 + B * r^3 + C * r^2 + (1 - A - B - C) * r
  struct CameraOpticsParameters
  {
    double  focalLength = 1.0;            //!< Focal length of lens (lens pupil to sensor plane distance) - in millimeters
    double  focusDistance = 1.0;          //!< Focus plane distance - in scene units
    //
    double  opticalAttenuation = 0.0;     //!< Light loss percentage from passing through the lens optics (0..1) where 0 = vacuum, or no-loss
    double  opticalWidthSqueeze = 1.0;    //!< Non-linear image compression in X (width) - ie anamorphic compression
    //
    double  opticalDistortionA = 0.0;     //!< Distortion a polynomial
    double  opticalDistortionB = 0.0;     //!< Distortion b polynomial
    double  opticalDistortionC = 0.0;     //!< Distortion c polynomial
  };


  //----------------------------------------------------------------------


  /*!
  */
  class NDK_API Camera
  {
  public:
    struct NDK_API Sample
    {
      fdk::TimeValue               time;          //!< Time value of sample - relative or absolute depending on usage context
      XformParameters              xform;
      CameraSensorParameters       sensor;        //!< Standard camera sensor parameters
      CameraMountParameters        mount;         //!< Standard camera mount parameters
      CameraLensApertureParameters lensAperture;  //!< Standard camera lens iris parameters
      CameraOpticsParameters       optics;        //!< Standard camera optical parameters


      //! Recalculate the derived values from the current property set.
      //! These are standard camera calculations like lens magnification =
      //! focal-length / aperture-width.
      //! All property values are assumed to be 'legal' values - for example aperture-width
      //! is > 0.
      //! If \a lensProjection is provided then \a _ndcTransform and \a _projectionMatrix
      //! are retrieved from it and left as-is. Otherwise a standard perspective projection
      //! is calculated for \a _projectionMatrix, and \a _ndcTransform is derived via
      //! BuildNDCTransform().
      void  updateDerivedValues(const LensProjectionFunction* lensProjection);

      //!
      void printValues(const char* prefix, std::ostream& o) const;

      //-----------------------------------------------------------------------------

      //! Horizontal/vertical sensor size - in millimeters
      const fdk::Vec2d&  apertureSize() const { return sensor.apertureSize; }

      //! Set the sensor physical size - in millimeters.
      void  setApertureSize(const fdk::Vec2d& sizeInMm) { sensor.apertureSize = sizeInMm; }

      //! Returns the sensor offset in millimeters by converting apertureWindowTranslate from
      //! square (1:1) NDC units via scaling with apertureSize.x.
      //! Important - apertureSize.x must already be set for this to work correctly, and note
      //! that the conversion from NDC units must ONLY use apertureSize.x and NEVER
      //! apertureSize.y, since aperture NDC space is always 1:1 square.
      fdk::Vec2d  apertureOffset() const;

      //! Sets apertureWindowTranslate using sensor aperture offset units (millimeters).
      //! Important - apertureSize must be already configured as \a offsetInMm is normalized
      //! (scaled) to NDC units by apertureSize.x ONLY, keeping the NDC x/y change in
      //! distance ratio 1:1.
      void  setApertureOffset(const fdk::Vec2d& offsetInMm) { setApertureOffset(offsetInMm, sensor.apertureSize.x); }

      //! Same but aperture width is passed as well This is for convenience when apertureSize is not
      //! set in millimeters (for example in cameras with non-physical sensor sizes like [1.0 2.0]).
      void  setApertureOffset(const fdk::Vec2d& offsetInMm, double apertureWidthInMm);
      void  setApertureOffset(double apTx, double apTy, double apertureWidthInMm) { setApertureOffset({apTx, apTy}, apertureWidthInMm); }

      //! NDC window translations in square (1:1) NDC units.
      const fdk::Vec2d&  apertureWindowTranslate() const { return sensor.apertureWindowTranslate; }

      //! Sets NDC window translations in square (1:1) NDC units.
      void  setApertureWindowTranslate(const fdk::Vec2d& winTxy) { sensor.apertureWindowTranslate = winTxy; }
      void  setApertureWindowTranslate(double winTx, double winTy) { sensor.apertureWindowTranslate.set(winTx, winTy); }

      //! NDC window rotation.
      double  apertureWindowRotate() const { return sensor.apertureWindowRotate; }

      //! Sets NDC window rotation.
      void  setApertureWindowRotate(double angleInDegrees) { sensor.apertureWindowRotate = angleInDegrees; }

      //! NDC window scale.
      const fdk::Vec2d&  apertureWindowScale() const { return sensor.apertureWindowScale; }

      //! Sets NDC window scale.
      void  setApertureWindowScale(const fdk::Vec2d& winSxy) { sensor.apertureWindowScale = winSxy; }
      void  setApertureWindowScale(double apSx, double apSy) { sensor.apertureWindowScale.set(apSx, apSy); }

      //--------------------------------------------------------------------------
      // Derived values - do not use unless you know they've been calculated:

      //! Lens magnification ('zoom'), usually derived from (apertureSize.x / focalLength)
      float  lensMagnification() const { return _lensMagnification; }

      //! Lens pupil diameter, usually derived from (focalLength / fStop) / 2
      float  lensPupilRadius() const { return _lensPupilRadius; }

      //! If false the optics distortion parameters can be ignored (may still be a non-linear projection!)
      bool   distortionEnabled() const { return _distortionEnabled; }

      //! Apply the barrel distortion function 'r_src = a*r^4 + b*r^3 + c*r^2 + (1-a-b-c)*r',
      //! where 'r_src'(\a lensRadialDistance) is the radial distance from the lens position
      //! being evaluated to the center of the lens, and 'r' is the output corrected radius.
      double  opticalDistortionFunction(double lensRadialDistance) const;

      //! NDC transform matrix, usually derived from apertureOffset, apertureRotate, and apertureScale.
      //! The inverse of this is often incorporated into \a projectionMatrix.
      const fdk::Mat4d&  ndcTransform() const { return _ndcTransform; }
      bool               ndcTransformIsIdentity() const { return _ndcTransformIsIdentity; }

      //! Projection matrix (only valid for linear projections,) usually derived from focalLength, apertureSize, etc.
      const fdk::Mat4d&  projectionMatrix() const { return _projectionMatrix; }

      //! Combined matrix to project world-space coordinate through projection to ndc-space.
      const fdk::Mat4d&  worldToNdc() const { return _worldToNdc; }

      //--------------------------------------------------------------------------
      // Derived vars intentionally left public for ease of assignment:
      float       _lensMagnification = 1.0f;
      float       _lensPupilRadius = 0.0f;
      bool        _distortionEnabled = false;
      double      _opticalDistortionD = 1.0;
      fdk::Mat4d  _ndcTransform = fdk::Mat4d(1.0); // constructed from ndcTranslate, ndcScale, ndcRotate
      bool        _ndcTransformIsIdentity = true;
      fdk::Mat4d  _projectionMatrix = fdk::Mat4d(1.0);
      fdk::Mat4d  _worldToNdc = fdk::Mat4d(1.0);
    };

  public:
    Camera() = default;
    virtual ~Camera() {}

    //! Set Camera::Sample parameters to the standard defaults for this camera.
    virtual void  setSampleToDefault(Camera::Sample& properties) const = 0;

    //! Call this after changing any values to let a camera subclass do any custom
    //! calculations required before use. Calls the the virtual _validateCamera()
    //! method.
    void  validateCamera();

    //--------------------------------------------------------------------------

    //! Build a transform matrix from a Camera sample's sensor aperture TRS values.
    //! Order of application is rotateZ, scaleXY, then translateXY.
    //! Set \a inverse = true to get a matrix suitable for concatenating with a
    //! projection matrix.
    static fdk::Mat4d  BuildNDCTransform(const Camera::Sample& cameraContext,
                                         bool                  inverse = false);

    //! Build a standard linear projection matrix from a Camera sample.
    //! If \a applyApertureTransform is true then the sensor aperture TRS values are
    //! applied to the matrix by calling \a buildNDCTransform().
    //! If \a isPerspective is false then the result is an orthographic projection.
    static fdk::Mat4d  BuildLinearProjection(const Camera::Sample& cameraContext,
                                             bool                  applyApertureTransform,
                                             bool                  isPerspective = true);

    //--------------------------------------------------------------------------

    //! Draw the interface icon for the Camera in object-local space. That is, the drawing code
    //! should assume that the world transform has already been applied and all coordinates are
    //! in local space.
    //! Default implementation does nothing.
    virtual void drawCamera(const DisplayOptions& options,
                            const Camera::Sample& sample) const {}

  protected:
    //! Subclass implemention of \a validateCamera(). Default implementation does nothing.
    virtual void _validateCamera() {}

  };

  using CameraSampleList = std::vector<Camera::Sample>;


  NDK_API std::ostream& operator << (std::ostream&, const XformParameters&);
  NDK_API std::ostream& operator << (std::ostream&, const CameraSensorParameters&);
  NDK_API std::ostream& operator << (std::ostream&, const CameraMountParameters&);
  NDK_API std::ostream& operator << (std::ostream&, const CameraLensApertureParameters&);
  NDK_API std::ostream& operator << (std::ostream&, const CameraOpticsParameters&);
  NDK_API std::ostream& operator << (std::ostream&, const Camera::Sample&);

  inline fdk::Vec2d Camera::Sample::apertureOffset() const
  {
    return (sensor.apertureSize.x > 0.0) ? sensor.apertureWindowTranslate * (sensor.apertureSize.x / 2.0) : sensor.apertureWindowTranslate;
  }

  inline void Camera::Sample::setApertureOffset(const fdk::Vec2d& offsetInMm, double apertureWidthInMm)
  {
    sensor.apertureWindowTranslate = (apertureWidthInMm > 0.0) ? (offsetInMm / (apertureWidthInMm / 2.0)) : offsetInMm;
  }

  inline double Camera::Sample::opticalDistortionFunction(double lensRadialDistance) const
  {
    return (((((optics.opticalDistortionA * lensRadialDistance) +
                  optics.opticalDistortionB) * lensRadialDistance) +
                    optics.opticalDistortionC) * lensRadialDistance) + _opticalDistortionD;
  }

} // namespace ndk

#endif
