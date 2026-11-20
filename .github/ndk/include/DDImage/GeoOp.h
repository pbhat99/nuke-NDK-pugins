// GeoOp.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_GeoOp_h
#define DDImage_GeoOp_h

#include "DDImage/Op.h"
#include "DDImage/GeoInfo.h"
#include "DDImage/GeometryList.h"
#include "DDImage/Hash.h"
#include "DDImage/GeneralInfo.h"

#include <vector>

#ifdef _WIN32
  #pragma warning(disable:4251)
#endif

/* Increment this when a major change has been
    done to the geometry interface so that plugins
    can test against this instead of DD_IMAGE_VERSION:

 #define DD_GEOMETRY_SYSTEM 1: Circa 2001 - Nuke3.beta
 #define DD_GEOMETRY_SYSTEM 2: Circa 2002 - Nuke4.1
 #define DD_GEOMETRY_SYSTEM 3: Circa 2005 - Nuke4.5
 #define DD_GEOMETRY_SYSTEM 4: Circa 2007 - Nuke4.8
 */
#define DD_GEOMETRY_SYSTEM 4


namespace DD
{
  namespace Image
  {

    class Scene;

    /*! \class DD::Image::GeoOp

       Base class for an Op that produces a GeoInfo. The data is accessed
       by calling geometry_engine() and passing it a Scene to add a new GeoInfo
       to.

     */
    class DDImage_API GeoOp : public Op, public GeneralInfoProvider
    {
      friend class GeometryList;
    public:

      /*! Base class destructor destroys the cache if there is one.
       */
      ~GeoOp() override;

      /* Scene Methods: */
      Scene* scene() const { return scene_; }

      /*! Start the geometry construction process.  This would typically be called
          directly by a viewer, renderer or write node.
          If the scene's hash are different than the geo's, rebuild the scene.
          The output object list/cache is filled in by the GeoOp tree
          during the geometry_engine() call.  To reduce having to
          constantly recalculate primitive structures, we only rebuild
          those geometry groups whose rebuild flag is set in the cache's
          rebuild_mask.
       */
      void build_scene(Scene& scene);

      /*! Constructs GeoInfo structures and adds them to the passed-in Scene.
       */
      void get_geometry(Scene&, GeometryList& out);
      
      //! Create a scene if necessary
      //! and do prerequisite setup to allow drawing
      void setupScene();

      /* Point Convenience Methods: */

      /*! Check if matrix is non-identity and if so, apply it to all
          the points.  This 'cooks' the matrix into the points so that
          subsequent modifiers get the points in the correct space.
          The matrix is reset to identity.
       */
      bool evaluate_transform(int obj, GeometryList& out, bool transform_normals = true);

      /*! Transform all points by a matrix.
       */
      static void transform_points(PointList* out, const PointList* in, const Matrix4&);

      /*! Transform a range of points by a matrix.
       */
      static void transform_points(PointList* out, const PointList* in,
                                   int start, int end, const Matrix4&);

      /* Primitive Convenience Methods: */

    public:
      int minimum_inputs() const override { return 1; } // Base class has only one input
      int maximum_inputs() const override { return 1; }

      /*! Return a default op for input0. */
      Op* default_input(int input) const override;

      /*! Allows only GeoOp. */
      bool test_input(int input, Op* op) const override;
      GeoOp* input(int n) const { return (GeoOp*)(Op::input(n)); }
      GeoOp* input0() const { return (GeoOp*)(Op::input0()); }
      GeoOp* input1() const { return (GeoOp*)(Op::input1()); }

      /*! Draw a round sided box. */
      const char* node_shape() const override;
      unsigned node_color() const override;
      void knobs(Knob_Callback) override;
      void shadow_knobs(Knob_Callback f);
      void knobs(Knob_Callback, bool sourcegeo, bool geoselectSaves = false);

      /*! Forces tile_color & gl_color knob changes to update op.
       */
      int knob_changed(Knob* k) override;

      virtual void print_info(std::ostream& o);
      friend std::ostream& operator<<(std::ostream&, const GeoOp&);

      unsigned objects() const { return objects_; }
      unsigned vertices(int obj) const { return cache_list[obj].vertices; }
      std::vector<GeoInfo::Cache>& getCacheList() { return cache_list; }
      HandlesMode doAnyHandles(ViewerContext*) override;


      /*! Tree-climbing call.
       *  pass localMatrix if you're op contains it's own transform knob so that 
       *  nodes up the tree are displayed correctly
       */
      void build_matrix_handles(ViewerContext* ctx, const Matrix4* localMatrix = nullptr);

      /*! Internally this calls build_matrix_handles with a NULL matrix
       */
      void build_handles(ViewerContext* ctx) override;

      /*! Search up the tree to set the selected nodes and bounding box
          based on whether user has nodes selected and/or open.
       */
      virtual void select_geometry(ViewerContext*, GeometryList& scene_objects);

      /*! Assigns the current drawlist allocation for object \b obj
          to the GeoInfo.  If it doesn't exist yet create it.
       */
      void assign_drawlists(int obj, GeoInfo& info);

      Display3DMode display3d() const { return (Display3DMode)display3d_; }
      RenderMode render_mode() const { return (RenderMode)render_mode_; }
      bool selectable() const { return selectable_; }

      virtual bool selection3DChanged(ViewerContext* ctx) { return false; }
      virtual bool selection3DCleared() { return false; }
      virtual bool selection3DMouseDown(ViewerContext* ctx) { return false; }
      virtual bool selection3DMouseUp(ViewerContext* ctx) { return false; }
      virtual bool selection3DMouseDrag(ViewerContext* ctx) { return false; }

    public:
      /* Out-of-dateness geometry hashes: */
      const Hash& hash(int i) const { return geo_hash[i]; }
      /*! What groups need updating. */
      const GeometryMask& rebuild_mask() const { return rebuild_mask_; }
      bool rebuild(const GeometryMask& m = GeometryMask(Mask_All_Geometry)) const
      { return (rebuild_mask_ & m) != 0; }
      void set_rebuild(const GeometryMask& m) { rebuild_mask_ |= m; }
      void clear_rebuild(const GeometryMask& m) { rebuild_mask_ &= ~m; }
      static void clear_drawlist_trashbin();

      const GeneralInfo& generalInfo() const override
      {
        return info_;
      }

      //! Cast to a GeoOp. This is much cheaper and safer than using dynamic_cast.
      GeoOp* geoOp() override;
      const GeoOp* geoOp() const override;

      //! Return hints to control when and how this op will be evaluated by the top-down system.
      OpHints opHints() const override;

    protected:
 
      //! Base class constructor.
      /*! Only subclasses can create an instance. */
      GeoOp(Node*);

      /*! GeoOp subclass-dependent portion of validate().
          This builds
       */
      void _validate(bool for_real) override;

      virtual void updateGeneralInfo();

      // only constructors should call this, unless knobs() is not called:
      void display3d(Display3DMode v) { display3d_ = (int)v; }
      void selectable(bool v) { selectable_ = (int)v; }
      void render_mode(RenderMode v) { render_mode_ = v; }
      void backface_culling(int v) { backface_culling_ = v; }

      /*! Calculate the geometry hashes.  If the hashes are different
          from the cache's, set the cache rebuild flags.
       */
      void update_geometry_hashes();

      /*! Return a hash value which indicates the out-of-dateness of the
          object's primitives.  The default implementation validates the
          passed-in GeoOp and copies its hashes.  If the GeoOp depends on an
          input's geometry this should be called prior to other knob hashes being
          applied.
       */
      virtual void get_geometry_hash();

      /*! Delete all locally allocated geometry and clear the cache.
       */
      void delete_cache();

      /*! Synchronize the local cache objects with the output GeometryList.
          For an empty output this simply means adding objects to it,
          otherwise if a higher up GeoOp has already added objects we
          must compare our cache objects against the output objects and
          resolve any differences.

          For each object in the local cache find the corresponding object
          in the output list.  Rearrange, add or delete if different.
          Two pieces of information are needed for this:
            1) The unique hash id derived from the originating GeoOp Node and
              the index of the object.
            2) The version number, or copy number, incremented at each split.
          We must handle cases of an object disappearing from the output as
          well as new objects appearing.  There are two distinct update phases
          we must handle: The cache being brand new and the cache already
          existing but objects are in a different order.
       */
      void synchronize_cache(GeometryList& out);

      /*! Delete any allocated pointers in \b cache.  You better be the
          owner of the cache!
       */
      void delete_allocations(GeoInfo::Cache& cache);

      /*! Sub-class dependent portion of get_geometry().  This should be
          implemented on plugins to construct geometry.  The default
          implementation gets the input's geometry.
       */
      virtual void geometry_engine(Scene&, GeometryList& out);

      /*! Subclasses can call this in build_handles(). It will construct the
         output geometry and add callbacks to draw it in the viewer. You
         should only call this if ctx->connected is >= SHOW_OBJECT and
         should then set ctx->connected to CONNECTED before calling any
         inputs, so that objects are not drawn more than once. Also you
         must call validate(false) before this.
       */
      void add_draw_geometry(ViewerContext*);

    private:

      void saveSelected(ViewerContext* ctx, std::vector<bool>& isSelected);
      void restoreSelected(ViewerContext* ctx, const std::vector<bool>& isSelected);

    protected:

      /*! Out-of-dateness hashes. */
      Hash geo_hash[Group_Last];

      /* Output Geometry: */
      Scene* scene_;                      //!< Points to scene if it is turned on

      /* frame range */
      GeneralInfo info_;

      /* Cache: */
      std::vector<GeoInfo::Cache> cache_list; //!< Geometry cache structures
      Hash cache_hash[Group_Last];        //!< Cache out-of-dateness hashes.
      GeometryMask rebuild_mask_;         //!< What groups need updating
      bool cache_resursive;
      Hash cache_build_hash;

      int display3d_;        //!< GUI display setting
      bool selectable_;      //!< GUI selectable checkmark
      int render_mode_;      //!< Render setting
      int backface_culling_; //!< Backface cull setting
      bool cast_shadow_;
      bool receive_shadow_;

      static Hash sInitialGeoHash;
      static Hash sInitialCacheHash;

    private:
      unsigned objects_;     //!< Number of objects this op processes
      int build_version;     //!< Unique each time build_scene is called
      int visited;           //!< Number of times op was visited during build
      Hash build_hash;       //!< Includes build_version and visited
    };
    
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
// end of GeoOp.h
