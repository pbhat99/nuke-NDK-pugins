// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_PRIM_H
#define USG_GEOM_PRIM_H

#include "usg/base/Token.h"
#include "usg/geom/Attribute.h"
#include "usg/geom/ValueDictionary.h"
#include "usg/geom/Relationship.h"
#include "usg/geom/GeomTokens.h"
#include "usg/geom/ListPosition.h"

#include <fdk/base/Hash.h>

#include <memory>

namespace usg
{
  class Layer;
  class Stage;
  class ShaderDesc;

  using LayerRef = std::shared_ptr<Layer>;
  using LayerConstRef = std::shared_ptr<const Layer>;
  using StageRef = std::shared_ptr<Stage>;
  using StageConstRef = std::shared_ptr<const Stage>;

  //--------------------------------------------------------


  //! A wrapper for TfType. This is useful for speeding up calls to Prim::isA.
  class USG_API Type {
  public:
    Type(const usg::Token& typeName);
    ~Type();

    class Impl; //!< Wrapper abstraction for implementation type
    const Type::Impl* impl() const;
  private:
    std::unique_ptr<Type::Impl, void(*)(Type::Impl*)> _pImpl;
  };


  //--------------------------------------------------------


  //! class Prim
  //! Polymorphic wrapper class for SdfPrimSpec or UsdPrim
  class USG_API Prim
  {
  public:
    class Impl; //!< Wrapper abstraction for implementation type
    class SpecPrimHandle; //!< Abstraction handle for casting to SdfPrimSpec
    class UsdPrimHandle; //!< Abstraction handle for casting to UsdPrim


    //! Constructs an empty (invalid) Prim.
    Prim();
    Prim(const Prim&);
    Prim& operator = (const Prim&);

    virtual ~Prim();

    //! Equality - are the two Prims the same object or not? Compares the underlying Prim::Impls.
    bool operator == (const Prim& b) const;
    bool operator != (const Prim& b) const { return !(*this == b); }

    bool operator < (const Prim& b) const;

    //------------------------------------------------------

    //! Returns true if Prim::Impl is valid (has a valid object binding.)
    bool                isValid() const;
    explicit operator   bool()    const { return this->isValid(); }

    bool  isSdfPrim() const;
    bool  isUsdPrim() const;

    //! Returns true if we're holding a valid type of prim for our class. If not, returns false and invalidates
    //! this prim so that furutre.calls to isValid will return false;
    virtual bool validate();

    //! If the Prim is bound to an SdfPrimSpec return a pointer to the
    //! underlying SdfPrimSpec object if the usd_version passed in matches
    //! the Usd version this library was built with, otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to an SdfPrimSpec* and
    //! possibly double-check its validity since a Prim can be created
    //! without a valid binding. However, if Prim::isValid() returns
    //! true then the underlying binding will also be valid.
    class Prim::SpecPrimHandle*        getSdfPrimSpec(int usd_version);
    const class Prim::SpecPrimHandle*  getSdfPrimSpec(int usd_version) const;

    //! If the Prim is bound to a UsdPrim return a pointer to the
    //! underlying UsdPrim object if the usd_version passed in matches
    //! the Usd version this library was built with, otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to an UsdPrim* and
    //! possibly double-check its validity since a Prim can be created
    //! without a valid binding. However, if Prim::isValid() returns
    //! true then the underlying binding will also be valid.
    class Prim::UsdPrimHandle*        getUsdPrim(int usd_version);
    const class Prim::UsdPrimHandle*  getUsdPrim(int usd_version) const;

    //------------------------------------------------------

    //! Returns the name of the prim
    const std::string&  getName() const;

    //! Returns the name of the prim as a token
    Token               getNameToken() const;

    //! Returns the path of the prim
    Path                getPath() const;

    //! Returns the path of the prim as a string
    std::string         getPathAsString() const;

    //! Returns the name of the type of the prim
    Token               getTypeName() const;

    //! Return this prim's parent prim. Returns an invalid prim if this is a root prim.
    Prim                getParent() const;

    //! Returns a hash of the whole prim including metadata, attributes, and relationships.
    fdk::Hash           getHash() const;

    //! Returns true if the prim is an instance of the given type
    bool   isA(const Token& typeName) const;

    //! Returns true if the prim is an instance of the given type. This is faster than the isA method which takes a Token as it avoids a type name lookup.
    bool   isA(const Type& type) const;

    //! Returns true if the prim is an instance of a schema
    template<class T> bool isA() const { return T::isValidPrim(*this); }

    //! Returns true if the prim has the given API
    bool   hasApi(const Token& typeName) const;

    //! Returns true if the prim has the given multiple-apply schema API
    bool   hasApi(const Token& typeName, const Token& instanceName) const;

    //------------------------------------------------------

    void   setActive(bool active);
    bool   isActive() const;
    bool   clearActive();

    bool   clearTypeName();

    bool   isDefined() const;
    bool   isModel() const;
    bool   isGroup() const;
    bool   isAbstract() const;
    bool   hasDefiningSpecifier() const;
    bool   hasOverSpecifier() const;
    bool   hasClassSpecifier() const;
    bool   hasArcs() const;
    void   getLayerIntroducingArc(LayerRef& layer) const;
    void   getTargetPrimPathOfArc(Path& path) const;
    bool   isAncestral() const;

    //! Returns true if the prim is the pseudo root - ie '/'.
    bool   isPseudoRoot() const;

    void   setKind(const Token& kind);
    Token  getKind() const;

    void   setInstanceable(bool instanceable);
    bool   getInstanceable() const;
    bool   isInstanceable() const;
    bool   isInstance() const;
    bool   isInstanceProxy() const;

    bool   isPrototype() const;
    bool   isInPrototype() const;

    bool   hasPayload() const;
    void   clearPayload() const;
    bool   isLoaded() const;
    void   load(bool loadWithDescendants = true) const;
    void   unload() const;

    //! If \a active is false the prim's visibility attribute is set to 'invisible', otherwise
    //! it is set to 'inherited' which means the prim's actual visibility is determined by
    //! its parent's visibility state.
    void   setVisibility(bool active, fdk::TimeValue time = fdk::defaultTimeValue());

    //! Clear the prim's visibility attribute.
    void   clearVisibility();

    //! Returns the state of the prim's local visibility attribute, if authored. If authored
    //! and the value is 'invisible' then false is returned, otherwise true is returned if
    //! value is 'inherited' or was not authored (default is 'inherited'.)
    //! Warning, a returned value of true does not mean this prim must be visible!
    //! You must walk up the graph checking for any parents explicitly set 'invisible',
    //! and if no more parents left then this prim is -by default- visible.
    bool   visibility(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    //! Calculate the effective visibility of this prim, as defined by its most ancestral authored
    //! "invisible" opinion, if any.
    Token   computeVisibility(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    // deprecated, use setVisibility() instead
    void   setVisible(bool active, fdk::TimeValue time = fdk::defaultTimeValue()) { setVisibility(active, time); }
    // deprecated, use visibility() instead
    bool   isVisible(fdk::TimeValue time = fdk::defaultTimeValue()) const { return visibility(time); }

    //------------------------------------------------------

    bool   addAppliedSchema(const Token& schemaName, ListPosition position = ListPosition::PrependBack);
    bool   removeAppliedSchema(const Token& schemaName);
    TokenArray getAppliedSchemas() const;

    //------------------------------------------------------

    //! Sets the name of the app that created this prim.
    void   setCreator(const Token& creator);

    //! Gets the name of the app that created this prim.
    Token  creatorApp() const;

    //! True if creator is the prim's creator app.
    bool   isCreator(const Token& creator) const;

    //------------------------------------------------------

    //! Sets the metadata (or field for sdfPrim) value of given key.
    void   setMetadata(const std::string& key, const Value& value);

    //! Sets the dictionary metadata (or field for sdfPrim) value of given key and keyPath
    bool   setMetadataByDictKey(const Token& key, const Token& keyPath, const Value& value);

    //! Gets the dictionary metadata (or field for sdfPrim) value of given key and keyPath.
    bool   getMetadataByDictKey(const Token& key, const Token& keyPath, Value& value) const;

    //! Clears the dictionary metadata (or field for sdfPrim) of given key and keyPath.
    bool   clearMetadataByDictKey(const Token& key, const Token& keyPath);

    //! Sets the custom data value of given key.
    void   setCustomData(const std::string& key, const Value& value);

    //! Retrieves the custom data value for the key. Returns true if it exists.
    bool   getCustomData(const std::string& key, Value& value) const;

    //! Returns a valid value if the custom data exists for the key.
    Value  getCustomData(const std::string& key) const;

    //! Returns the prim's custom data.
    ValueDictionaryRef getCustomData() const;

    //! Checks whether the prim has custom data for the key.
    bool   hasCustomData(const std::string& key) const;

    //! Erases the custom data for the key.
    void   removeCustomData(const std::string& key);

    //! Clear the metadata for the key.
    bool   clearMetadata(const Token& key);

    //------------------------------------------------------

    //! Return this prim's metadata given a key token.
    bool   getMetadata(const Token& key, Value& value) const;

    //! Returns the metadata of type VtDictionary given a key token.
    bool getDictMetadata(const Token& key, ValueDictionaryRef& dict) const;

    //! Returns the list field names of the prim's metadata.
    TokenArray getMetadataFieldNames() const;

    //! Return this prim's metadata fallback value given a key.
    Value getMetadataFallbackValue(const std::string& key) const;

    //! Returns the list of authored metadata. This method does not return field keys for
    //! composition arcs.
    std::vector<Token> getAuthoredMetadata() const;

    //------------------------------------------------------

    enum class PrimSpecifier
    {
      Def,
      Over,
      Class,
    };

    enum {
      ActiveFlag = 0,
      LoadedFlag,
      ModelFlag,
      GroupFlag,
      AbstractFlag,
      DefinedFlag,
      HasDefiningSpecifierFlag,
      InstanceFlag,
      //
      PayloadFlag,
      ClipsFlag,
      DeadFlag,
      PrototypeFlag,
      InstanceProxyFlag,
      PseudoRootFlag,
      //
      NumFlags
    };
    using FlagMask = uint32_t;
    static constexpr FlagMask IsActiveMask             = 0x0001;
    static constexpr FlagMask IsLoadedMask             = 0x0002;
    static constexpr FlagMask IsModelMask              = 0x0004;
    static constexpr FlagMask IsGroupMask              = 0x0008;
    static constexpr FlagMask IsAbstractMask           = 0x0010;
    static constexpr FlagMask IsDefinedMask            = 0x0020;
    static constexpr FlagMask HasDefiningSpecifierMask = 0x0040;
    static constexpr FlagMask IsInstanceMask           = 0x0080;
    static constexpr FlagMask PayloadMask              = 0x0100;
    static constexpr FlagMask ClipsMask                = 0x0200;
    static constexpr FlagMask DeadMask                 = 0x0400;
    static constexpr FlagMask PrototypeMask            = 0x0800;
    static constexpr FlagMask InstanceProxyMask        = 0x1000;
    static constexpr FlagMask PseudoRootMask           = 0x2000;

    //! class FlagsSet
    //! Set of Prim flag bits used to control prim filtering and masking during traversals, retrievals, etc.
    //! If a flag bit is enabled then the corresponding value bit is used during Prim state testing where a
    //! true value bit means the flag's corresponding Prim flag state is to be tested for inclusion, and if
    //! false the Prim flag state is tested for exclusion.
    class USG_API FlagsSet
    {
    public:
      FlagsSet() : _flags(0x00), _values(0x00) {}
      FlagsSet(int32_t flag, bool value = true) : _flags(1<<flag), _values(_flags & (value ? 0xffff : 0x0000)) {}
      FlagsSet(FlagMask flags, FlagMask values) : _flags(flags), _values(values) {}

      explicit operator FlagMask() const { return (_flags & _values); }
      FlagMask flags() const { return _flags; }
      FlagMask values() const { return _values; }

      //! Is the flag bit enabled?
      bool flagEnabled(int32_t flag) const { return (((_flags & _values) & (1<<flag)) != 0); }
      //! Value for a flag bit. Value of true mean test for inclusion, otherwise exclusion.
      bool flagValue(int32_t flag) const { return (((_flags & _values) & (1<<flag)) != 0); }

      //!
      FlagsSet operator&&(const FlagsSet& b) const { return FlagsSet(_flags | b._flags, (_values & ~b._flags) | b._values); }
      FlagsSet operator&&(FlagMask flag)     const { return FlagsSet(_flags | (1<<flag), _values | (1<<flag)); }

      FlagsSet& operator&=(const FlagsSet& b);
      FlagsSet& operator&=(int32_t flag);
      FlagsSet& operator+=(const FlagsSet& b);
      FlagsSet& operator+=(int32_t flag);
      FlagsSet& operator-=(const FlagsSet& b);
      FlagsSet& operator-=(int32_t flag);

      //! Negation inverts the value of any enabled flags.
      FlagsSet operator-() const { return FlagsSet(_flags, (~_values & _flags)); }

      bool operator&(const FlagsSet& b) const { return (_flags & b._flags); }
      bool operator&(int32_t flag)      const { return (_flags & (1<<flag)); }

      bool operator==(const FlagsSet& b) const { return (_flags == b._flags && _values == b._values); }
      bool operator!=(const FlagsSet& b) const { return !(*this == b); }


    protected:
      FlagMask _flags;    //!< If a flag bit is on test that bit's value
      FlagMask _values;   //!< Flag value state to test against
    };

    // Predefined predicate masks used for prim filtering:
    using FlagsPredicate = FlagsSet;
    static const FlagsPredicate IsActive;
    static const FlagsPredicate IsLoaded;
    static const FlagsPredicate IsModel;
    static const FlagsPredicate IsGroup;
    static const FlagsPredicate IsAbstract;
    static const FlagsPredicate IsDefined;
    static const FlagsPredicate HasDefiningSpecifier;
    static const FlagsPredicate IsInstance;
    static const FlagsPredicate IncludeInstanceProxies;

    //! Default filtering mask. Same as (IsActive && IsDefined && IsLoaded && -IsAbstract)
    static const FlagsPredicate DefaultPredicate;
    //! No filtering mask (no flags enabled.) Same as FlagsSet().
    static const FlagsPredicate AllPrimsPredicate;


  public:
    //------------------------------------------------------
    // Create:
    //------------------------------------------------------

    //! Creates a Prim definition in the owner Layer, returning a constructed reference.
    //! Convenience function that calls Layer::definePrim().
    static Prim  defineInLayer(const LayerRef& layer,
                               const Path&     path,
                               const Token&    typeName);

    //! Creates a Prim override in the owner Layer, returning a constructed reference.
    //! Convenience function that calls Layer::overridePrim().
    static Prim  overrideInLayer(const LayerRef& layer,
                                 const Path&     path,
                                 const Token&    typeName);

    //! Creates the missing parent prims in a Prim path with a specific type.
    //!
    //! Other define and override prim methods default to creating missing parent
    //! prims as untyped pure overs, which may cause the prim to act as if they
    //! doesn't exist.
    //!
    //! Be careful with this method as it requires a composed source stage to
    //! determine whether a prim is 'missing' from the path or not, and this may
    //! lead to unexpected overhead costs when defining prims.
    static void  createMissingParents(const StageRef& referenceStage,
                                      const LayerRef& layer,
                                      const Path&     path,
                                      const Token&    parentPrimType = GeomTokens.types.Scope);



  public:
    //------------------------------------------------------
    // Attribute management:
    //------------------------------------------------------

    //! Creates and attaches an intrinsic(non-custom) attribute to this Prim.
    //! If the named Attribute object already exists returns it at its current
    //! value.
    //!
    //! If this Prim has just been defined on a Layer then the new
    //! attribute will be defined on that Layer.
    //!
    //! If this Prim is being edited then the attribute will be created
    //! on the editable Stage and saved to the Stage's current
    //! edit target Layer.
    //!
    //! \a variability_hint is only declaring the *expected* time-variance
    //! of the new attribute. In practice this does not do much as setting
    //! time samples later on is not prohibited if the attribute is declared
    //! here as uniform.
    Attribute  createIntrinsicAttr(const Token&    name,
                                   Value::Type     type,
                                   TimeVariability variability_hint = TimeVariability::Uniform);

    //! Creates and attaches a custom(arbitrary) attribute to this Prim.
    //! If the named Attribute object already exists returns it at its current
    //! value.
    //!
    //! If this Prim has just been defined on a Layer then the new
    //! attribute will be defined on that Layer.
    //!
    //! If this Prim is being edited then the attribute will be created
    //! on the editable Stage and saved to the Stage's current
    //! edit target Layer.
    //!
    //! \a variability_hint is only declaring the *expected* time-variance
    //! of the new attribute. In practice this does not do much as setting
    //! time samples later on is not prohibited if the attribute is declared
    //! here as uniform.
    Attribute  createCustomAttr(const Token&    name,
                                Value::Type     type,
                                TimeVariability variability_hint = TimeVariability::Uniform);

    //! Creates an attribute with the given name, type, temporal variability, and
    //! optionally setting the default value.
    //!
    //! If the named Attribute object already exists the default value will still
    //! be set if provided.
    //!
    //! \a variability_hint is only declaring the *expected* time-variance
    //! of the new attribute. In practice this does not do much as setting
    //! time samples later on is not prohibited if the attribute is declared
    //! here as uniform.
    Attribute  createAttr(const Token&    name,
                          Value::Type     type,
                          bool            is_custom = false,
                          TimeVariability variability_hint = TimeVariability::Varying,
                          const Value&    defaultValue = Value());

    //! Sets an existing attribute's value at a specific time. Returns a valid Attribute
    //! if successful otherwise an invalid one on failure.
    //!
    //! This templated method does not support custom user data types, only the common
    //! ones found in the Value::Type 'Base data types' list.
    //!
    //! Assumes the attrib has already been created and configured for the correct
    //! Value::Type and interpolation rate. Use createAttr() for this prior to calling
    //! setAttr().
    //!
    //! If the attribute does not already exist setAttr() will create it as a custom
    //! uniform attribute using the default Value::Type corresponding to the data type
    //! passed in. For example if setting UVs you would likely want the attribute to be
    //! a TexCoord2fArray Value::Type, but this method will create it as a Float2Array.
    //!
    //! To set array-variant types (FloatArray, Color3fArray, etc) pass in a std::vector.
    //! Many of the common array types are already defined in geo/scene/Array.h or in the
    //! vec/mat/box math types.
    //! For example to set an array of normals:
    //!   Vec3fArray normalsArray;
    //!   prim.setAttr(GeomTokens.primvars.normals, normalsArray, 100.0);
    template <typename T>
    Attribute  setAttr(const Token&          name,
                       const T&              v,
                       const fdk::TimeValue& time = fdk::defaultTimeValue());

    //! If the named attribute exists on the Prim returns it otherwise return an empty (invalid) Attribute object.
    Attribute  getAttr(const Token& name) const;

    //! If the named attribute exists on the Prim fills in the provided
    //! Attribute object and returns true, otherwise returns false.
    bool      getAttr(const Token& name,
                      Attribute&   attr) const;

    //! Return the value of the named attribute at a specific time. If the attribute
    //! does not exist an invalid Value is returned.
    Value     getValueAt(const Token&          attributeName,
                         const fdk::TimeValue& time = fdk::earliestTimeValue()) const;

    //! Returns the list of attributes.
    std::vector<Token> getAttributes() const;
    std::vector<Attribute> getUsgAttributes() const;
    //! Returns the list of authored attributes.
    std::vector<Token> getAuthoredAttributes() const;
    //! Returns the list of authored relationships.
    std::vector<Token> getAuthoredRelationships() const;
    //! Returns the list of authored properties. This includes attributes and relationships.
    std::vector<Token> getAuthoredProperties() const;
    //! Removes a property if this prim is an SDF representation.
    void removeProperty(const Token& propertyName);

    //! Copy the attribute from another prim at the given time.
    Attribute  copyAttr(const Prim&    fromPrim,
                        const Token&   name,
                        fdk::TimeValue time);


  public:
    //------------------------------------------------------
    // Inter-prim linkages - ie relationships:
    //------------------------------------------------------

    //! Creates and returns an empty relationship
    Relationship  createRelationship(const Token& name, bool custom = false);

    //! If the named relationship exists on the Prim returns it otherwise
    //! returns an empty (invalid) Relationship object.
    Relationship  getRelationship(const Token& name) const;

    //! If the named relationship exists on the Prim fills in the provided
    //! Relationship object and return true, otherwise return false.
    bool          getRelationship(const Token&  name,
                                  Relationship& rel) const;

    //! Fills in the list of relationship names, returning the number of relationship.
    size_t        getRelationships(std::vector<const char*>& relationships) const;

    //! Adds a inherit path to the prim's inherits list.
    void          addInherit(const Path&  path,
                             ListPosition position = ListPosition::PrependBack);

    //! Return the prim's inherits list (applied/composed).
    PathArray     getInherits();

    //! Return the prim's inherits list for a specific list-op type.
    PathArray     getInherits(ListPosition listOpType);

    //! Removes a inherit path to the prim's inherits list.
    void          removeInherit(const Path& path);

    //! Sets inherits to the prim's inherits list. If empty list is given, clears the inherits list.
    void          setInherits(const PathArray& paths);

    //! Adds a reference to the prim's reference list.
    void          addReference(const std::string& identifier,
                               const Path&        path,
                               ListPosition       position = ListPosition::PrependBack,
                               double offset = 0.0,
                               double scale = 1.0);

    //! Return the prim's references list (applied/composed).
    std::vector<std::pair<std::string, Path>> getReferences();

    //! Return the prim's references list for a specific list-op type.
    std::vector<std::pair<std::string, Path>> getReferences(ListPosition listOpType);

    //! Removes a reference to the prim's reference list.
    void          removeReference(const std::string& identifier,
                                  const Path&        path,
                                  double offset = 0.0,
                                  double scale = 1.0);

    //! Sets references to the prim's reference list. If empty list is given, clears the reference list.
    void          setReferences(const std::vector<std::pair<std::string, Path>>& identifierAndPaths,
                                double offset = 0.0,
                                double scale = 1.0);

    //! Adds a payload to the prim's payload list.
    void          addPayload(const std::string& identifier,
                             const Path&        path,
                             ListPosition       position = ListPosition::PrependBack,
                             double offset = 0.0,
                             double scale = 1.0);

    //! Return the prim's payloads list (applied/composed).
    std::vector<std::pair<std::string, Path>> getPayloads();

    //! Return the prim's payloads list for a specific list-op type.
    std::vector<std::pair<std::string, Path>> getPayloads(ListPosition listOpType);

    //! Removes a payload to the prim's payload list.
    void          removePayload(const std::string& identifier,
                                const Path&        path,
                                double offset = 0.0,
                                double scale = 1.0);

    //! Sets payloads to the prim's payload list. If empty list is given, clears the payload list.
    void          setPayloads(const std::vector<std::pair<std::string, Path>>& identifierAndPaths,
                              double offset = 0.0,
                              double scale = 1.0);

    //! Adds a specialize path to the prim's specializes list.
    void          addSpecialize(const Path&   path,
                                ListPosition  position = ListPosition::PrependBack);

    //! Return the prim's specializes list (applied/composed).
    PathArray     getSpecializes();

    //! Return the prim's specializes list for a specific list-op type.
    PathArray     getSpecializes(ListPosition listOpType);

    //! Removes a specialize path to the prim's specializes list.
    void          removeSpecialize(const Path& path);

    //! Sets specializes to the prim's specializes list. If empty list is given, clears the specializes list.
    void          setSpecializes(const PathArray& paths);

    //! Adds a variant set to the prim.
    void          addVariantSet(const std::string& variantSetName,
                                ListPosition       position = ListPosition::PrependBack);

    //! Adds a variant to a variant set
    void          addVariant(const std::string& variantSetName, const std::string& variantName);

    //! Sets the variant selection for a variant set
    void          setVariantSelection(const std::string& variantSetName, const std::string& selection);

    //! Returns the list of variant sets authored on the prim
    std::vector<std::string> getVariantSets() const;

    //! Returns the list of variants for a variant set on this prim
    std::vector<std::string> getVariants(const std::string& variantSetName) const;

    //! Returns the variant selection for a given variant set
    std::string getVariantSelection(const std::string& variantSetName) const;

  public:
    //------------------------------------------------------
    // Child Prims:
    //------------------------------------------------------

    //! Fills in a list of child Prims directly underneath this one (ie does
    //! not include any children of the children) and returns the list size.
    //!
    //! This only functions if Prim is resident on a valid stage.
    size_t getChildren(std::vector<Prim>& children,
                       bool activeOnly = true,
                       bool includeClasses = false,
                       bool includeOverrides = true) const;

    // Removes the child. Returns true if successful, false if failed.
    bool removeChild(const usg::Prim& child);

    //! Returns a possible layer which defined this prim. This returns the layer containing the first "def" for the prim.
    LayerRef  getDefiningLayer() const;
    //! Returns a list of layers which contibuted to the creation of this layer. Only works with
    //! prims coming from a stage, returns an empty list otherwise.
    std::vector<LayerRef> getLayerStack() const;

  public:
    //! Debugging - prints some key info about the prim to a stream.
    virtual void  print(const char*   prefix,
                        std::ostream& o,
                        bool          printAttribContents = false) const;


  public:
    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    Prim(const Prim::Impl&);
    Prim::Impl*         impl()       { return _pImpl.get(); }
    const Prim::Impl*   impl() const { return _pImpl.get(); }
    //! Remove the binding making the Prim invalid. For internal use only!
    void invalidate();

    // Deprecated, do not use.
    virtual bool  applyAttributeOverrides(const ShaderDesc&, [[maybe_unused]] bool v = true) { return false; }


  protected:
    void initialize(const Prim& prim);

  protected:
    std::unique_ptr<Prim::Impl, void(*)(Prim::Impl*)> _pImpl;
  };



  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/


  //! Print out terse info about the prim to a stream.
  USG_API  std::ostream& operator << (std::ostream&, const Prim&);
  //! Print out FlagSet state to a stream.
  USG_API  std::ostream& operator << (std::ostream&, const Prim::FlagsSet&);

} // namespace usg

#endif
