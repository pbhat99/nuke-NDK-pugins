// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_COSHADER_H
#define SLR_COSHADER_H

#include "DDImage/SlrShader.h"

namespace slr
{

  //-----------------------------------------------------------------------------


  /*! \class DD::Image::SlrCoShader

      A 'CoShader' provides arbitrary input values to another shader but also
      a full fragment Pixel output which allows easy chaining of multi-channel
      shaders together which is common for Nuke.

      Normally there is only one surface shader per shading network which may an
      arbitrarily-large coshader input network feeding it. Surface shaders apply
      lighting calculations and are the final shading point before fragment
      integration.

      See SlrSurfaceShader and SlrLightShader for surface & light shading specifics.
  */
  class DDImage_API SlrCoShader : public SlrShader
  {
  public:
    //!
    SlrCoShader(SlrEngineContext*     slrtx,
                const InputKnobList&  inputs,
                const OutputPortList& outputs,
                void*                 inputs_storage_data=nullptr,
                size_t                inputs_storage_size=0);


    //! Returns the class name, must implement.
    const char*  shaderClass() const override { return "SlrCoShader"; }

    //!
    SlrCoShader*  asCoShader() override { return this; }

    //! Default gets the format from input(connection 0).
    void _validateShader() override;

    //! Default gets the port's varying state from input(connection 0).
    void _configureOutputPort(OutputPort& port, SlrShader* connectedShader) override;

    //!
    static SlrCoShader*  create(SlrEngineContext*, const std::string_view& slr_shader_class);
    static SlrCoShader*  create(SlrEngineContext*, const usg::ShaderDesc* shader_desc);


  public:
    //--------------------------------------------------------------------
    //
    // Slr2 co-shader specific methods
    //
    // 'Co' shaders are used for passing values between shaders, for any
    // general use -except- lighting calculations or fragment merging,
    // which is handled by surface shaders.
    //
    // See SlrSurfaceShader and SlrLightShader for surface & light shading
    // specifics.
    //--------------------------------------------------------------------

    //! Convenience to get another SlrCoShader input. Calls getConnection() and tests its shader type.
    SlrCoShader*  getCoShaderForInput(const std::string& input_name) const;

    /*! Shader multi-channel output, prior to a surface shader
        applying any lighting or fragment blending.

        This is a convenience to skip binding inputs - the downstream
        shader can simply test for this shader's validity and then
        call this method to get all this shader's output channels
        at once.

        \a get ChannelSet overrides the one contained in the Pixel so that
        a subset of the Pixel channels can be filled, possibly outside the
        Pixel's assigned set.
    */
    virtual void  getResult(const SlrShadingContext&     stx,
                            const DD::Image::ChannelSet& get,
                            DD::Image::Pixel&            out) = 0;


  private:
    //! Disabled copy constructor.
    SlrCoShader(const SlrCoShader&) = delete;
    //! Disabled assignment operator.
    SlrCoShader& operator = (const SlrCoShader&) = delete;

  };

  using SlrCoShaderList = std::vector<SlrCoShader*>;
  using SlrCoShaderMap  = std::unordered_map<std::string, SlrCoShader*>;


} // namespace slr


#endif
