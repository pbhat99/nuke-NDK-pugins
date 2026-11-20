// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_ENGINE_GEOMENGINE_H
#define USG_ENGINE_GEOMENGINE_H

#include "usg/engine/api.h"
#include "usg/engine/GeomState.h"
#include "usg/engine/GeomStateTarget.h"
#include "usg/engine/GeomSceneContext.h"
#include "usg/engine/EngineTokens.h"

#include "usg/geom/Stage.h"
#include "usg/geom/GeomTokens.h"
#include "usg/base/ArgSet.h"

#include <memory>
#include <string>

namespace usg {

  class GeomSceneOutput;
  using TimeGeomStateMap = std::map<fdk::TimeValue, GeomState>;

  //--------------------------------------------------------------------------------


  /*! Scenegraph processing engine node that can build a usg::Stage.
  */
  class USG_ENGINE_API GeomEngine
  {
  public:
    GeomEngine();
    virtual ~GeomEngine();

    //! Name of the engine. Not saved locally as name can change dynamically. Must implement.
    virtual std::string  name() const =0;

    //----------------------------------------------------------------------

    //! Return the number of GeomEngine inputs. Must implement.
    virtual size_t       nInputs() const =0;

    //! Returns the GeomEngine pointer from input index. Must implement.
    virtual GeomEngine*  inputEngine(uint32_t input) const = 0;
    virtual GeomEngine*  inputEngine0() const { return inputEngine(0); }
    virtual GeomEngine*  inputEngine1() const { return inputEngine(1); }

    //----------------------------------------------------------------------
    // GeomState change targets
    //----------------------------------------------------------------------

    /*! Assign a change target to engine. It's assumed the \a target object is a
        local variable in this class and will not disappear or become invalid!
        Not thread safe.
    */
    void  assignTarget(GeomStateTarget& target);

    /*! Add a change target to engine, parented to a parent change target group.
        It's assumed the \a target object is a local variable in this class and
        will not disappear or become invalid!
        Not thread safe.
    */
    void  connectTargetToParent(GeomStateTarget&  target,
                                const usg::Token& parent_id);
    /*! Add a change target to engine, parented to an existing parent target.
        Not thread safe.
    */
    void  connectTargetToParent(GeomStateTarget& target,
                                GeomStateTarget& parent_target);

    // GeomStateTarget convenience functions:
    void  assignDefineGeometryTarget(GeomStateTarget& target) { connectTargetToParent(target, usg::EngineTokens.target.defineGeometry); }
    void  assignModifyValuesTarget(GeomStateTarget& target)   { connectTargetToParent(target, usg::EngineTokens.target.modifyValues); }
    void  assignEditStageTarget(GeomStateTarget& target)      { connectTargetToParent(target, usg::EngineTokens.target.editStage); }


    //! Retrieve a named change target. Not thread safe.
    GeomStateTarget*  getTarget(const usg::Token& target_id) const;

    /*! Retrieve the set of change targets assigned to this engine.
        Not thread safe.
    */
    const GeomStateTargetPtrMap&  targets() const { return _changeTargetMap; }


    //! Pre-assigned target that affects GeomState::defineGeometryState
    const GeomStateTarget&  defineGeometryTarget() const { return _defineGeometryTarget; }

    //! Pre-assigned target that affects GeomState::modifyValuesState
    const GeomStateTarget&  modifyValuesTarget() const { return _modifyValuesTarget; }

    //! Pre-assigned target that affects GeomState::editStageState
    const GeomStateTarget&  editStageTarget() const { return _editStageTarget; }

    //! Pre-assigned target that affects GeomState::affectsStereoState
    const GeomStateTarget&  affectsSteroTarget() const { return _affectsStereoTarget; }


    //----------------------------------------------------------------------
    // Scenegraph State
    //----------------------------------------------------------------------


    //! Read access to geometry states calculated in validateGeomState().
    const GeomState&  getInputState()  const { return _inputState; }
    const GeomState&  getLocalState()  const { return _localState; }
    const GeomState&  getOutputState() const { return _outputState; }

    /*! Does the GeomEngine need to re-process?
        This could be because the inputs GeomState changed or the local GeomState changed.
        Note that this only indicates that the engine -may- need to reprocess.
        Check the other dirty indicators to determine what specifically needs updating.
    */
    bool  isDirty() const;

    bool  isDefineGeometryStateDirty() const;
    bool  isModifyValuesStateDirty() const;
    bool  isEditStageStateDirty() const;

    //! Input GeomEngine(s) have changed.
    bool  isInputStateDirty() const;
    //! Input GeomEngine's define-geometry state has changed.
    bool  isInputDefineGeometryStateDirty() const;
    //! Input GeomEngine's modify-values state has changed.
    bool  isInputModifyValuesStateDirty() const;
    //! Input GeomEngine's edit-stage state has changed.
    bool  isInputEditStageStateDirty() const;

    //! This GeomEngine's local state has changed.
    bool  isLocalStateDirty() const;
    //! This GeomEngine's local Layer define-geometry state has changed.
    bool  isLocalDefineGeometryStateDirty() const;
    //! This GeomEngine's local Layer modify-values state has changed.
    bool  isLocalModifyValuesStateDirty() const;
    //! This GeomEngine's local edit-stage state has changed.
    bool  isLocalEditStageStateDirty() const;

    //----------------------------------------------------------------------

    //! Return true if the output state != the input state.
    //! Does not indicate if the engine needs to process the scene, only that the engine will
    //! be run when dirty.
    bool  mayAffectScenegraph() const;
    //! Return true if the output define-geometry state != the input define-geometry state.
    //! Does not indicate if the engine needs to process the scene, only that the engine will
    //! be run when dirty.
    bool  mayAffectComposedStage() const;
    //! Return true if the output modify-values state != the input modify-values state.
    //! Does not indicate if the engine needs to process the scene, only that the engine will
    //! be run when dirty.
    bool  mayModifyAttributeValues() const;
    //! Return true if the output edit-stage state != the input edit-stage state.
    //! Does not indicate if the engine needs to process the scene, only that the engine will
    //! be run when dirty.
    bool  mayEditStage() const;

    //----------------------------------------------------------------------

    /*! Returns the nominal processing mode the engine should handle in processScenegraph()
        based on the engine's current dirty state flags and what state targets the engine
        has declared it wants to handle.
        This value is assigned to the GeomSceneContext before processScenegraph() is called.
    */
    EngineMode  nominalProcessingMode() const;



    //----------------------------------------------------------------------
    // Request / Configuration State
    //----------------------------------------------------------------------


    /*! Rounds the time value to three decimal places (ie .001, a thousandth of a frame)
        to make it more useful as a key value for things like std::map, etc.
    */
    static fdk::TimeValue  trimFrame(const fdk::TimeValue& time);

    /*! The 'request' ArgSet can be used during engine evaluation to influence
        its behavior, typically set by the system managing the GeomEngine network.
        The args are not used by the GeomEngine base class.
    */
    const usg::ArgSet&  requestArgs() const { return _requestArgs; }
    //! Add request args to the engine.
    void                addRequestArgs(const usg::ArgSet& args);
    //! Empty the arg set.
    void                clearRequestArgs() { _requestArgs.clear(); }


    //----------------------------------------------------------------------
    // Stage Access
    //----------------------------------------------------------------------

    //! Returns the evaluated stage  from\a input.. If \a composeStage is true, the stage is
    //! fully composed. You can set this to false if you don't care about the stage but simply want
    //! the input engines and their edit layers to be brought up to date.
    //! Note that if multiple inputs lead back to the same stage via a split in the graph, you may get
    //! the same stage back for these inputs, the second call modifying the stage
    //! you got back from the first call, so you should be careful to deal gracefully with this possibility.
    virtual usg::StageRef  buildStageFromInput(const GeomSceneContext& context,
                                               int32_t           input,
                                               bool composeStage = true);

    //! Subclasses can override this to return true if they need a new stage to be created at this engine.
    virtual bool needsNewStage();

    //! Subclasses can override this to return a new stage. This is generally only required for an engine which
    //! needs to set the asset resolver context.
    virtual usg::StageRef createStage();

    //----------------------------------------------------------------------
    // Scenegraph Processing
    //----------------------------------------------------------------------

    /*! Return the edit layer for the engine. Base class returns the local \a _editLayer var.
        This layer is where all local prim/attribute defines & edits are stored.
    */
    usg::LayerRef  editLayer();

    /*! Return the root layer for the engine. Base class returns the local \a _rootLayer var.
        If there are local edits present in the \a _editLayer after processScenegraph() has
        run the \a edit_layer will be inserted into \a _rootLayer as the highest-priority
        sublayer (layer 0.)
    */
    usg::LayerRef  rootLayer();

    /*! Called in buildScenegraph() when defineGeometryState is dirty, to clear out _editLayer
        in preparation for defining prims, attribs. A subclass can implement this to perform
        custom initialization if the default clearing of _editLayer is not desired, such as
        an importer which caches the input layer, or to defer clearing until processScenegraph()
        is called.

        Note that clearing a Layer's content can cause a global stage change notification since
        it may force a stage recomposition. To avoid this an engine should also pay attention
        to the modifyValuesTarget() state, which indicates when the attribute's contents
        should be cleared vs. the attribute's definition.

        Base class implementation calls _editLayer->clear().
        GeomSceneContext is provided in case request scope args are required.
    */
    virtual void           initializeEditLayer(GeomSceneContext& context);


    /*! Called in buildScenegraph() when defineGeometryState is dirty, to clear out _rootLayer
        in preparation for defining prims, attribs, or layering operations. A subclass can
        implement this to perform custom initialization if the default clearing of _rootLayer
        is not desired, or to defer clearing until processScenegraph() is called.

        Note that clearing a Layer's content can cause a global stage change notification since
        it may force a stage recomposition. To avoid this an engine should also pay attention
        to the modifyValuesTarget() state, which indicates when the attribute's contents
        should be cleared vs. the attribute's definition.

        Base class implementation calls _rootLayer->clear().
        GeomSceneContext is provided in case request scope args are required.
    */
    virtual void           initializeRootLayer(GeomSceneContext& context);


    /*! Called in buildScenegraph() when modifyValuesState is dirty, to clear all currently
        authored properties (default values, time samples, relationships, etc) from _editLayer,
        while retaining their definitions, avoiding Stage path-change notifications and a
        possible recompose.

        Base class implementation calls _editLayer->clearAuthoredProperties(). A subclass can
        implement this to perform custom behavior such as deferring any work until
        processScenegraph() is called.      

        GeomSceneContext is provided in case request scope args are required.
    */
    virtual void           clearEditLayerAuthoredProperties(GeomSceneContext& context);

    /*! Called in buildScenegraph() when modifyValuesState is dirty, to clear all currently
        authored properties (default values, time samples, relationships, etc) from _rootLayer,
        while retaining their definitions, avoiding Stage path-change notifications and a
        possible recompose.

        Base class implementation calls _rootLayer->clearAuthoredProperties(). A subclass can
        implement this to perform custom behavior such as deferring any work until
        processScenegraph() is called.      

        GeomSceneContext is provided in case request scope args are required.
    */
    virtual void           clearRootLayerAuthoredProperties(GeomSceneContext& context);



    //! Returns the active stage layer. This is the layer on the stage that the edit layer will be copied to after processing.
    LayerRef activeStageLayer(const GeomSceneContext& context) const;

    //! Returns a stage set up for use in processScenegraph. Engines can use this to make changes to the stage using
    //! raw UsdStage APIs (for example via Python). If an engine calls this method, it must not make any changes to the
    //! edit layer. All changes must be made via the returned stage.
    StageRef editableStage(const GeomSceneContext& context);

    /*! Calculate the input and output GeomStates and update the dirty status
        of the GeomEngine.
        A subclass can manipulate the state vars in a _validateGeomState()
        implementation.
    */
    void          validateGeomState();


    /*! Current processing version for this engine. This number is incremented
        each time this engine runs processScenegraph() and reset to zero when
        the define or modify states are dirty.

        A change in this number is a good indicator that a modification may
        have been made to a Layer or Stage attribute value contents.
    */
    int32_t       processedVersion() const { return _processedVersion; }

    /*! Returns the processing state of the engine which can be used to track if
        processing changes may have been applied to the engine's layers or stage.
        This hash is a combination of the outputState and the current processed
        version number.
    */
    fdk::Hash     getProcessedState() const;


    //! Concatenated set of times passed to buildScenegraph() - used for cache testing
    const TimeGeomStateMap&  processedTimes() const { return _processedTimes; }

    /*! Add a single processed time to the \a _processedTimes set. If \a state is
        provided then its value is stored otherwise the engine's \a _outputState is.
    */
    void          addProcessedTime(const fdk::TimeValue& time,
                                   const GeomState*      state = nullptr);

    //! Has the engine processed this time already?
    bool          hasProcessedTime(const fdk::TimeValue& time) const;

    /*! Retrieve the GeomState of the processed time, returning true if there was
        an existing GeomState entry for the time.
    */
    bool          getProcessedTime(const fdk::TimeValue& time,
                                   GeomState&            state) const;

    /*! Returns the set of TimeValues to process for this GeomEngine, which is the
        TimeValues from the \a times set minus the ones in the \a _processedTimes
        that match the engine's current \a _outputState.

        In other words this method removes the already processed times from the
        passed-in set.
    */
    fdk::TimeValueSet  getActiveProcessingTimes(const fdk::TimeValueSet& times) const;
    //! Version that takes a time/GeomState map.
    TimeGeomStateMap   getActiveProcessingTimes(const TimeGeomStateMap& times) const;

    /*! Same as getActiveProcessingTimes() but returns true if all times in \a times
        are already in _processedTimes.
    */
    bool          hasProcessedTimes(const fdk::TimeValueSet& times) const;
    //! Version that takes a time/GeomState map.
    bool          hasProcessedTimes(const TimeGeomStateMap& times) const;

    //! Add times to the processed set.
    void          addProcessedTimes(const fdk::TimeValueSet& times);
    //! Add times and GeomStates to the processed set.
    void          addProcessedTimes(const TimeGeomStateMap& times);

    //! Clear all the processed times.
    void          clearProcessedTimes();


    /*! Process/build the scene, filling in any Layers this GeomEngine owns
        and updating the output Stage contained in the GeomSceneContext
        \a context.

        This method examines the dirty states of the GeomEngine to determine
        whether to call the virtual processScenegraph() method so the engine
        subclass can perform any work defined by the dirty state.

        When this method completes the engine's dirty state is cleared and
        the 'last' GeomStates updated to the current ones.

        If \a composeStage is true the stage's root sublayer stack will
        be updated to this engine likely causing the stage to recompose if
        the stack is different than the one current in it. This is useful
        when an engine needs its input results composed so it can query
        before doing additional scenegraph processing.
    */
    StageRef      buildScenegraph(GeomSceneContext& sceneCtx);

    /*! Called in processScenegraph when the engine is dirty due to any reason.
        We double check that the dirty state is in fact different from the state on the stage, if so
        we re-copy layers,  this is for both composition and non composition changes, and we leave
        it to the copyLayers method to handle these cases. Specifically it sets
        the stage root layer's sublayer stack which ultimately determines the stage's
        composition state, and thus calling this method will usually cause a full stage
        re-composition.

        Default implementation checks the engine's current sublayer state hash against the
        stage's current root layer state hash and only updates the stack if they differ.
        This can be overriden by passing \a force = true.
        If updating is needed the stage's root layer is updated with the engine's current sublayer
        list and the stage's root state hash updated to the engine's current sublayer state hash.


        A subclass can override this to perform any additional work related to setting
        the root state.
    */
    virtual void  updateStageRootSublayers(usg::GeomSceneContext& context,
                                           bool                   force = false);

    /*! Directly runs processScenegraph() for all TimeValues in \a times, which
        is used instead of the times in \a src_context. A new GeomSceneContext
        is constructed with the new TimeValueSet and passed to processScenegraph().
        After processing the times are added to the current processed set.

        This method does not test any states and is equivalent to calling the
        following code:
          GeomSceneContext new_context(src_context, times);
          processScenegraph(new_context);
          addProcessedTimes(times);
    */
    void          processScenegraphForTimes(const GeomSceneContext&  src_context,
                                            const fdk::TimeValueSet& times);

    /*! Merges the layers from \a inputs into the \a _rootLayer.
        These are merged in reverse order.
    */
    virtual void  mergeFromInputs(GeomSceneContext&           context,
                                  const std::vector<int32_t>& inputs);

    //! Gets the modified paths list for this engine.
    const usg::PathArray& getModifiedPaths() const;

    //! Sets the modified paths list for this engine.
    void setModifiedPaths(const usg::PathArray& lastModified);

    //! Gets the created paths list for this engine.
    const usg::PathArray& getCreatedPaths() const;

    //! Sets the created paths list for this engine.
    void setCreatedPaths(const usg::PathArray& lastModified);

    //! Returns a possible path to be used as the default parent prim for downstream engines
    const usg::Path& getDefaultParentPath() const;

    //! Sets the default parent path
    void setDefaultParentPath(const usg::Path& path);

    //! Host application has requested that the engine should abort its processing
    virtual bool aborted() const;

    //----------------------------------------------------------------------


    //! Returns the output stage for this engine. This does not bring the stage up to date; use buildScenegraph to evaluate the stage.
    usg::StageRef&        outputStage();
    const usg::StageRef&  outputStage() const;

    //! Returns the input stage for \a input. This does not bring the stage up to date; use buildStageFromInput to evaluate the stage.
    const usg::StageRef inputStage(int input) const;

    //! Returns the layer on the output stage that corresponds to the edit layer at the last evaluation
    usg::LayerRef outputStageLayer();

    /*! Return the companion arbitrary output data for this engine.
        Base class returns the local \a _sceneOutputData var.
    */
    virtual usg::GeomSceneOutput*  sceneOutputData();


    //! Returns the currently executing GeomEngine (if any). This should only be used for error reporting.
    static GeomEngine* ExecutingEngine();
    //! Sets the currently executing GeomEngine, returning the previous one. This should only be used for error reporting.
    static GeomEngine* SetExecutingEngine(GeomEngine* engine);

    /*! Unwind the stage for the engine called before processScenegraph()
        is called.
    */
    virtual void unwindStage(const GeomSceneContext& /*context*/) {}

    //----------------------------------------------------------------------


  protected:
    /*! Subclass portion of validateGeomState().

        GeomEngine subclasses can mess with the local GeomState here to take
        into account input parameters, internal calculations, etc.

        Default implementation does nothing.
    */
    virtual void         _validateGeomState() {}


    /*! Subclass-specific engine implementation called by buildScenegraph().

        Default implementation gets the root layer from inputEngine0() by
        calling buildScenegraph() on it, then copies the SubLayer paths from
        the root layer.
    */
    virtual void         processScenegraph(GeomSceneContext& context);


    /*! Subclass-specific engine implementation called by buildScenegraph(),
        after processScenegraph(), and only if affectsComposedStage()==true.

        Default implementation does nothing.
    */
    virtual void editComposedStage(GeomSceneContext& context);


  protected:
    friend class GeomEngineImpl;
    std::unique_ptr<class GeomEngineImpl> _pImpl;

    //----------------------------------------------------------------------
    // Local cache data
    usg::LayerRef    _rootLayer;            //!< Cached local root Layer for input layer stack
    usg::LayerRef    _editLayer;            //!< Cached local edit Layer

    //----------------------------------------------------------------------
    // Current/pending state
    GeomState        _inputState;           //!< Concatenation of all input GeomEngine output states
    GeomState        _localState;           //!< Local changes to geometry
    GeomState        _outputState;          //!< Concatenated state includes input and local states
    usg::ArgSet      _requestArgs;          //!< Provides additional request context for _validateGeomState()
    int32_t          _processedVersion;     //!< Incremented each time this engine runs processScenegraph()
    TimeGeomStateMap _processedTimes;       //!< Which times have been processed since the last dirty define-prims pass

    //----------------------------------------------------------------------
    // Previous state to detect change requirements, updated after buildScenegraph() has been called:
    GeomState        _lastInputState;       //!< Concatenation of all previous input GeomEngine output states
    GeomState        _lastLocalState;       //!< Previous local changes to geometry
    GeomState        _lastOutputState;      //!< Concatenated state includes previous input and local states

    //----------------------------------------------------------------------
    // Change targets:
    GeomStateTargetPtrMap _changeTargetMap; //!< What the engine intends to affect

    // Pre-defined change targets bound to hashes in GeomState:
    GeomStateTarget  _defineGeometryTarget; //!< Pre-assigned target that affects GeomState::defineGeometryState
    GeomStateTarget  _modifyValuesTarget;   //!< Pre-assigned target that affects GeomState::modifyValuesState
    GeomStateTarget  _editStageTarget;      //!< Pre-assigned target that affects GeomState::editStageState
    GeomStateTarget  _affectsStereoTarget;  //!< Pre-assigned target that affects GeomState::affectsStereoState

    usg::GeomSceneOutput* _sceneOutputData;   //!< Optional engine-specific output arbitrary scene data
  };



  //--------------------------------------------------------------------------------
  // Simple utility class to print debug spaces
  struct USG_ENGINE_API printSpaces
  {
    void indent2()  { nSpaces += 2; }
    void indent3()  { nSpaces += 2; }
    void indent4()  { nSpaces += 4; }
    void outdent2() { nSpaces -= 2; }
    void outdent3() { nSpaces -= 2; }
    void outdent4() { nSpaces -= 4; }
    printSpaces& operator += (int n) { nSpaces += n; return *this; }
    printSpaces& operator -= (int n) { nSpaces -= n; return *this; }
    printSpaces() : nSpaces(0) {}
    int nSpaces;
  };

  USG_ENGINE_API std::ostream& operator << (std::ostream& o, const printSpaces& b);

} // namespace usg


#endif
