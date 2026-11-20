// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_ATTRIBUTE_H
#define USG_GEOM_ATTRIBUTE_H

#include "usg/base/Value.h"
#include "usg/base/Token.h"
#include "usg/geom/ValueDictionary.h"
#include "usg/geom/Path.h"

#include "fdk/base/Hash.h"
#include "fdk/base/Time.h"
#include "fdk/base/StringTypes.h"

#include <memory>
#include <optional>

namespace usg
{

  //-----------------------------------------------------------------------


  //! Time-variability of attribute values
  enum class TimeVariability {
    Varying,          //!< Value of property changes between time samples - interpolatable depending on data type
    Uniform,          //!< Value is constant over time. No value or count changes between time samples (no time samples required)
    NumVariabilities  //!<
  };


  //-----------------------------------------------------------------------


  //! Geometry attributes (or properties) contain arbitrary values and are
  //! owned by Prims which are stored on either a Layer or Stage depending on the
  //! define/edit phase of the geometry system.
  //!
  //! Attributes are always attached to Prims so use the Prim interface to create or
  //! gain access to them.
  //!
  //! To create Attributes use Prim::createIntrinsicAttr() or Prim::createCustomAttr().
  //! To access an existing Attribute use Prim::getAttr().
  class USG_API Attribute
  {
  public:
    class Impl; //!< Wrapper abstraction for implementation type
    class AttrSpecHandle; //!< Abstraction handle for casting to SdfAttributeSpecHandle
    class UsdAttrHandle; //!< Abstraction handle for casting to UsdAttribute

    //! Default ctor builds an empty (invalid) attribute.
    Attribute();
    //! Copy constructors
    Attribute(const Attribute&);
    Attribute& operator = (const Attribute&);


    //------------------------------------------------------

    //! True if Attribute::Impl is valid (has a valid object binding.)
    bool      isValid() const;
    operator  bool()    const { return this->isValid(); }

    //! Is this Attribute bound to an SdfAttributeSpec?
    bool  isSdfAttributeSpec() const;

    //! Is this Attribute bound to a UsdAttribute?
    bool  isUsdAttribute() const;

    //! If the Attribute is bound to an SdfAttributeSpecHandle return a
    //! pointer to the underlying SdfAttributeSpecHandle object, assuming
    //! the usd_version passed in matches the Usd version this library was
    //! built with, otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to a SdfAttributeSpecHandle*
    //! and possibly double-check its validity since a SdfAttributeSpecHandle
    //! can be created without a valid binding. However, if Attribute::isValid()
    //! returns true then the underlying binding will also be valid.
    Attribute::AttrSpecHandle*        getSdfAttributeSpecHandle(int usd_version);
    const Attribute::AttrSpecHandle*  getSdfAttributeSpecHandle(int usd_version) const;

    //! If the Attribute is bound to a UsdAttribute return a pointer to the
    //! underlying UsdAttribute object, assuming the usd_version passed
    //! in matches the Usd version this library was built with, otherwise
    //! return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to a UsdAttribute* and
    //! possibly double-check its validity since a UsdAttribute can be
    //! created without a valid binding. However, if Attribute::isValid()
    //! returns true then the underlying binding will also be valid.
    Attribute::UsdAttrHandle*        getUsdAttribute(int usd_version);
    const Attribute::UsdAttrHandle*  getUsdAttribute(int usd_version) const;


    //------------------------------------------------------

    //! Returns the name of the attribute
    const std::string&  getName() const;

    //! Returns the name of the attribute as a token
    Token               getNameToken() const;

    //! Returns the base name of the attribute
    const std::string&  getBaseName() const;

    //! Returns the base name of the attribute as a token
    Token               getBaseNameToken() const;

    //! Returns the full path including the Prim parent.
    Path                getPath() const;

    //! Returns the attribute's Value type
    Value::Type         getType() const;

    //! Returns the attribute's type name.
    const std::string&  getTypeName() const;

    //! Returns a hash of the whole attribute.
    fdk::Hash           getHash() const;

    //! Returns the number of data elements for each value. For example, 3 for a Vector3 or 16 for a Matrix4.
    size_t              getNumDataElements() const;

    //! Returns the number of bytes for a data element. For example, 4 for a float or 8 for a double.
    size_t              getBytesPerDataElement() const;

    //! Returns the variability of this attribute.
    TimeVariability     getVariability() const;

    //! Returns the display name of the attribute
    std::string getDisplayName() const;

    //! Sets the display name of the attribute
    bool setDisplayName(const std::string& displayName);

    //! Returns the display group of the attribute
    std::string getDisplayGroup() const;

    //! Sets the display group of the attribute
    bool setDisplayGroup(const std::string& displayGroup);

    //! Returns the displayGroup as a sequence of groups to be nested, or an empty vector if
    //! displayGroup is empty or not authored.
    std::vector<std::string> getNestedDisplayGroups() const;

    //! Returns the documentation for the attribute
    std::string getDocumentation() const;

    //------------------------------------------------------

    //! Returns true if the attribute is defined
    bool  isDefined()  const;

    //! Returns true if the attribute has any authored values, either a default or time samples,
    //! or if there's a fallback value provided by a schema. Returns false if the attribute has
    //! been blocked and there is no fallback value.
    bool  hasValue() const;

    //! Returns true if the attribute has any authored values in any layer.
    bool  isAuthored() const;

    //! Returns true if the attribute has any authored values, either a default or time samples.
    //! Returns false if the attribute has been blocked.
    bool  hasAuthoredValue() const;

    //! Returns true if the attribute has a fallback value provided by a schema.
    bool  hasFallbackValue() const;

    /// Returns the fallback value for the attribute.
    Value getFallbackValue() const;

    //! Is the attribute currently blocked? Same as calling !hasAuthoredValue().
    bool  isBlocked() const;


    //------------------------------------------------------

    //! Returns true if this Attribute has a set of allowed Tokens.
    bool        hasAllowedTokens() const;

    //! Returns an array of the allowed Tokens for this Attribute.
    TokenArray  getAllowedTokens() const;

    //------------------------------------------------------

    //! Sets a value on the attribute, returning true on success.
    template <typename T>
    bool    setValue(const T&              src,
                     const fdk::TimeValue& time = fdk::defaultTimeValue());

    //! Sets a value on the attribute, returning true on success.
    bool    setValue(const Value&          srcValue,
                     const fdk::TimeValue& time = fdk::defaultTimeValue());


    //! Retrieves the value from the attribute, returning 0 on fail, 1 for single-value type,
    //! or size of array for an array type.
    template <typename T>
    size_t  getValue(T&                    dst,
                     const fdk::TimeValue& time = fdk::earliestTimeValue()) const;

    size_t  getValue(Value&                dstValue,
                     const fdk::TimeValue& time = fdk::earliestTimeValue()) const;

    //! Returns the number of time samples set on the attribute.
    size_t  getNumTimeSamples() const;

    //! Fills the array with all time samples set on the attribute, in ascending order. Returns the size of the array.
    size_t  getTimeSamples(fdk::TimeValueList& times) const;

    //! Returns an array with all time samples set on the attribute, in ascending order.
    fdk::TimeValueList  getTimeSamples() const;

    //! Copies the time sample from the other attribute at the other time into this attribute at given time.
    bool    copyTimeSample(const Attribute& other, fdk::TimeValue otherTime, fdk::TimeValue time);

    //! Returns true if the attribute has authored time samples.
    bool    isAnimating() const;


    //------------------------------------------------------

    //! Removes all authored values, including the default one.
    void clear();

    //! Removes the default value, leaving time samples unaffected.
    void clearDefault();

    //! Removes all time samples from the attribute, leaving the default value unaffected.
    void clearAllTimeSamples();

    //! Removes a specific time sample, ignoring default value.
    void clearAtTime(const fdk::TimeValue& time);

    //! Removes all authored values, leaving the attribute as if unauthored.
    void block();


    //------------------------------------------------------

    //! Returns the number of values in the attribute, possibly at a specific time.
    //! 0 is returned if Attribute not valid or assigned.
    //! 1 is returned if Attribute is not an array type, otherwise array size is returned.
    size_t  getSize(const fdk::TimeValue& time = fdk::earliestTimeValue()) const;

    //! If the Attribute is an array type returns its length(size), otherwise 0, possibly at a specific time.
    size_t  getArraySize(const fdk::TimeValue& time = fdk::earliestTimeValue()) const;

    //----------------------------------------------------------
    // Fields (or 'metadata' in the UsdAttribute context)
    // are named arbitrary Values stored on the Attribute.
    //----------------------------------------------------------

    //! Gets a list of field names as strings defined on the Attribute.
    [[deprecated("Please use getFieldNames returning usg::TokenArray")]]
    void   getFieldNames(fdk::StringList& field_names) const;

    //! Gets a list of field names as tokens defined on the Attribute.
    usg::TokenArray getFieldNames() const;

    //! Gets a list of authored field names.
    usg::TokenArray getAuthoredFieldNames() const;

    //! Returns true if Attribute has the named field and it has a value.
    bool   hasField(const Token& field_name) const;

    //! Returns the content of the non-empty named field, otherwise an invalid Value.
    Value  getField(const Token& field_name) const;

    //! Sets the contents of a field, creating it if required.
    bool   setField(const Token& field_name,
                    const Value&       value);

    //! Removes the named field from the Attribute.
    bool   deleteField(const Token& field_name);

    //! Sets the interpolation for this attribute
    void   setInterpolation(const Token& interpolation);

    //! Gets the interpolation for this attribute
    Token  getInterpolation() const;

    //----------------------------------------------------------

    //! Returns the attribute's composed limits dictionary.
    ValueDictionaryRef getLimits() const;

    //! Returns true if a limits dictionary is authored for this attribute.
    bool  hasAuthoredLimits() const;

    //----------------------------------------------------------

    //! Sets a value in the custom data for this attribute
    void   setCustomData(const std::string& key, const Value& value);

    //! Returns the value for \a key in the custom data for this attribute.
    bool   getCustomData(const std::string& key, Value& value) const;

    //! Returns the attribute's custom data.
    ValueDictionaryRef getCustomData() const;

    //! Returns a top-level metadata field that holds a dictionary, or an empty dict if the
    //! field is absent or is not a dictionary type.
    ValueDictionaryRef getFieldAsDict(const Token& field_name) const;

    //! Sets if Attribute is custom or not
    void setCustom(bool isCustom);

    //! Gets if Attribute is custom or not
    bool getCustom() const;

    //----------------------------------------------------------
    // Make/remove connections from this attribute to another.
    //
    // Multiple connections are possible allowing individual
    // sub elements of the attribute to come from different
    // source attributes.
    //
    // Each connection is a full path to the other attribute.
    //----------------------------------------------------------

    //! Return true if this attribute has any authored connection opinions.
    bool    hasAuthoredConnections() const;

    //! Replaces connections with a new list.
    bool    setConnections(const PathArray& sources);

    //! Clears any existing connections then sets it to this one. Same as clearConnections() followed by addConnection().
    bool    setConnectionTo(const Path& source);

    //! Appends a connection to the current list.
    bool    addConnectionTo(const Path& source);

    //! Prepends a connection to the current list.
    bool    prependConnectionTo(const Path& source);

    //! Removes connection to source, if it exists.
    bool    removeConnectionTo(const Path& source);

    //! Removes all connections.
    void    clearConnections();

    //! Returns the number of assigned connections.
    size_t  numConnections() const;

    //! Returns the connection at index \a i
    Path    getConnection(int i = 0) const;


    //------------------------------------------------------

    //! Calls through to << operator on the contained data
    friend USG_API std::ostream& operator << (std::ostream& o, const Attribute& b);


  public:
    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    Attribute(const Attribute::Impl&);
    Attribute& operator = (const Attribute::Impl&);
    ~Attribute(); // required for unique_ptr

    //! Takes ownership of 'b' - returns true if resulting Attribute is valid.
    bool  setFromImpl(Attribute::Impl&& b);

    //! Returns the wrapped object
    Attribute::Impl*        impl()       { return _pImpl.get(); }
    const Attribute::Impl*  impl() const { return _pImpl.get(); }


  protected:
    std::unique_ptr<Attribute::Impl, void(*)(Attribute::Impl*)> _pImpl;
  };


  //! Calls through to << operator on the contained data
  USG_API  std::ostream& operator << (std::ostream& o, const Attribute&);


  template<typename T>
  std::optional<T> ValueFromAttribute(Attribute const& attr, const fdk::TimeValue& t = fdk::earliestTimeValue())
  {
    T actualValue;
    const auto res = attr.getValue<T>(actualValue, t);
    if (res == 0) {
      return {};
    }
    return actualValue;
  }
} // namespace usg

#endif
