// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_SCHEMAPROPERTY_H
#define USG_GEOM_SCHEMAPROPERTY_H

#include "usg/geom/Attribute.h"

#include "fdk/base/StringUtils.h"

#include <memory>

namespace usg
{

  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------


  /*! Describes a single connection to an upstream shader node and output property.
  */
  struct ShaderConnection
  {
    Path         upstreamNodePath;      //!< Full path of upstream node
    std::string  upstreamOutputName;    //!< Output port name of upstream node to connect to

    ShaderConnection() {}
    ShaderConnection(const char* node_path,
                     const char* node_output_name) : upstreamNodePath(node_path), upstreamOutputName(node_output_name) {}
    ShaderConnection(const Path&        node_path,
                     const std::string& node_output_name) : upstreamNodePath(node_path), upstreamOutputName(node_output_name) {}


    bool operator==(const ShaderConnection& b) const
    {
      return (upstreamNodePath   == b.upstreamNodePath &&
              upstreamOutputName == b.upstreamOutputName);
    }

  };

  using ShaderConnectionList = std::vector<ShaderConnection>;
  using ShaderConnectionMap  = std::unordered_map<std::string, ShaderConnectionList*>;


  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------

  class ShaderProperty;
  class SchemaPropertyHandle;


  /*! Property specialization categories which modify how the property is created
      when added to a prim, usually by adding metadata tags.
  */
  enum {
    InputProp = 0,  //!< Connectable input property (connectability = full, namespace 'inputs:')
    LocalInput,     //!< Non-connectable input property for local use (connectability = interfaceOnly, namespace 'inputs:')
    LocalProp,      //!< Local property with no namespace
    OutputProp      //!< Output port property (namespace 'outputs:')
  };


  /*! Attribute prototype that is used on the Schema prim prototypes.

      This is the generic front end to a ShaderProperty without any shader-specific
      specializations, however under the hood the SchemaPropertyHandle class is
      really a 'ShaderPropertyHandle' class. The only difference is supporting the
      additional notion of the 'LocalProp' category that's explicitly not an
      unconnected input and does not get the 'inputs:' namespace prefixed to the
      property's name. ie a local property is normally just an InputProp with no
      connection and an assigned value, but we want to avoid 'inputs:' namespace
      so we're clearly delineating LocalProp as a distinct property category.

      This allows us to extend the ShaderProperty plugin interface for Prim
      definitions using the same mechanism that's being used for shaders,
      specifically for supporting light and camera materials that have both a
      Prim and material/shader component.

      TODO: allow overrides to be applied directly to the property rather than
      handling it as part of a ShaderDesc. If we do that we need to change the
      precedence rules in ShaderPropertyHandle.

  */
  class USG_API SchemaProperty
  {
  public:
    //! Default ctor builds an empty (invalid) SchemaProperty.
    SchemaProperty();

    /*! Simple ctor for input properties and no extras.
        Populates the localized (overriden) property copy.
        If 'arraySize' is not specified (0) and 'value' is an array type then
        'value' is queried for the array size and 'arraySize' ignored.
    */
    SchemaProperty(int          category,
                   Value::Type  type,
                   const char*  name,
                   const Value& value,
                   size_t       arraySize = 0);

    /*! Simple ctor for input properties with metadata.
        Populates the localized (overriden) property copy.
    */
    SchemaProperty(int                     category,
                   Value::Type             type,
                   const char*             name,
                   const Value&            value,
                   const fdk::KeyValueMap& metadata);

    /*! Simple ctor for typical property with metadata and no default value.
        Populates the localized (overriden) property copy.
    */
    SchemaProperty(int                     category,
                   Value::Type             type,
                   const char*             name,
                   const fdk::KeyValueMap& metadata = fdk::KeyValueMap());

    /*! Ctor with all params exposed.
        Populates the localized (overriden) property copy.
        If 'arraySize' is not specified (0) and 'value' is an array type then
        'value' is queried for the array size and 'arraySize' ignored.
    */
    SchemaProperty(int                     category,
                   Value::Type             type,
                   const char*             name,
                   const Value&            value,
                   size_t                  arraySize,
                   const fdk::KeyValueMap& metadata,
                   const fdk::KeyValueMap& hints,
                   const fdk::StringList&  options);

    //! If const property reference is valid any localized property data is NOT copied.
    SchemaProperty(const SchemaProperty&);
    SchemaProperty& operator = (const SchemaProperty&);

    //! Dtor in implementation required for std::unique_ptr to work
    virtual ~SchemaProperty();


    //------------------------------------------------------

    //! True if SchemaPropertyHandle has either a valid reference or contains localized property data.
    bool                         isValid() const;
    operator                     bool()    const { return this->isValid(); }

    //------------------------------------------------------


    //! Property's identifer name.
    const std::string&   getName() const;

    //! Property's identifer name with any additional namespace modifiers.
    virtual std::string  getFullName() const;


    /*! Return the type name string assigned to the property.
        This *should* match the list of recognized Value::Types but it's possible
        the type name may be something unrecognized, and the Value::Type will
        be Invalid.
    */
    const std::string&   getType() const;

    /*! Return the property Value::Type enumeration, only valid if type name is
        a recognized type.
    */
    Value::Type          getValueType() const;

    /*! Return the converted type from GetType() into a SdfValueTypeName as a
        property Value::Type enumeration. Only valid if type name is a
        recognized type.
    */
    Value::Type          getSdfValueType() const;


    //------------------------------------------------------


    //! True if SchemaPropertyHandle is bound to const property reference.
    bool                isReference() const;

    //! True if SchemaPropertyHandle contains localized property data.
    bool                isOverriden() const;

    //! Is this property an output?
    bool                isOutput() const;

    //! Is property an array type?
    bool                isArray() const;

    //! Is this property's array type dynamically-sized?
    bool                isDynamicArray() const;

    /*! Gets this property's array size.
        
        If this property is a fixed-size array type, the array size is returned.
        In the case of a dynamically-sized array, this method returns the array
        size that the parser reports, and should not be relied upon to be
        accurate. A parser may report -1 for the array size, for example, to
        indicate a dynamically-sized array. For types that are not a fixed-size
        array or dynamic array, this returns 0.
    */
    int                 getArraySize() const;


    /*! Returns the value from the reference or localized (overriden) assignment.

        Check isReference() or isOverriden() to determine if the property is
        being used as a reference or a localized override.
    */
    const Value       getValue() const;

    //! Assigns a time to the value stored in the property, for animating overrides.
    void              setValueTime(const fdk::TimeValue& time);

    //! Returns the assigned time the value represents, for animating overrides.
    //! If not set this returns the default time.
    fdk::TimeValue    getValueTime() const;

    //!
    fdk::KeyValueMap  getMetadata() const;


  public:
    //------------------------------------------------------
    // Property interconnection between schema nodes
    //------------------------------------------------------

    /*! Whether this property can be connected to other properties.
    */
    bool         isConnectable() const;

    /*! Return true if this property can be connected an output (isOutput()=true)
        property, and the data types are compatible.
    */
    bool         canConnectTo(const SchemaProperty& output) const;


  public:
    //! Print the name, type and default value to stream.
    void print(std::ostream&) const;
    friend std::ostream& operator << (std::ostream& o, const SchemaProperty& b) { b.print(o); return o; }


  public:
    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    //! Assigns _pImpl and takes ownership of \a handle. For internal use only!
    explicit SchemaProperty(SchemaPropertyHandle* handle);
    SchemaPropertyHandle*        impl()       { return _pImpl.get(); }
    const SchemaPropertyHandle*  impl() const { return _pImpl.get(); }

  protected:
    std::unique_ptr<SchemaPropertyHandle, void(*)(SchemaPropertyHandle*)> _pImpl;

  };

  using ShaderPropertyList = std::vector<ShaderProperty>;
  using ShaderPropertyMap  = std::unordered_map<std::string, ShaderProperty*>;
  using SchemaPropertyList = ShaderPropertyList;
  using SchemaPropertyMap  = ShaderPropertyMap;



} // namespace usg

#endif
