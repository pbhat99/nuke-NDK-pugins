// Polygon.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Polygon_H
#define DDImage_Polygon_H

#include "DDImage/PolygonPrimitive.h"

namespace DD
{
  namespace Image
  {

    /*!
       Polygon primtive class consisting of n vertices.  Can be defined
       as open or closed.  If closed, the edge between the first and
       last vertex is added.  All vertices must be coplanar otherwise
       unpredictable results will occur.
     */
    class DDImage_API Polygon : public PolygonPrimitive
    {
    protected:
      bool closed_;

      /*! Destructive copy. */
      void copy(const Polygon*);

    public:
      const char* Class() const override;
      Polygon(unsigned vertices, bool closed);
      Polygon(int p0, int p1, bool closed);
      Polygon(int p0, int p1, int p2, bool closed);
      Polygon(int p0, int p1, int p2, int p3, bool closed);
      Polygon(const Polygon &);

      Polygon& operator = (const Polygon& b) { copy(&b);
                                               return *this; }

      /*! Copy this Polygon into a new one, and return a pointer to it.
          Vertex data is duplicated, point indices are unchanged.
       */
      Primitive* duplicate() const override;

      /*! Indicated whether the polygon is open or closed.  If closed, the
         edge between the first and last vertex is added. */
      bool closed() const { return closed_; }
      void closed(bool v) { closed_ = v; }

      /*! Test for the intersection of this primitive with a given ray .  */
      bool IntersectsRay(const Ray &ray, int n, const PointList* pointList, CollisionResult *result) const override 
      {
        return Primitive::PolyIntersectsRay(ray, n, pointList, result);
      }

      void validate(PrimitiveContext*) override;

      /*! Subdivide the polygon into triangles and add them to the
          rendering pipeline.
       */
      void tessellate(Scene*, PrimitiveContext*) const override;
      unsigned faces() const override { return 1; }

      void draw_wireframe(ViewerContext*, PrimitiveContext*,
                                      Primitive* prev_prim = nullptr) const override;
      void draw_solid(ViewerContext*, PrimitiveContext*,
                                  Primitive* prev_prim = nullptr) const override;

      /*! Draw the single face as a filled polygon. */
      void draw_solid_face(int n, ViewerContext*, PrimitiveContext*) const override;
      void draw_primitive_num(ViewerContext*, PrimitiveContext*) const override;
      void draw_primitive_normal(ViewerContext*, PrimitiveContext*) const override;

      void print_info() const override;

      PrimitiveType getPrimitiveType() const override { return ePolygon;  }
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
