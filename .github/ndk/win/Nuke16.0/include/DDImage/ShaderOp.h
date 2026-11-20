//------------------------------------------------------------------------------
// ShaderOp.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_SHADEROP_H
#define DDIMAGE_SHADEROP_H

#include "DDImage/MaterialOpI.h"

#include "ndk/geo/render/RayContext.h" // for SidesMode

#include <vector>

namespace DD
{
  namespace Image
  {
    class Iop;


    /*! \class DD::Image::ShaderOp

        Op base class for shader creating nodes.

        Note that unlike the legacy Material class, ShaderOp is explicitly
        *not* an Iop subclass and will not automatically work as a 'texture'
        source for other shaders.

        Connection of ShaderOps to legacy Material class Ops is not allowed! This
        is explicitly to avoid blending 'Shader' nodes with image nodes within the
        same graph branch to better delineate which is a texture source to a shader
        and what is the shader using those textures.

        The previous Material class conflated these concepts leading to confusing
        results when Material nodes were mixed with Iop nodes. For example it did
        not make much sense to have a Diffuse shader followed by a Blur node since
        the Blur is not processing random, spatially-incoherent surface samples like
        a surface shader does. Plus the output of Blur did not affect the output of
        the Diffuse node, and yet that connection was allowed.
    */
    class DDImage_API ShaderOp : public Op,
                                 public MaterialOpI
    {
    protected:
      Op* _overrideOp;      //!< Optional override Op to use for shader property value updating


    public:
      //! Return the Op this interface is attached to, or the override Op if assigned.
      Op* getOp() override { return (_overrideOp) ? _overrideOp : this; }

      void setOverrideOp(Op* op) override { _overrideOp = op; }
      void clearOverrideOp() override { _overrideOp = nullptr; }
      Op*  getOverrideOp() const override { return _overrideOp; }

      //! Return this ShaderOp cast to the MaterialOpI interface, if it supports translation.
      virtual MaterialOpI*  asMaterialOp() { return nullptr; }


    public:
      ShaderOp(Node*);

      //! Define the material op's shape in the DAG.
      const char*  node_shape() const override;
      //! All material operators default to a orangish color.
      unsigned     node_color() const override;

      /*! Allow ShaderOp or Iop connections *only*.

          Connection to legacy 'Material' class is not allowed! This is done
          explicitly to avoid mixing up 'shader' nodes with 'image' nodes.

          An Iop connection is explicitly a texture source, and a ShaderOp
          connection is explicitly a shader source.

          connectionTypeForNodeInput() is queried to further restrict the connection
          to explicitly Iops or explicitly ShaderOps. If the input is a
          ShaderOp its getOutputType() method is queried to match the connection
          types, and false is returned if they don't.

          Other input Op types are not handled in the base class implementation.
      */
      bool         test_input(int node_input,
                              Op* op) const override;

      /*! Returns nullptr for all inputs, including 0.

          This means any disconnected input will not be connected to any valid Op
          by default, and all Op::input() tests should check for a null pointer result.

          Note that the default output of this Op type is NOT an Iop so do not expect
          the 'Black' default Iop to be automatically connected to a dangling input.
      */
      Op*          default_input(int node_input) const override { return nullptr; }

      //-------------------------------

      //! Return the input Op cast to an Iop, if possible.
      Iop*         textureInput(int32_t node_input) const;

      //-------------------------------

      /*! Return the nominal Format if the shader represents an image/texture/pattern source,
          otherwise null.
          A shader representing an image should return a locally-constructed Format or one from
          an Iop input source that represents the shader's nominal Format.
          For example the Project3DShader ShaderOp returns the Format from its input 0 which
          is the texture image being projected.

          Default implementation returns the nominal Format of the surface by checking if input 0 is
          an Iop and returning its Format, or if input 0 is another ShaderOp pass on the outputFormat
          request, otherwise null.
      */
      virtual const Format*    outputFormat() const;


      //-------------------------------

      //! Create the ShaderDescs required to interface to an Iop texture source.
      static usg::ShaderDesc*  createIopInterfaceGraph(Iop*                   input_iop,
                                                       int32_t                output_type,
                                                       const MaterialContext& rtx,
                                                       usg::ShaderDescGroup&  shader_group);

      //! Update the ShaderDescs required to interface to an Iop texture source.
      static void              updateIopInterfaceGraphOverrides(Iop*                   input_iop,
                                                                int32_t                output_type,
                                                                const MaterialContext& rtx,
                                                                usg::ShaderDescGroup&  shader_group);

      //-------------------------------

      //! Create the ShaderDescs required to interface to a CoShader source.
      static usg::ShaderDesc*  createCoShaderInterfaceGraph(ShaderOp*              input_coshader,
                                                            int32_t                output_type,
                                                            const MaterialContext& rtx,
                                                            usg::ShaderDescGroup&  shader_group);

      //! Update the ShaderDescs required to interface to a CoShader source.
      static void              updateCoShaderInterfaceGraphOverrides(ShaderOp*              input_coshader,
                                                                     int32_t                output_type,
                                                                     const MaterialContext& rtx,
                                                                     usg::ShaderDescGroup&  shader_group);

    };


    //------------------------------------------------------------------------


    /*! Convenience specialization of ShaderOp that sets the output type to
        OUTPUT_TYPE_COSHADER, and defaults input connection type to
        INPUT_TYPE_COSHADEROP.

        A 'CoShader' provides arbitrary input values to another shader but also
        a full fragment Pixel output which allows easy chaining of multi-channel
        shaders together which is common for Nuke.
    */
    class DDImage_API CoShaderOp : public ShaderOp
    {
     public:
      CoShaderOp(Node* node) : ShaderOp(node) {}

      MaterialOpI*  asMaterialOp() override { return static_cast<MaterialOpI*>(this); }
      Op*           getOp() override { return this; }
      int32_t       getOutputType() const override { return OUTPUT_TYPE_COSHADER; }

      int minimum_inputs() const override { return 1; }
      int maximum_inputs() const override { return 1; }
    };


    //------------------------------------------------------------------------


    /*! Temp shader object used during ShaderDesc network creation to inject the
        default surface shader between a co-shader input (non-surface shader) and
        the Material output.

        Calls the static methods on the ShaderOp base class that do this work.

        Before calling updateShaderGraphOverrides() assign the overrideOp if updating
        shaders with a different input Op than the one used to initialize the interface.
    */
    class DDImage_API CoShaderMaterialInterface : public ShaderOp
    {
     protected:
      Op* _baseOp;    //!< Op to use for shader creation and type testing

     public:
      static const Description description;
      const char*  Class()     const override { return "CoShaderMaterialInterface"; }
      const char*  node_help() const override { return ""; }

      // Initializes the baseOp used to create the shader graph.
      CoShaderMaterialInterface(Op* base_op);

      //! Returns the overrideOp or baseOp depending on what's assigned.
      Op* getOp() override { return (_overrideOp) ? _overrideOp : _baseOp; }

      MaterialOpI*      asMaterialOp() override { return static_cast<MaterialOpI*>(this); }
      const char*       getOutputSchema() const override;
      int32_t           getOutputType() const override;

      //! Calls ShaderOp::createIopInterfaceGraph().
      usg::ShaderDesc*  createShaderGraph(int32_t                output_type,
                                          const MaterialContext& rtx,
                                          usg::ShaderDescGroup&  shader_group) override;
      //! Calls ShaderOp::updateIopInterfaceGraphOverrides().
      void              updateShaderGraphOverrides(int32_t                output_type,
                                                   const MaterialContext& rtx,
                                                   usg::ShaderDescGroup&  shader_group) override;
    };


    //------------------------------------------------------------------------


    /*! Specialization for surface shader interfaces providing common surface shader
        Knobs.
    */
    class DDImage_API SurfaceShaderOp : public ShaderOp
    {
    public:
      //! Intersection testing sides mode
      enum SidesMode
      {
        SIDES_BOTH,
        SIDES_FRONT,
        SIDES_BACK
      };

      //! Visibility traits of the surface.
      struct VisibilityTraits
      {
        int32_t  sidesMode;               //!< Which side this material applies to (default is all)
        bool     cameraVisibility;        //!< Is this surface visible to camera rays?
        bool     shadowVisibility;        //!< Is this surface visible to shadow rays?
        bool     specularVisibility;      //!< Is this surface visible to specular rays?
        bool     diffuseVisibility;       //!< Is this surface visible to diffuse rays?
        bool     transmissionVisibility;  //!< Is this surface visible to transmitted rays?

        VisibilityTraits()
          : sidesMode(SIDES_BOTH)
          , cameraVisibility(true)
          , shadowVisibility(true)
          , specularVisibility(true)
          , diffuseVisibility(true)
          , transmissionVisibility(true)
        {
          //
        }
      };

      //! Sample blending operations.
      //! TODO ndk14: this enum is duplicated on SlrSurfaceShader! We need a 'common' shader
      //!             include headers that let us share these definitions since the schemas
      //!             also need the same values.
      enum class BlendOp {
        None, Replace,
        // ones needing alpha which only affect color channels are first:
        Over, Stencil, Mask,
        // cutout operations also use presence*opacity and affect all channels including data:
        Cutout, Reveal,
        // the rest don't need alpha and treat all color channels equally:
        Plus, Max, Min
      };

      //----------------------------------------------------------------------------

    protected:
      VisibilityTraits _visibility;                   //!< Visibility traits of the surface
      const char*      _uvcoord_primvar_name;         //!< Name of the primvar containing UV st-coordinate info
      const char*      _uvwcoord_primvar_name;        //!< Name of the primvar containing UV w-coordinate info
      Channel          _opacity_channel;              //!< Channel to read opacity values from
      float            _opacity_factor;               //!< Multiplier against value returned by opacity input
      float            _opacity_presence_threshold;   //!< Threshold value switching surface presence between 0 or 1.
      //
      Channel          _presence_channel;             //!< Channel to read presence values from
      //
      BlendOp          _blend_op;                     //!< Which blend operation to use when combining this surface sample

    public:
      SurfaceShaderOp(Node*);

      MaterialOpI*  asMaterialOp() override { return static_cast<MaterialOpI*>(this); }
      Op*           getOp() override { return this; }
      int32_t       getOutputType() const override { return OUTPUT_TYPE_SURFACESHADER; }

      //! Adds the knobs to allow the user to control the material's raytracing behavior.
      //! Default implementation add ray and sides visibility controls.
      virtual void  addRayControlKnobs(Knob_Callback f);

      //! Adds the knobs to allow user to specify the UV coordinate primvar the surface shader
      //! will bind to by default.
      virtual void  addUVPrimvarKnobs(Knob_Callback f);

      //! Adds the opacity options controls, and optionally the sample presence threshold control.
      virtual void  addOpacityKnobs(Knob_Callback f,
                                    int32_t       opacity_input_index,
                                    bool          add_presence_threshold = false);

      //! Adds the sample presence options controls.
      virtual void  addPresenceKnobs(Knob_Callback f,
                                     int32_t       presence_input_index);

      //! Adds the sample merge options controls.
      virtual void  addMergeKnobs(Knob_Callback f);

    };


  } // namespace Image
} // namespace DD

#endif
