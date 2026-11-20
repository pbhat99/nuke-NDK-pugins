// Material.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

// Material class describes how to shade a object

#ifndef DDImage_Material_h
#define DDImage_Material_h

#include "DDImage/Iop.h"
#include "DDImage/VertexContext.h"

#include <vector>

//Uncomment this to use fast_normalize functions in the shaders:
// Deprecated - fast normalize is deprecated in Vector class since cpu sqrt is no longer a bottleneck
#define USE_FAST_NORMALIZE 1

namespace DD
{
  namespace Image
  {

    class LightContext;

    // Default Material class is simply to allow connections:
    /*! \class DD::Image::Material

       This subclass of Iop provides some convienence functions so that it
       is easier to shade a 3D geometry object, such as calls to calculate
       the blinn or other lighting models, and the output has been modified
       to produce an image of a sphere lit using this material.

       To make a useful Material, you almost certainly need to override
       these Iop functions:
     * bool Iop::shade_GL(ViewerContext*, GeoInfo&);
     * void Iop::fragment_shader(Scene*, VertexContext*, Pixel& out);

     */
    class DDImage_API Material : public Iop
    {
    public:
      Material(Node*);
      ~Material() override;

      /*! Define the material op's shape in the DAG. */
      const char* node_shape() const override;

      /*! All material operators default to a orangish color. */
      unsigned node_color() const override;

      void knobs(Knob_Callback) override;

      bool set_texturemap(ViewerContext* ctx, bool gl) override { return input0().set_texturemap(ctx, gl); }
      void unset_texturemap(ViewerContext* ctx) override { input0().unset_texturemap(ctx); }


      /*! Not implemented in the library but you can define it */
      friend std::ostream& operator << (std::ostream&, const Material&);

      static const Vector4 gDefaultAmbient;
      static const Vector4 gDefaultDiffuse;
      static const Vector4 gDefaultSpecular;
      static const Vector4 gDefaultEmission;

      //----------------------------------------------------------------------------
      // Deprecated methods
      //----------------------------------------------------------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("get_geometry_hash(Hash*) has been deprecated.")
        virtual void get_geometry_hash(Hash* geo_hashes);
      mFnDeprecatedLegacy3DSystemInNuke14("shade_GL(ViewerContext*, GeoInfo&) has been deprecated.")
        bool   shade_GL(ViewerContext*, GeoInfo&) override;
      mFnDeprecatedLegacy3DSystemInNuke14("vertex_shader(VertexContext&) has been deprecated.")
        void   vertex_shader(VertexContext&) override;
      mFnDeprecatedLegacy3DSystemInNuke14("fragment_shader(const VertexContext&, Pixel& out) has been deprecated.")
        void   fragment_shader(const VertexContext&, Pixel& out) override;
      mFnDeprecatedLegacy3DSystemInNuke14("displacement_shader(const VertexContext&, VArray&) has been deprecated.")
        void   displacement_shader(const VertexContext& vtx, VArray& out) override;
      mFnDeprecatedLegacy3DSystemInNuke14("displacement_bound() has been deprecated.")
        float  displacement_bound() const override;
      mFnDeprecatedLegacy3DSystemInNuke14("blending_shader(const Pixel& in, Pixel& out) has been deprecated.")
        void   blending_shader(const Pixel& in, Pixel& out) override;
      mFnDeprecatedLegacy3DSystemInNuke14("set_blending_shader(const VertexContext&) has been deprecated.")
        void   set_blending_shader(const VertexContext& vtx)
        { if (vtx.blending_shader == nullptr)
            ((VertexContext*)&vtx)->blending_shader = this;}
      mFnDeprecatedLegacy3DSystemInNuke14("render_state(GeoInfoRenderState& state) has been deprecated.")
        void   render_state(GeoInfoRenderState& state) override;
      mFnDeprecatedLegacy3DSystemInNuke14("default_lighting(const VertexContext&, Pixel&) has been deprecated.")
        static void default_lighting(const VertexContext&, Pixel&);
      //----------------------------------------------------------------------------
      //----------------------------------------------------------------------------


    protected:
      void _validate(bool) override;
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;

      /*! If a Material is used as an Iop, it produces an image of a sphere
         rendered with the shader atop a gray background, with a light at the
         upper-left corner.
       */
      void engine(int y, int x, int r, ChannelMask, Row &) override;

    };

    //! Default solid vertex shader.
    /*! Solid Shader.  Interpolates the foreground vertex color. */
    class DDImage_API SolidShader : public Iop
    {
    public:
      const char* Class() const override { return "SolidShader"; }
      const char* node_help() const override { return ""; }
      SolidShader(Node*);

      mFnDeprecatedLegacy3DSystemInNuke14("bool shade_GL(ViewerContext*, GeoInfo&) has been deprecated.")
        bool shade_GL(ViewerContext*, GeoInfo&) override;
      //! Enable interpolation of Cf channels.
      mFnDeprecatedLegacy3DSystemInNuke14("void vertex_shader(VertexContext&) has been deprecated.")
        void vertex_shader(VertexContext&) override;
      //! This renders using the foreground color Cf.
      mFnDeprecatedLegacy3DSystemInNuke14("void fragment_shader(const VertexContext&, Pixel& out) has been deprecated.")
        void fragment_shader(const VertexContext&, Pixel& out) override;
      bool set_texturemap(ViewerContext* ctx, bool gl) override;
      void unset_texturemap(ViewerContext*) override;

      /*! global solid shader */
      static SolidShader sSolidShader;

    protected:
      void _validate(bool) override;
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;
      void engine(int y, int x, int r, ChannelMask, Row &) override;
    };

    class DDImage_API ColoredShader : public SolidShader
    {
    public:
      const char* Class() const override { return "ColoredShader"; }
      const char* node_help() const override { return ""; }
      ColoredShader(Node*);      

      mFnDeprecatedLegacy3DSystemInNuke14("shade_GL(ViewerContext*, GeoInfo&) has been deprecated.")
        bool shade_GL(ViewerContext*, GeoInfo&) override;
      mFnDeprecatedLegacy3DSystemInNuke14("fragment_shader(const VertexContext&, Pixel& out) has been deprecated.")
        void fragment_shader(const VertexContext&, Pixel& out) override;
      void unset_texturemap(ViewerContext*) override;
    }; 

    //! Default wireframe shader.
    class DDImage_API WireframeShader : public Iop
    {
    public:
      const char* Class() const override { return "WireframeShader"; }
      const char* node_help() const override { return ""; }
      WireframeShader(Node*);

      mFnDeprecatedLegacy3DSystemInNuke14("vertex_shader(VertexContext& vtx) has been deprecated.")
        void vertex_shader(VertexContext& vtx) override;
      mFnDeprecatedLegacy3DSystemInNuke14("void fragment_shader(const VertexContext&, Pixel& out) has been deprecated.")
        void fragment_shader(const VertexContext&, Pixel& out) override;
      mFnDeprecatedLegacy3DSystemInNuke14("shade_GL(ViewerContext*, GeoInfo&) has been deprecated.")
        bool   shade_GL(ViewerContext*, GeoInfo&) override;

      /*! global wireframe shader */
      static WireframeShader sWireframeShader;

    protected:
      void _validate(bool) override;
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;
      void engine(int y, int x, int r, ChannelMask, Row &) override;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
