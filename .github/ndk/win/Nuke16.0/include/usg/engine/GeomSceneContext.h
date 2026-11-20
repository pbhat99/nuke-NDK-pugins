// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_ENGINE_GEOMSCENECONTEXT_H
#define USG_ENGINE_GEOMSCENECONTEXT_H

#include "usg/engine/api.h"
#include "usg/engine/GeomState.h"

#include "fdk/base/Time.h"
#include "usg/base/ArgSet.h"

#include "usg/geom/Stage.h"


namespace usg
{

  //-----------------------------------------------------------------------------


  //! Global engine processing modes in lowest-priority order.
  enum EngineMode
  {
    None,                 //!< Nothing is dirty and no times need processing
    EditSceneState,       //!< Edit the scene's state (stage metadata editing, etc)
    //
    ModifyProperties,     //!< Add property time samples, if required (definitions already exist)
    InitializeProperties, //!< Assign default values to emptied/cleared properties (definitions already exist)
    //
    DefineGeometry,       //!< Prim and property definitions should be created (stage recomposition)
  };


  //-----------------------------------------------------------------------------


  /*! Context for GeomEngine getScenegraph(), processScenegraph() to fill in the
      scenegraph Stage the scene consumer wants.

      A GeomEngine 

      This class also contains an arbitrary set of scene-global key/value attributes
      that can help guide or define behaviors. Some are scoping parameters set by
      downstream GeomEngines to be passed upstream to guide the creation of prims or
      materials.
  */
  class USG_ENGINE_API GeomSceneContext
  {
  public:
    //! Default ctor creates a single default TimeValue to process.
    GeomSceneContext();

    //! Build the context for a stage and set the process times and scope vars.
    GeomSceneContext(const usg::StageRef&     stage,
                     const fdk::TimeValueSet& times,
                     const ArgSet&            scope_vars = ArgSet());

    //! Copies the second context while updating the StageRef.
    GeomSceneContext(const GeomSceneContext&  b,
                     const usg::StageRef&     new_stage);

    //! Copies stage and mode from a second context and either replaces or adds to the
    //! process times. Note this does not copy the scope vars.
    GeomSceneContext(const GeomSceneContext&  b,
                     const fdk::TimeValueSet& times,
                     bool                     append_times = false);

    //-------------------------------------------------------------

    //! Additional request context scope vars for upstream GeomEngines to use.
    const ArgSet&  requestScopeVars() const { return _requestScopeVars; }

    //! Overwrites the current request context scope vars.
    void           setRequestScopeVars(const ArgSet& vars) { _requestScopeVars = vars; }

    //-------------------------------------------------------------

    //! The current usg::Stage being built.
    //! If \a processMode() is \a DefineGeometry then the Stage will be composed at some
    //! point in the engine graph, usually the bottom.
    const usg::StageRef&  stage() const { return _stage; }

    //-------------------------------------------------------------

    //! The nominal processing mode the engine should handle in processScenegraph().
    //! This is determined by the current engine dirty state flags and what state targets
    //! the engine has declared it wants to handle.
    //!
    //! If there are multiple enabled dirty flags the highest priority mode is used. For
    //! example if \a define-geometry and \a modify-values are both true then the mode
    //! will be \a EngineMode::DefineGeometry.
    //!
    //! An engine can further examine its dirty states if more processing context is
    //! needed.
    EngineMode  processMode() const { return _processMode; }

    //! Should the engine make any changes to layer/prim/property contents.
    bool        doGeometryProcessing() const { return (_processMode >= EngineMode::ModifyProperties); }

    //! Should the engine define or initialize layer/prim/property contents.
    bool        doGeometryInitialization() const { return (_processMode >= EngineMode::InitializeProperties); }

    //! Should the engine add values to properties.
    bool        doModifyPropertyValues() const { return (initializeProperties() || modifyProperties()); }

    //-------------------------------------------------------------

    //! Should the engine define Layer contents.
    bool        defineGeometry() const { return (_processMode == EngineMode::DefineGeometry); }

    //! Should the engine just initialize property defaults. This should happen after properties
    //! are defined or the properties have been cleared due to a parameter change.
    //!
    //! An engine must be prepared to re-assign these property defaults whenever the engine modify-values
    //! state becomes dirty as the contents of all properties get cleared.
    bool        initializeProperties() const { return (_processMode == EngineMode::InitializeProperties); }

    //! Should the engine just modify property contents, typically because more time
    //! samples have been requested. If the engine does not create time samples this
    //! mode can be ignored.
    bool        modifyProperties() const { return (_processMode == EngineMode::ModifyProperties); }

    //! Should the engine affect the composed Stage dynamically - ie apply stage edits.
    //! These changes are intended to be transitory/temporary and not exported to a .usd file.
    //!
    //! An example of this would be for extra stage metadata used internally to affect the Hydra
    //! delegates, etc, or to manipulate any non-Stage data in the GeomSceneOutput.
    //!
    //! This is effectively the same as checking mayEditStage() since even after
    //! the first processing pass we want to know if the engine -intends- to edit
    //! stage values on each subsequent process pass.
    bool        editStageState() const { return (_processMode == EngineMode::EditSceneState); }

    //-------------------------------------------------------------

    //! The set of time values the engine should process at, typically to add new time
    //! samples to any animating properties. If the engine does not produce any time
    //! samples this can be ignored.
    //!
    //! The engine is not restricted to processing and creating just these times, however
    //! if the engine is creating time-sampled properties then at a minimum it should
    //! create them for these times.
    //!
    //! Important note - an engine can be repeatedly called by downstream consumers to
    //! process additional times in any random order, so engines should determine if
    //!   a) do they need to create property time samples, and
    //!   b) do so in an efficient way so handling additional times is as cheap as possible.
    //! For example an engine should avoid calculating the topology of a mesh for every
    //! process time request, especially if the mesh is not animating.
    const fdk::TimeValueSet&  processTimes() const { return _processTimes; }

    //! Returns the minimum number of times to process, requested by downstream consumers.
    size_t                    numProcessTimes() const { return _processTimes.size(); }

    //-------------------------------------------------------------

    friend USG_ENGINE_API std::ostream& operator << (std::ostream&, const GeomSceneContext&);


  protected:
    friend class GeomEngine; // so GeomEngine can directly manipulate

    usg::StageRef      _stage;              //!< The stage we're building
    EngineMode         _processMode;        //!< Nominal engine processing mode to use
    fdk::TimeValueSet  _processTimes;       //!< Times to process the scene at
    ArgSet             _requestScopeVars;   //!< Provides additional context for upstream GeomEngines
  };


  USG_ENGINE_API std::ostream& operator << (std::ostream&, const GeomSceneContext&);

} // namespace usg

#endif
