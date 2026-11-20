// Triangle.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Triangle_H
#define DDImage_Triangle_H

#include "DDImage/PolygonPrimitive.h"

namespace DD
{
  namespace Image
  {

    /*! Simple triangle primitive.
     */
    class DDImage_API Triangle : public PolygonPrimitive
    {
    protected:

      /*! Destructive copy. */
      void copy(const Triangle*);

    public:
      const char* Class() const override;

      /*! Constructor accepting 3 points. */
      Triangle(int p0 = 0, int p1 = 1, int p2 = 2);

      /*! Copy constuctor. */
      Triangle(const Triangle &);

      Triangle& operator = (const Triangle& b) { copy(&b);
                                                 return *this; }
      /*! Copy this Triangle into a new one, and return a pointer to it.
          Vertex data is duplicated, point indices are unchanged.
       */
      Primitive* duplicate() const override;

      //! Returns the xyz center average of the triangle.
      /*! Return the primitive's xyz local-space center. */
      Vector3 average_center(const PointList*) const override;

      //! Test for the intersection of this primitive with a given ray.
      /*! If a collision is detected, information is returned in the collision result
      */
      bool IntersectsRay(const Ray &ray, int n, const PointList* pointList, CollisionResult *result) const override 
      {
        return Primitive::PolyIntersectsRay(ray, n, pointList, result);
      }

      /*! Calculate the surface normal - points out with a ccw winding.
       */
      void validate(PrimitiveContext*) override;

      unsigned faces() const override { return 1; }

      void draw_wireframe(ViewerContext*, PrimitiveContext*,
                                      Primitive* prev_prim = nullptr) const override;
      void draw_solid(ViewerContext*, PrimitiveContext*,
                                  Primitive* prev_prim = nullptr) const override;

      /*! Draw the single face as a filled triangle. */
      void draw_solid_face(int n, ViewerContext*, PrimitiveContext*) const override;
      void draw_primitive_num(ViewerContext*, PrimitiveContext*) const override;
      void draw_primitive_normal(ViewerContext*, PrimitiveContext*) const override;

      void print_info() const override;

      PrimitiveType getPrimitiveType()  const override { return eTriangle;  }
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
