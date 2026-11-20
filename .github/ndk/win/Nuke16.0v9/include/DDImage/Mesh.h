// Mesh.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Mesh_H
#define DDImage_Mesh_H

#include "DDImage/GeoInfo.h"
#include "DDImage/PolygonPrimitive.h"

namespace DD
{
  namespace Image
  {

    // Plane orientations:
    enum { PLANE_XY = 0, PLANE_YZ, PLANE_ZX };
    extern DDImage_API const char* const plane_orientation_modes[];

    // Subdivision methods:
    enum MeshSubdivisionMethod {
      ROWS,
      COLUMNS,
      ROWS_COLUMNS,
      QUADS,
      TRIANGLES_A,
      TRIANGLES_B,
      ALT_TRIANGLES
    };
    extern const char* const subdivision_modes[];

    // define min row/col subdivisions
#define kMinGLDrawRow 20
#define kMinGLDrawCol 20

    /*! A 2-dimensional mesh of m*n vertices.
     */
    class DDImage_API Mesh : public PolygonPrimitive
    {
    protected:
      int columns_, rows_; //!< Number of rows and columns.
      bool wrapU_, wrapV_; //!< Whether mesh wraps around - only one can be true
      MeshSubdivisionMethod subd_method_; //!< What method to subdivide the mesh

      /*! Destructive copy. */
      void copy(const Mesh*);

    private:
      bool subdivideMeshData(int minRows, int minCols, ViewerContext* ctx, PrimitiveContext* ptx,
                             std::vector<Vector3>& newPntArray, std::vector<Vector3>& newNormalArray, std::vector<Vector4>& newUVOrColArray,
                             bool& validUV, int& newRows, int& newCols) const;

    public:
      const char* Class() const override;
      Mesh(int columns, int rows, bool wrapU = false, bool wrapV = false, MeshSubdivisionMethod subd = QUADS);
      Mesh(const Mesh &);
      ~Mesh() override;

      /*! Copy this Mesh into a new one, and return a pointer to it.
          Vertex data is duplicated, point indices are unchanged.
       */
      Primitive* duplicate() const override;

      int  columns() const { return columns_; }
      int  rows() const { return rows_; }
      bool wrapU() const { return wrapU_; }
      bool wrapV() const { return wrapV_; }
      MeshSubdivisionMethod subd_method() const { return subd_method_; }
      void subd_method(MeshSubdivisionMethod v)  { subd_method_ = v; }

      static int mesh_index(int x, int y, int w) { return y * w + x; }

      int vertex_index(int x, int y) const { return mesh_index(x, y, columns_); }
      unsigned point_index(int x, int y) const { return vertex(mesh_index(x, y, columns_)); }

      // Pointlist convenience functions:

      /*! Add the points for the mesh to the GeoInfo's PointList.
          This is a convenience function.
       */
      int  add_points(PointList*, int plane = PLANE_ZX);
      void remap_points(PointList*, int plane = PLANE_ZX, float sx = 1.0f, float sy = 1.0f) const;

      /*! Add uvs to the points.  U/V will map 0->1.
          This is a convenience function.
       */
      void create_point_uvs(Attribute*, float scaleU = 1.0f, float scaleV = 1.0f) const;

      /*! Add uvs to the points.  U/V will map (minU, minV) -> (maxU, maxV).
          This is a convenience function.
       */
      void create_point_uvs(Attribute* attrib, float minU, float minV, float maxU, float maxV) const;

      /*! Add normal N to each point.
          This is a convenience function.
       */
      void create_point_normals(Attribute*, const Vector3& N) const;

      //! Test for the intersection of this primitive with a given ray.
      /*! If a collision is detected, information is returned in the collision result
      */
      bool IntersectsRay(const Ray &ray, int n, const PointList* pointList, CollisionResult *result) const override
      {
        return Primitive::PolyIntersectsRay(ray, n, pointList, result);
      }

      /*! If Normal attribute doesn't exist, create it from the constituent faces.
       */
      void validate(PrimitiveContext*) override;

      /*! Return a normal that best represents the normal at \b point's
          location on the mesh surface.
       */
      Vector3 vertex_normal(unsigned v, const PointList*) const override;
      Vector3 get_face_normal(int n, const PointList* points) const override;

      /*! Subdivide a single face of the mesh into triangles and add
        them to the rendering pipeline.
       */
      void tessellateFace(int faceIndex,
                                       Scene* scene,
                                       PrimitiveContext* ptx,
                                       VertexContext *vtx) const override;
      
      void tessellate(Scene* scene, PrimitiveContext* ptx) const override;
    
      unsigned faces() const override { return (columns_ - 1) * (rows_ - 1); }

      /*! Return the subface's average local-space center. */
      Vector3 face_average_center(int n, const PointList*) const override;
      unsigned face_vertices(int n) const override { return 4; }

      /*! Fill the pre-allocated array with vertices constituting the sub face.
       */
      void get_face_vertices(int n, unsigned* array) const override;

      void draw_wireframe(ViewerContext*, PrimitiveContext*,
                                      Primitive* prev_prim = nullptr) const override;
      void draw_solid(ViewerContext*, PrimitiveContext*,
                                  Primitive* prev_prim = nullptr) const override;

      /*! Draw the specified face as a filled quad. */
      void draw_solid_face(int n, ViewerContext*, PrimitiveContext*) const override;

      //! Draw poly's normal
      void draw_primitive_normal(ViewerContext*, PrimitiveContext*) const override;

      void print_info() const override;

      PrimitiveType getPrimitiveType()  const override { return eMesh;  }
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
