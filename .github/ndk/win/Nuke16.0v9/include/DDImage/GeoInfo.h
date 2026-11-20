// GeoInfo.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_GeoInfo_h
#define DDImage_GeoInfo_h

#include "DDImage/Box3.h"
#include "DDImage/Channel.h"
#include "DDImage/Channel3D.h"
#include "DDImage/Matrix3.h"
#include "DDImage/Matrix4.h"
#include "DDImage/GeoSelection.h"
#include "DDImage/Hash.h"
#include "DDImage/Primitive.h"
#include "DDImage/RefCountedObject.h"
#include "DDImage/Allocators.h"
#include "DDImage/VertexContext.h"
#include "DDImage/OutputContext.h"
#include "DDImage/SubdivisionContext.h"

#include <vector>

namespace DD
{
  namespace Image
  {

    class GeoOp;
    class Pixel;
    class ViewerContext;

    enum Display3DMode : int; // Defined in ViewerContext.h
    enum RenderMode : int; // Defined in ViewerContext.h

    //------------------------------------------------------------------

    /*! A vector of Vector3 with a custom BlockAllocator. */
    class PointList : public RefCountedObject, public std::vector< Vector3, STL3DAllocator<Vector3> >
    {
    public:

      /*! Constructs an empty vector of points. */
      PointList()
      {
      }

      /*! Constructs a vector of points by copying the contents of \b other. */
      PointList(const PointList& other)
      {
        copy(other);
      }

      /*! Copies the contents of \b other into this vector of points. */
      void operator=(const PointList& other)
      {
        copy(other);
      }

      /*! Copies the contents of \b other into this vector of points. */
      void copy(const PointList& other)
      {
        assign(other.begin(), other.end());
      }

    };

    typedef RefCountedPtr<PointList> PointListPtr;

    /*! A vector of Primitive with a custom BlockAllocator. */
    class PrimitiveList : public RefCountedObject
    {
      typedef std::vector< Primitive*, STL3DAllocator<Primitive*> > PrimList;
      PrimList _prims;

    public:
      /*! Constructs an empty vector of primitives. */
      PrimitiveList()
      {
      }

      /*! Deletes all primitives on destruction. */
      ~PrimitiveList() override
      {
        for (size_t i = 0; i < _prims.size(); i++) {
          delete _prims[i];
        }
      }

      /*! Access the underlying Primitive data. */
      const PrimList& data() const
      {
        return _prims;
      }

      /*! Access the underlying Primitive data. */
      PrimList& data()
      {
        return _prims;
      }

      /*! How many Primitive are in this vector. */
      size_t size() const
      {
        return _prims.size();
      }

      /*! Accesses the Primitive at index \b i. Doesn't check for out of bounds. */
      Primitive*& operator[](const size_t i)
      {
        return _prims[i];
      }

      /*! Accesses the Primitive at index \b i. Doesn't check for out of bounds. */
      const Primitive* operator[](const size_t i) const
      {
        return _prims[i];
      }

      /*! Reserves a buffer for \b i Primitives. */
      void reserve(int i)
      {
        _prims.reserve(i);
      }

      /*! Returns how many Primitives the buffer can hold before it needs to be resized. */
      size_t capacity() const
      {
        return _prims.capacity();
      }

      /*! Adds a Primitive to the end of the vector. */
      void push_back(Primitive* p)
      {
        _prims.push_back(p);
      }

      /*! Returns whether there are Primitives in this vector. */
      bool empty() const
      {
        return _prims.empty();
      }
    };

    typedef RefCountedPtr<PrimitiveList> PrimitiveListPtr;

    //------------------------------------------------------------------

    /*! Geometry part groups. */
    enum GroupType {
      Group_None       = -1,
      //
      Group_Primitives = 0,
      Group_Vertices   = 1,
      Group_Points     = 2,
      Group_Object     = 3,
      //
      Group_Matrix     = 4,
      Group_Attributes = 5,
      //
      Group_Last = 6
    };

    /*! Geometry part mask flags. */
    enum {
      Mask_No_Geometry  = 0x00000000,
      Mask_Primitives   = 0x00000001, //!< Primitive list
      Mask_Vertices     = 0x00000002, //!< Vertex group
      Mask_Points       = 0x00000004, //!< Point list
      Mask_Geometry     = Mask_Primitives | Mask_Vertices | Mask_Points,
      //
      Mask_Object       = 0x00000008, //!< The Object
      Mask_Matrix        = 0x00000010, //!< Local->World Transform Matrix
      Mask_Attributes   = 0x00000020, //!< Attribute list
      //
      Mask_All_Geometry = Mask_Geometry | Mask_Attributes | Mask_Object | Mask_Matrix,
    };

    /*! Geometry part mask.  Each bit represents a logical geometric component. */
    typedef unsigned GeometryMask;

    /*! Group name strings. */
    extern DDImage_API const char* const group_names[];

    //------------------------------------------------------------------

    /*! Attribute reference class.
     */
    class DDImage_API AttribContext
    {
    public:
      AttribContext() : group(Group_None), name(nullptr), type(INVALID_ATTRIB),
        attribute(), channel(0), varying(false), recursive(false) {}

      GroupType group;        //!< Geometry group attribute is associated with
      const char* name;       //!< Attribute name - should be a NYI dictionary reference!!
      AttribType type;        //!< Attribute type - should be a NYI dictionary reference!!
      AttributePtr attribute; //!< The allocation
      unsigned channel;       //!< Starting channel in a packed float array
      bool varying;           //!< Whether the attribute should be interpolated
      bool recursive;         //!< Whether the attribute should be recursed

      bool empty() const { return !attribute || !attribute->size(); }
      bool not_empty() const { return !empty(); }

      // Data access convenience methods:
      void* array(const unsigned* indices) const
      { return attribute->array(indices[group]); }
      float& flt(const unsigned* indices) const
      { return attribute->flt(indices[group]); }
      Vector2& vector2(const unsigned* indices) const
      { return attribute->vector2(indices[group]); }
      Vector3& vector3(const unsigned* indices) const
      { return attribute->vector3(indices[group]); }
      Vector4& vector4(const unsigned* indices) const
      { return attribute->vector4(indices[group]); }
      Vector3& normal(const unsigned* indices) const
      { return attribute->normal(indices[group]); }
      int& integer(const unsigned* indices) const
      { return attribute->integer(indices[group]); }
      char*& string(const unsigned* indices)  const
      { return attribute->string(indices[group]); }
      std::string& stdstring(const unsigned* indices)  const
      { return attribute->stdstring(indices[group]); }
      void*& pointer(const unsigned* indices) const
      { return attribute->pointer(indices[group]); }
      Matrix3& matrix3(const unsigned* indices) const
      { return attribute->matrix3(indices[group]); }
      Matrix4& matrix4(const unsigned* indices) const
      { return attribute->matrix4(indices[group]); }

      /*! Copy the float data from the attribute to a VArray structure. */
      void copy_to_channels(const unsigned* indices, VArray& out) const;

      /*! Print some information about the reference to stdout.
       */
      void print_info() const;
    };

    typedef std::vector<AttribContext> AttribContextList;

    /*! Store the rendering state of a geometric object.
    */		
    struct DDImage_API GeoInfoRenderState
    {
      bool castShadow;             //!< Cast shadow
      bool receiveShadow;          //!< Receive shadow
      bool preMultiplierAlphaComp; //!< Force the pre-multiplier alpha compositing.
                                   //!< When enabled avoid to use Iop::blending_shader for the sample composition.
      bool polygonEdge;            //!< When enabled keeps polygons edges information
      DisplacementCoefficients displacement;  //!< displacement coefficients

      GeoInfoRenderState();
    };

    //------------------------------------------------------------------


    /*! \class DD::Image::GeoInfo

        \brief Contains a list of points and geometric primitives.
     */
    /*! 3D data is stored in a Scene object. The Scene contains a vector of
       these, each one describes a single geometric object that is shaded with
       a single set of shaders and transformed by a single matrix.
     */
    class DDImage_API GeoInfo
    {
      friend class GeoOp;
      friend class GeometryList;
      friend class Primitive;

    public:

      class DDImage_API FacePrimitiveMappingHelper
      {
      public:
        struct FacePrimitiveMappingHelperImpl;

        FacePrimitiveMappingHelper(const GeoInfo* geo);
        ~FacePrimitiveMappingHelper();

        /*! Given a face number, find out which primitive it belongs to, and get the face index
            relative to this primitive. Return true if succesful, false if the face number is out
            of range.
         */
        bool getPrimitiveAndFaceIndex(int faceNumber, int& primIndexOut, int& faceIndexOut) const;

      private:

        FacePrimitiveMappingHelper& operator=(const FacePrimitiveMappingHelper&); // disable assignment operator
        FacePrimitiveMappingHelper(const FacePrimitiveMappingHelper &); // disable copy constructor

        FacePrimitiveMappingHelperImpl* _impl;
      };

      /* Object Globals: */
      Matrix4 matrix;           //!< Transformation matrix
      Iop* material;            //!< Overall material assignment
      bool useMaterialContext;  //!< Use OutputContext with material
      DD::Image::OutputContext materialContext; //!< OutputContext for material

      /* Rendering/display parameters: */
      RenderMode render_mode;   //!< Turn Render output on/off
      Display3DMode display3d;  //!< How to draw (see ViewerContext style3d)
      bool selected;            //!< Draw with selected wireframe
      bool selectable;          //!< Object can be selected

      /* Rendering state */
      GeoInfoRenderState renderState;

      /* Parent Ops: */
      GeoOp* source_geo;        //!< Last non-merge GeoOp
      GeoOp* final_geo;         //!< Last GeoOp before a merge
      GeoOp* select_geo;        //!< Last selectable GeoOp
      GeoOp* recursion_geo;     //!< GeoOp which started a recursing loop

      /* source node gl color */
      bool valid_source_node_gl_color; //!< true if gl node color is valid
      unsigned source_node_gl_color; //!< Source gl node color for drawing

    public:
      /* Primary attribute references: */
      const AttribContext* UV_ref;  //!< Reference to the primary UV
      const AttribContext* N_ref;   //!< Reference to the primary normal
      const AttribContext* Cf_ref;  //!< Reference to the primary surface color
      const AttribContext* PW_ref;  //!< Reference to the world-space point
      const AttribContext* VEL_ref; //!< Reference to the point velocity

      Attribute* selections[Group_Last]; //!< Group selection attributes

      /*! OpenGL call list management. */
      unsigned selected_items_list;
      unsigned int  selected_items_IB;
      unsigned int  selected_items_CB;
      unsigned int  selected_indices_count;
      struct DDImage_API DrawLists : public RefCountedObject
      {
        unsigned solid_call_list;
        unsigned transparent_call_list;
        unsigned wireframe_call_list;
        unsigned icon_call_list;
        unsigned icon_draw_mask;

        unsigned int pick_ids_list;
        unsigned selectable_items_list;

        // Gl vertex buffers
        unsigned int  object_VB;
        unsigned int  object_FVB;
        unsigned int  wireframe_IB;
        unsigned int  wireframe_indices_count;

        DrawLists() : solid_call_list(0), transparent_call_list(0), wireframe_call_list(0), icon_call_list(0),
          icon_draw_mask(0), pick_ids_list(0), selectable_items_list(0), 
          object_VB(0), object_FVB(0), wireframe_IB(0), wireframe_indices_count(0)
        {}
      };

      typedef RefCountedPtr<DrawLists> DrawListsPtr;

      /*! Geometry pointers maintained by GeoOps. */
      struct DDImage_API Cache
      {
        PrimitiveListPtr primitives;          //!< Primitive list
        unsigned vertices;                    //!< Total number of vertices in all primitives
        PointListPtr points;                  //!< Point list
        AttribContextList attributes;         //!< List of attribute reference structures
        //
        DrawListsPtr call_lists;              //!< OpenGL call list management structure
        //
        enum CacheType { SOURCE, REFERENCE }; //!< Cache types
        CacheType type;                       //!< Type of cache
        int version;                          //!< Copy number
        Hash src_id;                          //!< Unique source object identifier
        Hash out_id;                          //!< Unique output list object identifier
        unsigned recursion_mask;              //!< Which groups recurse
        Box3 bbox;                            //!< Bounding-box

        Cache() : vertices(0), 
          type(REFERENCE), version(0),
          src_id(0), out_id(0), recursion_mask(0), bbox()
        { attributes.reserve(10); }
      };

    public:

      enum GeoFlags
      {
        DEFAULT = 0,
        NO_RAY_CAST = 1 << 0,
        POLYGON_OFFSET = 1 << 1
      };

      /*! Constructor clears all the geometry lists and nulls the
          materials.
       */
      GeoInfo();
      //  GeoInfo(const GeoInfo &b);

      /*! Point data is duplicated and a shallow copy of primitive data is made.
       */
      void copy(const GeoInfo* b);
//      /*! Append another GeoInfo's geometry into this one.
//          Point data is duplicated and every primitive is copied.
//       */
//      void append(const GeoInfo &b);

      //! Get primitives and attributes ready for display/rendering.
      /*! Re-assign the standard attribute references from the current list.
       */
      void validate();
      
      //! Calculate axis-aligned bounding box.
      /*! Get the GeoInfo's bounding-box up-to-date by finding a bounding
          volume that fits around all points.  Alternate bounding geometry
          such as spheres can be calculated at this time.
          Produces bounding-box by adding each point to the local
          bounding box.
       */
      void update_bbox();

      bool ignoreForRayCasting() const { return _geoInfoFlags & NO_RAY_CAST; }
      void setGeoInfoFlag( unsigned int flags ) {  _geoInfoFlags |= flags; }
      void clearGeoInfoFlag( unsigned int flags ) { _geoInfoFlags &= ~( flags ); }

      /*! Get primitive. */
      const Primitive* primitive(unsigned n) const { return cache.primitives ? (*cache.primitives)[n] : nullptr; }

      const Box3& bbox() const { return cache.bbox; }
      unsigned output_index() const { return output_index_; }

      /* Read-only access methods: */
      
      unsigned primitives() const { return cache.primitives ? unsigned(cache.primitives->size()) : 0; }
      const Primitive** primitive_array() const
      {
        if (!cache.primitives)
          return nullptr;

        return (const Primitive**)(&cache.primitives->data()[0]);
      }
      unsigned vertices() const { return cache.vertices; }
      unsigned points() const { return cache.points ? unsigned(cache.points->size()) : 0; }
      const PointList* point_list() const { return &*cache.points; }
      const Vector3* point_array() const { return (cache.points && !cache.points->empty()) ? &((*cache.points)[0]) : nullptr; }

      DrawListsPtr callLists() const { return cache.call_lists;  }

      const Cache* get_cache_pointer() const { return &cache; }

      /*! Allows cache vertex count to be manually overridden to keep attributes
          in sync with primitives.  Some Primitive types do not have vertex counts that
          can be predicted at the
          moment the primitive is added to the GeoInfo leading to a difference in total
          vertex count causing vertex Attributes to have incorrect sizes. */
      void setVertexCount(int vertices) { cache.vertices = vertices; }

      /*! Return the object ids. */
      const Hash& src_id() const { return cache.src_id; }
      const Hash& out_id() const { return cache.out_id; }

      void set_out_id(DD::Image::Hash newId) { cache.out_id = newId; }

      /*! Returns the size of the group list. */
      unsigned size(int group) const;

      /* Read-only Attribute methods: */

      /*! Find a valid attribute by name at each level of possible
          assignment in this order of importance:
          Vertex, Point, Material, Primitive, then Object.
          If found return a pointer to the Attribute, otherwise NULL.
       */
      const Attribute* get_attribute(const char* name) const;

      /*! Search for an attribute by name and type, returning its pointer if found.
       */
      const Attribute* get_typed_attribute(const char* name, int type) const;

      /*! Search for an attribute by name and group, returning its pointer
          if found.
       */
      const Attribute* get_group_attribute(int group, const char* name) const;

      /*! Search for an attribute by name, group and type, returning its pointer
          if found.
       */
      const Attribute* get_typed_group_attribute(int group, const char* name, int type) const;

      int get_attribcontext_count() const;

      const AttribContext* get_attribcontext(int index) const;

      /*! Find a valid attribute by name at each level of possible
          assignment in this order of importance:
          Vertex, Point, Material, Primitive, then Object.
          If found return a pointer to the AttribContext structure which has
          the attribute pointer and it's group info.
       */
      const AttribContext* get_attribcontext(const char* name) const;

      /*! Search for an attribute by name and type and return a const reference
          to it if found.
       */
      const AttribContext* get_typed_attribcontext(const char* name, int type) const;

      /*! Search for an attribute by name and group and return a const reference
          to it if found.
       */
      const AttribContext* get_group_attribcontext(int group, const char* name) const;

      /*! Search for an attribute by name, group and type, returning its pointer
          if found.
       */
      const AttribContext* get_typed_group_attribcontext(int group, const char* name, int type) const;

      /*! Clears the reference pointer to the attribute, effectively deleting it.
       */
      void delete_group_attribute(int group, const char* name, int type = INVALID_ATTRIB);

      PrimitiveContext* getPrimitiveContext();

      /* Geometry methods: */
      
      bool construct_normals(int group, Attribute*, float threshold = 0.0f) const;

      /*! Point the normals of the passed in attribute at point Pl.
          Returns true if successful.
       */
      bool orient_normals(int group, Attribute*, const Vector3& P,
                          bool invert = false) const;

      /* OpenGL drawing methods: */

      /*! Draw the GeoInfo's geometry in OpenGL.
       */
      void draw(ViewerContext*);
      void draw_icons(ViewerContext*);

      /*! Draw the surrounding bbox.
          This is drawn separately from the object primitives because
          the bbox has already been transformed by the matrix.
       */
      void draw_bbox(ViewerContext*);

      /*! Draw wireframe of the object, by calling draw_wireframe on all
         the primitives. This compiles an OpenGL call list, which is reused
         until invalidate() is called. Returns false if there is some
         problem creating the call list (though everything ignores this)
       */
      bool draw_wireframe(ViewerContext*);

      /*! Draw shaded version the object, by calling draw_solid on all
         the primitives. This compiles an OpenGL call list, which is reused
         until invalidate() is called. Returns false if there is some
         problem creating the call list (though everything ignores this)
       */
      bool draw_solid(ViewerContext*);

      /*! Draw selectable parts of the object. This compiles an OpenGL call list, which is reused
         until invalidate() is called. Returns false if there is some problem creating the call
         list (though everything ignores this).
       */
      bool draw_pick_ids(ViewerContext* ctx, unsigned int offset);
      bool draw_selectable_items(ViewerContext* ctx);

      /*! Draw selected parts of the object. This compiles an OpenGL call list, which is reused
         until invalidate() is called. Returns false if there is some problem creating the call
         list (though everything ignores this).
       */
      bool draw_selected_items(ViewerContext* ctx, const GeoSelection& selection);

      /*! Primary use is to print GeoInfo information into Nuke's info box.
       */
      void print_info(std::ostream& o);

      void trash_display_list(bool trash = true);
      void trash_selected_display_list();   // When only the selection changes.

      Box3 getTransformedBBox();

    private:
      /* OpenGL lists: */
      Hash _lastSelectionHash;
      void trash_lists();

      unsigned int buildSelectedVerticesDisplayList(ViewerContext* ctx, const GeoSelection& selection);
      unsigned int buildSelectedFacesDisplayList(ViewerContext* ctx, const GeoSelection& selection);
      void drawSelectedFacesForPrimitive(Primitive *p, ViewerContext* ctx);

      void buildVertexPickIDList(ViewerContext* ctx, unsigned int offset);
      void buildFacePickIDList(ViewerContext* ctx, unsigned int offset);
      void buildObjectPickIDList(ViewerContext* ctx, unsigned int offset);

      /* Geometry list management: */

      /*! Empty the PrimitiveList. This delete's each primitive's individual
          allocation but leaves the PrimitiveList allocation.
       */
      void clear_primitives();

      /*! Empty the PointList but leave the allocation.
       */
      void clear_points();

      /*! Clears the attributes for group \b group.
       */
      void clear_attributes(int group);

      /*! Clears the attribute pointers for group \b group.
       */
      void clear_attribute_pointers(int group);

      /*! Empty the point and primitive lists.
       */
      void clear_geometry();

      /*! Zero out the geometry list pointers.  
       *  Nothing is deleted (except the Primitives, which are reference counted)
       */
      void clear_pointers();

      /*! Clear the default attribute reference pointers. */
      void clear_attribcontexterences();

    private:
      Cache cache;            //!< The local cache pointers
      unsigned output_index_; //!< Index of object in output list
      bool trash_lists_;
      bool trash_selected_list_;
      unsigned int _geoInfoFlags;
    };

    /*! Get a valid and not empty attribute context from GeoInfo */
    inline const AttribContext* GetGeoInfoAttribContext( const GeoInfo* info, const char* attribName, AttribType attribType ) 
    {
      int count = info->get_attribcontext_count();
      for (int i=0; i<count; i++) {
        const AttribContext* context = info->get_attribcontext(i);
        if ( context && 
             !context->empty() && 
             context->type == attribType && 
             strcmp(attribName, context->name) == 0 )
        {
          return context;
        }
      }      
      return nullptr;
    }

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
// end of GeoInfo.h
