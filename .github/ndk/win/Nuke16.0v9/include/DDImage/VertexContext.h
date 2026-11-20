// VertexContext.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_VertexContext_h
#define DDImage_VertexContext_h

#include "DDImage/DDImage_API.h"
#include "DDImage/MatrixArray.h"
#include "DDImage/ChannelSet.h"
#include "DDImage/Channel3D.h"
#include "DDImage/Box3.h"

namespace DD
{
  namespace Image
  {

    class TextureFilter;
    class Iop;
    class Pixel;
    class Scene;
    class GeoInfo;
    class Primitive;
    class rPrimitive;
    class PrimitiveTestCache;
    struct GeoInfoRenderState;

    //----------------------------------------------------------------------

#define VARRAY_CHANS 28

    /*!
       Array of floating point numbers that are interpolated across
       a primitive. A VertexContext contains 4 of these: the current
       value, the derivatives horizontally and vertically, and the
       value at X=0.
     */
    class DDImage_API VArray
    {
    public:
      ChannelSet channels;
      float chan[VARRAY_CHANS];

      VArray() : channels((ChannelSetInit)Mask_P_) {}
      VArray(const ChannelSetInit c) : channels(c) { chan[0] = 0; }
      VArray(const ChannelSet &c) : channels(c) { chan[0] = 0; }
      VArray(Channel c) : channels(c) { chan[0] = 0; }
      VArray(const VArray &b) : channels(b.channels) { memcpy(chan, b.chan, sizeof(chan)); }

      float& operator [] (Channel z) { return chan[z]; }
      const float& operator [] (Channel z) const { return chan[z]; }
      float* array() { return chan; }

      // Assignment operator:
      VArray& operator =  (const VArray& v)
      {
        channels = v.channels;
        memcpy(chan, v.chan, sizeof(chan));
        return *this;
      }

      // Point access convenience methods:
      Vector4& P() const { return (Vector4 &)chan[Chan_P_]; }
      float x() const { return P().x; }
      float y() const { return P().y; }
      float z() const { return P().z; }
      float w() const { return P().w; }
      Vector3& PL() const { return (Vector3 &)chan[Chan_PL_]; }
      Vector3& PW() const { return (Vector3 &)chan[Chan_PW_]; }
      Vector3& MB() const { return (Vector3 &)chan[Chan_MB_]; }
      Vector4& UV() const { return (Vector4 &)chan[Chan_UV_]; }
      Vector3& N() const { return (Vector3 &)chan[Chan_N_]; }
      Vector4& Cf() const { return (Vector4 &)chan[Chan_Red]; }
      float r() const { return Cf().x; }
      float g() const { return Cf().y; }
      float b() const { return Cf().z; }
      float a() const { return Cf().w; }
      Vector3& VEL() const { return (Vector3 &)chan[Chan_VEL_]; }

      /*! Destructive linear interpolate between this and VArray b by t. */
      void lerp(const VArray& b, float t)
      {
        foreach(z, b.channels)
        chan[z] = chan[z] * (1 - t) + b.chan[z] * t;
      }

      /*! Sets it to b+delta*t */
      void offset(const VArray& b, const VArray& delta, float t)
      {
        foreach(z, channels) chan[z] = b.chan[z] + delta.chan[z] * t;
      }

      /*! Scale the VArray. */
      void scale(float s)
      {
        foreach(z, channels) chan[z] *= s;
      }

      /*! Sets it to b+dx*x+dy*y */
      void offset(const VArray& b, const VArray& dx, float x, const VArray& dy, float y)
      {
        foreach(z, channels)
        chan[z] = b.chan[z] + dx.chan[z] * x + dy.chan[z] * y;
      }

      /*! Sets it to the midpoint between a and b ((a+b)/2). */
      void midpoint(const VArray& a, const VArray& b)
      {
        foreach(z, channels)
        chan[z] = (a.chan[z] + b.chan[z]) * 0.5f;
      }

      //! Divide all the channels by w, and put 1/w into w.
      /*!
         Divide all the channels by w, and put 1/w into w. The resulting values
         can then be linearly interpolated in screen space. Don't forget to divide
         by w to get the original values back, unless the divided value is useful
         (which is only true for xyz).
       */
      void w_normalize();
    };


    /*!
       This class is intended to be passed up a vertex shader tree.
       It's initialized with basic vertex info like the point location,
       a UV coordinate and a normal.  A shader can then modify the
       parameters producing effects like point displacement, UV
       projection, and normal remapping.
     */
    class DDImage_API VertexContext
    {
    public:
      VArray vP;    //!< Current values for fragment_shader
      VArray vdX;    //!< Change if X of render pixel is increased by 1
      VArray vdY;    //!< Change if Y of render pixel is increased by 1

      Vector3 ambient; //!< Global ambient setting

      /*! Points at the final shader in the chain. */
      Iop* blending_shader;
      /*! Whether shaders should sample their textures. */
      bool texture_sampling;

      /* Private data */
      void* render_context;

      /*! Callback to redefine the texture sampler */
      typedef void (*TextureSampler)(Iop* material, const Vector2& center, const Vector2& dU, const Vector2& dV, TextureFilter* f, Pixel& out, void* context);

    private:
      // Rendering and OpenGL:
      Scene* scene_;            //!< To get at the lights or other objects
      const GeoInfo* geoinfo_;  //!< The current object
      const GeoInfoRenderState* renderstate_; //!< The current renderstate (as we might not have any geometry)
      const Primitive* primitive_; //!< The current parent primitive
      MatrixArray* transforms_; //!< All the matrices one could ever hope for

      // Rendering only:
      rPrimitive* rprimitive_;  //!< Rendering primitive
      Iop* rmaterial_;          //!< Rendering material
      unsigned face_clipmask_;  //!< Primitive face clipmask
      Box3 face_uv_bbox_;       //!< Primitive face UV bounds

      /* RayCast */
      PrimitiveTestCache* ray_test_cache_; //!< Primitive test cache

      /* Textures */
      TextureSampler texture_sampler_; //!< Texture sampler

    public:
      VertexContext() : blending_shader(nullptr), texture_sampling(true),
        scene_(nullptr), geoinfo_(nullptr), renderstate_(nullptr), primitive_(nullptr), transforms_(nullptr),
        rprimitive_(nullptr), rmaterial_(nullptr), ray_test_cache_(nullptr),
        texture_sampler_(nullptr) {}

      VertexContext(const VertexContext& vtx)
      { 
        memcpy(this, &vtx, sizeof(VertexContext)); 
      }

      // Rendering and OpenGL:
      Scene* scene() const { return scene_; }
      void set_scene(Scene* s) { scene_ = s; }
      const GeoInfo* geoinfo() const { return geoinfo_; }
      void set_geoinfo(const GeoInfo* g) { geoinfo_ = g; }
      const Primitive* primitive() const { return primitive_; }
      void set_primitive(const Primitive* p) { primitive_ = p; }
      const GeoInfoRenderState* get_renderstate() const { return renderstate_; }
      void set_renderstate(const GeoInfoRenderState* r) { renderstate_ = r; }

      // Rendering only:
      rPrimitive* rprimitive() const { return rprimitive_; }
      void set_rprimitive(rPrimitive* rp) { rprimitive_ = rp; }
      Iop* rmaterial() const { return rmaterial_; }
      void set_rmaterial(Iop* m) { rmaterial_ = m; }

      /* Face UV Bounds */
      Box3& face_uv_bbox() { return face_uv_bbox_; }
      unsigned& face_clipmask() { return face_clipmask_; }

      /* Transforms */
      void set_transforms(MatrixArray* m) { transforms_ = m; }
      MatrixArray* transforms() const { return transforms_; }
      const Matrix4& matrix(int n) const { return transforms_->matrix(n); }
      const Matrix4& inverse_matrix(int n) const { return transforms_->inverse_matrix(n); }

      /* PrimitiveTestCache */
      PrimitiveTestCache* ray_test_cache() const {return ray_test_cache_;};
      void set_ray_test_cache(PrimitiveTestCache* r) { ray_test_cache_ = r; }

      /* Texture Sampler */
      TextureSampler texture_sampler() const { return texture_sampler_; };
      void set_texture_sampler(TextureSampler t) { texture_sampler_ = t; }

      // Point access convenience methods:
      Vector4& P() const { return vP.P(); }
      Vector4& dPdu() const { return vdX.P(); }
      Vector4& dPdv() const { return vdY.P(); }
      float x() const { return vP.P().x; }
      float y() const { return vP.P().y; }
      float z() const { return vP.P().z; }
      float w() const { return vP.P().w; }
      Vector3& PL() const { return vP.PL(); }
      Vector3& dPLdu() const { return vdX.PL(); }
      Vector3& dPLdv() const { return vdY.PL(); }
      Vector3& PW() const { return vP.PW(); }
      Vector3& dPWdu() const { return vdX.PW(); }
      Vector3& dPWdv() const { return vdY.PW(); }
      Vector3& MB() const { return vP.MB(); }
      Vector3& dMBdu() const { return vdX.MB(); }
      Vector3& dMBdv() const { return vdY.MB(); }

      // Standard attribute convenience methods:
      Vector4& UV() const { return vP.UV(); }
      Vector4& dUVdu() const { return vdX.UV(); }
      Vector4& dUVdv() const { return vdY.UV(); }
      Vector3& N() const { return vP.N(); }
      Vector3& dNdu() const { return vdX.N(); }
      Vector3& dNdv() const { return vdY.N(); }
      Vector4& Cf() const { return vP.Cf(); }
      Vector4& dCfdu() const { return vdX.Cf(); }
      Vector4& dCfdv() const { return vdY.Cf(); }
      float r() const { return Cf().x; }
      float g() const { return Cf().y; }
      float b() const { return Cf().z; }
      float a() const { return Cf().w; }
      Vector3& VEL() const { return vP.VEL(); }
      Vector3& dVELdu() const { return vdX.VEL(); }
      Vector3& dVELdv() const { return vdY.VEL(); }

      // Texture sampling calls:

      /*!
         Use the UV and the derivatives of UV to sample the incoming image.
         UV of 0,0 is mapped to the lower-left corner of the image's format,
         and 1,1 is mapped to the upper-right corner.
       */
      void sample(Iop* material, Pixel& out) const;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
