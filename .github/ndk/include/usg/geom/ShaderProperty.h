// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_SHADERPROPERTY_H
#define USG_GEOM_SHADERPROPERTY_H

#include "usg/geom/SchemaProperty.h"

#include <memory>

namespace usg
{
  using ShaderPropertyHandle = SchemaPropertyHandle;


  /*! Specialization of SchemaProperty intended for shader nodes.
  */
  class USG_API ShaderProperty : public SchemaProperty
  {
  public:
    //! Default ctor builds an empty (invalid) ShaderProperty.
    ShaderProperty() : SchemaProperty() {}
    //! Simple ctor for input properties and no extras - populates the local property data.
    ShaderProperty(int          inputOrOutput,
                   Value::Type  type,
                   const char*  name,
                   const Value& value,
                   size_t       arraySize = 0);

    //! Simple ctor for typical output property with a default value and metadata - populates the local property data.
    ShaderProperty(int                     inputOrOutput,
                   Value::Type             type,
                   const char*             name,
                   const Value&            value,
                   const fdk::KeyValueMap& metadata);

    //! Simple ctor for typical output property with metadata and no default value - populates the local property data.
    ShaderProperty(int                     inputOrOutput,
                   Value::Type             type,
                   const char*             name,
                   const fdk::KeyValueMap& metadata = fdk::KeyValueMap());

    //! Ctor with all params exposed - populates the local property data.
    ShaderProperty(int                     inputOrOutput,
                   Value::Type             type,
                   const char*             name,
                   const Value&            value,
                   size_t                  arraySize,
                   const fdk::KeyValueMap& metadata,
                   const fdk::KeyValueMap& hints,
                   const fdk::StringList& options);

    //! If const property reference is valid any local property data is NOT copied.
    ShaderProperty(const ShaderProperty&);
    ShaderProperty& operator = (const ShaderProperty&);


  public:
    //----------------------------------------------------------
    // Convenience methods for commonly-used metadata tags
    //----------------------------------------------------------

    /*! Gets a string with basic information about this property. Helpful for
        things like adding this property to a log.
    */
    std::string         getInfoString() const;

    //! Metadata tag 'label' value, if it exists.
    const std::string&  getLabel() const;

    //! Metadata tag 'help' value, if it exists.
    std::string         getHelp() const;

    //! Metadata tag 'page' value, if it exists.
    const std::string&  getPage() const;

    //! Metadata tag 'widget' value, if it exists.
    const std::string&  getWidget() const;

    //------------------------------------------------------

    /*! Any UI "hints" that are associated with this property. "Hints" are
        simple key/value pairs.
    */
    fdk::KeyValueMap  getHints() const;

    /*! If the property has a set of valid values that are pre-determined, this
        will return the valid option names and corresponding string values (if
        the option was specified with a value).
        Commonly used for enumeration types.
    */
    fdk::StringList  getOptions() const;


  public:
    //! Assigns _pImpl and takes ownership of \a handle. For internal use only!
    explicit ShaderProperty(ShaderPropertyHandle* handle);

  };


} // namespace usg

#endif
