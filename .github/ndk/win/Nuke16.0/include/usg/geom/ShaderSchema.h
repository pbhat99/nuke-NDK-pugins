// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_SHADERSCHEMA_H
#define USG_GEOM_SHADERSCHEMA_H

#include "usg/geom/Schema.h"
#include "usg/geom/ShaderProperty.h"
#include "usg/base/ArrayTypes.h"    // convenience for ShaderSchema plugins
#include "usg/geom/ShaderSource.h"  // convenience for ShaderSchema plugins

#include <memory>

namespace usg
{

  class ShaderSchema;
  class CoShaderSchema;
  class SurfaceShaderSchema;
  class LightShaderSchema;
  class VolumeShaderSchema;
  class ShaderDesc;
  class ShaderDescGroup;
  class ShaderSource;

  //-----------------------------------------------------------------------------


  //! ShaderSchema class creates ShaderDesc definitions stored in the registry.
  class USG_API ShaderSchema : public usg::Schema
  {
  public:
    //! The shader class name is taken from the ShaderSchemaDescription::schemaName(),
    //! so the subclass must pass its ShaderSchemaDescription.
    ShaderSchema(const SchemaDescription&  pluginDescription,
                 const ShaderPropertyList& properties,
                 const fdk::KeyValueMap&   metadata = fdk::KeyValueMap());

    //! Return a string indicating this is a ShaderSchema.
    const Token&   baseTypeName() const override { return GeomTokens.usgShader; }

    //! Used to cast a Schema to a ShaderSchema.
    ShaderSchema*  asShaderSchema() override { return this; }


    //! Used to cast a ShaderSchema to a CoShaderSchema.
    virtual CoShaderSchema*       asCoShader()       { return nullptr; }
    //! Used to cast a ShaderSchema to a SurfaceShaderSchema.
    virtual SurfaceShaderSchema*  asSurfaceShader()  { return nullptr; }
    //! Used to cast a ShaderSchema to a LightShaderSchema.
    virtual LightShaderSchema*    asLightShader()    { return nullptr; }
    //! Used to cast a ShaderSchema to a VolumeShaderSchema.
    virtual VolumeShaderSchema*   asVolumeShader()   { return nullptr; }

    //! Return the implementation code string for a specific sourceType when
    //! the shader is the single shader in a material network.
    //! Subclass should have already declared the type in its list of supported
    //! types, so if there are multiple sources the type string should be
    //! checked.
    virtual const std::string& sourceCode(const Token& sourceType) const;

    //! Creates the source code for the shader for a specific target (e.g. glslfx)
    //! using the ShaderSource generator previously created via \a shaderSourceGenerator().
    //! If a ShaderSource generator was not created by the shader for the requested source
    //! type then this method is never called.
    //! Default implementation does nothing.
    virtual void sourceCode(ShaderSource& shaderSource) const;

    //! Return the input/output shader properties.
    virtual const ShaderPropertyList& shaderProperties() const;
  };

  using ShaderSchemaPtrMap       = std::unordered_map<std::string, ShaderSchema*>;
  using ShaderSchemaConstPtrMap  = std::unordered_map<std::string, const ShaderSchema*>;


  //-----------------------------------------------------------------------------


  /*!
  */
  class USG_API CoShaderSchema : public ShaderSchema
  {
  private:
    //! Disabled copy constructor.
    CoShaderSchema(const CoShaderSchema&) = delete;
    //! Disabled assignment operator.
    CoShaderSchema& operator = (const CoShaderSchema&) = delete;


  public:
    //!
    CoShaderSchema(const SchemaDescription&  pluginDescription,
                   const ShaderPropertyList& properties,
                   const fdk::KeyValueMap&   metadata = fdk::KeyValueMap());

    //! Used to cast a ShaderSchema to a CoShaderSchema.
    CoShaderSchema*  asCoShader() override { return this; }

  public:
    // TODO: add something!

  };


  //-----------------------------------------------------------------------------


  /*!
  */
  class USG_API SurfaceShaderSchema : public ShaderSchema
  {
  private:
    //! Disabled copy constructor.
    SurfaceShaderSchema(const SurfaceShaderSchema&) = delete;
    //! Disabled assignment operator.
    SurfaceShaderSchema& operator = (const SurfaceShaderSchema&) = delete;


  public:
    //!
    SurfaceShaderSchema(const SchemaDescription&  pluginDescription,
                        const ShaderPropertyList& properties,
                        const fdk::KeyValueMap&   metadata = fdk::KeyValueMap());

    //! Used to cast a ShaderSchema to a SurfaceShaderSchema.
    SurfaceShaderSchema*  asSurfaceShader() override { return this; }

  public:
    // TODO: add something!

  };


  //-----------------------------------------------------------------------------


  /*!
  */
  class USG_API LightShaderSchema : public ShaderSchema
  {
  private:
    //! Disabled copy constructor.
    LightShaderSchema(const LightShaderSchema&) = delete;
    //! Disabled assignment operator.
    LightShaderSchema& operator = (const LightShaderSchema&) = delete;


  public:
    //!
    LightShaderSchema(const SchemaDescription&  pluginDescription,
                      const ShaderPropertyList& properties,
                      const fdk::KeyValueMap&   metadata = fdk::KeyValueMap());

    //! Used to cast a ShaderSchema to a LightShaderSchema.
    LightShaderSchema*   asLightShader() override { return this; }


  public:
    // TODO: add something!

    //!
    //virtual int     lightType() const { return 0/*ePointLight*/; }

  };


} // namespace usg

#endif
