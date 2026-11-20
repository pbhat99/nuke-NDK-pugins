//------------------------------------------------------------------------------
// DDImage/MaterialOpI.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_MATERIALOP_H
#define DDIMAGE_MATERIALOP_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Knobs.h"

#include "usg/geom/ShaderDesc.h"


namespace DD
{
  namespace Image
  {
    class Op;


    /*! \class DD::Image::MaterialOpI

        Interface class for abstracting Ops which create shaders.

        These Ops are typically containers for shader parameters which are translated
        into actual shader implementations, usually by loading a translator plugin.

        Note that the word 'material' in the class name is used generically here.
        Attaching the interface to an Op simply provides a mechanism to create shaders,
        it does not imply that the Op creates an entire 'Material' in the USD sense
        of the word.

    */
    class DDImage_API MaterialOpI
    {
    public:
      enum
      {
        INPUT_TYPE_NONE=-1,           //!< Don't allow input to connect to anything
        INPUT_TYPE_OP,                //!< Generic Op connection (can connect to any Op types)
        INPUT_TYPE_MATERIALOP,        //!< MaterialOpI input
        INPUT_TYPE_COSHADEROP,        //!< CoShader input
        INPUT_TYPE_TEXTUREOP,         //!< Iop or CoShader input as texture
        INPUT_TYPE_AXISOP,            //!< AxisOp input
        INPUT_TYPE_CAMERAOP,          //!< CameraOp input
        INPUT_TYPE_LIGHTOP            //!< LightOp input
      };

      enum
      {
        OUTPUT_TYPE_NONE=-1,          //!< Invalid output, don't use (used for error cases)
        OUTPUT_TYPE_COSHADER,         //!< Output shader is a 'co-shader', providing values for another shader's input
        OUTPUT_TYPE_SURFACESHADER,    //!< Output shader should provide final surface/displacement outputs
        OUTPUT_TYPE_LIGHTSHADER,      //!< Output shader should provide final light shading outputs
        OUTPUT_TYPE_VOLUMESHADER      //!< Output shader should provide final volume shading outputs
      };


      //----------------------------------------------------------------------------

      /*! Material-specific context to use for creating/overriding properties.
      */
      class DDImage_API MaterialContext
      {
      public:
        usg::Token     shaderFamily;    //!< Family name for consuming shading system - 'slr', 'glslfx', 'mtlx', etc.
        OutputContext  outputContext;   //!< Output time/view to set or override material at

        fdk::TimeValue  outputTime() const { return outputContext.frame(); }
        int32_t         outputView() const { return outputContext.view(); }

      public:
        MaterialContext(const std::string&   shaderFamily,
                        const OutputContext& outputContext);
      };

      //----------------------------------------------------------------------------


    public:
      //!
      MaterialOpI() {}
      //!
      virtual ~MaterialOpI() {}


      //! Return the Op this interface is attached to. Should return 'this'.
      virtual Op*  getOp() { return nullptr; }

      /*! Assign an optional override Op location used for shader property value updating
          which may have a different time or view than the base Op the MaterialOpI is
          bound to.

          Subclasses can return this override Op in getOp() so Knob sampling is performed
          on the override rather than the base Op.
      */
      virtual void setShaderInstanceOverride(const OpGraphLocation& opLocation) {}

      //! Disable any assigned override Op location.
      virtual void clearShaderInstanceOverride() {}


      //----------------------------------------------------------------------------
      //
      //----------------------------------------------------------------------------

      //! Returns a list of API's this output shader must implement.
      virtual usg::TokenArray getAppliedApiSchemas() const { return {}; }

      //! Returns a list of material families supported by this
      //! shader e.g. mtlx.
      virtual usg::TokenArray getMaterialFamilies() const { return {}; }

      /*! Return a Schema name to use when building the output shader.
          Must implement.
      */
      virtual const char*  getOutputSchema() const
#if (kDDImageVersionMajorNum > 17)
                                                   = 0;
#else
                                                   { return ""; }
#endif


      /*! Return the type of the output Schema, used to determine Op interconnection
          compatibility.
          Must implement.
      */
      virtual int32_t      getOutputType() const
#if (kDDImageVersionMajorNum > 17)
                                                 = 0;
#else
                                                 { return OUTPUT_TYPE_NONE; }
#endif

      //! Convenience to check output type.
      bool  isCoShader()      const { return (getOutputType() == OUTPUT_TYPE_COSHADER); }
      bool  isSurfaceShader() const { return (getOutputType() == OUTPUT_TYPE_SURFACESHADER); }
      bool  isLightShader()   const { return (getOutputType() == OUTPUT_TYPE_LIGHTSHADER); }
      bool  isVolumeShader()  const { return (getOutputType() == OUTPUT_TYPE_VOLUMESHADER); }


      /*! Return whether this is a custom Nuke material.

          Custom Nuke materials are converted to glslfx code, in order to be handled
          by the HydraViewer. If this material is already supported, for example
          MaterialX nodes in the renderer, this should return false.
      */
      virtual bool isCustomNukeMaterial() { return true; }


      /*! Get a unique name for the output shader node(s) this shader interface produces.
          If this material outputs multiple shader nodes then this name should be used as
          the base for them.

          These names are ultimately used to name the output ShaderPrims added to the stage
          and to find the same ShaderPrims later on when updating property values. So, they
          need to be uniquely named underneath the MaterialPrim with the assumption they
          will all live in a single 'pool' of shaders - ie there's no additional prim
          hierarchy structure underneath the MaterialPrim.)

          The default name is the Op's node name with the schema name appended to the end.
          Since Nuke Node names are quaranteed to be unique this is a reliable way to
          identify the shader without resorting to something like UUIDs or hashing
          the Node pointer. For example the Nuke Node named 'Project3D5' will return the
          shader node name 'Project3D5_Project3DTexture' as 'Project3DTexture' is the output
          schema name for the Project3DShaderOp plugin.

          If 'shaderFamily' is not empty it is prepended to the name with an underscore '_'
          separator. So 'glslfx' shader family will produce 'glslfx_Project3D5_Project3DTexture'.

          Nested shader graph groups are not supported and all shader node names are assumed
          to be within the *same* group namespace - ie all the node names must be unique.

          Note that these are node *names* not paths, so the names should not include any
          pathing characters like '/' or '.', and underscores '_' are recommended as a
          separator for compound names.
      */
      virtual std::string  getShaderNodeName(const usg::Token& shaderFamily = usg::Token());


      /*! Return the preferred input type enum for the Node input number. This is really
          only a hint as a subclass is allowed to perform whatever connection logic it
          wants.

          Currently the ShaderOp class respects this setting for the test_input()
          method, so if a subclass returns INPUT_TYPE_TEXTUREOP then test_input() will
          return false for that input if the connection Op is not an Iop.

          Base class returns INPUT_TYPE_OP, any connection type is ok.
      */
      virtual int32_t      connectionTypeForNodeInput(int32_t node_input) const { return INPUT_TYPE_OP; }


      //----------------------------------------------------------------------------
      //
      // ShaderDesc creation / update functions
      //
      //----------------------------------------------------------------------------

      /*! Allocate a series of ShaderDescs this Op and its input graph produces, and
          return the output connection point.

          Each created ShaderDesc must be assigned a unique prim name inside the
          group that can be reliably retrieved in updateShaderGraphOverrides() when
          the shader overrides may get changed. To be reliably retrievable the shader
          names are usually tied to a Nuke Node name which is reliably unique.
          For example if there are multiple chained MergeShader nodes then the name
          of each shader in the group will typcially contain the name of the Nuke node
          for this reason.
          The getShaderNodeName() method will build a unique shader name for you
          based on the Op/Node this interface is attached to.

          If you need to build an absolute prim path for the ShaderDesc use the
          ShaderDescGroup's 'material_prim_path' as the parent path.

          Calling object takes ownership of all created allocations.
      */
      virtual usg::ShaderDesc*  createShaderGraph(int32_t                outputType,
                                                  const MaterialContext& rtx,
                                                  usg::ShaderDescGroup&  shaderGroup) { return nullptr; }

      /*! Same as createShaderGraph() but automatically handles input Op as a MaterialOpI
          or a built-in interface shader.

          By default if the input Op is *not* a MaterialOpI any created shader is added
          to the ShaderDescGroup with a default name but no output assignment is made.

          If a ShaderDesc with the same name already exists the shader is not created
          or added. This commonly happens when a texture source is shared between
          several shaders.
      */
      virtual usg::ShaderDesc*  createShaderGraphFromOp(const OpGraphLocation& opLocation,
                                                        const MaterialContext& rtx,
                                                        usg::ShaderDescGroup&  shaderGroup);

      //-------------------------------

      /*! Update the ShaderDescs this Op and its input graph produces.

          To be updated properly the ShaderDescs in the ShaderDescGroup must have
          reliably reproducable names that tie the shader to a specific Nuke Node. For
          example if there are multiple chained MergeShader nodes then the name of each
          shader in the group will typcially contain the name of the Nuke node for this
          reason.

          Subclasses implementing this method should look up the ShaderDesc it produces
          by name in the ShaderDescGroup using the ShaderDescGroup::getShaderNode()
          method, then assign the property overrides, also calling
          updateShaderGraphOverrides() on any MaterialOpI inputs.

          Default implementation does nothing.
      */
      virtual void  updateShaderGraphOverrides(int32_t                outputType,
                                               const MaterialContext& rtx,
                                               usg::ShaderDescGroup&  shaderGroup) {}

      /*! Same as updateShaderGraphOverrides() but automatically handles input Op as a
          MaterialOpI or a built-in interface shader.
      */
      virtual void  updateShaderGraphFromOp(const OpGraphLocation& opLocation,
                                            const MaterialContext& rtx,
                                            usg::ShaderDescGroup&  shaderGroup);



    public:
      //----------------------------------------------------------------------------
      //
      // ShaderSchema / ShaderDesc utility functions
      //
      //----------------------------------------------------------------------------


      /*! Convenience function builds a suffix depending on if the op's node has clones.
          Return empty string if the node has no clones or else '_inst<instance index>'.
      */
      static std::string       getNodeInstanceSuffix(Node* node);

      //! Convenience function builds a standard shader name with pattern '<name>_<suffix>'.
      static std::string       buildName(const std::string& name,
                                         const char*        suffix);
      //! Convenience function builds a standard shader name with pattern '<shaderFamily>_<name>_<suffix>'.
      static std::string       buildName(const usg::Token&  shaderFamily,
                                         const std::string& name,
                                         const char*        suffix);


      /*! Return the type of the appropriate schema interface to use for the
          the provided Op. Returns INPUT_TYPE_NONE if no schema is appropriate.

         These base Op types are currently supported:
           Op:             INPUT_TYPE_OP          ( generic Op* access )
           Iop:            INPUT_TYPE_TEXTUREOP   ( Iop* usually for texture access )
           AxisOp:         INPUT_TYPE_AXISOP,     ( AxisOp* )
           CameraOp:       INPUT_TYPE_CAMERAOP    ( CameraOp* )
           LightOp:        INPUT_TYPE_LIGHTOP     ( LightOp* )

         Shader Ops (INPUT_TYPE_MATERIALOP) are handled by testing for the presence
         of a MaterialOpI interface on the Op. INPUT_TYPE_COSHADER is for allowing
         only CoShaderOps specializations on an input.
      */
      static int32_t           getShaderTypeFromOp(Op* op);

      //! Build the unique shader node name to use for the Op location using the location node
      //! name and output schema name.
      //! Prepends \a shaderFamily (if not empty) to final name with an underscore '_' separator.
      static std::string       getShaderNodeNameFromOp(const usg::Token&      shaderFamily,
                                                       const OpGraphLocation& opLocation);


      /*! Returns the built-in shader Schema name for input 'op_type'.

          The built-in Op interface ShaderSchemas are:
            'NukeOpInterface'         INPUT_TYPE_OP         ( generic Op* access )
            'NukeIopInterface'        INPUT_TYPE_TEXTUREOP  ( Iop* usually for texture access )
            'NukeAxisOpInterface'     INPUT_TYPE_AXISOP,    ( AxisOp* )
            'NukeCameraOpInterface'   INPUT_TYPE_CAMERAOP   ( CameraOp* )
            'NukeLightOpInterface'    INPUT_TYPE_LIGHTOP    ( LightOp* )
      */
      static const char*       inputShaderTypeName(int32_t op_type);

      /*! Skips any leading 'inputs:' or 'outputs:' prefix in the property name.
          This commonly happens when the property names are in a prim rather than
          a shader. For example a Schema property named 'inputs:color' will
          be changed to 'color'.
      */
      static const char*       removePrefix(const char* property_name);

      /*! Create an Op interface ShaderDesc from the provided interface type,
          setting its referencing properties from the Op.

          The Op's absolute Node path (its Op::node_name()) and OutputContext are
          used for this as those uniquely identify an allocated Op.
          These properties are then used to reaquire the Op* when consumers like
          Hydra or SlrShaders need to evaluate values from the Op via the shader
          interface.
      */
      static usg::ShaderDesc*  createOpInterfaceShaderDesc(int32_t                op_type,
                                                           const OpGraphLocation& opLocation);


      /*! Create an Op interface ShaderDesc from the provided Op, setting its
          referencing properties.

          The ShaderSchema name to create is retrieved via inputShaderTypeName()
          using the Op* type and passed to the createOpInterfaceShaderDesc(op_type, op)
          method.
      */
      static usg::ShaderDesc*  createOpInterfaceShaderDesc(const OpGraphLocation& opLocation);


      /*! Convenience to retrieve a ShaderDesc from the named input of another ShaderDesc.
          It does this by searching the ShaderConnections for the input and matching the
          one connected to the named output port of the upstream shader. If the port name
          is matched then the upstream shader name is retrieved from shaderGroup.

          Note - if output port name is missing the leading 'outputs:' this prefix will be
          added to the port name before matching.
      */
      static usg::ShaderDesc*  getInputShaderDesc(const usg::ShaderDesc* shaderDesc,
                                                  const std::string&     inputName,
                                                  const std::string&     upstreamOutputPortName,
                                                  usg::ShaderDescGroup&  shaderGroup);


      //----------------------------------------------------------------------------


      /*! Creates a path representing the op, typically put into a shader 'file' input property.

          The path includes the node path and instance(clone), OutputContext(frame, view, gsv hash, proxy info)
          and follows this syntax:
            /NkRoot/<node-path>:<frame-num>:<view-name>:0x<gsv-hash>:<proxy-info>:0x<op-hash>.<opclass-ext>

          <opclass-ext> is specialized to the Op type - ie Iop, AxisOp, CameraOp, LightOp, or just
          Op, depending on the shader plugin that needs to be created by the USD plugin system.

          <frame-num> is a Nuke script frame index (not a time) and can be floating point allowing
          access to fractional frame samples.

          Note that this path is *FAKE* and only used as a way to communicate the Op pointer through
          the USD system. NodeI::findOp() is used to retrieve the correct matching Op instance,
          so this is why the node path + instance, view, frame, gsv-hash and proxy-info are provided
          to uniquely identify a specific Op*.

          Also note the path *MUST* be an absolute path (starting with '/') otherwise USD will fail
          to parse the path correctly!

          If a ShaderSchema declares an AssetPath input (see NukeIopInterface schema for an
          example) that path is resolved by USD by checking for a matching plugin that's been
          registered to handle the extension (ex. '.nkiop' for NukeIopInterface.)
      */
      static std::string buildAssetPath(Op* input_op);

      /*! Parses a Node path string to extract its node name and output context.

          The syntax matching presumes the path was created via buildAssetPath(), and must be
          an absolute path starting with '/NkRoot/', although it can contain a leading URI
          token like 'nkop:' or 'raw:' which will be ignored.

          Important note - the contructed \a outputContext *must match* the OutputContext of
          the Op passed to buildAssetPath() otherwise the incorrect Op may be retrieved leading
          to anomalous texture behavior in Hydra and ScanlineRender.

          Returns false on any failure.
      */
      static bool parseAssetPath(const std::string& path,
                                 std::string&       nodePath,
                                 OutputContext&     outputContext);

      /*! Uses parseAssetPath() to extract the Op path info then calls Op::retrieveOp()
          to get the Op*. If \a outputContext is provided then it will be filled in with
          the result from parseAssetPath().

          See parseAssetPath() for info on possible problems related to Op retrieval due
          to incorrect texture asset path encodings.

          Return null on failure, either due to parsing or an Op* retrieval problem.
      */
      static Op* retrieveOpFromAssetPath(const std::string& path,
                                         OutputContext*     outputContext = nullptr);



      //----------------------------------------------------------------------------
      //
      // Input property override functions
      //
      //----------------------------------------------------------------------------

      //!
      static usg::Value  sampleKnobValue(Knob*                  knob,
                                         const fdk::FrameValue& frame,
                                         int32_t                view,
                                         usg::Value::Type       value_type_hint);

      /*! Copy the value from a local Op Knob to the ShaderDesc which will assign or clear
          local input overrides. If Knob::not_default() is false the override is applied,
          otherwise the override is cleared.

          'target_property_type_hint' is used to indicate the preferred output value type from
          Knobs that support them, such as Enumeration_knob which can either return the integer
          index or the string of the current selection, and Boolean_knob which can return either
          a boolean or int type. This is not done automatically so that Ops can better manage
          the conversion of knob values to shader overrides.
          For example pass in usg::Value::Type::String for an Enumeration_knob's string value
          and usg::Value::Type::Int for a Boolean_knob's integer value. Or if the Op has a
          Double_knob and it's writing to a shader float then pass usg::Value::Type::Float.

          If greater control is required, such as splitting an 4-component AColor_knob into
          separate color & opacity destinations, a plugin should directly call the
          ShaderDesc::overrideInput() & ShaderDesc::clearInputOverride() methods.

          These are virtual to allow subclasses to manage custom translation of property
          names, etc.

          If \a ignoreDefaultTest is false we test if the knob is at its default value and only
          writes the override if it is off default. USD, like Nuke, will use the default value of
          the knob as fallback if an explicit value is not authored.

          However, often the Nuke knob's default will not be the same as the USD shader property,
          default and in those cases we always want to author the USD property so that we don't
          allow USD to use the default fallback.

          Returns true if the override was applied successfully.
      */
      virtual bool  overrideShaderDescInput(const char*            knob_name,
                                            bool                   ignoreDefaultTest,
                                            const MaterialContext& rtx,
                                            usg::ShaderDesc&       shader_desc,
                                            const char*            target_property_name,
                                            usg::Value::Type       target_property_type_hint = usg::Value::Type::InvalidType);

      /*! Specialization which automatically uses the Knob's default value to determine whether to
          author the override (ignoreDefaultTest = false).
      */
      virtual bool  overrideShaderDescInput(const char*            knob_name,
                                            const MaterialContext& rtx,
                                            usg::ShaderDesc&       shader_desc,
                                            const char*            target_property_name,
                                            usg::Value::Type       target_property_type_hint = usg::Value::Type::InvalidType);

      /*! Specialization for Channel-source override which is translated/aliased to an int32_t.
      */
      virtual bool  overrideShaderDescInput(const Channel&         chan,
                                            const MaterialContext& rtx,
                                            usg::ShaderDesc&       shader_desc,
                                            const char*            target_property_name);

      /*! Specialization for ChannelArray-source override which is translated/aliased to an int32_t array.
      */
      virtual bool  overrideShaderDescInput(const Channel*         chan_array,
                                            uint32_t               num_channels,
                                            const MaterialContext& rtx,
                                            usg::ShaderDesc&       shader_desc,
                                            const char*            target_property_name);

      /*! Specialization for ChannelSet-source override which is translated/aliased to an int32_t array.
      */
      virtual bool  overrideShaderDescInput(const ChannelSet&      channels,
                                            const MaterialContext& rtx,
                                            usg::ShaderDesc&       shader_desc,
                                            const char*            target_property_name);


    };


  } // namespace Image
} // namespace DD

#endif
