// DepthMapRender.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DepthMapRender_H
#define DDImage_DepthMapRender_H

#include "DDImage/DDImage_API.h"

#include "DDImage/Render.h"
#include "DDImage/CameraOp.h"

namespace DD
{
  namespace Image
  {

    class LightOp;

    //! Depth map rendering for a shadow buffer
    class DDImage_API DepthMapRender : public Render
    {
    private:
      CameraOp *light_cam;  //!< Camera conformed to light's view
      int scene_index;      //!< Which scene index parent renderer is referring to

      Format _out_format;

    protected:
      int width_;       //!< Size width/height in pixels of buffer
      GeoOp* geometry_; //!< Geometry list to render

      void _request(int x, int y, int r, int t, ChannelMask, int) override;

      /*! Initialize each sample Scene object and generate it's
          renderable primitives.
       */
      bool generate_render_primitives() override;
      void engine(int y, int x, int r, ChannelMask, Row &) override;

    public:
      DepthMapRender(Node * node, int width = 1024, int index = 0);
      void set_camera( CameraOp* light ) { light_cam = light; }
      void set_geometry(GeoOp* geometry) { geometry_ = geometry; }
      const GeoOp* geometry() const { return geometry_; }
      int width() const { return width_; }

      void _validate(bool) override;

      CameraOp* render_camera(int sample = 0) override { return light_cam; }
      GeoOp* render_geo(int sample = 0) override { return geometry_; }

      /*! Returns the inverse camera matrix for a sample.
       */
      Matrix4 camera_matrix(int sample = 0) override;
      Matrix4 projection_matrix(int sample = 0) override;

      //  void initialize_span_context(Span& span);

      const char* Class() const override { return "DepthMapRender"; }
      const char* node_help() const override { return "internal shadow buffer"; }

    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
