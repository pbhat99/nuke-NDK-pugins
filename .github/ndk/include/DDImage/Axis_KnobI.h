// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_AXIS_KNOBI_H
#define DDIMAGE_AXIS_KNOBI_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Deprecation.h"

#include "fdk/math/Vec3.h"
#include "fdk/math/Mat4.h"

#include "DDImage/Vector3.h"
#include "DDImage/Matrix4.h"

namespace DD
{

  namespace Image
  {
    class Knob;
    class OutputContext;

    class DDImage_API Axis_KnobI
    {
    public:
      enum TransformOrder { 
        eSRT = 0, 
        eSTR, 
        eRST, 
        eRTS, 
        eTSR, 
        eTRS 
      };

      enum RotationOrder { 
        eXYZ = 0, 
        eXZY, 
        eYXZ, 
        eYZX, 
        eZXY, 
        eZYX 
      };

      enum Transformation {
        eTranslate = 0,
        eRotate,
        eScale,
        eUniformScale,
        eSkew,
        ePivot,
        ePivotRotate
      };

      static const TransformOrder kDefaultTransformOrder = eSRT;
      static const RotationOrder kDefaultRotationOrder = eZXY;


    public:
      Axis_KnobI() { }
      virtual ~Axis_KnobI() { }

      //! Return the Knob this interface is on. Implementations should just "return this;"
      virtual Knob* baseKnob() = 0;

      virtual Knob* translateKnob() = 0;
      virtual Knob* rotateKnob() = 0;
      virtual Knob* scaleKnob() = 0;
      virtual Knob* uniformScaleKnob() = 0;
      virtual Knob* skewKnob() = 0;
      virtual Knob* pivotTranslateKnob() = 0;
      virtual Knob* pivotRotateKnob() = 0;

      //! Return whether a child knob has an expression set.
      virtual bool hasExpression(Transformation knob) const = 0;

      //! @brief Forces AxisKnob to always be drawn in node select mode
      //! independently on the current selection mode on the viewer.
      //! @param value true to enable, false otherwise.
      virtual void setAxisVisibleForAllSelectionModes(bool visible) = 0;

      //! @brief Returns true if this AxisKnob instance is always drawn in
      //! independently on the current selection mode on the viewer.
      //! @return true if enabled, false otherwise.
      virtual bool isAxisVisibleForAllSelectionModes() const = 0;

      virtual void hide() = 0;
      virtual void show() = 0;

      virtual void setRotationOrder(RotationOrder order) = 0;
      virtual void setTransformOrder(TransformOrder order) = 0;
      virtual void setTranslate(const fdk::Vec3d& translation) = 0;
      virtual void setRotate(const fdk::Vec3d& rotate) = 0;
      virtual void setScale(const fdk::Vec3d& scale) = 0;
      virtual void setUniformScale(double uniformScale) = 0;
      virtual void setSkew(const fdk::Vec3d& skew) = 0;
      virtual void setPivot(const fdk::Vec3d& pivot_translation) = 0;
      virtual void setPivotRotate(const fdk::Vec3d& pivot_rotate) = 0;

#if kDDImageVersionMajorNum <= 17
      mFnDeprecatedInNuke17("rotationOrder(const OutputContext*) has been deprecated, use rotationOrder() instead.")
      virtual RotationOrder  rotationOrder(const OutputContext* context) const = 0;
      mFnDeprecatedInNuke17("transformOrder(const OutputContext*) has been deprecated, use transformOrder() instead.")
      virtual TransformOrder transformOrder(const OutputContext* context) const = 0;
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 17."
#endif
      virtual RotationOrder  rotationOrder() const = 0;
      virtual TransformOrder transformOrder() const = 0;

      virtual double         uniformScale(const OutputContext* context = nullptr) const = 0;
      virtual fdk::Vec3d     translate(const OutputContext* context = nullptr) const = 0;
      virtual fdk::Vec3d     rotate(const OutputContext* context = nullptr) const = 0;
      virtual fdk::Vec3d     scale(const OutputContext* context = nullptr) const = 0;
      virtual fdk::Vec3d     skew(const OutputContext* context = nullptr) const = 0;
      virtual fdk::Vec3d     pivot(const OutputContext* context = nullptr) const = 0;
      virtual fdk::Vec3d     pivotRotate(const OutputContext* context = nullptr) const = 0;

      //! Return the concatenated transform of all constituent controls, at an optional 
      //! OutputContext. If OutputContext is null then Knob::uiContext() will be used.
      virtual fdk::Mat4d     transform(const OutputContext* context = nullptr) const = 0;


    public:
      //----------------------------------------------------------------------------
      // Deprecated functions - will disappear in a Nuke major rev after 16
      //----------------------------------------------------------------------------
      virtual void setTranslate(const Vector3& translation) = 0;
      virtual void setRotate(const Vector3& rotate) = 0;
      virtual void setScale(const Vector3& scale) = 0;
      virtual void setSkew(const Vector3& skew) = 0;
      virtual void setPivot(const Vector3& pivot_translation) = 0;
      virtual void setPivotRotate(const Vector3& pivot_rotate) = 0;

      virtual RotationOrder getRotationOrder(const OutputContext& context) const = 0;
      virtual TransformOrder getTransformOrder(const OutputContext& context) const = 0;
      virtual double getUniformScale(const OutputContext& context) const = 0;
      virtual Vector3 getTranslate(const OutputContext& context) const = 0;
      virtual Vector3 getRotate(const OutputContext& context) const = 0;
      virtual Vector3 getScale(const OutputContext& context) const = 0;
      virtual Vector3 getSkew(const OutputContext& context) const = 0;
      virtual Vector3 getPivot(const OutputContext& context) const = 0;
      virtual Vector3 getPivotRotate(const OutputContext& context) const = 0;

      //mFnDeprecatedLegacy3DSystemInNuke14("matrix(const OutputContext&) has been deprecated, use transformAt() instead.")
      virtual const Matrix4& matrix(const OutputContext&) const = 0;
    };

  }
}

#endif
