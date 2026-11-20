// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_PositionVector_KnobI_h
#define DD_Image_PositionVector_KnobI_h

#include "DDImage/Vector3.h"
#include "DDImage/Matrix4.h"

namespace DD {
  namespace Image {

    // Forward declarations.
    class OutputContext;

    /// The interface for position vector knobs.
    class DDImage_API PositionVector_KnobI
    {
    public:
      virtual ~PositionVector_KnobI();

      virtual Vector3 getFrom(const OutputContext& oc) const = 0;
      virtual Vector3 getTo(const OutputContext& oc) const = 0;

      virtual void setFrom(const OutputContext& oc, const Vector3 pos) = 0;
      virtual void setTo(const OutputContext& oc, const Vector3& pos) = 0;

      /// Set the from and to positions to a single (un-animated) value.
      virtual void setFrom(const Vector3 pos) = 0;
      virtual void setTo(const Vector3& pos) = 0;

      virtual Matrix4 toMatrix(const OutputContext& oc) const = 0;
    };

  } // namespace Image
} // namespace DD

#endif // DD_Image_PositionVector_KnobI_h
