// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_SHADERDESC_H
#define USG_GEOM_SHADERDESC_H

#include "usg/base/ArgSet.h"

#include "usg/geom/Attribute.h"
#include "usg/geom/ShaderProperty.h"
#include "usg/geom/ShaderSchema.h"
#include "usg/geom/GeomTokens.h"

#include "fdk/base/PathUtils.h"
#include "fdk/base/Hash.h"
#include "fdk/base/Time.h"

#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace usg
{
  class Prim;

  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------

  class ShaderDescHandle;


  /*! Describes a single shader as an abstract representation with local property
      overrides and connections.

      It contains a reference to the ShaderSchema default configuration with the
      default values of all properties and a separate list of property overrides.

      A ShaderDesc copy of a ShaderSchema is not directly modifiable, that is you
      cannot change the input or output properties directy. Modify it by assigning
      property overrides and connections.

      This should be as abstract as possible to help translation to
      various shader targets.

  */
  class USG_API ShaderDesc
  {
  public:
    //! Default ctor builds an empty (invalid) ShaderDesc.
    ShaderDesc();
    //! Move constructor
    ShaderDesc(ShaderDesc&&);
    /*! Construct from a ShaderSchema registry description.
        Direct copying ShaderDesc is disabled, use the ShaderDesc::createFromSchema()
        method instead to create additional copies.
    */
    ShaderDesc(const SchemaDescription* schema);

    //! Must have a destructor in implementation otherwise std::unique_ptr fails.
    ~ShaderDesc();

    //------------------------------------------------------

    //! True if ShaderDescHandle is valid (has a valid object binding.)
    bool                     isValid() const;
    operator                 bool()    const { return this->isValid(); }

    //------------------------------------------------------

    //! Returns the name of the underlying ShaderSchema.
    const std::string&  schemaName() const;


    /*! Return the unique identifier of the ShaderSchema which may or may not
        contain the schemaName() string. ie don't use it to extract the
        shader class or type name, use schemaName() instead.

        The ShaderSchema id is used to retrieve the singleton description from the
        registry and MUST BE THE SAME for shader define/declaration code, shader
        editing code (ie changing property values,) and any shader consumers
        like renderers or viewers.
    */
    const std::string&  getIdentifier() const;


    /*! Convenience method prefixes the prim_name with the target renderer name,
        if not empty, separated with an underscore '_'.
    */
    static std::string  buildName(const std::string& target_renderer,
                                  const std::string& prim_name);

    /*! Set the local name of the shader node within the material graph. This is
        the prim name, not a path, so should not include a leading '.' or '/'.
    */
    void                setMaterialRelativeName(const std::string& prim_name);


    /*! Return the local name of the shader node within the material graph.
        This does not include the Material parent or anything above it.

        If the local name mistakenly has a leading '/' it should be interpreted as
        '<material-path>/'. ie the '/' is replaced by the material parent path.
    */
    const std::string&  getMaterialRelativeName() const;


    /*! Returns a newly-allocated ShaderDesc copied from a ShaderSchema registry
        entry matching the schema_name.
        Calling code takes ownership of allocation.

        If there isn't one currently registered the schema_name is
        used to construct a plugin filename and the plugin path searched
        for a match, loading it if found.

        Modify it by assigning property overrides and connections. The ShaderSchema
        copy is not directly modifiable, ie you can't change the input or output
        properties directy.

        For example:
          if (ShaderDesc* shader = ShaderDesc::createFromSchema("MyShader"))
          {
            shader->overrideInput("diffuseColor", Value(Vec3f(0.1f, 0.2f, 0.3f)));
            shader->connectInput("opacity", "./MyOpacityTexture.outputs:r");
          }
    */
    static ShaderDesc*  createFromSchema(const char* schema_name);

    /*! Same as createFromSchema(schema_name) but sets the prim node name after
        creation, for convenience.

        The resulting ShaderDesc's material-relative name is assigned via
        setMaterialRelativeName(), see that method for naming info.
    */
    static ShaderDesc*  createFromSchema(const char*        schema_name,
                                         const std::string& prim_name);

    /*! Returns a ShaderDesc copied from a ShaderSchema registry entry matching
        the unique identifier string.

        This is a wrapped registry ShaderDesc and not editable.
    */
    static ShaderDesc   createFromIdentifier(const std::string& identifier);

    //!
    const std::string&  getSourceType() const;
    //!
    const std::string&  getSourceCode() const;
    //!
    const std::string&  getResolvedImplementationURI() const;
    const std::string&  getResolvedDefinitionURI() const;


    /*! Returns a constructed ShaderSchema from this ShaderDesc. Basically the
        reverse of \a createFromSchema().
        Calling code takes ownership of returned allocation.
    */
    ShaderSchema*       createShaderSchema() const;


  public:
    //-------------------------------------------------------------------------
    // Inputs, Locals and Outputs
    //
    // An input or output is also generically referred to as a 'property'.
    //
    // A local property is a non-connectable input property without the
    // 'inputs:' prefix. Typically set from a Knob or some other parameter
    // that is local to the shader implementation, and often mappable to the
    // same-named property on a Prim that is shared with the shader.
    //-------------------------------------------------------------------------

    //! Retrieve the list of input property names.
    size_t           getInputNames(fdk::StringPtrList& names) const;
    //! Returns true if named input exists.
    bool             hasInput(const std::string& input_name) const;

    /*! Get a wrapped shader input property by name. Returned property is invalid
        if an input with the given name does not exist.
    */
    ShaderProperty   getInput(const std::string& input_name) const;

    //! Return the value of the input property with override (if any) applied.
    Value            getInputValue(const std::string& input_name) const;

    //! Return the default value (non-overriden) of the input property.
    Value            getInputDefaultValue(const std::string& input_name) const;

    //! Retrieve a list of inputs and their default values.
    TokenValueMapSorted  getInputValues() const;


    /*! Set an override value on a named input property, if the property exists.

        If the property exists it will be returned with the override applied
        if the provided value type matches the property. ie if the property
        is a Value::Color and the provided value is a string the override is
        ignored and the returned property is unchanged.
    */
    ShaderProperty*  overrideInput(const std::string&    input_name,
                                   const Value&          override_value,
                                   const fdk::TimeValue& override_time = fdk::defaultTimeValue());

    /*! Set a series of overrides at once. Returns false if any failed.
    */
    bool             overrideInputs(const ShaderPropertyMap& overrides);

    //! Remove a named input override, returning true if successful.
    bool             clearInputOverride(const std::string& input_name);

    /*! Get a shader input override property override by name, otherwise
        null.
    */
    ShaderProperty*  getInputOverride(const std::string& input_name) const;

    //! Returns the list of all input overrides currently assigned.
    const ShaderPropertyMap&  getInputOverrides() const;

    //! Retrieve a list of overriden inputs and their values.
    TokenValueMapSorted  getInputOverrideValues() const;


    /*!
        Disconnect the input by passing a null or invalid source_shader or an empty
        source_shader_output string.
    */
    ShaderConnectionList*   connectInput(const ShaderProperty& input_property,
                                         const ShaderDesc*     source_shader,
                                         const std::string&    source_shader_output_name);
    /*!
        Disconnect the input by passing a null or invalid source_shader or an empty
        source_shader_output string.
    */
    ShaderConnectionList*   connectInput(const std::string& input_name,
                                         const ShaderDesc*  source_shader,
                                         const std::string& source_shader_output_name);
    //
    ShaderConnectionList*   connectInput(const ShaderProperty& input_property,
                                         const std::string&    connection_path);
    ShaderConnectionList*   connectInput(const std::string& input_name,
                                         const std::string& connection_path);

    bool                    connectInputs(const ShaderConnectionMap& connections);


    //! Returns the list of all input connections currently assigned.
    const ShaderConnectionMap&    getInputConnections() const;

    //! Return a list of connections if any have been assigned to the named input, otherwise null.
    const ShaderConnectionList*   getInputConnection(const std::string& input_name) const;


    //------------------------------------------------------


    //! Retrieve the list of local property names.
    size_t           getLocalPropertyNames(fdk::StringPtrList& names) const;
    //! Returns true if named local property exists.
    bool             hasLocalProperty(const std::string& prop_name) const;

    /*! Get a wrapped shader local property by name. Returned property is invalid
        if a local with the given name does not exist.
    */
    ShaderProperty   getLocalProperty(const std::string& prop_name) const;

    /*! Set an override value on a named local property, if the property exists.

        If the property exists it will be returned with the override applied
        if the provided value type matches the property. ie if the property
        is a Value::Color and the provided value is a string the override is
        ignored and the returned property is unchanged.
    */
    ShaderProperty*  overrideLocalProperty(const std::string&    prop_name,
                                           const Value&          override_value,
                                           const fdk::TimeValue& override_time = fdk::defaultTimeValue());


    //------------------------------------------------------


    //! Retrieve the list of output property names.
    size_t           getOutputNames(fdk::StringPtrList& names) const;
    //! Returns true if named output exists.
    bool             hasOutput(const std::string& output_name) const;

    /*! Get a wrapped shader output property by name. Returned property is invalid
        if an output with the given name does not exist.
    */
    ShaderProperty   getOutput(const std::string& output_name) const;


  public:
    //!
    void print(const char*   prefix,
               std::ostream& o,
               bool          print_contents = false) const;
    friend std::ostream& operator << (std::ostream& o, const ShaderDesc& b) { b.print("", o); return o; }


  public:
    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    ShaderDescHandle*        impl()       { return _pImpl.get(); }
    const ShaderDescHandle*  impl() const { return _pImpl.get(); }

  protected:
    std::unique_ptr<ShaderDescHandle, void(*)(ShaderDescHandle*)> _pImpl;

  private:
    // Disallow the copy methods
    ShaderDesc(const ShaderDesc&) = delete;
    ShaderDesc& operator = (const ShaderDesc&) = delete;

  };

  using ShaderDescPtrList      = std::vector<ShaderDesc*>;
  using ShaderDescConstPtrList = std::vector<const ShaderDesc*>;
  using ShaderDescPtrSet       = std::unordered_set<ShaderDesc*>;
  using ShaderDescConstPtrSet  = std::unordered_set<const ShaderDesc*>;
  using ShaderDescPtrMap       = std::unordered_map<std::string, ShaderDesc*>;
  using ShaderDescConstPtrMap  = std::unordered_map<std::string, const ShaderDesc*>;


  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------

 
  /*! A list of ShaderDesc pointers usually organized into a shader graph. The group's output
      shader(s) are also noted as well as the name (if any) of a renderer to target.

      Added ShaderDesc allocations are owned by this class and deleted in the destructor.
  */
  class USG_API ShaderDescGroup
  {
  public:
    std::string      target_renderer;     //!< If not empty (ie universalRenderContext) the shaders are associated with a named renderer
    std::string      material_base_name;  //!< Base name for the material (may not be the final MaterialPrim name)
    Path             material_prim_path;  //!< Prim path of MaterialPrim owner
    //
    ShaderDescPtrMap shaders;             //!< Map of ShaderDesc pointers keyed by shader material-relative names
    ShaderDesc*      output;              //!< The output shader for the group
    ShaderDesc*      displacement_output; //!< The output displacement shader for the group (may be the same as surface output)
    //
    fdk::Hash        topology_hash;       //!< Does the shader graph need to be rebuilt?
    fdk::Hash        property_hash;       //!< Do the shaders need parameter updating?


    ShaderDescGroup(const std::string& _target_renderer = std::string());
    ~ShaderDescGroup();

    size_t  size() const { return shaders.size(); }
    void    reserve(size_t n) { shaders.reserve(n); }


    /*! Add a ShaderDesc to the group, taking ownership of the allocation.

        Important - this requires that the ShaderDesc already have a unique
        name assigned as getMaterialRelativeName() is called on the ShaderDesc
        to map the storage of the pointer. If the name is not assigned or is
        not unique then the ShaderDesc will NOT be added to the group and
        false returned.

        The ShaderDesc pointer will be deleted when the group is destroyed.

        Returns true is ShaderDesc was added to group.
    */
    bool    addShaderDesc(ShaderDesc* shader);

    /*! Add a ShaderDesc to the group, taking ownership of the allocation
        and setting the shader's name at the same time.
        If the name is empty or is not unique then the ShaderDesc will NOT be
        added to the group and false returned.

        The ShaderDesc pointer will be deleted when the group is destroyed.

        Returns true is ShaderDesc was added to group.
    */
    bool    addShaderDesc(ShaderDesc*        shader,
                          const std::string& prim_name);

    //! Assign the primary output shader (sub type doesn't matter - ie surface, volume, light, etc)
    void    setOutput(ShaderDesc* shader) { output = shader; }

    /*! Assign the primary displacement output shader.
    */
    void    setDisplacementOutput(ShaderDesc* shader) { displacement_output = shader; }


    /*! Delete the ShaderDesc pointers and empty the group.
        Leaves the material_prim_path and target_renderer intact.
    */
    void    deleteShaders();


    //! Return the shader matching the name, or null if not in group.
    ShaderDesc*  getShaderNode(const std::string& shader_node_name) const;

    //! Computes and returns the topology hash for the shader group.
    fdk::Hash getTopologyHash();


  private:
    ShaderDescGroup(const ShaderDescGroup&) = delete;
    ShaderDescGroup& operator = (const ShaderDescGroup&) = delete;
  };

  using ShaderDescGroupPtrList = std::vector<ShaderDescGroup*>;
  using ShaderDescGroupPtrMap  = std::unordered_map<std::string, ShaderDescGroup*>;


} // namespace usg

#endif
