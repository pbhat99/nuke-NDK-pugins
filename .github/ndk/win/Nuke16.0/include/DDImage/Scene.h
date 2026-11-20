// Scene.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Scene_H
#define DDImage_Scene_H

#include "DDImage/GeoInfo.h"
#include "DDImage/CameraOp.h"
#include "DDImage/GeometryList.h"
#include "DDImage/MatrixArray.h"
#include "DDImage/LightContext.h"
#include "DDImage/SubdivisionContext.h"
#include "DDImage/Raycast.h"
#include <vector>

#define USE_QUADTREE 1

#ifdef USE_QUADTREE
  #include "DDImage/Quadtree.h"
#endif

#ifdef __INTEL_COMPILER
#pragma warning(disable:444)  
#endif

namespace DD
{
  namespace Image
  {

    class LightOp;
    class rPrimitive;
    class ViewerContext;
    class PrimitiveContext;
    class TextureFilter;
    class VertexContext;

    //------------------------------------------------------------------

    class SceneImpl;

    //! Scene motion vector types
    enum MotionVectorType {
      eDisableMotionVector  = 0,     //!< the motion vector is disable
      eClassicMotionVector  = 1,     //!< classic and old way to do motion vector
      eVelocityMotionVector = 2,     //!< store velocity in motion vector channel
      eDistanceMotionVector = 3      //!< store distance in motion vector channel
    };

    /*!
       We create a Scene structure for every rendering sample.
       This structure is a snapshot in time of all objects and transforms
       that are needed to render a certain camera view.
       To that end it's *very* important to have local copies of
       renderable primitives, their transforms, the taking
       camera's transform and all lights that affect this scene.

       The Scene contains everything a renderer needs to generate
       an image.
     */
    //This object derives from a Box* type, which doesn't have a virtual destructor. This
    //is why warning 444 has been disabled for the intel compiler at the top of this file - the important
    //message is to not delete via a Box* pointer, as the destructor will not
    //get called.
    class DDImage_API Scene : public Box3
    {
      std::unique_ptr<SceneImpl> _pImpl;

      friend class GeoOp;
      friend class Render;
      friend class DepthMapRender;

      /// nonassign 
      Scene& operator=(const Scene&);
      /// noncopy 
      Scene(const Scene &);

    protected:
      // !Geometry Op for this scene:
      GeoOp* geo_;
      /*! Geometry GeoInfo instances: */
      GeometryList object_list_;  //!< Geometry object list

      MotionVectorType mb_type_;  //!< Motion vector type
      Scene* mb_scene_;           //!< Next scene in time to motionblur to

      /* Primitives rendering: */
#ifdef USE_QUADTREE
      Quadtree<unsigned>* quadtree_;                //!< Spatial subdivision tree
#endif
      std::vector< rPrimitive*, STL3DAllocator<rPrimitive*> > render_primitives_;       //!< Primitives to render
      std::vector< rPrimitive*, STL3DAllocator<rPrimitive*> > mb_render_primitives_;    //!< Motionblur primitives
      std::vector< rPrimitive*, STL3DAllocator<rPrimitive*> > displacement_primitives_; //!< Displacement primitives      
      std::vector<MatrixArray> object_transforms_;                                    //!< Rendering transforms

      /*! Scene transforms(object, camera, projection, format) */
      MatrixArray transforms_;
      const Format* format_;  //!< Output render format

      /* Rendering info: */
      int projection_mode_;   //!< Point projection mode
      int max_tessellation_;  //!< Threshold to limit recursive subdivision
      bool transparency_;     //!< Transparency setting from renderer

      Box screen_bbox_;       //!< Screen-projected bounding box
      ChannelSet channels_;   //!< Intersected channels of all materials
      ChannelSet shadow_channels_; //!< shadow channels for all lights
      TextureFilter* filter_;
      bool shadow_scene_;     //!< Is this a shadow scene?
      float _time;            //!< Scene time


      std::map<Iop*, Box> _materialBoxes; //!< 2D bounding boxes for each material input

      /* Raycasting */
      RegularGrid* _grid;
      std::vector<DrawableGeo> _geoList;
      Lock _lock;

      int _id;                //!< id for current rendering scene

    public:
      /* Camera: */
      CameraOp* camera;               //!< The shooting camera
      CameraOp::LensNFunc* lens_func; //!< Camera projection function to use
      Axis cam_vectors;               //!< Camera PXYZ vectors in world-space

      /* Lights: */
      std::vector<LightContext*> lights;      //!< Lights and their vectors
      std::vector<MatrixArray> light_transforms; //!< Light transforms
      std::vector<Render*> light_renderers;   //!< Renderer attached to light
      Vector3 ambient;                        //!< Ambient RGB color

    public:

      /*! Constructor creates a single GeoInfo object.
       */
      Scene();

      /*! Destructor.  Delete and clear the primitives list. */
      virtual ~Scene();

      /*! Calculate the scene's bounding box which surrounds all of it's
          constituent objects.  We keep two current, the entire scene's
          bbox in world space, and that same bbox projected into screen space.
       */
      virtual void validate(int sample = 0);

      /*! Call request on all the materials.
       */
      virtual void request(const ChannelSet& channels, int count);

      MotionVectorType mb_type() const { return mb_type_; }
      Scene* mb_scene() const { return mb_scene_; }

      void geo( GeoOp* geo) { geo_ = geo; }
      GeoOp* geo() const { return geo_; }
      const GeometryList* object_list() const { return &object_list_; }
      GeometryList* object_list() { return &object_list_; }

      std::vector<DrawableGeo>* geo_list() {return &_geoList;}
      unsigned objects() const { return object_list_.size(); }
      GeoInfo& object(unsigned n = 0) { return object_list_[n]; }
      int sceneId() const {return _id; }
      void time(float t) {_time = t;}
      float time()const {return _time;}

      /* Output */
      void format(const Format* f) { format_ = f; }
      const Format* format() const { return format_; }
      void projection_mode(int p) { projection_mode_ = p; }
      int projection_mode() const { return projection_mode_; }
      void max_tessellation(int t) { max_tessellation_ = t; }
      int max_tessellation() const { return max_tessellation_; }
      const Box& screen_bbox() const { return screen_bbox_; }
      const ChannelSet& channels() const { return channels_; }
      const ChannelSet& shadow_channels() const { return shadow_channels_; }

      /* Scene Transforms */
      MatrixArray* transforms() { return &transforms_; }
      const Matrix4& matrix(int n) { return transforms_.matrix(n); }
      const Matrix4& inverse_matrix(int n) { return transforms_.inverse_matrix(n); }
      /* Output Transforms */
      const Matrix4& camera_matrix() const { return transforms_.camera_matrix(); }
      const Matrix4& projection_matrix() const { return transforms_.projection_matrix(); }
      const Matrix4& format_matrix() const { return transforms_.format_matrix(); }
      /* Object Transforms */
      MatrixArray* object_transforms(unsigned n)
      {
        if (n >= object_transforms_.size())
          return nullptr;
        else
          return &object_transforms_[n];
      }

      /* Lighting: */
      void set_shadow_scene(bool s) { shadow_scene_ = s;}
      bool is_shadow_scene() const { return shadow_scene_; }
      bool transparency() const { return transparency_; }
      void transparency(bool v) { transparency_ = v; }

      /*! Add a LightOp to the Scene's light list.
       */
      virtual void add_light(LightOp*);

      /*! Add LightOps from a LightOp list.
       */
      virtual void add_lights(const std::vector<LightOp*>* light_list);

      /*! Clear the light list.
       */
      virtual void clear_lights();

      /*! Transform the Scene's lights into the scene's camera(eye) space.
          The renderer needs a local copy of the light transforms because the
          lights can be used in multiple scenes and it's cheaper to pre-transform
          them than to do it during rendering.
          This call is separate from generate_primitives() so that Materials
          can generate their shader previews.
          If the call is aborted early because of a user interrupt, return false
          so the calling function can abort too.
       */
      virtual bool evaluate_lights();

      TextureFilter* filter() const { return filter_; }
      void filter(TextureFilter* v) { filter_ = v; }

      void copyInfo(const Scene* scene);

      /* Render primitives: */
#ifdef USE_QUADTREE
      const Quadtree<unsigned>* quadtree();
#endif

      /* Ray Casting */
      bool testRayIntersection(Ray& ray, const VertexContext* vtx = nullptr, const Iop* material = nullptr );

      /*! Create renderable primitives from the GeoInfo's primitive list.
          A primitive may not be directly renderable and may need
          tessellation(subdivision) into simpler primitives - a NURBS surface
          for example.  Usually these primitives will create triangles,
          and this generally will only need to be done to support a
          scanline rendering algorithm as raytracing usually intersect tests
          directly on implicit surfaces.

          Because this routine can be time intensive, we check frequently for
          user aborts so we can clean up and bail out quickly.
      */
      virtual bool generate_render_primitives();
      unsigned render_primitives() const { return unsigned(render_primitives_.size()); }
      unsigned displacement_primitives() const { return unsigned(displacement_primitives_.size()); }

      /*! Add an unclipped primitive to the renderable list.  This call calls
          clip() on the primitive.  If you know the primitive is completely
          visible, use add_clipped_render_primitive() instead.
          This is usually called directly from a rPrimitive to either add a
          reference to a copy of itself, or to add a sub-primitive during
          tessellation.
          rPrimitives should not add a reference to themselves as the Scene
          deletes all its rPrimitive pointers after rendering.

       ****The pointer 'p' is deleted after rendering is complete!****
       */
      virtual void add_render_primitive(rPrimitive*, PrimitiveContext*);

      /*! Add an already clipped primitive to the renderable list.
          This is usually called directly from a rPrimitive to either add a
          reference to a copy of itself, or to add a sub-primitive during
          tessellation.
          rPrimitives should not pass a reference to itself as the Scene
          deletes all its rPrimitive pointers after rendering.

       ****The pointer 'p' is deleted after rendering is complete!****
       */
      virtual void add_clipped_render_primitive(rPrimitive*);
      virtual void add_clipped_displacement_render_primitive(rPrimitive*);
      rPrimitive* renderable_primitive(unsigned n) const { return render_primitives_[n]; }
      rPrimitive* displacement_primitive(unsigned n) const { return displacement_primitives_[n]; }

      /* rendering primitive in the range [from,to) define a primitive edges */
      void buildCircularTriangleList(unsigned from, unsigned to); 

      /*! Delete all added renderable primitives and their associated
          allocations.
       */
      virtual void delete_render_primitives();

      /* Clear the list of render primitives */
      virtual void clear_render_primitives() { render_primitives_.clear(); }

      /*! Delete all allocated light context.
      */
      virtual void delete_light_context();

      void print_info(const char* title);

      void generateObjectTransforms();
    };

  }
}

#ifdef __INTEL_COMPILER
#pragma warning(default:444)  
#endif

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
