// rPoint.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_rPoint_H
#define DDImage_rPoint_H

#include "DDImage/Point.h"
#include "DDImage/rPrimitive.h"
#include "DDImage/VertexContext.h"

namespace DD
{
  namespace Image
  {

    /*! Renderable point primitive.
     */
    class DDImage_API rPoint : public rPrimitive
    {
    public:
      VArray v;
      Point::RenderMode render_mode;
      float radius;
      Vector4 _color;

    public:
      const char* Class() override { return "rPoint"; }
      rPoint(const GeoInfo * info, const Primitive * p, Point::RenderMode mode, float r);
      rPoint(const GeoInfo * info, const Primitive * p, int mode, float r);
      rPoint(const GeoInfo * info, const Primitive * p, Point::RenderMode mode, const Vector4 &color, float r);
      rPoint(const GeoInfo * info, const Primitive * p, int mode, const Vector4 &color, float r);
      rPoint(const rPoint &);

      /*! Adds the point to the render if it is beyond the near plane. */
      bool add_to_render(Scene*, PrimitiveContext*) override;
      bool intersect(float x, float y) const override;
      bool intersect(float x, float y, float* dist) const override;
      bool intersect_dist_squared(float x, float y, float* dist_squared) const override;

      /*! Intersect the point with a scanline.  Return whether it was
          successful and the left and right intersection vertices.
       */
      bool intersect_scanline(float y, int& X, int& R,
                                          VArray& out, VArray& du, VArray& dv) const override;

      rPrimitiveType getPrimitiveType() const override { return eRPoint; }

      /*! Not implemented yet.
       */
      float trace(Scene*) override;

      /*!
         Distance the given screen-space point is away from the object.
         Returns 0 or negative if inside the object.
       */
      float distance(float x, float y) const override;

      /*!
         Square of the distance to the nearest edge the object in screen-space.
         Returns 0 or negative if inside the object.
       */
      float distance_squared(float x, float y) const override;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
