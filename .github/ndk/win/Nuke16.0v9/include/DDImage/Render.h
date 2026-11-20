// Render.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Render_H
#define DDImage_Render_H

#include "DDImage/DDImage_API.h"

#include "DDImage/Iop.h"
#include "DDImage/Shutter.h"
#include "DDImage/Box3.h"
#include "DDImage/Scene.h"
#include "DDImage/TextureFilter.h"
#include "DDImage/Material.h"
#include "DDImage/LightContext.h"
#include "DDImage/VertexContext.h"
#include "DDImage/rPrimitive.h"
#include "DDImage/SubdivisionContext.h"

#include <vector>

// Disable Windows DLL interface warning
#ifdef FN_OS_WINDOWS
  #pragma warning(disable:4251)
#endif


namespace DD
{
  namespace Image
  {
    class GeoOp;
    class GeoInfo;
    class CameraOp;
    class rPrimitive;
    class PrimitiveContext;
    class ViewerContext;


    /* Used for multisampling patterns. */
    struct DDImage_API Intersection
    {
      float weight;
      float dx, dy;
    };

#define MAX_SAMPLES 66
#define MAX_MULTISAMPLES 13

    //! Base class for rendering a 3D scene to an image
    class DDImage_API Render : public Iop
    {
    public:

      /*! A Span represents the intersection of an rPrimitive with a scan line,
         the Render op uses these in it's implementation. The
         rPrimitive::intersect_scanline() method fills this in.
         This is the thread-safe part of the rendering system so one of these
         spans exist for each thread that's working on a primitive.
       */
      class DDImage_API Span
      {
      public:
        VertexContext vtx;                  //!< Argument for Iop::fragment_shader()
        VArray vX;                          //!< Value for vtx.vP at x==0
        int x, r;                           //!< Range of pixels the primitive intersects
        bool always_hit;                    //!< Used by antialiasing
        bool never_hit;                     //!< Used by antialiasing
        

        Span();

        Span(const Span& s);
        ~Span();

        Span& operator = (const Span& s);

        /*! Call the rPrimitive::intersect_scanline and otherwise setup. */
        void initialize(Scene* scene, rPrimitive* p, int Y);

        // Convenience Functions:

        /*! Return the parent primitive stored in the VertexContext. */
        const Primitive* primitive() const { return vtx.primitive(); }
        /*! Return the render primitive stored in the VertexContext. */
        rPrimitive* rprimitive() const { return vtx.rprimitive(); }
        
      };

    protected:
      TextureFilter texture_filter_;   //!< Default filter to use in texture filtering

      int projection_mode_;       //!< Point projection mode
      int max_tessellation_;   //!< Threshold to limit recursive subdivision
      int blend_operation_;       //!< Default blending operation for textures
      int multisampling_;        //!< Output sampling modes

      int samples_;            //!< User set number of samples to render
      ShutterControls shutter_; ///< shutter controls
      double spatial_jitter_;  //!< User set jittering of sample in raster
      double temporal_jitter_; //!< User set jittering of sample in time
      const Intersection* jitter; //!< XY jitter array to use - based on total samples

      unsigned input_scenes;   //!< Number of scenes renderer gets from it's input
      unsigned render_scenes;  //!< Number of scenes to render
      const Intersection* multisample_array_; //!< Multisampling array to use

      double overscanX_, overscanY_;  //!< overscan size of the frame buffer
      Format overscanFormat_;         //!< overscan format

      int junk;

      Box3 world_bbox;         //!< Intersected world-space bbox of all scenes
      Box screen_bbox;         //!< Intersected screen-space bbox of all scenes

      int output_motion_vectors; //!< Whether renderer outputs motion vector channels
      Channel MB_channel[3];   //!< Motion vector channels to output

      //  Lock lock;
      bool generate_primitives; //!< Threading lock to keep other threads waiting

    public:

      /*! Return a CameraOp for 'sample'.  The base class returns null.
       */
      virtual CameraOp* render_camera(int sample = 0);

      /*! \fn GeoOp* Render::render_geo(int sample)

          Return a GeoOp for 'sample'.  This GeoOp will have it's cache enabled
          which will be created during _validate().  The base class returns 0.
       */
      virtual GeoOp* render_geo(int sample = 0) = 0;

      /*! Calculate the camera's inverse local matrix for 'sample'.
          Base class pulls the camera away in Z by the format's width,
          and XY centered.
          Remember, this is the camera's *inverse* matrix, so translations are
          reversed.
       */
      virtual Matrix4 camera_matrix(int sample = 0);

      /*! Calculate the camera projection matrix for 'sample'.
          Base class implementation sets up a simple projection with a lens of 1.
       */
      virtual Matrix4 projection_matrix(int sample = 0);

      /*! obsolete virtual function, do not use. Call get_format_matrix() instead */
      virtual void format_matrix(int sample = 0);

      /*! Calculate the transformation from post-projection NDC to pixel
         space. cx,cy are where in the pixel the -1,-1 corner should
         be.
       */
      Matrix4 get_format_matrix(float cx, float cy);

      /*! Calculate the transformation from post-projection NDC to pixel
         space. cx,cy are where in the pixel the -1,-1 corner should
         be.
       */
      static Matrix4 get_format_matrix(const Format& format, float cx, float cy);

    protected:

      /*! Base class constructor.
       */
      Render(Node* node, bool enable_mip_filter =  false);

      std::vector<Scene*> scenes_; //!< List of Scenes, one per sample

      /*! Base-class implementation allocates a Scene structure for each time
          sample and enables the generation of primitives in the first engine() call.
       */
      void _validate(bool) override;

      /** called in Render::_validate, to allow the deriving class to override
       * the bbox. */
      virtual void overrideBBox(Box& f) const { }

      /*!
         Render just texture-maps the geometry with input0(). So the request
         figures out what uv's will be requested and that is the area needed.
       */
      void _request(int x, int y, int r, int t, ChannelMask, int) override;

      /*!
         Default engine calls initialize() (which calls generate_render_primitives())
         and then calls draw_primitives() with bg=0.
       */
      void engine(int y, int x, int r, ChannelMask, Row &) override;

      /*! Initialize each sample Scene object and generate it's
          renderable primitives.
       */
      virtual bool generate_render_primitives();

      /*!
         Evaluate all the scene lights.
       */
      virtual bool evaluate_lights(Scene* scene);

      /*! engine() must call this before it can look at the primitives */
      virtual void initialize();

      /*!
         Flat renderer used by gridwarp. If bg is non-zero, it is a row containing
         a background image which is used anywhere there are no primitives (note
         that it does not do an OVER, instead the alpha channel is replaced by
         the rendered ones just like all other channels).
       */
      void draw_primitives(int y, int x, int r, ChannelMask, Row &, const Row * bg);
      void _close() override;

    public:
      ~Render() override;

      enum { SINGLE_SAMPLE = 0, MULTISAMPLE_LOW,  MULTISAMPLE_MED, MULTISAMPLE_HIGH };

      //! Output sampling modes:
      static const char* multisampling_table[];

      void knobs(Knob_Callback) override;
      void display_knobs(Knob_Callback);
      void texture_knobs(Knob_Callback);
      void sampling_knobs(Knob_Callback);
      void jitter_knobs(Knob_Callback);
      void render_knobs(Knob_Callback); //!< All of the above
      unsigned node_color() const override;
      double overscanX() { return overscanX_; }
      double overscanY() { return overscanY_; }

      int projection_mode() const { return projection_mode_; }
      int blend_operation() const { return blend_operation_; }
      const Filter* texture_filter() const { return &texture_filter_; }
      int multisampling() const { return multisampling_; }

      /*!  Return the distance away that a span/pixel cannot intersect
       */
      float hitThresholdDistance() const;

      /*! Return multisample array for the current output sampling mode.
       */
      static const Intersection* multisample_array(int n);

      /*! Determine the size of jitter array to use based on the sample count 'samples'.
       */
      static unsigned int jitter_array_size(int samples);

      /*! Determine best jitter array to use based on the sample count 'samples'.
       */
      static const Intersection* jitter_array(int samples);

      /*!  Return the number of samples in the current output sampling mode.
       */
      virtual int multisamples() const;
      virtual Scene* scene(int n = 0) const { return scenes_[n]; }

      virtual double shutter() const { return shutter_.getDuration(); }
      virtual double offset() const { return shutter_.calcOffset(); }

      /* Return legal sample range: 1->MAX_SAMPLES: */
      virtual unsigned samples() const { return clamp(samples_, 1, MAX_SAMPLES); }

      HandlesMode doAnyHandles(ViewerContext* ctx) override;

      /*! Draw the internal geometry only when the panel's open.
       */
      void build_handles(ViewerContext*) override;
      
      //! this sets the objects in the scene to be drawable (textured) and then calls
      //! GeoInfo::build_handles()
      void buildGeoInfoHandle(ViewerContext* ctx, DrawHandleCallbackFunc cb);

      //! Sample the frame buffer
      virtual void probe(const Vector3& center, const VertexContext& vtx, Pixel& out);
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
