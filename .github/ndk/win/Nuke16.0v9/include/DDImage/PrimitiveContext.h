// PrimitiveContext.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_PrimitiveContext_H
#define DDImage_PrimitiveContext_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Attribute.h"
#include "DDImage/MatrixArray.h"
#include "DDImage/GeoInfo.h"

#include "DDImage/rPrimitive.h"
#include "DDImage/rTriangle.h"

#include "DDImage/Scene.h"

#include <vector>

namespace DD
{
  namespace Image
  {
    /*! A context structure to reference environmental information for the
        primitive during rendering.  Contains pointers to the primitive's
        parent GeoInfo, the valid MatrixArray for this object, and
        attribute link pointers to the standard attribute types.
     */
    class DDImage_API PrimitiveContext
    {
      friend class Scene;
      friend class GeoInfo;
      friend class Primitive;
      friend class ParticlesSprite;
      friend class PolygonPrimitive;
      friend class Mesh;

      /* References assigned by parent GeoInfo: */
      GeoInfo* geoinfo_, * mb_geoinfo_; //!< Parent GeoInfos
      /*! Object/Scene transforms. */
      MatrixArray* transforms_, * mb_transforms_;
      Primitive* primitive_;            //!< Current primitive

      /* Attribute indexes: */
      unsigned indices_[Group_Last];    //!< Attribute index array

      Box3 face_uv_bbox_;               //!< Primitive face clipmask
      unsigned face_clipmask_;          //!< Primitive face UV bounds
    public:

      /* Private storage for primitive tessellation, used
         during tessellation into render primitives. */
      std::vector<unsigned int> face_vertices_;  //!< vertex indices of a polygon face
      std::vector<unsigned int> triangulation_;  //!< face vertex order that defines triangulation of a polygon face



      /*! Basic constructor just zeros all pointers.
       */
      PrimitiveContext();
      virtual ~PrimitiveContext() {}

      /* Parent references: */
      void set_geoinfo(GeoInfo* g, GeoInfo* mg = nullptr)
      { 
        geoinfo_ = g;
        mb_geoinfo_ = mg; 
      }

      GeoInfo* geoinfo() const { return geoinfo_; }
      GeoInfo* mb_geoinfo() const { return mb_geoinfo_; }
      Primitive* primitive() const { return primitive_; }

      /* UV Bounds */
      const Box3& face_uv_bbox() { return face_uv_bbox_; }
      unsigned face_clipmask() { return face_clipmask_; }
      
      /* Transforms */
      void set_transforms(MatrixArray* m, MatrixArray* mb = nullptr)
      { transforms_ = m;
        mb_transforms_ = mb; }
      MatrixArray* transforms() const { return transforms_; }
      const Matrix4& matrix(int n) const { return transforms_->matrix(n); }
      const Matrix4& inverse_matrix(int n) const { return transforms_->inverse_matrix(n); }
      /* Motionblur Transforms */
      MatrixArray* mb_transforms() const { return mb_transforms_; }
      const Matrix4& mb_matrix(int n) const { return mb_transforms_->matrix(n); }
      const Matrix4& mb_inverse_matrix(int n) const { return mb_transforms_->inverse_matrix(n); }

      /* Attribute indices: */
      const unsigned* indices() const { return indices_; }
      unsigned index(int n) const { return indices_[n]; }

      void setPrimitive(Primitive* primitive) { primitive_ = primitive; }
      void setPrimitiveIndex(unsigned int index) { indices_[Group_Primitives] = index; }

      /* Add a render primitive to a Scene within this context. Sub-classes can override this, for example if they
       wish to save a list of primitives to add and delay the actual addition until later, e.g. if the render
       primitive generation can happen in multiple threads. */
      virtual void addToScene(rPrimitive *primitiveToAdd, Scene *scene) { scene->add_clipped_render_primitive(primitiveToAdd); }

      /* Context-dependent function to create a new rTriangle instance. Sub-classes can override this, e.g. to use 
         an existing rTriangle pointer from a memory pool. */
      virtual DD::Image::rTriangle *newRenderTriangle(const GeoInfo * info, const Primitive * p) { return new rTriangle(info, p); }
      virtual DD::Image::rTriangle *newRenderTriangle(const rTriangle &t) { return new rTriangle(t); }

      /* Delete an rPrimitive within this context. Sub-classes can override this in the case that calling
         the primitive's delete operator directly is not appropriate, for example if the primitive
         resides in a memory pool.*/
      virtual void deleteRenderPrimitive(DD::Image::rPrimitive *p) { delete p; }
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
