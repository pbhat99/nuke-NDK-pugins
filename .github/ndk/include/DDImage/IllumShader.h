// IllumShader.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

// IllumShader class describes how to shade a object

#ifndef DDImage_IllumShader_h
#define DDImage_IllumShader_h

#include "DDImage/Material.h"
#include <vector>

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::IllumShader

       \brief This subclass of Material provides some convienence functions so that it
       is easier to write a lighting surface shader.

     */
    /*!
        Base class of illumination shaders that adds convenience functions
        and a map input for typical lighting shaders.
     */
    class DDImage_API IllumShader : public Material
    {
    public:

      /*! Default the shader channels to RGB.
       */
      IllumShader(Node*);

      int minimum_inputs() const override { return 2; }
      int maximum_inputs() const override { return 2; }

      /*! For input 0 it uses the default Material input0.
          Input 1 is allowed to be NULL so we can test whether it's connected.
       */
      Op* default_input(int input) const override;

      /*! Input 0 has no label, input 1 is 'map'. */
      const char* input_label(int input, char* buffer) const override;

      /*! Set up a channel knob for the shader. */
      void knobs(Knob_Callback f) override;

      /*! Pass call to input0. */
      bool shade_GL(ViewerContext*, GeoInfo&) override;

      /*! Pass call to input0. */
      void unset_texturemap(ViewerContext*) override;

      /*! Pass call to input0. */
      void vertex_shader(VertexContext&) override;
      void fragment_shader(const VertexContext&, Pixel& out) override;

    protected:
      /*! The light shader call.  Pure virtual, must be implemented by subclasses. */
      virtual void surface_shader(Vector3& P, Vector3& V, Vector3& N,
                                  const VertexContext&, Pixel& surface) = 0;

      /*! Add surface channels to info.
       */
      void _validate(bool) override;

      /*! Add surface channels to request.
       */
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;

    protected:
      Channel channel[4];        //!< Layer to apply shading to
      ChannelSet surface_channels; //!< ChannelMask built from channel[]

    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
