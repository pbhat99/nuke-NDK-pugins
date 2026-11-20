// PolygonPrimitive.h
// Copyright (c) 2014 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_PolygonPrimitive_H
#define DDImage_PolygonPrimitive_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Primitive.h"

namespace DD
{
  namespace Image
  {
    
    /*! Base class for a polygon-based primitive with 
      generic tessellation functions to turn the 
      primitive into renderable primitives.
      All polygon-based primitives tessellate to 
      DD::Image::rTriangles.*/
    class DDImage_API PolygonPrimitive : public Primitive
    {
      class TessellationHelper;

    public:
      PolygonPrimitive(unsigned vertices = 0)
        : Primitive(vertices)
        {}

      ~PolygonPrimitive() override {}

      /* Tessellate a polygon-based primitive into renderable primitives */
      void tessellate(Scene*, PrimitiveContext*) const override;

      /* Tessellate a single face of a polygon-based primitive
         into renderable primitives and add them into the
         scene via the primitive context. */
      virtual void tessellateFace(int faceIndex,
                                  Scene* scene,
                                  PrimitiveContext* ptx,
                                  VertexContext *vtx) const;
    };



  }
}

#endif
