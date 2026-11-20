// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_GEOMLAYER_H
#define USG_GEOM_GEOMLAYER_H

#include "usg/base/ArrayTypes.h"
#include "usg/geom/Prim.h"

#include <map>
#include <memory>

namespace usg {

  class Layer;
  using LayerRef = std::shared_ptr<Layer>;
  using LayerConstRef = std::shared_ptr<const Layer>;

  using FileFormatArgs = std::map<std::string, std::string>;  //!< A dictionary used to pass file format arguments

  //! The wrapper class for a reference to a layer.
  //! When passing layer references, use LayerRef instead of Layer as that
  //! provides a pointer-like interface with access via the -> operator.
  class USG_API Layer
  {
  public:
    class Handle; //!< Abstraction handle for casting to underlying implementation type

    //! Creates a new layer with the given identifier
    static LayerRef Create(const std::string&            identifier,
                           const fdk::KeyValueMapSorted& file_args = fdk::KeyValueMapSorted());

    //! Creates a new anonymous layer with the given identifier
    static LayerRef CreateAnonymous(const std::string&            identifier = std::string(),
                                    const fdk::KeyValueMapSorted& file_args = fdk::KeyValueMapSorted());

    //! Gets a vector of all currently loaded layers.
    static std::vector<LayerRef> GetLoadedLayers();

    //! Searches for a layer with the given identifier in the layer cache. If
    //! the identifier string is empty nothing happens and a null reference is
    //! returned. If the layer can't be found a null reference is returned.
    //!
    //! The identifer can be just a local session name like 'MyEditLayer' or it can
    //! be a file path pointing at an external Usd file like '/Foo/MyEditLayer.usd'.
    //!
    //! If this Layer will be written to a file you should provide the appropriate
    //! file extension (.usd, .usda, .usdc, etc) at this time, and any arguments
    //! to be passed to the file format plugin.
    static LayerRef Find(const std::string&            identifier,
                         const fdk::KeyValueMapSorted& file_args = fdk::KeyValueMapSorted());

    //! Searches for a layer with the given identifier in the layer cache, opening it if it's not already
    //! open. If the identifier string is empty nothing happens and a null reference is returned.
    //!
    //! The identifer can be just a local session name like 'MyEditLayer' or it can
    //! be a file path pointing at an external Usd file like '/Foo/MyEditLayer.usd'.
    //!
    //! If this Layer will be written to a file you should provide the appropriate
    //! file extension (.usd, .usda, .usdc, etc) at this time, and any arguments
    //! to be passed to the file format plugin.
    static LayerRef FindOrOpen(const std::string&            identifier,
                               const fdk::KeyValueMapSorted& file_args = fdk::KeyValueMapSorted());

    //! Searches for a layer with the given identifier in the layer cache. If
    //! the identifier string is empty nothing happens and a null reference is
    //! returned. If the layer can't be found a null reference is returned.
    //!
    //! The identifier will be resolved relative to the anchor layer. If the
    //! anchor layer is invalid nothing happens and a null reference is
    //! returned.
    //!
    //! The identifer can be just a local session name like 'MyEditLayer' or it
    //! can be a file path pointing at an external Usd file like
    //! '/Foo/MyEditLayer.usd'.
    //!
    //! If this Layer will be written to a file you should provide the
    //! appropriate file extension (.usd, .usda, .usdc, etc) at this time, and
    //! any arguments to be passed to the file format plugin.
    static LayerRef FindRelativeToLayer(const LayerRef&               anchor,
                                        const std::string&            identifier,
                                        const fdk::KeyValueMapSorted& file_args = fdk::KeyValueMapSorted());

    //! Searches for a layer with the given identifier in the layer cache, opening it if it's not already
    //! open. If the identifier string is empty nothing happens and a null reference is returned.
    //!
    //! The identifier will be resolved relative to the anchor layer. If the anchor layer is invalid nothing
    //! happens and a null reference is returned.
    //!
    //! The identifer can be just a local session name like 'MyEditLayer' or it can
    //! be a file path pointing at an external Usd file like '/Foo/MyEditLayer.usd'.
    //!
    //! If this Layer will be written to a file you should provide the appropriate
    //! file extension (.usd, .usda, .usdc, etc) at this time, and any arguments
    //! to be passed to the file format plugin.
    static LayerRef FindOrOpenRelativeToLayer(const LayerRef&               anchor,
                                              const std::string&            identifier,
                                              const fdk::KeyValueMapSorted& file_args = fdk::KeyValueMapSorted());

    //------------------------------------------------------

    //! Returns true if the layer has a wrapped layer
    bool                    isValid() const;
    operator                bool()    const { return this->isValid(); }

    //! Returns a pointer to the underlying SdfLayer object if the usd_version
    //! passed in matches the Usd version this library was built with,
    //! otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to an SdfLayerRefPtr* and
    //! possibly double-check its validity since a Layer can be created
    //! without a valid binding. However, if Layer::isValid() returns
    //! true then the underlying binding will also be valid.
    class Layer::Handle*        getSdfLayerRefPtr(int usd_version);
    const class Layer::Handle*  getSdfLayerRefPtr(int usd_version) const;

    //------------------------------------------------------

    //! Clears the layer, clearing out any existing prims.
    //! Note that this will likely cause a Stage recompose since it's removing all prim
    //! and property definitions.
    //! Use clearAttributeContents() instead if you want to retain the definitions but
    //! empty all property value assignments.
    void clear();

    //! Returns true if the layer contains no prims.
    bool isEmpty() const;

    //! Restores the layer to a state as if it had just been created with findOrOpen().
    void reload();


    //------------------------------------------------------

    //! Returns the identifier string passed originally to the constructor.
    const std::string&  getIdentifier() const;

    //! Sets the layer identifier
    void                setIdentifier(const std::string& identifier);

  public:
    //------------------------------------------------------
    // Layer editing
    //------------------------------------------------------

    //! Adds all sublayers of \a layer to this layer
    void   combineSubLayers(const LayerRef& layer);

    //! Fetches the sublayer identifier list for this layer.
    fdk::StringList  getSubLayers() const;

    //! Fetches the sublayer identifier list for this layer. Returns the number of sublayers.
    size_t           getSubLayers(fdk::StringList& sublayerIdentifiers) const;

    //! Sets the list of the layer sublayers.
    void   setSubLayers(const fdk::StringList& sublayerIdentifiers);

    //! Sets the list of the layer sublayers to be the same as \a layer
    void   setSubLayers(const LayerRef& layer);

    //! Inserts a new sublayer at the given index. The default index of -1 means insert at the end.
    //! Sublayer index 0 has the strongest opinion and the last sublayer in the list has the weakest.
    void   insertSubLayer(const std::string& sublayerIdentifier,
                          int                index = -1);
    //! Inserts a new sublayer. The default index of -1 means insert at the end.
    //! Sublayer index 0 has the strongest opinion and the last sublayer in the list has the weakest.
    void   insertSubLayer(const LayerRef& sublayer,
                          int             index = -1);

    //! Returns the number of sublayers..
    size_t getNumSubLayers() const;

    //! Sets a value in the custom data for this layer
    void   setCustomLayerData(const Token& key, const Value& value);

    //! Sets a value in the custom data for this layer
    void   setCustomLayerData(const Token& key, const ValueDictionaryRef& value) const;

    //! Returns the value for \a key in the custom data for this l;ayer.
    Value  getCustomLayerData(const Token& key) const;

    //! Returns the custom layer data dictionary.
    ValueDictionaryRef getCustomLayerData() const;

    //! Returns true if the layer has a custom layer data dictionary.
    bool   hasCustomLayerData() const;

    //! Clears the custom layer data dictionary.
    void  clearCustomLayerData();

    //! Sets the layer's owner.
    void setOwner(const std::string& owner);
    
    //! Gets the layer's owner.
    std::string getOwner() const;

    //! returns true if the layer has an owner.
    bool hasOwner() const;

    //! Clears the layer's owner.
    void clearOwner();

    //! Sets the layer's session owner.
    void setSessionOwner(const std::string& sessionOwner);

    //! Gets the layer's session owner.
    std::string getSessionOwner() const;

    //! Sets the layer's comment.
    void setComment(const std::string& comment);

    //! Gets the layer's comment.
    std::string getComment() const;

    //! Sets the layer's documentation.
    void setDocumentation(const std::string& documentation);

    //! Gets the layer's documentation.
    std::string getDocumentation() const;

    //! Sets the layer's color configuration.
    void setColorConfiguration(const AssetPath& colorConfiguration);

    //! Gets the layer's color configuration.
    AssetPath getColorConfiguration() const;

    //! Sets the layer's color configuration.
    void setColorManagementSystem(const Token& colorManagementSystem);

    //! Gets the layer's color configuration.
    Token getColorManagementSystem() const;

    //! Sets the start time code.
    void setStartTimeCode(double timeCode);

    //! Gets the start time code.
    double getStartTimeCode() const;

    //! Returns true if the layer has a start time code.
    bool hasStartTimeCode() const;

    //! Clear the start time code.
    void clearStartTimeCode();

    //! Sets the end time code.
    void setEndTimeCode(double timeCode);

    //! Gets the end time code.
    double getEndTimeCode() const;

    //! Returns true if the layer has an end time code.
    bool hasEndTimeCode() const;

    ///! Clears the end time code.
    void clearEndTimeCode();

    //! Sets the frames per second.
    void setFramesPerSecond(double framesPerSecond);

    //! Gets the frames per second.
    double getFramesPerSecond() const;

    //! Returns true if the layer has a frames per second.
    bool hasFramesPerSecond() const;

    //! Clears the frames per second.
    void clearFramesPerSecond();

    //! Sets the time codes per second.
    void setTimeCodesPerSecond(double timeCodesPerSecond);

    //! Gets the time codes per second.
    double getTimeCodesPerSecond() const;

    //! Returns true if the layer has a time codes per second.
    bool hasTimeCodesPerSecond() const;

    //! Clears the time codes per second.
    void clearTimeCodesPerSecond();

    //! Sets the frame precision.
    void setFramePrecision(int framePrecision);

    //! Gets the frame precision.
    int getFramePrecision() const;

    //! Returns true if the layer has a frame precision.
    bool hasFramePrecision() const;

    //! Clears the frame precision.
    void clearFramePrecision();

    //! Sets the name of the node which owns this layer.  This can be used to enable the node selection mechanism for custom layers.
    void setOwnerNodeName(const std::string& owner);

    //! Returns the name of the node which owns this layer, or an empty string if none.
    std::string getOwnerNodeName() const;

    //! Sets whether the contents of this layer are selectable. This can be used to disable selection of prims for the selection system.
    void setSelectable(bool selectable);

    //! Returns the whether the contents of this layer are selectable, defaulting to true if not set.
    bool getSelectable() const;

    //! Fetches the composition asset identifiers for this layer. Returns the number of assets.
    size_t getCompositionAssetDependencies(fdk::StringList& assetIdentifiers) const;

    //! Returns the up axis for the layer. If not set, returns "Y"/
    Token getUpAxis() const;

    //! Sets the up axis for the layer.
    void setUpAxis(const Token& upAxis);

    //! Returns true if the layer has an up axis.
    bool hasUpAxis() const;

    //! Clears the up axis.
    void clearUpAxis();

    //! Returns the expression variables for the layer.
    //! Returns an empty map if none are authored.
    //! String and token values are included; other types are silently skipped.
    void getExpressionVariables(std::map<std::string, std::string>& variables) const;

    //! Sets the expression variables for the layer.
    void setExpressionVariables(const std::map<std::string, std::string>& variables);

    //! Returns true if the layer has expression variables.
    bool hasExpressionVariables() const;

    //! Clears the expression variables.
    void clearExpressionVariables();

    //! Returns the meters per unit for the layer. If not set, returns "Y"/
    double getMetersPerUnit() const;

    //! Sets the meters per unit for the layer.
    void setMetersPerUnit(const double metersPerUnit);

    //! Returns true if the layer has a meters per unit.
    bool hasMetersPerUnit() const;

    //! Clears the meters per unit.
    void clearMetersPerUnit();

    //! Add/enable a level of change notification blocking so that a series of prim or property
    //! changes done to the layer can be bundled up as one big change.
    void  changeBlockPush();

    //! Remove one level of change blocking. If this is the last pop the blocker is removed
    //! and the change notification is done.
    void  changeBlockPop();

  public:
    //------------------------------------------------------
    // Time samples
    //------------------------------------------------------

    //! Fills in the list of unique times samples in the layer, and returns the total count.
    size_t getTimeSamples(std::set<fdk::TimeValue>& times) const;

    //! Set the time scale and offset for the layer.
    void setSubLayerScaleAndOffset(int index, double offset, double scale);

    //! Get the time scale and offset for the layer.
    void getSubLayerScaleAndOffset(int index, double& offset, double& scale) const;

    //------------------------------------------------------
    // Create / get Prims
    //------------------------------------------------------

    //! Defines an abstract Prim in this Layer assigned to the 'schema_class'
    //! name. The 'schema_class' is the prim's schema name like 'Mesh', 'Scope',
    //! 'Xform', etc.
    //! The created Prim will have a 'def' specifier.
    //!
    //! If a prim already exists at 'path' it is returned unchanged.
    //!
    //! If the parent prims in the path do not exist they will be created but will lack
    //! any specific schema and have an 'over' specifier.
    Prim    definePrim(const Path&  path, const Token& typeName);

    //! Same as definePrim() but the created Prim will have an 'over' specifier.
    Prim    overridePrim(const Path&  path, const Token& typeName);

    //! Same as overridePrim(path, typeName) but the created Prim will have an no Type.
    Prim    overridePrim(const Path&  path);

    //! Similar to definePrim() but the created Prim will have an 'over' specifier
    //! and the schema class is taken from the provided source Prim.
    Prim    overridePrim(const Prim& src_prim);

    //! Same as definePrim() but the created Prim will have an 'class' specifier.
    Prim    classPrim(const Path&  path, const Token& typeName);


    //! Returns the root prim of the layer
    Prim    getRootPrim() const;

    //! Returns the root prims of the layer
    std::vector<Prim>    getRootPrims() const;

    //! Returns the prim defined at the given path
    Prim    getPrimAtPath(const Path& path) const;

    //! Returns all prim paths defined in this layer
    size_t  getPrimPaths(PathArray& prim_paths) const;

    //! Returns all attribute paths defined in this layer
    size_t  getAttrPaths(PathArray& attr_paths) const;

    //! Returns all attributes defined in this layer
    std::vector<usg::Attribute> getAllAttrs() const;


    //! Returns true if the layer has a default prim
    bool    hasDefaultPrim() const;
    //! Returns the name of the default prim of the layer, or a empty token if none.
    Token   defaultPrim() const;
    //! Sets the name of the default prim of the layer
    void    setDefaultPrim(const Token& name);
    //! Clears the default prim of the layer
    void    clearDefaultPrim();

    //! Clears all authored prim definitions in the layer.
    //! This will cause Stage path-change notifications and a possible recompose.
    //! Note this is different than calling Layer::clear() which will remove everything
    //! from the Layer including sublayers and custom data.
    void    clearAuthoredPrims();

    //! Clears all authored property/relationship values in the layer while retaining
    //! their definitions, avoiding Stage path-change notifications and a possible
    //! recompose.
    //! Clearing authored values will cause 'Info Path' change notifications, but these
    //! should not precipitate a Stage recomposition event.
    //!
    //! Removes each property's default value and all time samples.
    void    clearAuthoredProperties();

    //------------------------------------------------------
    // Create paths convenience
    //------------------------------------------------------

    //! Defines a parenting path of Scope prims to anchor another prim set to.
    //! If the prims in the path already exist but are not Scope types they're
    //! left alone.
    //! Returns the last Prim in the path, regardless of whether it's a Scope or not.
    Prim   defineScopeParents(const Path&  path,
                              const Token& last_prim_schema = "Scope");

    //! Defines a parenting path of Scope prims with a final Xform prim to anchor
    //! another prim to.
    //! If the prims in the path already exist but are not Scope types they're
    //! left alone.
    //! Returns the last Prim in the path, regardless of whether it's a Scope or not.
    Prim   defineScopeXformParents(const Path& path) { return defineScopeParents(path, "Xform"); }


    //! Returns true if the layer is anonymous
    bool isAnonymous() const;

    //! Returns the resolved file path of the layer
    std::string getRealPath() const;

    //! Saves the layer
    bool save(bool force = false);

    //! Exports the flattened layer to a file
    bool exportToFile(const std::string&    filepath,
                      const std::string&    comment = std::string(),
                      const FileFormatArgs& fileFormatArgs = FileFormatArgs()) const;

    //! Exports the flattened layer to a string
    bool exportToString(std::string& text) const;

    //! Imports the layer from a string
    bool importFromString(const std::string& text) const;

    //! Transfers the content of \a layer into this layer
    void transferContent(const LayerRef& layer);

    //! Stitches the content of \a weakLayer into this layer
    void stitch(const LayerRef& weakLayer);

  #ifdef WITH_FN_CRATE_CACHE
    //! Returns Layer as bytes
    bool serialize(std::vector<char>& dest);
    bool serialize(UCharArray& dest);

    //! Import Layers from bytes
    bool deserialize(const char* src, size_t length);
    bool deserialize(const std::vector<char>& src);
    bool deserialize(const UCharArray& src);
  #endif

    void copySpec(const Path& path, const LayerRef& targetLayer, const Path& targetPath);

    //! Returns a list of paths which may have been modified. This means any prim with a 'def' specifier, or an over with property definitions.
    PathArray getModifiedPrimPaths() const;

    //! Output the contents of the layer to a stream. Calls exportToString().
    void printContents(const char* prefix, std::ostream&) const;

    //! Helper function to detect cycles in layer sublayer hierarchy
    bool hasCycle(std::string& message) const;

  public:
    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    class Impl; //!< Wrapper abstraction for implementation type
    Layer(const Layer::Impl&);
    ~Layer(); // required for unique_ptr

    //! Returns the handle to the wrapped object.
    Layer::Impl*        impl()       { return _pImpl.get(); }
    const Layer::Impl*  impl() const { return _pImpl.get(); }

    //! Creates a layer wrapper from the given layer
    static LayerRef Create(const Layer::Impl& handle);

  private:
    Layer() = delete;
    Layer(const Layer& layer) = delete;
    Layer& operator=(const Layer& layer) = delete;

    std::unique_ptr<Layer::Impl, void(*)(Layer::Impl*)> _pImpl;

    // Allow access for attribute caches:
    friend class Prim;
    friend USG_API std::ostream& operator << (std::ostream&, const Layer&);
    friend USG_API bool operator == (const Layer& lhs, const Layer& rhs);
    friend USG_API bool operator != (const Layer& lhs, const Layer& rhs);
  };

  //! Print out Layer components to a stream.
  USG_API std::ostream& operator << (std::ostream&, const Layer&);
  //! Compares the handles to underlying sdf layers.
  USG_API bool operator == (const Layer& lhs, const Layer& rhs);
  USG_API bool operator != (const Layer& lhs, const Layer& rhs);

  //! RAII helper class to ensure changeBlockPop() is called even if an exception is thrown.
  //! Usage: LayerChangeBlock block(layer);
  class LayerChangeBlock
  {
  public:
    explicit LayerChangeBlock(LayerRef layer)
      : m_layer(std::move(layer))
    {
      if (m_layer) {
        m_layer->changeBlockPush();
      }
    }

    ~LayerChangeBlock()
    {
      if (m_layer) {
        m_layer->changeBlockPop();
      }
    }

    LayerChangeBlock(const LayerChangeBlock&) = delete;
    LayerChangeBlock& operator=(const LayerChangeBlock&) = delete;

  private:
    LayerRef m_layer;
  };

} // namespace usg

#endif
