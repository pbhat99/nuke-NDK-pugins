// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_SHADERPRIM_H
#define USG_GEOM_SHADERPRIM_H

#include "usg/geom/Prim.h"
#include "usg/geom/ShaderDesc.h"

namespace usg
{
  class SchemaDescription;
  class MaterialPrim;


  /*!
  */
  class USG_API ShaderPrim : public usg::Prim
  {
  public:
    //! Default ctor builds an empty (invalid) prim.
    ShaderPrim();
    /*! Attempt to cast Prim to a ShaderPrim. ShaderPrim will be invalid if not successful.
        If a valid ShaderPrim the SchemaDescription will be retrieved from it.
    */
    ShaderPrim(const Prim&);
    ShaderPrim(const ShaderPrim&);
    ~ShaderPrim();


    /*! Call this after construction to validate this Prim as a ShaderPrim type, returning
        true if it's still valid.
        If not the Prim::Impl will be invalidated and isValid() will then return false.
    */
    bool  validate() override;

    //! Returns true if prim is a ShaderPrim subclass.
    static bool  isValidPrim(const Prim& prim);


    //--------------------------------------------------------------

    /*! Convenience function that sets the 'info:implementationSource' and 'info:id'
        properties to the correct values for the ShaderPrim contents to be loaded, typically
        via a Schema plugin.

        Specifically 'info:implementationSource' is set to 'id' so that the plugin system
        will look at the value of 'info:id', and 'info:id' is set to the name of the
        Schema name to load.
    */
    void  initializeSchemaSourceInfo(const ShaderDesc& shader_desc);

    //-------------------------------

    /*! Assign/override the 'info:id' attribute, returning true on success.
        The identifer string should come from a valid SchemaDescription found
        in the shader registry.
    */
    bool         setIdentifierAttr(const usg::Token& identifier);

    //! Get the value of the 'info:id' attribute, if there is one.
    usg::Token   getIdentifier() const;

    //! Get the value of the `info:id` attribute, if the shader's info:implementationSource is id.
    bool         getShaderId(usg::Token& info_id) const;

    //-------------------------------

    /*! Assign/override the 'info:implementationSource' attribute, returning true on
        success.

        The implementation source attribute is a hint to the Hydra material resolution
        system for where the shader code comes from.
        Current options are:
          'id':          Attribute 'token info:id' value is used to determine the shader source
                         from the shader registry entry.

          'sourceAsset': Attribute 'asset info:\<namespace\>:sourceAsset' value defines the path
                         to the shader source code asset.
                         ie 'asset info:glslfx:sourceAsset = @./foo.glslfx@' defines the path
                         to a file containing the glslfx implementation source code.

                         TODO: An optional 'token info:sourceAsset:subIdentifier' attribute is
                         also taken into account but it's unclear how this is different then
                         'info:\<namespace\>:sourceAsset'...

          'sourceCode':  Attribute 'string info:sourceCode' provides the shader source code
                         text.

        For more information reference the 'shaderDefs.usda' example shader schema,
        currently at pxr/usd/usdHydra/shaders/ in the Pixar USD source package.
    */
    bool         setImplementationSourceAttr(const usg::Token& implementation_source);

    //! Get the value of the 'info:implementationSource' attribute, if there is one.
    usg::Token   getImplementationSource() const;

    //-------------------------------

    /*! Assign/override the 'info:\<source_namespace\>:sourceAsset' attribute, returning true on success.
    */
    bool         setSourceAssetAttr(const Path&        source_assert_path,
                                    const std::string& source_namespace ="");
    std::string  getSourceAsset(const std::string& source_namespace ="") const;

    //-------------------------------

    /*! Assign/override the 'info:\<source_namespace\>:sourceCode' attribute, returning true on success.
    */
    bool         setSourceCodeAttr(const std::string& source_code,
                                   const std::string& source_namespace ="");
    std::string  getSourceCode(const std::string& source_namespace ="") const;


    //--------------------------------------------------------------


    /*! Defines a Shader prim in the owner Layer, returning a constructed reference.

        If a valid ShaderDesc is provided the ShaderSchema will be retrieved from it
        and any overrides or connections it contains will be created in the resulting
        ShaderPrim by this method calling ShaderPrim::connectInputs() and
        ShaderPrim::applyInputOverrides().
    */
    static ShaderPrim  defineInLayer(const ShaderDesc& source_shader_desc,
                                     const LayerRef&   layer,
                                     const Path&       material_path);

    /*! Retrieves an editable Shader prim in the Layer and applies any changes from
        the provided ShaderDesc to it.

        If a valid ShaderDesc is provided the path to the ShaderPrim will be constructed
        from the material_path + the ShaderDesc's material-relative path.
    */
    static ShaderPrim  updateInLayer(const ShaderDesc& source_shader_desc,
                                     const LayerRef&   layer,
                                     const Path&       material_path);

    /*! Retrieves an editable Shader prim in the owner Stage and applies any changes from
        the provided ShaderDesc to it.

        If a valid ShaderDesc is provided the path to the ShaderPrim will be constructed
        from the material_path + the ShaderDesc's material-relative path.
    */
    static ShaderPrim  updateInStage(const ShaderDesc& source_shader_desc,
                                     const StageRef&   stage,
                                     const Path&       material_path);



    /*! Defines a Material prim in the owner Layer, returning a constructed reference.
    */
    static ShaderPrim  defineInLayer(const LayerRef& layer,
                                     const Path&     path);

    /*! Retrieves an editable Shader prim in the owner Stage, or an invalid one if none exists.
    */
    static ShaderPrim  getInStage(const StageRef& stage,
                                  const Path&     path);

    //! Retrieves any overrides from the ShaderDesc and writes them to the
    //! matching Prim Attributes, optionally skipping any that have
    //! connections defined.
    //!
    //! SchemaProperties tagged as 'usg:localProp' (a LocalProp) will not have the
    //! 'inputs:' prefix added to the property name so Schema overrides can be
    //! applied to generic attributes.
    //!
    //! Returns false if any underlying error occurred.
    bool  applyAttributeOverrides(const ShaderDesc& schema_desc,
                                  bool              skip_connected = true) override;


    //--------------------------------------------------------------
    //
    //--------------------------------------------------------------

    /*! Retrieve any connections from the ShaderDesc and write them to ShaderPrim
        input Attributes.
        Returns false if any underlying error occurred.
    */
    bool  connectInputs(const ShaderDesc& source_shader_desc,
                        const Path&       material_path);

    /*! Retrieve any output properties from the ShaderDesc and write them to ShaderPrim
        output Attributes.
        Returns false if any underlying error occurred.
    */
    bool  defineOutputs(const ShaderDesc& source_shader_desc,
                        const Path&       material_path);


  public:
    //--------------------------------------------------------------
    //
    //--------------------------------------------------------------

    /*! Return the ShaderSchema class, if one has successfully retrieved from the 'info:id' attribute.
    */
    const SchemaDescription*         getSchema() const { return _schema; }

    /*! Use the prim's 'info:id' attribute to retrieve the shader's schema from the registry.
        Returns null if 'info:id' doesn't exist or its value isn't found in registry.
    */
    static const SchemaDescription*  retrieveShaderSchema(const Prim& prim);


    //--------------------------------------------------------------
    //
    //--------------------------------------------------------------

    /*! Create a ShaderDesc instance from this ShaderPrim, filling in any
        overrides or connections present. The parent_material prim is used
        to create a relative path for the resulting ShaderDesc inside the
        material group.

        Uses the 'info:id' for the registry id, then converts each property
        override and connection.

        Enabling debug will print shader creation debug info to terminal (cout).

        Calling code takes ownership of ShaderDesc allocation.
    */
    ShaderDesc*  createShaderDesc(const MaterialPrim& parent_material,
                                  bool                debug_info =false) const;


  protected:
    const SchemaDescription*  _schema;      //!< Reference to the const ShaderSchema registry node

  };

}

#endif
