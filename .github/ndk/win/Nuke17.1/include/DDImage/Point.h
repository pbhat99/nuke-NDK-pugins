// Point.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Point_H
#define DDImage_Point_H

#include "DDImage/Primitive.h"
#include "DDImage/GeoInfo.h"

namespace DD
{
  namespace Image
  {

    /*! Point primitive.
     */
    class DDImage_API Point : public Primitive
    {
    public:
      enum RenderMode { POINT, PARTICLE, DISC, SQUARE, SPHERE };

    protected:
      RenderMode render_mode_;
      float radius_;

      /*! Destructive copy. */
      void copy(const Point*);

    public:
      const char* Class() const override;
      Point(int type, float radius = 1.0f, unsigned point = 0);
      Point(RenderMode type, float radius = 1.0f, unsigned point = 0);
      Point(const Point &);

      float radius() const { return radius_; }
      void radius(float v) { radius_ = v; }
      RenderMode render_mode() const { return render_mode_; }
      void render_mode(RenderMode v) { render_mode_ = v; }

      Point& operator = (const Point& b) { copy(&b);
                                           return *this; }

      const Vector4& getColor(PrimitiveContext* ptx) const;

      /*! Copy this Point into a new one, and return a pointer to it.
          Vertex data is duplicated, point indices are unchanged.
       */
      Primitive* duplicate() const override;

      /*! Returns the xyz center average of the point. */
      Vector3 average_center(const PointList* points) const override
      { return (*points)[vertex_[0]]; }

      /*! Returns the xyz center average of the primitive, plus local space bounds. */
      Vector3 average_center_and_bounds(int f, const PointList* points, Vector3& min, Vector3 & max) const override
      { min = (*points)[vertex_[0]];  max = (*points)[vertex_[0]];  return (*points)[vertex_[0]]; }

      /*! Push a single rPoint primitive into the rendering stream.
       */
      void tessellate(Scene*, PrimitiveContext*) const override;
      unsigned faces() const override { return 0; }

      void draw_wireframe(ViewerContext*, PrimitiveContext*,
                                      Primitive* prev_prim = nullptr) const override;
      void draw_solid(ViewerContext*, PrimitiveContext*,
                                  Primitive* prev_prim = nullptr) const override;
      void draw_primitive_num(ViewerContext*, PrimitiveContext*) const override;

      PrimitiveType getPrimitiveType()  const override { return ePoint;  }
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
