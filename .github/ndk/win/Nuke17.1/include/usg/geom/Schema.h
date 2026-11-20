// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_SCHEMA_H
#define USG_GEOM_SCHEMA_H

#include "usg/geom/Attribute.h"
#include "usg/geom/GeomTokens.h"
#include "usg/geom/SchemaProperty.h"

#include <memory>

namespace usg
{

  class ShaderSchema;
  class ShaderSchemaHandle;
  class ShaderDescHandle;
  class Schema;
  using SchemaSchemaHandle = ShaderSchemaHandle;
  using SchemaDescHandle   = ShaderDescHandle;

  //-----------------------------------------------------------------------------


  /*! This structure creates a subclass of Schema, possibly from loading
      a plugin file or a compiled-in SchemaDescription.
  */
  class USG_API SchemaDescription
  {
  private:
    const char*             _plugin_path;       //!< Set to the plugin filename path
    const char*             _schema_name;       //!< Name of node schema from the ctor - *NOT* the plugin .so filename!
    Token                   _schema_base_type;  //!< Name of base schema type - 'UsgSchema', 'UsgShader', etc.
    TokenSet                _source_types;      //!< Source-type strings for additional source-specific schemas
    const SchemaDescHandle* _registry_node;     //!< Singleton node reference in registry
    int32_t                 _status;            //!< For internal use


  public:
    //! Constructor method definition used for 'build()' methods in plugins.
    typedef Schema* (*PluginBuilder)(void);
    PluginBuilder builder_method; // <<< Call this to construct a Schema object.


  public:
    //!
    SchemaDescription(const char*   schema_name,
                      PluginBuilder builder);

    /*! Prefix and suffix strings prepended and appended to schemaName() string to
        construct plugin filenames (vs. the loaded SchemaDescription name)
    */
    static const char*  pluginPrefix() { return "shd"; }
    static const char*  pluginSuffix() { return ""; }

    //! Name of the shader class without any leading pluginPrefix() or trailing pluginSuffix().
    const char*      schemaName()     const { return _schema_name; }

    //! Name of base schema type - 'UsgSchema', 'UsgShader', etc.
    const Token&     schemaBaseType() const { return _schema_base_type; }

    //! Set of type names this description will produce. Assigned by querying getSchemaTypes() on Schema instance.
    const TokenSet&  sourceTypes()    const { return _source_types; }


    //! Pointer to singleton shader node stored in registry, searching for it if this is the first time.
    const SchemaDescHandle*  registryNode() const;
    //! Unique identifier used for registry retrieval
    const std::string&       registryIdentifier() const;


    /*! Find a dso description by Schema name.

        If a SchemaDescription already exists it's immediately returned otherwise pluginPrefix()
        and pluginSuffix() are added to the schemaName() string to search for the
        plugin .so filename.

        Returns nullptr if not found.
    */
    static const SchemaDescription*  find(const char* schema_name);


    /*! Find a dso description by shader registry unique identifier string.
        If it's not already in the registry null is returned and a plugin search is
        NOT performed.
    */
    static const SchemaDescription*  getRegistryIdentifier(const std::string& identifier);

  };


  //-----------------------------------------------------------------------------


  /*! Schema class creates SchemaDesc definitions stored in the registry.

  */
  class USG_API Schema
  {
  public:
    /*! The shader class name is taken from the SchemaDescription::schemaName(),
        so the subclass must pass its SchemaDescription.
    */
    Schema(const SchemaDescription&  pluginDescription,
           const SchemaPropertyList& properties,
           const fdk::KeyValueMap&   metadata = fdk::KeyValueMap());

    virtual ~Schema();

    //! Name of the shader class without any leading pluginPrefix() or trailing pluginSuffix().
    const char* schemaName() const;

    /*! Return the base schema type string used to determine the handling of property types
        and where to support source types.
        Only a few are currently supported:
          'UsgSchema' schema describes a set of properties for a generic Prim, the default.
          'UsgShader' schema describes a ShaderSchema, which has specific property type restrictions
    */
    virtual const Token&  baseTypeName() const { return GeomTokens.usgSchema; }


    /*! Get the set of additional source types to create for this Schema. Currently only used for
        ShaderSchema subclasses. An example of an additional source type would be 'glslfx'.
        Default implementation does nothing.
    */
    virtual void  getSourceTypes(TokenSet& types) const;

    //!
    virtual ShaderSchema*  asShaderSchema() { return nullptr; }


    //! Find a SchemaDescription by schema name and optional source-type - *NOT* the plugin .so filename!
    static const SchemaDescription* find(const char* schema_name) { return SchemaDescription::find(schema_name); }



  protected:
    friend class SchemaBuiltinParser;
    std::unique_ptr<ShaderSchemaHandle, void(*)(ShaderSchemaHandle*)> _pImpl;

    /*! Create a Schema singleton instance based on a shader class name or an existing shader
        plugin description. This should only be called during SchemaDescription ctor.
    */
    static Schema*  create(const char* schema_name);


  private:
    friend SchemaDescription;

    //! Disabled copy constructor.
    Schema(const Schema&) = delete;
    //! Disabled assignment operator.
    Schema& operator = (const Schema&) = delete;
  };

  USG_API extern const char* (*ddimage_plugin_find)(const char* name, const char* const* extensions);
  USG_API extern const char* (*ddimage_plugin_load)(const char* name, const char* altname);
  USG_API extern const char* (*ddimage_plugin_error)();
  USG_API extern const char* (*ddimage_plugin_name)();

} // namespace usg

#endif
