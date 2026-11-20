// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_GEOMSTAGE_H
#define USG_GEOM_GEOMSTAGE_H

#include "usg/geom/Layer.h"
#include "usg/geom/PrimRange.h"

#include "fdk/base/Hash.h"
#include "fdk/math/Box3.h"

#include <memory>
#include <string>
#include <utility>

namespace usg
{
  using StageRef = std::shared_ptr<Stage>;
  using StageConstRef = std::shared_ptr<const Stage>;


  //! class Stage
  //! The wrapper class for a reference to a stage.
  //! When passing stage references, use LayerRef instead of Layer as that
  //! provides a pointer-like interface with access via the -> operator.
  class USG_API Stage
  {
  public:
    class Handle; //!< Abstraction handle for casting to underlying implementation type

    //! Load rules for setting the locations to be included / excluded during loading
    enum LoadRule {
      AllRule,
      OnlyRule,
      NoneRule
    };
    //! A Structure for passing load rules to the stage
    typedef std::vector<std::pair<Path, LoadRule>> LoadRules;

    //! Creates a new Stage from the root layer.
    //! If \a compose_state is provided it's copied into the Stage::Impl and
    //! retrievable via Stage::composeState().
    static StageRef Create(const LayerRef&  rootLayer,
                           const fdk::Hash* composeState = nullptr,
                           bool             loadAll = true);

    //! Creates a new Stage from the given root layer and session layer.
    //! If \a compose_state is provided it's copied into the Stage::Impl and
    //! retrievable via Stage::composeState().
    static StageRef Create(const LayerRef&  rootLayer,
                           const LayerRef&  sessionLayer,
                           const fdk::Hash* composeState = nullptr,
                           bool             loadAll = true);

    //! Creates a new stage from the given root layer limiting its population
    //! to the prims provided by \a maskedPaths. If \a composeState is provided
    //! it's copied into the Stage::Impl and retrievable via Stage::composeState().
    static StageRef CreateMasked(const LayerRef& rootLayer,
                                 const PathArray& maskedPaths,
                                 const fdk::Hash* composeState = nullptr,
                                 bool loadAll = true);
    //! Creates an empty Stage.
    static StageRef CreateInMemory(bool loadAll = true, const std::string& arResolverContext = "");
    static StageRef
    CreateInMemory(bool loadAll,
                   const std::vector<std::pair<std::string, std::string>>& arResolverContexts);

    //! Creates a new Stage from the given file path
    static StageRef Open(const std::string& filePath);

    //! Creates a new Stage from the given file path, limiting its population to
    //! the prims provided by \a maskedPaths.
    static StageRef OpenMasked(const std::string& filePath,
                               const PathArray&   maskedPaths,
                               bool               loadAll = true);

    //! Same, but accepts an ArResolverContext identifier.
    static StageRef OpenMasked(const std::string& filePath,
                               const std::string& arResolverContext,
                               const PathArray&   maskedPaths,
                               bool               loadAll = true);

    //------------------------------------------------------

    //! True if Stage::Impl is valid (has a valid object binding.)
    bool                    isValid() const;
    operator                bool()    const { return this->isValid(); }

    //! Return a pointer to the underlying UsdStage object if the usd_version
    //! passed in matches the Usd version this library was built with,
    //! otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to a UsdStageRefPtr* and
    //! possibly double-check its validity since a Stage can be created
    //! without a valid binding. However, if Stage::isValid() returns
    //! true then the underlying binding will also be valid.
    Stage::Handle*        getUsdStageRefPtr(int usd_version);
    const Stage::Handle*  getUsdStageRefPtr(int usd_version) const;

    //------------------------------------------------------

    //! Sets the edit target layer of the stage
    void      setEditTarget(const LayerRef& edit_layer);

    //! Set the load rules for the stage
    void      setLoadRules(const LoadRules& rules);

    //! Get the load rules for the stage
    LoadRules getLoadRules();

    //! Returns a set of all loaded paths.
    PathSet getLoadSet() const;

    //! Returns the root layer of the stage
    LayerRef  getRootLayer() const;

    //! Returns the session layer of the stage
    LayerRef  getSessionLayer() const;

    //! Returns the root prim of the stage
    Prim      getRootPrim() const;

    //! Returns the prim at \a path
    Prim      getPrimAtPath(const Path& path) const;

    //! Returns all prim paths in the stage.
    //! Caution - this can potentially be a very expensive operation on large stages!
    size_t getPrimPaths(PathArray& prim_paths) const;

    std::vector<LayerRef> getLayerStack() const;

    //! Indicates how the stage should be traversed while searching for prims.
    enum class TraversePolicy {
      DontTraverse,           //!< Don't traverse the stage, searching only the given paths.
      FirstInSubTree,         //!< Traverse the stage but stop at the first occurence in a sub-tree.
      All                     //!< Fully traverse the stage.
    };

    //! Given a list of paths, iterates through them and optionally their descendents,
    //! returning a list of all prims matching the type T.
    //! The stage traversal is controlled by TraversePolicy.
    template<class T>
    PathArray findPrimsOfType(const PathArray& primPaths, TraversePolicy policy) const;

    //! Given a list of paths, iterates through them and optionally their descendents,
    //! returning a list of all prims matching the type T.
    template<class T>
    PathArray findPrimsOfType(const PathArray& primPaths,
                              bool             matchDescendents = true) const;

    //! Given a list of paths, iterates through them and their descendents, returning
    //! a list of all prims of type T.
    template<class T> std::vector<T> findDescendentPrims(const PathArray& primPaths) const;

    //! Returns all paths that can be loaded from the stage.
    PathArray findLoadable(const Path& rootPath = Path::AbsoluteRootPath()) const;

    //! Batch load the given path set. This is equivalent to calling Prim::load()
    // for each item in the pathSet, however this method is more efficient as
    // all operations are committed in a single batch.
    void load(const PathSet& pathSet, bool loadWithDescendants = true, bool replace = false);

    //! Batch load and unload the given path sets. This is equivalent to calling Prim::load()
    // and Prim::unload, for each item in loadSet and unloadSet, respectively, however this method
    // is more efficient as all operations are committed in a single batch.
    void loadAndUnload(const PathSet& loadSet, const PathSet& unloadSet, bool loadWithDescendants);

    //! An operation for setPopulationMask
    enum class MaskOperation {
      Set,
      Clear,
      Union,
      Intersection
    };

    //! Set the population mask for the stage. Use this to limit the prims that are composed.
    //! This is useful for isolating prims.
    void setPopulationMask(const PathArray& primPaths, MaskOperation op = MaskOperation::Set);

    //! Retrieve the population masks stored in the stage.
    PathArray getPopulationMask() const;

    //! Mutes and unmutes layers on the stage
    void muteAndUnmuteLayers(const std::vector<std::string> &muteLayers, const std::vector<std::string> &unmuteLayers);

    //! Returns the list of muted layers
    const std::vector<std::string> getMutedLayers() const;

    //! Build a PrimRange traverser using the Prim::DefaultPredicate mask (ie Prim::IsActive &&
    //! Prim::IsDefined && Prim::IsLoaded && -Prim::IsAbstract).
    PrimRange traverse() const;

    //! Build a PrimRange traverser using a specific predicate mask.
    PrimRange traverse(const Prim::FlagsPredicate& mask) const;

    //! Build a PrimRange traverser using the Prim::AllPrimsPredicate (ie no filtering.)
    PrimRange traverseAll() const;

    usg::PathArray getPrimPathsByTypeName(const usg::Token& typeName,
                                          bool traverseModelHierarchy = true);

    //! Returns paths of all prims that have the given API schema applied.
    usg::PathArray getPrimPathsByAPI(const usg::Token& apiSchemaName,
                                     bool traverseModelHierarchy = true);

    //------------------------------------------------------

    //! Compute the world-space bounding-box for the entire stage at a specific time (required.)
    fdk::Box3d  getWorldBbox(const fdk::TimeValue& time) const;

    //! Set the metersPerUnit metadata tag on the stage.
    void    setMetersPerUnit(double metersPerUnit);

    //! Is the metersPerUnit metadata tag authored on the stage?
    bool    hasMetersPerUnit() const;

    /*! Return the metersPerUnit metatada tag if authored, otherwise 0.1, the USD default
        of centimeters.
        Note the default unit for Nuke is meters so always check hasMetersPerUnit() to
        avoid getting centimeters by mistake.
    */
    double  getMetersPerUnit() const;

    //------------------------------------------------------
    //! Returns the flattened stage as a single anonymous layer
    LayerRef flatten(bool addComment = true);

    //! Returns the flattened stage as a single anonymous layer. Unlike 'flatten', this preserves variants and some composition arcs.
    LayerRef flattenLayerStack();

    //! Exports the flattened stage to a file
    bool  exportToFile(const std::string&    filepath,
                       bool                  addComment = true,
                       const FileFormatArgs& fileFormatArgs = FileFormatArgs());

    //! Exports the flattened stage to a string
    bool  exportToString(std::string& text,
                         bool         addComment = true);

    //------------------------------------------------------

    //! Start of active time sample range.
    void            setStartTimeCode(fdk::TimeValue time);
    fdk::TimeValue  getStartTimeCode();

    //! End of active time sample range.
    void            setEndTimeCode(fdk::TimeValue time);
    fdk::TimeValue  getEndTimeCode();

    //! Falls back to framesPerSecond, then to a default value of 24.0
    void    setTimeCodesPerSecond(double timeCodesPerSecond);
    double  getTimeCodesPerSecond() const;
    //!
    void    setFramesPerSecond(double fps);
    double  getFramesPerSecond() const;

    //! Copies the populationMask, load rules and muted layers from \a stage
    void copyFrom(const usg::StageRef& stage);

    void   printReport(std::ostream& o,
                       bool          print_contents = true) const;

    //------------------------------------------------------

    // Resolves the given identifier based on the Stage's current ArResolverContext 
    // and the layer of its EditTarget as an anchor for relative references.
    std::string resolveIdentifierToEditTarget(const std::string& identifier);

    //! Updates a stage by copying from fromStage. Returns the updated stage. This may not be the stage that was
    //! passed in, as sometimes the stage needs to be recreated if the asset resolver context changes. \a stage
    //! can be null, in which case a new stage will be created. The copied stage will contain copies of the root layers
    //! of \a fromStage and the copying is done in such a way that the minimum number of stage notifications will
    //! be issued.
    static StageRef UpdateStage(StageRef& stage, const StageRef& fromStage);

  public:
    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    class Impl; //!< Wrapper abstraction for implementation type
    Stage(const Stage::Impl&);
    virtual ~Stage(); // required for unique_ptr

    Stage::Impl*        impl()       { return _pImpl.get(); }
    const Stage::Impl*  impl() const { return _pImpl.get(); }

    //! Set the compose state hash explicitly (usually passed to the latest stage Create()).
    void       setComposeState(const fdk::Hash& compose_state);
    //! Get the compose state hash currently assigned to the stage.
    fdk::Hash  composeState() const;

    //! Set hashes to help track the lastest modify state affecting the stage.
    void       setProcessState(const fdk::Hash& process_state);
    fdk::Hash  processState() const;

    //! Set hashes to help track the latest edit state of the stage.
    void       setEditState(const fdk::Hash& edit_state);
    fdk::Hash  editState() const;

    //! Set a state hash to help track the root layer's sublayer list state. Normally this
    //! is used to quickly test sublayer list status.
    void       setRootState(const fdk::Hash& root_state);
    fdk::Hash  rootState() const;


  private:
    Stage() = delete;
    Stage(const Stage&) = delete;
    Stage& operator=(const Stage&) = delete;

    std::unique_ptr<Stage::Impl, void(*)(Stage::Impl*)> _pImpl;
    LayerRef _sessionLayer;
  };


  //-----------------------------------------------------------------------------------------


  //! class StageNotifications
  //! A class for receiving notifications when changes are made to a stage.
  //! The default implementatin just keeps a changed flag.
  class USG_API StageNotifications {
  public:
    class ChangedObjects {
    public:
      using PathPairVector = std::vector<std::pair<Path, Path>>;
      virtual ~ChangedObjects() {}
      virtual PathArray getChangedPaths() const = 0;
      virtual PathArray getChangedInfoPaths() const = 0;
      virtual TokenArray getChangedFields(const Path& path) const = 0;
      virtual PathPairVector getRenamedPaths() const { return {}; }
    };

    explicit StageNotifications(const StageRef& stage, bool trackRenames = false);
    virtual ~StageNotifications();

    virtual void handleObjectsChanged(const ChangedObjects& changedObjects);
    virtual void handleContentsChanged() { _changed = true; }
    virtual void handleLayerMutingChanged() { _changed = true; }
    virtual void handleEditTargetChanged() { _changed = true; }

    void clearChanged() { _changed = false; }
    bool isChanged() const { return _changed; }

    class StageNotificationsImpl;
  private:
    std::unique_ptr<StageNotificationsImpl, void(*)(StageNotificationsImpl*)> _pImpl;
    bool _changed = false;
  };

} // namespace usg

#endif
