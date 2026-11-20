// PolyMesh.h

#ifndef DDImage_PolyMesh_H
#define DDImage_PolyMesh_H

#include "DDImage/GeoInfo.h"
#include "DDImage/PolygonPrimitive.h"


namespace DD 
{ 
  namespace Image 
  {
  
    /// PolyMesh is a connected series of polygons comprising a contiguous surface.
    class DDImage_API PolyMesh : public PolygonPrimitive 
    {
      struct SubFace
      {
        // number of vertices in the subface
        unsigned numVertex;

        // starting vertex index in Primitive::vertex_ array
        unsigned startVertex;
      };

      // sub faces array
      std::vector<SubFace, STL3DAllocator<SubFace> > _subFaces;
      
    public:

      PolyMesh();
      PolyMesh(unsigned icount, unsigned fcount);

      ~PolyMesh() override;

      
      const char* Class() const override { return "PolyMesh"; }

      
      Primitive* duplicate() const override { return new PolyMesh(*this); }
   
      
      PrimitiveType getPrimitiveType() const override { return ePolyMesh; };

      /*! Add a face to the mesh, return the face index of the new face or -1 if failure. */
      int add_face(unsigned nVerts, const int* vertIndices,  bool reverse = false);

      
      unsigned faces() const override;

      
      Vector3 face_average_center(int n, const PointList*) const override;

      
      unsigned face_vertices(int n) const override;

      
      void get_face_vertices(int n, unsigned* array) const override;

      
      Vector3 get_face_normal(int n, const PointList* points) const override;

      
      void validate(PrimitiveContext*) override;
      
      
      void draw_wireframe(ViewerContext*,
                          PrimitiveContext*,
                          Primitive* prev_prim=nullptr) const override;
      
      void draw_solid(ViewerContext*,
                      PrimitiveContext*,
                      Primitive* prev_prim=nullptr) const override;

      
      void draw_primitive_normal(ViewerContext*, PrimitiveContext*) const override;

      
      void draw_primitive_num(ViewerContext*, PrimitiveContext*) const override;

      
      void draw_vertex_num(ViewerContext*, PrimitiveContext*) const override;

      
      void draw_solid_face(int n, ViewerContext* ctx, PrimitiveContext* ptx) const override;

      
      void print_info() const override;

      
      bool IntersectsRay(const Ray &ray, int n, const PointList* pointList, CollisionResult *result) const override
      {
        return Primitive::PolyIntersectsRay(ray, n, pointList, result);
      }
    };

  } 
} // namespace DD::Image

#endif

// end of PolyMesh.h
