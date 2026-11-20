// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_RELATIONSHIP_H
#define USG_GEOM_RELATIONSHIP_H

#include "usg/geom/Attribute.h"
#include "usg/geom/ListPosition.h"

#include <memory>

namespace usg
{

  /*! Relationships are always attached to Prims so use the Prim interface to create
      or get access to them.

      To create a new Relationship use Prim::createIntrinsicRelationship() or
      Prim::createCustomRelationship().
      To access an existing Relationship use Prim::getRelationship().

      TODO: We're only supporting a single relationship target currently!
            Need multiple targets for PointInstancer, maybe other classes
  */
  class USG_API Relationship
  {
  public:
    class RelationshipSpecHandle; //!< Abstraction handle for casting to SdfRelationshipSpecHandle
    class UsdRelationshipHandle; //!< Abstraction handle for casting to UsdRelationship


    //! Default ctor builds an empty (invalid) relationship.
    Relationship();
    //! Copy constructors
    Relationship(const Relationship&);
    Relationship& operator = (const Relationship&);


    //------------------------------------------------------

    //! True if Relationship holds UsdRelationship, not SdfRelationshipSpec, and is authored.
    bool              isAuthored() const;

    //! True if RelationshipHandle is valid (has a valid object binding.)
    bool              isValid() const;
    operator          bool()    const { return this->isValid(); }

    //! If the Relationship is bound to an SdfRelationshipSpecHandle return a pointer to
    //! the underlying SdfRelationshipSpecHandle object if the usd_version passed in
    //! matches the Usd version this library was built with, otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to an SdfRelationshipSpecHandle* and
    //! possibly double-check its validity since a Relationship can be created
    //! without a valid binding. However, if Relationship::isValid() returns
    //! true then the underlying binding will also be valid.
    Relationship::RelationshipSpecHandle*        getSdfRelationshipSpec(int usd_version);
    const Relationship::RelationshipSpecHandle*  getSdfRelationshipSpec(int usd_version) const;

    //! If the Relationship is bound to a UsdRelationship return a pointer to the
    //! underlying UsdRelationship object if the usd_version passed in matches
    //! the Usd version this library was built with, otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to an UsdRelationship* and
    //! possibly double-check its validity since a Relationship can be created
    //! without a valid binding. However, if Relationship::isValid() returns
    //! true then the underlying binding will also be valid.
    Relationship::UsdRelationshipHandle*        getUsdRelationship(int usd_version);
    const Relationship::UsdRelationshipHandle*  getUsdRelationship(int usd_version) const;

    //------------------------------------------------------

    //!
    const std::string&  getName() const;

    //!
    Token               getNameToken() const;

    //! Returns the base name of the relationship
    const std::string&  getBaseName() const;

    //! Returns the base name of the relationship as a token
    Token               getBaseNameToken() const;

    //! Full path including the Prim parent.
    Path                getPath() const;

    //! Returns a hash of the whole relationship.
    fdk::Hash           getHash() const;

    //! Returns the display name of the relationship.
    std::string getDisplayName() const;

    //! Sets if Relationship is custom or not
    void setCustom(bool isCustom);

    //! Gets if Relationship is custom or not
    bool getCustom() const;

    //! Gets a list of field names as tokens defined on the Relationship.
    usg::TokenArray getFieldNames() const;

    //! Gets a list of authored field names.
    usg::TokenArray getAuthoredFieldNames() const;

    //! Returns the content of the non-empty named field, otherwise an invalid
    //! Value.
    Value getField(const Token &fieldName) const;

    //! Sets the contents of a field, creating it if required.
    bool setField(const Token &fieldName, const Value &value);

    //! Sets a value in the custom data for this relationship.
    void setCustomData(const std::string& key, const Value& value);

    //! Returns the value for \a key in the custom data for this relationship.
    bool getCustomData(const std::string& key, Value& value) const;

    //! Returns the relationship's custom data.
    ValueDictionaryRef getCustomData() const;

  public:
    //------------------------------------------------------
    // Target path management:
    //------------------------------------------------------

    //! Sets the relationship's target path to a single path.
    void      setTarget(const Path& targetPath);

    //! Retrieve the first target path.
    Path      getTarget() const;

    //! Sets the relationship's target paths.
    void      setTargets(const PathArray& targetPaths);

    //! Retrieves the relationship's target paths.
    PathArray getTargets() const;

    //! Adds a target path to the relationship at the specified position.
    void      addTarget(const Path& targetPath, const ListPosition position = ListPosition::PrependBack);

    //! Removes a target path from the relationship.
    void      removeTarget(const Path& targetPath);


    //------------------------------------------------------

    //! Calls through to << operator on the contained data
    friend USG_API std::ostream& operator << (std::ostream& o, const Relationship& b);


  public:
    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    class Impl; //!< Wrapper abstraction for implementation type
    Relationship(const Relationship::Impl&);
    Relationship& operator = (const Relationship::Impl&);
    ~Relationship(); // required for unique_ptr

    //! Takes ownership of 'b' - returns true if resulting Relationship is valid.
    bool  setFromImpl(Relationship::Impl&& b);

    //!
    Relationship::Impl*       impl()       { return _pImpl.get(); }
    const Relationship::Impl* impl() const { return _pImpl.get(); }

  protected:
    std::unique_ptr<Relationship::Impl, void(*)(Relationship::Impl*)> _pImpl;
  };


  //--------------------------------------------------------------------------------------------------


  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/

  USG_API  std::ostream& operator << (std::ostream& o, const Relationship&);



} // namespace usg

#endif
