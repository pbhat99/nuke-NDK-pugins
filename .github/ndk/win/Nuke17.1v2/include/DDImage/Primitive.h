// Primitive.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Primitive_H
#define DDImage_Primitive_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Allocators.h"
#include "DDImage/Attribute.h"
#include "DDImage/Box3.h"
#include <vector>

#ifdef _WIN32
  #pragma warning(disable:4251)
#endif

namespace DD
{
  namespace Image
  {

    class Iop;
    class Pixel;
    class /*DDImage_API*/ ViewerContext;
    class VertexContext;
    class PrimitiveContext;
    class VArray;
    class Scene;
    struct CollisionResult;
    struct Ray;
    class GeoInfo;
    class PointList;

    enum  PrimitiveType
    {
      eUnknownPrimitive = -1,
      eTriangle,
      ePolygon,
      eMesh,
      ePoint,
      eParticles,
      eParticlesSprite,
      ePolyMesh,

      ePrimitiveTypeCount
    };

    /*! \class DD::Image::Primitive

        This is the base class of anything renderable!

        We use this class to handle renderable objects that can have simple
        geometry such as triangles, spheres, etc.

        Primitives are contained within GeoInfo structures which are responsible
        for their maintenance.
     */
    class DDImage_API Primitive
    {
      friend class GeoInfo;
      friend class GeometryList;
      friend class Scene;

    protected:
      std::vector< unsigned, STL3DAllocator<unsigned> > vertex_; //!< Point indices
      unsigned vertex_offset_;      //!< Vertex attribute starting index
      Vector3 normal_;              //!< Normal determined from vertex positions
      Iop* material_;               //!< Material assignment

      /* Vertex OpenGL drawing: */

      /*! Draw just the xyz position of a vertex in OpenGL */
      void draw_wireframe_vertex(int v, ViewerContext*, PrimitiveContext*) const;

      /*! Draw the vertex with normals and point color. */
      void draw_solid_vertex(int v, ViewerContext*, PrimitiveContext*) const;

      /*! Destructive copy. */
      void copy(const Primitive*);

      /*! Polygon face test for the intersection of this primitive face with a given ray .  */
      bool PolyIntersectsRay(const Ray &ray, int n, const PointList* pointList, CollisionResult *result) const;

    public:
      /* Vertex OpenGL drawing: */
      
      /*!
         Fill in the VArray with the correct values for this vertex.
         The values in this array as set by each vertex in the primitive are
         linerly interpolated by the renderer and passed to the fragment_shader.

         Before being passed to the fragment_shader, everything is divided by
         w and w is replaced with 1/w. This is to produce a linearly-interpolated
         value. Probably this function should do the division, not the caller!

         I guess subclasses could move the points around, but that probably
         could be done by the shaders instead...
       */
      virtual void vertex_shader(int v, Scene*, PrimitiveContext*, VArray& out, const Vector3* normal = nullptr) const;
      // // As above, but uses an existing VertexContext rather than making a temporary one.
      virtual void vertex_shader(int v, Scene*, PrimitiveContext*, VertexContext&, VArray& out, const Vector3* normal = nullptr) const;

      Primitive(unsigned vertices = 0);

      /*! Copy constructor. */
      Primitive(const Primitive &);

      /*! Base destructor. */
      virtual ~Primitive();
      virtual const char* Class() const = 0;
      virtual Primitive* duplicate() const = 0;

      /*! Return the data in three arrays pntArray, normalArray, and uvOrColArray.
          If the validUV is set to true than the uv/color array contains the uv values or else color values.
          This method is based on the draw_solid_vertex().
       */
      void getDataArray(int n, ViewerContext* ctx, PrimitiveContext* ptx, std::vector<Vector3>& pntArray,
                        std::vector<Vector3>& normalArray, std::vector<Vector4>& uvOrColArray, bool& validUV) const;

      /* Vertex handling: */
      const unsigned& vertex(unsigned v) const { return vertex_[v]; }
      unsigned& vertex(unsigned v) { return vertex_[v]; }
      unsigned vertices() const { return unsigned(vertex_.size()); }

      /*! Offset point list indices for all vertices.
       */
      void offset_point_indices(int offset);
      unsigned vertex_offset() const { return vertex_offset_; }
      void build_index_array(unsigned* indices, unsigned primitive, unsigned vertex) const;

      /*! Append a vertex to a primitive and assign it a point index.
          Returns the starting index in the vertex list.
       */
      unsigned add_vertex(unsigned point_index);

      /*! Add 'num' of vertices to primitive.  Returns the starting index in the
          vertex list.
          */
      unsigned add_vertices(unsigned num);

      //! Returns the xyz center average of the primitive.
      /*! Return the primitive's xyz local-space center. */
      virtual Vector3 average_center(const PointList*) const;

      /*! Returns the xyz center average of the primitive, plus local space bounds. */
      virtual Vector3 average_center_and_bounds(int f, const PointList* points, Vector3& min, Vector3 &max) const;

      /*! Returns the number of sub faces this primitive generates.  Pure virtual. */
      virtual unsigned faces() const = 0;
      /*! Returns the xyz center average of the sub face.  Base class returns the primitive center. */
      virtual Vector3 face_average_center(int n, const PointList* points) const
      { return average_center(points); }
      /*! Returns the number of vertices for the sub face. */
      virtual unsigned face_vertices(int n) const { return vertices(); }
      /*! Fill the pre-allocated array with vertices constituting the sub face. */
      virtual void get_face_vertices(int n, unsigned* array) const;

      /*! Returns the primitive bounding box */
      virtual Box3 get_bbox(const GeoInfo* info) const;

      /*! Returns the normal for face. */
      virtual Vector3 get_face_normal(int n, const PointList* points) const { return normal_; }

      /*! Return the geometric normal for vertex. */
      virtual Vector3 get_geometric_normal(int v, const PointList* points) const { return Vector3(0,0,0); }

      /*! Return the number of faces that vertex connects to and fills in the list of face indices. */
      virtual int get_vertex_faces(int v, std::vector<int>& faces) const { return 0; }

      /*! Test for the intersection of this primitive face with a given ray .  */
      virtual bool IntersectsRay(const Ray &ray, int n, const PointList* pointList, CollisionResult *result) const { return false; }

      /*! Initialize any variables prior to display or rendering. */
      virtual void validate(PrimitiveContext*) {}

      /*! Does this face in this primitive reference this vertex? */
      virtual bool faceUsesVertex(unsigned int faceIdx, unsigned int vertIdx) const;

      /* Shader functions: */
      Iop* material() const { return material_; }

      /* return the shader material used at rendering time*/
      Iop* renderingShader(const GeoInfo* info) const;

      /*! \fn const Vector3& Primitive::normal() const
         Returns a normal vector for the whole primitive. The subclass
         initializes this (by setting the normal_ member) to a vector that
         points out of "front" surface of the polygon. This normal is
         used if there is no normal attribute or zero-length normal for
         the points. It is also used to flip normals that appear to be
         backwards when rendering in OpenGL.
       */
      const Vector3& normal() const { return normal_; }

      /*! \fn Vector3 Primitive::vertex_normal(unsigned v, const PointList*) const
          Returns a normal that best represents the normal at \b point's
          location on the primitive.  Base class ignores the vertex argument and
          returns the primitive's base normal.
       */
      virtual Vector3 vertex_normal(unsigned v, const PointList*) const { return normal_; }

      /* Rendering: */
      virtual void tessellate(Scene*, PrimitiveContext*) const = 0;

      /* OpenGL drawing methods: */
      virtual void draw_wireframe(ViewerContext*, PrimitiveContext*,
                                  Primitive* prev_prim = nullptr) const = 0;
      virtual void draw_solid(ViewerContext*, PrimitiveContext*,
                              Primitive* prev_prim = nullptr) const = 0;

      /*! Draw a sub-face as a filled polygon. */
      virtual void draw_solid_face(int n, ViewerContext*, PrimitiveContext*) const;

      /* OpenGL component drawing: */

      /*! Draw primitive's normal as a short line coming from it's center.
       */
      virtual void draw_primitive_normal(ViewerContext*, PrimitiveContext*) const;

      /*! Draw primitive's index number.
       */
      virtual void draw_primitive_num(ViewerContext*, PrimitiveContext*) const;

      /*! Draw vertex number.
       */
      virtual void draw_vertex_num(ViewerContext*, PrimitiveContext*) const;

      /*! Draw vertex normals as short lines coming from vertex's position.
       */
      virtual void draw_vertex_normals(ViewerContext*, PrimitiveContext*) const;

      /*! Print the vertex UV value at the vertex's position.
       */
      virtual void draw_vertex_uvs(ViewerContext*, PrimitiveContext*) const;

      virtual void print_info() const;
      
      /*! OpenGL helper function to draw a single normal expressed in geometry local coordinate */
      static void draw_normal(const Vector3& p, const Vector3& n, ViewerContext*, PrimitiveContext*);      

      /*! Only for internal Nuke use, it will be deprecated */
      virtual PrimitiveType getPrimitiveType() const { return eUnknownPrimitive; }
      
      /* Required new/delete operator overload */
      void* operator new(size_t size);
      void  operator delete(void* mem);
      void* operator new[](size_t size);
      void  operator delete[](void* mem);
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
