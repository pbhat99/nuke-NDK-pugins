// CurveI.h
// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_CurveI_h
#define DDImage_CurveI_h

#include <stddef.h>
#include <deque>

#include "DDImage/DDImage_API.h"
#include "DDImage/Matrix4.h"
#include "DDImage/RefCountedObject.h"
#include "DDImage/Vector2.h"
#include "DDImage/Vector3.h"
#include "DDImage/Vector4.h"

namespace DD
{
  namespace Image
  {
    class OutputContext;


    // Some API thoughts:
    // -Do we need to be able to programmatically set an expression for an attribute? What about toggling whether the attribute is animated? Maybe later...?

    // -We use OutputContext to avoid the explicit use of baked curves.
    // -Might also be useful to have a function for returning the nearest control point to some given coords?
    // -What about a method to return the points at which the curve intersects a pixel row?
    class DDImage_API CurveI : public RefCountedObject
    {
      public:
        typedef RefCountedPtr<CurveI> CurvePtr;
      protected:
        CurveI();
      //private:
       // CurveI(const CurveI &);
      public:
        ~CurveI() override;
        virtual size_t numControlPoints() const = 0;
        //! Number of valid dimentions in this.
        virtual size_t numDimensions() const = 0;
        //! Number of points per point.
        virtual size_t numCoords() const = 0;
        virtual bool isOpen() const = 0;
        virtual float getT(const OutputContext &, const size_t controlPointIndex) const = 0;

        virtual void addControlPoint(const OutputContext &ctx, const float t, const Vector2 &initialCoords) = 0;
        virtual void addControlPoint(const OutputContext &ctx, const float t, const Vector3 &initialCoords) = 0;
        virtual void addControlPoint(const OutputContext &ctx, const float t, const Vector4 &initialCoords) = 0;
        virtual void removeControlPoint(const size_t controlPointIndex) = 0;

        virtual void setControlPointCoords(const size_t controlPointIndex, const Vector2 &coords) = 0;
        virtual void setControlPointCoords(const size_t controlPointIndex, const Vector3 &coords) = 0;
        virtual void setControlPointCoords(const size_t controlPointIndex, const Vector4 &coords) = 0;
        virtual void setControlPointCoords(const OutputContext &ctx, const size_t controlPointIndex, const Vector2 &coords) = 0;
        virtual void setControlPointCoords(const OutputContext &ctx, const size_t controlPointIndex, const Vector3 &coords) = 0;
        virtual void setControlPointCoords(const OutputContext &ctx, const size_t controlPointIndex, const Vector4 &coords) = 0;

        virtual Vector4 controlPointCoords(const OutputContext &ctx, const size_t controlPointIndex) const = 0;
        virtual Vector4 curveCoordsAtT(const OutputContext &ctx, const float t) const = 0;

        virtual void setCurveAttribute(const char* attrName, const float value) = 0;
        virtual void setCurveAttribute(const OutputContext &ctx, const char* attrName, const float value) = 0;
        virtual float curveAttribute(const OutputContext &ctx, const char* attrName) const = 0;
        virtual float controlPointAttribute(const OutputContext &ctx, const size_t controlPointIndex, const char* attrName) const = 0;
 
        // Need different versions because you may only want to check using the first N components, e.g. when the 3rd component is pressure rather than z.
        virtual bool isNearCurve(const OutputContext &ctx, const float maxDistance, const Vector2 &coords) const = 0;
        virtual bool isNearCurve(const OutputContext &ctx, const float maxDistance, const Vector3 &coords) const = 0;
        virtual bool isNearCurve(const OutputContext &ctx, const float maxDistance, const Vector4 &coords) const = 0;
        //! Check wether a point is within a curve.
        virtual bool isInsideCurve(const OutputContext &ctx, const Vector2 &coords) const = 0;
        //! This is not yet implemented
        virtual bool isInsideCurve(const OutputContext &ctx, const Vector3 &coords) const = 0;
        //! This is not yet implemented
        virtual bool isInsideCurve(const OutputContext &ctx, const Vector4 &coords) const = 0;

        /*!
         * Convenience to the above for checking multiple points.
         * Uses a deque<bool> to avoid vector<bool> problems.
         */
        virtual void isInsideCurve(const OutputContext &ctx, const std::vector<Vector2> & coords, std::deque<bool> &results) const = 0;

        //! This is known as the 'extra matrix' in the UI.
        virtual Matrix4 getTransformMatrix(const OutputContext &ctx) const = 0;
        //! This is known as the 'extra matrix' in the UI.
        virtual void setTransformMatrix(const OutputContext &ctx, const Matrix4 &) = 0;
    };
  }
}

#endif

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.
