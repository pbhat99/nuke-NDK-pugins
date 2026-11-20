// Copyright (c) 2025 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_XFORMABLESCENEOPI_H
#define DDIMAGE_XFORMABLESCENEOPI_H

#include "DDImage/Op.h"
#include "DDImage/ConstrainOpI.h"
#include "DDImage/KnobMixIns.h"
#include "DDImage/AxisVals.h"
#include "DDImage/Authoring.h"
#include "DDImage/ArrayKnobI.h"
#include "DDImage/GeometryProviderI.h"

#include "ndk/geo/camera/Camera.h" // for XformParameters

#include "usg/engine/GeomState.h"

#include <variant>
#include <tuple>

namespace usg {
  class XformablePrim;
}

namespace ndk {
  class SceneOpNode;
}

namespace DD {
  namespace Image {
    class AxisOp;
    class AxisSceneOp;
    class CameraSceneOp;
    class LightSceneOp;
    class SceneOpI;
    class WorldMatrixProvider;

    //! Round off value to two decimal places
    inline double RoundToTwoDecimalPlaces(double v) { return std::round(v * 100.0) / 100.0; }

    //! Round off value to three decimal places
    inline double RoundToThreeDecimalPlaces(double v) { return std::round(v * 1000.0) / 1000.0; }

    //! Round off value to four decimal places
    inline double RoundToFourDecimalPlaces(double v) { return std::round(v * 10000.0) / 10000.0; }

    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------

    /*! \class DD::Image::AuthoringProvider

        Uses a component identifier to specify which part of the schema/Op is to be provided.
        Using an identifier is easier to manage than making individual provider subclasses
        for each component provider.
    */
    class DDImage_API AuthoringProvider : public Authoring
                                        , public ValueProvider
    {
    public:
      //! Assign the component id, whos meaning is determined by the subclass.
      //! This can be an enum or a hash from a Token.
      AuthoringProvider(Knob* knob, uint64_t componentId);

      //---------------------------------------------------

      //! True when the knob is not in authoring set mode and there is a source Xformable prim.
      bool  provideValuesEnabled(const Knob* knob, const OutputContext&) const override;
      int   providerSourceType(const Knob* knob, const OutputContext&) const override;
      bool  isDefault(const Knob*, const OutputContext&) const override;
      bool  isAnimated(const Knob* knob, const OutputContext&) const override;

      //! Returns the values to be displayed in the output knob.
      //! First checks if the scene data evaluator needs to be run, then calls the subclass _provideValues().
      void  provideValues(double* values, size_t nValues, const ArrayKnobI* arrayKnob, const OutputContext&) const override;
#if kDDImageVersionMajorNum <= 17
      std::vector<double> provideValues(const ArrayKnobI* arrayKnob, const OutputContext&) const override;
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 17."
#endif

      //---------------------------------------------------

      //! Convenience to extract a value from one sample.
      template<typename T>
      static void  CopyValue(const T* valIn, double* valOut);

      //! Convenience to extract a value from two samples, with interpolation support.
      template<typename T>
      static void  CopyValue(const T* valInA, const T* valInB, float t, double* valOut);

      ~AuthoringProvider() override {}

    protected:
      friend class ndk::SceneOpNode;
      uint64_t        _componentId;   //!< Which component to provide
      usg::Attribute  _sourceAttrib;  //!< Primary attribute to provide data from

      //! Return the scene op owner of this provider. Must implement.
      virtual SceneOpI*  _sceneOpI() const = 0;

      //! Update any local parameters from the source prim. Version number changes when
      //! the source scene changes.
      virtual void  _update(bool enable, const usg::Prim& prim, uint64_t version) {}

      //! Subclass can enable or disable depending on component. Must implement.
      virtual bool  _isProviderEnabled() const = 0;

      //! Fill \a values with the appropriate default for the component. Must implement.
      virtual void  _getDefault(double* values, size_t nValues) const = 0;

      //! Return true if the source component is at its default.
      virtual bool  _isDefault() const = 0;

      //! Return true if the source component is an animating attribute.
      virtual bool  _isAnimated() const = 0;

      //! Subclass implemention of provideValues(). Must implement.
      virtual bool  _provideValues(double* values, size_t nValues, const ArrayKnobI* arrayKnob, const OutputContext&) const = 0;
    };

    using AuthoringProviderKnobMap = std::unordered_map<Knob*, AuthoringProvider*>;


    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------


    /*! \class DD::Image::SceneOpNodeI
    */
    class DDImage_API SceneOpNodeI
    {
    public:
      //! Node configuration data (from knobs) - shared between clones.
      struct ConfigParams
      {
        bool       readFromSceneEnabled;  //!< Read live data from prim
        bool       fileModeEnabled;       //!< Read prim data from a file source
        usg::Path  sourcePrimPath;        //!< Prim to read from source scene
        //
        bool       eulerFilterEnable;     //!< Enable euler filter on decomposed rotations
        bool       parentExtractEnable;   //!< Extract parent xform
        bool       localExtractEnable;    //!< Extract local xform
        bool       parentCombineEnable;   //!< Combine the parent and local xform, disable parent xform extract
        //
        usg::Path  primPath;              //!< Prim to create
      };

      //! Derived per-Node data and shared by all Ops owned by it - NOT shared between clones.
      //! Similar to the data stored in a GeomEngine.
      struct SceneData
      {
        fdk::Hash       sourceSceneState;   //!< State from last extractSceneData() run
        fdk::TimeValue  firstKey;           //!< First available key in source scene (may be default time)
        fdk::TimeValue  lastKey;            //!< Last available key in source scene (may be default time or same as firstKey)
        AxisValsSet     axisValsSet;        //!< Per-frame decomposed & filtered transform cache (animates if size > 1)
        bool            isDefaultXforms;    //!< True if AxisValsSet is comprised of all default transforms
      };

      //! Data stored in SceneOpNode and shared by all Ops owned by it. Similar to the data stored
      //! in a GeomEngine.
      struct DDImage_API SharedData
      {
        ConfigParams  params;       //!< Control params driven by Node knobs - shared between clones
        SceneData     sceneData;    //!< Per-node derived data - shared between Ops
 
        //! Copy the parameters from src but not the constructed values (editLayer, axisValsSet, etc).
        //! Used for updating clones.
        virtual void copyParamsFrom(const SharedData& src);
      };

      struct SceneOpKnobs {
        Knob* readFromSceneKnob;
        Knob* sourcePathKnob;
        //
        Knob* fileEnabledKnob;
        Knob* filePathKnob;
        //
        Knob* decomposeEulerFilterEnableKnob;
        //
        Knob* parentExtractEnableKnob;
        Knob* localExtractEnableKnob;
        Knob* parentCombineEnableKnob;
        //
        Knob* pathKnob;
      };

      //----------------------------------------

      SceneOpNodeI();
      virtual ~SceneOpNodeI() {}

      //! Name of the Node description to use for constructor.
      static const char* descriptionName() { return "SceneOpNode"; }

      //----------------------------------------

      //! Add prim import control knobs.
      virtual void  addImportOptionKnobs(Knob_Callback) = 0;

      //! Add transform decomposition control knobs.
      virtual void  addXformExtractOptionKnobs(Knob_Callback) = 0;

      virtual void  addLocalParentXformKnobs(Knob_Callback f, bool addSyncOptions) = 0;

      virtual void  addLocalXformKnobs(Knob_Callback f, bool addSyncOptions) = 0;

      //! Add prim export control knobs.
      virtual void  addExportOptionKnobs(Knob_Callback) = 0;

      //! Create link knobs for the knobs "live_source" and "source_prim_path".
      //!
      //! This creates link knobs with a custom suffix for additional tabs, allowing
      //! the user to choose a prim from there. The knobs will be created using the
      //! names "live_source" and "source_prim_path" with the suffix argument appended.
      virtual void createLinkedImportKnobs(Knob_Callback, const std::string& suffix) = 0;

      //! Reset the state targets in preparation for a Knob store cycle.
      virtual void  resetTargetState() = 0;

      //! Fast check if the scene node needs to rebuild its states. This is cheap to call
      //! for repeat visits.  Returns false if no change.
      //! If \a forceUpdate is true then the states will always be rebuilt.
      virtual bool  validateState(bool forceUpdate = false) = 0;

      //! Update the output geometry states. \a editVersionHash should come from the owner Op.
      virtual void  updateGeomState(const DD::Image::Hash& editVersionHash) = 0;

      virtual void  appendToDefineGeometryTarget(const fdk::Hash& versionHash) = 0;
      virtual void  appendToModifyValuesTarget(const fdk::Hash& versionHash, bool isAnimatingSource = false) = 0;

      //! Update the source scene data
      virtual void  updateSourceSceneData(SceneOpI* op, const OutputContext& opContext) = 0;

      //! Determine the source scene state hash which will cause extractSceneData() to be run
      //! when it changes.
      //! Subclasses can implement this to take any additional values into consideration when
      //! determining the source scene state, such as knobs.
      //! This state hash is stored in \a SceneData.sourceSceneState after extractSceneData() is run.
      //! Must implement.
      virtual fdk::Hash  buildSourceSceneState(SceneOpI* op, const fdk::Hash& hash) = 0;

      //!
      virtual void  processGeometryLayer(bool appendTo, const fdk::TimeValueSet& sampleTimes) = 0;

      //----------------------------------------

      //! Returns the shared knobs from the node.
      virtual const SceneOpKnobs&  opKnobs() const = 0;

      //! Returns the shared node data.
      virtual const SharedData&  sharedData() const = 0;

      //! Source stage the prim data will be extracted from, if valid.
      virtual const usg::StageRef&  sourceStage() const = 0;

      //! Prim that data will be extracted from, if valid.
      virtual const usg::Prim&  sourcePrim() const = 0;

      //! Cache of extracted transforms from the source prims, non-null if valid and non-empty.
      //! If set size is 1 then there is no animation.
      const AxisValsSet*  transformCache() const;

      //! Prim type to use when filling in parent prims.
      virtual const usg::Token&  parentPrimCreateType() const = 0;

      //! Returns the shared Layer containing the output scene data.
      virtual usg::LayerRef&  editLayer() = 0;

      //! Output geometry states representing state of editLayer.
      virtual const usg::GeomState&  outputGeomState() = 0;

      //! Output geometry has animating modifications.
      virtual bool  isVaryingModifier() = 0;

      //! Only valid after updateGeomState() has been called.
      virtual bool  isDefineGeometryStateDirty() const = 0;
      virtual bool  isModifyValuesStateDirty() const = 0;


    protected:
      friend class DD::Image::SceneOpI;

      //! Add a value provider to the shared list, which takes ownership of allocation.
      virtual void  _addAuthoringProvider(Knob* k, AuthoringProvider* provider) = 0;

      //! Retrieve a provider by Knob.
      virtual AuthoringProvider*  _getAuthoringProvider(Knob* k) = 0;

      //! Return the map of knobs that have authoring providers assigned.
      virtual const AuthoringProviderKnobMap&  _getAuthoringProviderKnobMap() const = 0;
    };


    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------


    /*! \class DD::Image::SceneOpI
        Provides the transformable scene Op standard transform knobs with prim edit
        functionality. This interface is only intended to be attached to AxisOp
        subclasses and there are a number of assumptions to that effect, such as the
        \a getAxisOp() method.

        This interface contains a local transform matrix and an optional local-parent
        transform matrix which are normally filled in by Knobs. The local-parent enables
        an arbitrary parenting hierarchy to be decomposed and contained within the AxisOp
        subclass allowing the node to be disconnected or disassociated from a parenting
        network or scenegraph. The node is thus effectively localized and can be copied,
        pasted into other scripts, etc.
    */
    class DDImage_API SceneOpI : public GeometryProviderI
                               , public ConstrainOpI
    {
    public:
      /*! \class DD::Image::SceneOpI::Description

          Specialized Op description class which requests the constructed SceneOp to be owned
          by a GeomOpNode rather than a generic OpNode, which allows the GeomOpNode to
          share GeomEngine(s) and the Stage stored inside it with all sibling GeomOps.
      */
      class DDImage_API Description : public Op::Description
      {
      public:
        //! Specialized Description which requests a SceneOpNode owner.
        //! 
        Description(const char* opClass, OpConstructor opCtor) :
          Op::Description(opClass, opCtor, SceneOpNodeI::descriptionName()) {}
      };

      //--------------------------------------------------------------------------------

      //! Which component of an AxisVals to provide.
      enum
      {
        TIME = 0,
        PARENT_T, PARENT_R, PARENT_S,
        XFORM_ORDER, ROT_ORDER,
        TRANSLATE, ROTATE, SCALE, UNIFORM_SCALE, SKEW, PIVOT_T, PIVOT_R,
        //
        LAST_XFORM_PARAM
      };

      // Extract transform values.
      class DDImage_API AxisValsProvider : public AuthoringProvider
      {
      public:
        //! Assigns the owner and component.
        AxisValsProvider(Knob* knob, SceneOpI* owner, uint64_t componentId);

        int animationTimes(std::set<double>& times, const OutputContext&) override;

        ~AxisValsProvider() override {}

      protected:
        SceneOpI*  _owner;
        SceneOpI*  _sceneOpI() const override { return _owner; }

        //!
        void  _update(bool enable, const usg::Prim& prim, uint64_t version) override;

        //! Parent TRS provider can disable depending on options.
        bool  _isProviderEnabled() const override;

        //! Fill scales with 1.0 and others with 0.0.
        void  _getDefault(double* values, size_t nValues) const override;

        //! Return true if the source component is at its default.
        bool  _isDefault() const override;

        //! Return true if the source component is an animating attribute.
        bool  _isAnimated() const override;

        //! Subclass implemention of provideValues().
        //! Default implementation extracts the value for the assigned AxisVals component.
        bool  _provideValues(double* values, size_t nValues, const ArrayKnobI* arrayKnob, const OutputContext&) const override;
      };

      //----------------------------------------------------------------------------

      //! Parent prim creation modes.
      enum
      {
        PARENT_XFORM,
        PARENT_SCOPE,
        PARENT_NONE
      };

      //----------------------------------------------------------------------------

    protected:
      ndk::XformComponents _xformComponents;  //!< Transform components
      //
      fdk::Vec3d  _localParentTranslate;    //!< Local-parent translation knob value
      fdk::Vec3d  _localParentRotate;       //!< Local-parent rotate knob value
      fdk::Vec3d  _localParentScale;        //!< Local-parent scale knob value
      //
      SnapKnobs   _snapKnobs;               //!< Snap menu options
      bool        _applyConstraintToParent; //!< Snap/constraint is applied to the local parent, not the local axis

      DD::Image::Hash _parentHash;          //!< Does parent path or transform need to be updated?
      DD::Image::Hash _constrainHash;       //!< The hash for the constrain input set in validate().

      WorldMatrixProvider* _worldXformProvider; //!< 'world_matrix' output knob - valid ONLY on firstOp()!
#if kDDImageVersionMajorNum <= 17
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
      bool  _selectable;                  //!< GUI selectable checkmark - deprecate or translate to new system?
      bool  _editable;                    //!< Can the user change the knobs? (i.e. knobs not disabled)
#endif

    public:
      SceneOpI(fdk::AxisDirection defaultOrientAxis = fdk::AxisDirection::Z_PLUS);
      virtual ~SceneOpI();

      //! Return the AxisOp subclass the interface is attached to. Should return 'this'.
      //! This can be further cast to an AxisSceneOp (or CameraSceneOp/LightSceneOp) using
      //! 'op_cast()' like so:
      //! 'AxisSceneOp* axisSceneOp = op_cast<AxisSceneOp*>(getAxisOp())'.
      //! Must implement.
      virtual AxisOp*  getAxisOp() = 0;

      //! Convenience to cast this interface's Op to an AxisSceneOp.
      AxisSceneOp*  asAxisSceneOp();

      //! Convenience to cast this interface's Op to a CameraSceneOp.
      CameraSceneOp*  asCameraSceneOp();

      //! Convenience to cast this interface's Op to a LightSceneOp.
      LightSceneOp*  asLightSceneOp();

      //-----------------------------------------------------------------

      //! Return the node input to use for the scene source, usually a GeomOp.
      //! -1 indicates an internal scene source with no input connection.
      //! Default implementation returns -1.
      virtual int32_t  sceneInput() const { return -1; }

      //! Return the sceneInput() input cast to a GeomOp, if possible. Default returns null.
      virtual GeomOp*    sceneInputOp() const { return nullptr; }

      //! Return the AxisOp::parentInput() input cast to a SceneOpI, if possible. Default returns null.
      virtual SceneOpI*  parentInputSceneOp() const { return nullptr; }

      //! Return the AxisOp::constrainInput() input cast to a SceneOpI, if possible. Default returns null.
      virtual SceneOpI*  constrainInputSceneOp() const { return nullptr; }

      //-----------------------------------------------------------------

      //! Return the custom node owner.
      SceneOpNodeI*  sceneOpNode() const { return _sceneOpNode; }

      //! Return the name of the primary USD schema this node is handling.
      //! For example \a usg::GeomTokens.types.Camera, or usg::lux::LuxTokens.types.DistantLight.
      //! Must implement.
      virtual const usg::Token&  primarySchemaType() const = 0;

      //! Returns a list of additional named schema APIs this node is handling.
      //! For example a LightSceneOp would likely apply \a usg::lux::LuxTokens.types.LightAPI and
      //! optionally \a usg::lux::LuxTokens.types.ShapingAPI.
      //!
      //! Default implementation returns an empty array.
      virtual usg::TokenArray  getAppliedApiSchemaTypes() const { return {}; }

      //! Prim path to create/edit
      virtual const usg::Path&  primPath() const { return sharedData().params.primPath; }

      //! Return the prim type to create for missing prims.
      //! Default implementation returns the value of the 'parent_prim_type' knob.
      virtual const usg::Token&  parentPrimCreateType() const { return _sceneOpNode->parentPrimCreateType(); }

      //! Return the full path of the -primary- prim and all its parents created by this node.
      //! This is the 'top' prim where any child prims are rooted under.
      //!
      //! For example if this is an AxisOp it's typically a single Xform prim, but a CameraOp
      //! or LightOp may have additional prims after the main Xform (ie for a stereo camera
      //! rig) so this connection point needs to be explicitly identified.
      //!
      //! Retrieve the base prim name by calling \a name() on the returned path.
      //!
      //! Base class implementation 
      virtual usg::Path  buildBasePrimPath() const;

      //! Return the default filter string to apply in the prim path knob.
      //! Default is a filter for Xforms.
      virtual const char*  primPathFilter() const { return "filter:\"Type:Xform\""; }

      //-----------------------------------------------------------------

      //! Is the read from scene mode enabled?
      bool  readFromSceneEnabled() const { return sharedData().params.readFromSceneEnabled; }

      //! Returns true if there's a valid scene input prim to extract data from.
      bool  sourcePrimEnabled() const
      {
        return _sceneOpNode->sourceStage() && _sceneOpNode->sourceStage()->getPrimAtPath(sourcePrimPath()).isValid();
      }

      //! Prim path in source scene to read.
      const usg::Path&  sourcePrimPath() const { return sharedData().params.sourcePrimPath; }

      //! Returns the shared node configuration params.
      const SceneOpNodeI::ConfigParams&  params() const { return sharedData().params; }

      //! Returns the shared node scene data.
      const SceneOpNodeI::SceneData&  sceneData() const { return sharedData().sceneData; }

      //! Stage the prim data will be extracted from, if valid.
      const usg::StageRef&  sourceStage() const { return _sceneOpNode->sourceStage(); }

      //! Prim that data will be extracted from, if valid.
      const usg::Prim&  sourcePrim() const { return _sceneOpNode->sourcePrim(); }

      //! Pointer to cache of extracted transforms, if valid and non-empty.
      //! The cache is updated by the SceneOpNode during Op configuration -prior- to
      //! Op::knobs(store-main) being called.
      const AxisValsSet*  transformCache() const { return _sceneOpNode->transformCache(); }

      //! Returns the shared node data.
      const SceneOpNodeI::SharedData&  sharedData() const { return _sceneOpNode->sharedData(); }


      //----------------------------------------------------------------------------
      // Knobs
      //----------------------------------------------------------------------------

      //! Assigns the AuthoringProvider to the last created or stored knob.
      void  SetAuthoringProvider(Knob_Callback f, uint64_t componentId);

      //! Same, but with a knob specifier for a compound knob to supply, if needed.
      void  SetAuthoringProvider(Knob_Callback f, Knob* knob, uint64_t componentId);

      //! Assigns AuthoringProviders to the last-created AxisKnob component knobs using
      //! the standard component Ids for local TRS.
      void  SetAxisKnobAuthoringProvider(Knob_Callback f);

      //! Add a value provider to the node shared list, which takes ownership of allocation.
      void  addAuthoringProvider(Knob* k, AuthoringProvider* provider);

      //! Retrieve an authoring provider by Knob.
      AuthoringProvider*  getAuthoringProvider(Knob* k) const;

      //! A map of Knobs that have authoring providers created for them. This can be
      //! used to globally turn the authoring controls or providers on an off without
      //! having to scan through the Knobs list on the Op.
      const AuthoringProviderKnobMap&  getAuthoringProviderKnobMap() const;

      //! Convenience to get a Knob from the Op this interface is attached to. Same as
      //! 'getAxisOp()->knob(name)'.
      Knob* opKnob(const char* name) const;

      //-----------------------------

      //! Add prim import control knobs.
      //! Default implementation calls _sceneOpNode->addImportOptionKnobs() which creates knobs
      //! during the f.makeKnobs() phase, it does not store into any Op vars.
      //! Access the knob data via the SharedData.
      virtual void  addImportOptionKnobs(Knob_Callback f) { _sceneOpNode->addImportOptionKnobs(f); }

      //! Add transform decomposition control knobs.
      //! Default implementation calls _sceneOpNode->addXformExtractOptionKnobs() which creates
      //! creates knobs during the f.makeKnobs() phase, it does not store into any Op vars.
      //! Access the knob data via the SharedData.
      virtual void  addXformExtractOptionKnobs(Knob_Callback f) { _sceneOpNode->addXformExtractOptionKnobs(f); }

      //! Add prim export control knobs.
      //! Default implementation calls _sceneOpNode->addExportOptionKnobs() which creates knobs
      //! during the f.makeKnobs() phase, it does not store into any Op vars.
      //! Access the knob data via the SharedData.
      virtual void  addExportOptionKnobs(Knob_Callback f) { _sceneOpNode->addExportOptionKnobs(f); }

      //! Adds the OpenGL display option controls.
      virtual void  addDisplayOptionsKnobs(Knob_Callback f);

      //! Add the local-parent TRS knobs. If \a addSyncOptions is true the
      //! import-sync options are added.
      virtual void  addLocalParentXformKnobs(Knob_Callback f,
                                             bool          addSyncOptions = true);

      //! Adds the front panel local-axis transform knobs. If \a addSyncOptions is
      //! true the import-sync options are added.
      virtual void  addLocalXformKnobs(Knob_Callback f,
                                       bool          addSyncOptions = true);

      //! Adds the front panel local-parent & local-axis transform knobs. If
      //! \a addSyncOptions is true the import-sync options are added.
      //! Calls addLocalParentingKnobs() then addLocalXformKnobs(), adding dividers.
      virtual void  addTransformKnobs(Knob_Callback f,
                                      bool          addSyncOptions = true);

      //! Adds the constrain option knobs.
      virtual void  addConstrainOptionsKnobs(Knob_Callback f);

      //! Adds additional front-panel knobs. Called before addTransformKnobs().
      //! Base class adds nothing.
      virtual void  addExtraFrontPanelKnobs(Knob_Callback f);

      //-----------------------------

      const char*  localParentTranslateKnobName() const;
      const char*  localParentRotateKnobName() const;
      const char*  localParentScaleKnobName() const;
      const char*  localAxisKnobName() const;
      const char*  worldMatrixKnobName() const;

      //! Return the internal 'local_parent_translate' XYZ knob.
      Knob*  getLocalParentTranslateKnob() const;

      //! Return the internal 'local_parent_rotate' XYZ knob.
      Knob*  getLocalParentRotateKnob() const;

      //! Return the internal 'local_parent_scale' XYZ knob.
      Knob*  getLocalParentScaleKnob() const;

      //! Return internal 'transform' Axis_Knob for set/get internal local transformation.
      Axis_KnobI*  getLocalAxisKnob() const;

      //! Return internal 'world_matrix' Array_Knob.
      ArrayKnobI*  getWorldMatrixKnob() const;

#if kDDImageVersionMajorNum <= 17
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
      //! Enable or disable axis knobs.
      //! \param on true to enable the axis knobs ('transform')
      void  enableAxisKnobs(bool on);

      //! Enable or disable knobs based on array of knobnames.
      //! \param knobNames knob names to enable or disable
      //! \param on true to enable the knobs
      void  enableKnobs(const std::vector<std::string>& knobNames, bool on);
#endif

      //! Make read only the knobs the translate/rotate/scale knobs for the local parent xform.
      virtual void  setReadOnlyLocalParent(bool readOnly);

      //! Make read only the knobs in the local xform section.
      virtual void  setReadOnlyLocalXform(bool readOnly);

      //! Returns the name of the local axis xformOp for the xform given by \a Op.
      static std::string localAxisXformOpName(const Op* op);

      //! Returns the name of the local parent xformOp for the xform given by \a Op.
      static std::string localParentXformOpName(const Op* op);

      //----------------------------------------------

      //! Use this method if the Knob's value will affect the output scenegraph topology by
      //! adding/removing Prims or Attributes, or by changing their names or paths in the
      //! output scenegraph.
      //!
      //! If the knob is just changing the contents of an existing attribute rather than
      //! explicitly creating that attribute, use KnobModifiesExportedAttribValues() instead.
      void  KnobDefinesGeometry(const Knob_Closure& f);

      //! Use this method if the Knob's value is changing the contents of an existing
      //! attribute in an output prim.
      void  KnobModifiesAttribValues(const Knob_Closure& f);


      //----------------------------------------------------------------------------
      // GeometryProviderI handling
      //----------------------------------------------------------------------------

      //! GeometryProviderI: Allow subclasses to gain access to sibling functions.
      GeometryProviderI*  asGeometryProvider() override { return this; }

      //! GeometryProviderI: Returns the compose state of the exportLayer.
      fdk::Hash  geometryComposeState(const ndk::NodeEvalContext& engineGraphContext) override;

      //! GeometryProviderI: Returns the edit state of the exportLayer.
      fdk::Hash  geometryEditVersionState(const ndk::NodeEvalContext& engineGraphContext) override;

      //! GeometryProviderI: Return true if this SceneOp varies in time, which could be from a
      //! constraint input or local transform knobs animating. Note that the parent Op connection
      //! is not considered since it does not affect this SceneOp's local transforms.
      bool  geometryStateVaries(const ndk::NodeEvalContext& engineGraphContext) override;

      //! GeometryProviderI: Provides a fully constructed usg::Stage via buildGeometryStage().
      bool  canProvideGeometryStage() const override { return true; }

      //! GeometryProviderI: Return true if the AxisOp this is attached to should provide
      //! geometry for the given purpose. SceneOps usually do not generate renderable geometry
      //! like meshes, so by default if purpose is \a usg::GeomTokens.overlay then return
      //! true, otherwise false.
      bool  canProvideGeometryFor(const usg::Token& purpose) const override;

      //! GeometryProviderI: Provide the cached exported stage. The cached stage is only created
      //! when this method is called. If the cached stage has the same composeState as the
      //! exporter then buildGeometryLayer() is called with any additional sample times but the
      //! cached stage itself is left untouched. \a stage is then set to the exporter's cached
      //! stage.
      void  buildGeometryStageForContext(usg::StageRef&              stage,
                                         const usg::ArgSet&          requestArgs,
                                         const ndk::NodeEvalContext& engineGraphContext,
                                         const fdk::TimeValueSet&    sampleTimes = fdk::TimeValueSet()) override;

      //---------------------------------------------------

      //! Write prims this SceneOp generates to an external layer, ignoring the SceneOp's state.
      //!
      //! This method is assumed to be called from a separate object which owns the Layer and
      //! is handling the allocation and clearing of the layer & clearing any attributes.
      //!
      //! \a layer is assumed to have been created, configured and valid. The list of sample
      //! times is a request to the SceneOp and is not guaranteed to result in those exact
      //! time samples in the output layer. For example a Knob may not be animated and will
      //! only set the default sample, or the keyframes being output are sparse.
      void  writeScenePrimsToLayer(usg::LayerRef&            layer,
                                   bool                      appendTo,
                                   const fdk::FrameValueSet& times);

      //---------------------------------------------------

#if kDDImageVersionMajorNum <= 17
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
      bool selectable() const { return _selectable; }
      void setSelectable(bool v) { _selectable = v; }

      bool editable() const { return _editable; }
      void setEditable(bool v) { _editable = v; }

      //! Return true if a particular knob transformation is modifiable by user interaction in gui.
      virtual bool isGuiInteractive(Axis_KnobI::Transformation knob) const;
#endif


    protected:
      friend class ndk::SceneOpNode;
      SceneOpNodeI* _sceneOpNode;   //!< Node owner, set by SceneOpNode

      //! Subclass implementation of SetValueProvider to create the correct AuthoringProvider
      //! subclass and store in the SceneOpNode.
      //! Default implementation creates an AxisValsProvider.
      virtual AuthoringProvider*  _createAuthoringProvider(Knob* k, uint64_t componentId);


      //! Called by SceneOpNode when determining the global source scene state hash.
      //! Subclasses can implement this to take any additional values into consideration
      //! when determining extractor state, like local knob values, but must careful not to
      //! include possibly animating values like Op::hash().
      //!
      //! Note that is assumed to be called on the firstOp() and not any specific OutputContext,
      //! so all Knob value queries should to be done directly on a Knob and must avoid relying
      //! on Op::knobs() storing into local Op variables.
      //!
      //! This should always be called from the main thread.
      //!
      //! Base class does nothing.
      virtual fdk::Hash  buildSourceSceneState(const fdk::Hash& hash) { return hash; }

      //! Retrieve the first/last range of keyframes for all prim attributes that will be
      //! extracted from. If there are no keyframes (no animation) then set the first/last
      //! keys to fdk::defaultTimeValue(). This special value helps a provider quickly
      //! determine there are no animated values and thus speed up value retrievals.
      //!
      //! Default implementation retrieves any xform attribute keyframes, or if none sets
      //! the first/last keys to fdk::defaultTimeValue().
      virtual void  getExtractKeyRange(SceneOpNodeI::SharedData& data, fdk::TimeValue& firstKey, fdk::TimeValue& lastKey);

      //! Extract the scene data that needs to be cached or processed for the entire scene range.
      //! Usually called by SceneOpNode \a updateSourceSceneData() when the source scene state
      //! changes. This should always be called from the main thread.
      //!
      //! Important note: the particular Op instance this method is called on is unimportant as
      //! the data created by this method is intended to be shared by all sibling Ops. One
      //! random Op instance will be called for each configuration change (a stage, prim path
      //! or input change) so avoid storing any local data into this Op instance.
      //!
      //! Not all prim data needs to be cached if cheap to retrieve from USD, however a good
      //! example of expensive data is extracting and decomposing transform matrices which need
      //! to be euler filtered over the entire scene. In these cases it's cheaper to do the
      //! processing all at once and cache the result.
      //!
      //! Default implementation calls extractSceneXformData() and returns true.
      virtual bool  extractSceneData(SceneOpNodeI::SharedData& data,
                                     const fdk::TimeValueSet&  addlSampleTimes = fdk::TimeValueSet());

      //! Default implementation extracts the transform info by decomposing the parent and local
      //! matrices and runs a euler filter to remove flips, caching the resulting data for the
      //! PrimValueProviders to later retrieve.
      //! This work is only done when the stage contents change, either via recomposition or
      //! layer/stage edits.
      virtual void  extractSceneXformData(SceneOpNodeI::SharedData& data,
                                          AxisValsArray&            axisValsSamples,
                                          bool&                     isDefaultXforms,
                                          const fdk::TimeValueSet&  addlSampleTimes = fdk::TimeValueSet()) const;

      //----------------------------------------

      //! Retrieve the transform data for a single time sample at OutputContext.
      //! Assumes that extractSceneData() has already been called to first perform any global
      //! scene data processing and any caches are ready to be read from.
      //!
      //! Default implementation retrieves the AxisVals from the transform cache and does any
      //! required interpolation.
      virtual AxisVals  getXformDataAt(SceneOpNodeI::SharedData& data, const OutputContext& context) const;


      //----------------------------------------------------------------------------
      // Transforms
      //----------------------------------------------------------------------------

      //! Call this from owner (Axis)Op's implementation of Op::_validate() to build the
      //! local \a _xformComponents.localParentAxis, \a _xformComponents.localAxis and
      //! final local transform matrices for the current (Axis)Op's store state.
      //!
      //! Default implementation builds \a _xformComponents.localParentAxis from the
      //! local parent TRS knobs. \a _xformComponents.localAxis has already been built
      //! during Knob store by the 'transform' Axis_Knob but a subclass could override
      //! that result via \a _getSceneOpLocalAxisTransform().
      //!
      //! The local transform matrices are build by calling \a _buildLocalTransforms()
      //! with the updated individual transform components, then those are used to
      //! build the final concatentated output transforms in \a outputXformParams.
      //!
      //! Note that the concatenated world matrices (l2w, w2l) in \a outputXformParams
      //! should not be calculated in this method and should be handled by the caller.
      virtual void  _validateTransforms(bool                  for_real,
                                        ndk::XformParameters& outputXformParams);

      //! Build the final local transforms from the individual transform components,
      //! applying any additional constraints.
      //!
      //! By default this is called by \a _validateTransforms() and
      //! \a _getSceneOpWorldTransformAt() so the same transform calculations are
      //! done for both evaluation paths.
      //!
      //! Note that the concatenated world matrices (l2w, w2l) in \a finalLocalXforms
      //! should not be calculated in this method and should be handled by the caller.
      virtual void  _buildLocalTransforms(const ndk::XformComponents&   xformComponents,
                                          fdk::Mat4d&                   preConstraintXform,
                                          const ndk::ConstraintMatrixParams& constraintParams,
                                          ndk::XformParameters&         finalLocalXforms) const;

      //!
      virtual bool  _buildConstraintParams(AxisOp*                     constrainSourceOp,
                                           const ndk::XformComponents& xformComponents,
                                           fdk::Mat4d&                 preConstraintXform,
                                           ndk::ConstraintMatrixParams&     constraintParams) const;

      //!
      static bool  BuildConstraintParamsAt(const SceneOpI*             sceneOp,
                                           const OutputContext&        context,
                                           const ndk::XformComponents& xformComponents,
                                           fdk::Mat4d&                 preConstraintXform,
                                           ndk::ConstraintMatrixParams&     constraintParams);

      //----------------------------------------------------------------------------
      // Transform handling - Note these methods shadow the ones in AxisOp to reduce
      // code implementation repetition in AxisSceneOp/CameraSceneOp/LightSceneOp
      // branch base classes

      //! Does the local parent transform animate? This is only an indicator that the
      //! transform -MAY- vary in time, not that its value actually does.
      //! Default implementation returns true if the local parent knobs have animation
      //! enabled or if the constraint input Op varies and it is applied to the local
      //! parent TRS. If the knob(s) are animated and \a keyTimes provided then the
      //! knob animation keys times are appended to it. If \a view is > -1 then the
      //! times are retrieved from just that view. If any knobs have expressions but
      //! no keyframes then this will also return true, and \a nAnimatedExpressions
      //! will be set > 0, if provided.
      virtual bool  _getLocalParentTransformAnimation(fdk::TimeValueSet* keyTimes = nullptr,
                                                      int32_t*           nAnimatedExpressions = nullptr,
                                                      int32_t            view = -1) const;

      //! Does the local parent animate? This is only an indicator that the transform
      //! -MAY- vary in time, not that its value actually does.
      //! Default implementation returns true if the local parent knobs have animation
      //! enabled or animated expressions, or if the constraint input Op varies and it
      //! is applied to the local parent TRS. If the knob(s) are animated and \a keyTimes
      //! provided then the knob animation keys times are appended to it. If \a view
      //! is > -1 then the times are retrieved from just that view. If any knobs have
      //! expressions but no keyframes then this will also return true, and
      //! \a nAnimatedExpressions will be set > 0, if provided.
      virtual bool  _getLocalTransformAnimation(fdk::TimeValueSet* keyTimes = nullptr,
                                                int32_t*           nAnimatedExpressions = nullptr,
                                                int32_t            view = -1) const;

      //------------------------------------

      //! Return the transform components built \a _validateTransforms().
      virtual const ndk::XformComponents&  _getSceneOpTransformComponents() const { return _xformComponents; }
      //! Calculate the transform components at a specific OutputContext.
      virtual       ndk::XformComponents   _getSceneOpTransformComponentsAt(const OutputContext& context) const;

      //------------------------------------

      //! Return the input parent transform built in \a _validateTransforms().
      virtual const fdk::Mat4d&  _getSceneOpInputParentTransform() const { return _xformComponents.inputParentXform; }
      //! Calculate the input parent transform at a specific OutputContext.
      virtual       fdk::Mat4d   _getSceneOpInputParentTransformAt(const OutputContext& context) const;

      //! Return the local parent axis transform built in \a _validateTransforms().
      virtual const fdk::Mat4d&  _getSceneOpLocalParentAxisTransform() const { return _xformComponents.localParentAxis; }
      //! Calculate the local parent axis transform at a specific OutputContext.
      virtual       fdk::Mat4d   _getSceneOpLocalParentAxisTransformAt(const OutputContext& context) const;

      //! Return the total local parent transform built in \a _validateTransforms().
      virtual const fdk::Mat4d&  _getSceneOpParentTransform() const { return _xformComponents.localParentXform; }

      //------------------------------------

      //! Return the local axis transform built in \a _validateTransforms().
      virtual const fdk::Mat4d&  _getSceneOpLocalAxisTransform() const { return _xformComponents.localAxis; }
      //! Calculate the local axis transform at a specific OutputContext.
      virtual       fdk::Mat4d   _getSceneOpLocalAxisTransformAt(const OutputContext& context) const;

      //! Return the total local transform built in \a _validateTransforms().
      virtual const fdk::Mat4d&  _getSceneOpLocalTransform() const { return _xformComponents.localXform; }

      //------------------------------------

      //! Calculate the final world transform at a specific OutputContext.
      //!
      //! Note that this does the same transform calculation as _validateTransforms() but
      //! samples all knobs and inputs at a different context than the one the AxisOp is
      //! at by calling the individual transform component 'At(context)' methods.
      virtual fdk::Mat4d  _getSceneOpWorldTransformAt(const OutputContext& context) const;


      //----------------------------------------------------------------------------
      //----------------------------------------------------------------------------
      // IMPORTANT: Call _sceneOpResetTargetState() from the owner Op's
      //            implementation of Op::_knobStorePrep(), AND
      //            _sceneOpUpdateScenePrimState() from the owner Op's
      //            implementation of Op::_knobStorePost(), otherwise the
      //            scene op will not update properly:
      //----------------------------------------------------------------------------

      //! Call this from the owner Op's implementation of Op::_knobStorePrep(), otherwise
      //! the scene op will not update properly. This resets the local GeomStateTargets
      //! which are built from knobs during the knobs() store pass.
      void  _sceneOpResetTargetState() { _sceneOpNode->resetTargetState(); }

      //! Call this from the owner Op's implementation of Op::_knobStorePost(), otherwise
      //! the scene op will not update properly.
      void  _sceneOpUpdateScenePrimState(const DD::Image::Hash& newOpEditVersionHash);

      void  _appendKnobToDefineGeometryTarget(Knob* k);
      void  _appendKnobToModifyValuesTarget(Knob* k);

      //! Call this from owner Op::knob_changed().
      //! Enables/disables the local parent controls, and calls the SceneOpI interface
      //! to let it handle any changes.
      virtual int  _sceneOpKnobChanged(Knob* k, int32_t callAgain);

      //!
      usg::LayerRef buildGeometryLayerForContext(const ndk::NodeEvalContext& engineGraphContext,
                                                 bool                        appendTo,
                                                 const fdk::TimeValueSet&    sampleTimes = fdk::TimeValueSet()) override;

      //! Create (define) any parent prims in \a basePrimPath. These are prims in the path
      //! -before- the base prim which is always the last prim in the path.
      //! Base class implementation uses \a parentPrimCreateType() as the prim type to create.
      virtual void  _createParentPrims(usg::LayerRef& layer, const usg::Path& basePrimPath);

      //! Write prims into the layer. If \a appendTo is true then only additional time
      //! samples should be added to the prim (no define.)
      //! \a layer is assumed to be managed externally.
      //! Must implement.
      virtual void  _writeScenePrims(usg::LayerRef&           layer,
                                     bool                     appendTo,
                                     const fdk::TimeValueSet& times = fdk::TimeValueSet()) = 0;

      //! Write the transform data to an Xformable prim. If \a appendTo is true then
      //! only additional time samples should be added to the prim (no define.)
      //! A subclass can override this to do additional specialization.
      //!
      //! Base class implementation writes separate 'local_parent' and 'local_axis' matrix
      //! XformOps.
      virtual void  _writeTransformOps(usg::LayerRef&            layer,
                                       usg::XformablePrim&       xformable,
                                       bool                      appendTo,
                                       const fdk::FrameValueSet& times = fdk::FrameValueSet());

      //! Returns true if value was written or false if not due to error or knob value
      //! is at the default.
      bool  writeKnob(const char*               knobName,
                      const fdk::FrameValueSet& times,
                      usg::Prim&                outputPrim,
                      const usg::Token&         targetPropertyName,
                      usg::Value::Type          targetPropertyTypeHint) const;

      //! Same but controls the default value testing via the \a ignoreDefaultTest arg.
      //!
      //! If \a ignoreDefaultTest is false we test if the knob is at its default value and only
      //! write the knob if it is off default. USD, like Nuke, will use the default value of the
      //! knob as fallback if an explicit value is not authored.
      //!
      //! However, often the Nuke knob's default will not be the same as the USD property default,
      //! and in those cases we always want to author the USD property so that we don't allow USD
      //! to use the default fallback. Examples of this are the various CameraOp lens values like
      //! focal length, hAperture, vAperture, etc which historically have a default difference to
      //! the UsdGeomCamera schema. We don't want to suddenly change Nuke's (arbitrarily chosen)
      //! historic camera defaults to conform to USD's (arbitrarily chosen) defaults.
      bool  writeKnob(const char*               knobName,
                      bool                      ignoreDefaultTest,
                      const fdk::FrameValueSet& times,
                      usg::Prim&                outputPrim,
                      const usg::Token&         targetPropertyName,
                      usg::Value::Type          targetPropertyTypeHint) const;

      //! Same but with a Knob*.
      bool  writeKnob(Knob*                     knob,
                      bool                      ignoreDefaultTest,
                      const fdk::FrameValueSet& times,
                      usg::Prim&                outputPrim,
                      const usg::Token&         targetPropertyName,
                      usg::Value::Type          targetPropertyTypeHint) const;

      //! Same but the attribute Value can be filled in from multiple source ArrayKnobs.
      //! This is need when multiple source Knobs contribute to one attribute Value.
      //! If any of the source knobs are animated then the output attribute will
      //! have time samples.
      //!
      //! Note - \a targetPropertyType is required since a specfic type cannot be divined
      //! from multiple source knobs.
      bool  writeKnob(const ArrayKnobRefList&   knobRefs,
                      bool                      ignoreDefaultTest,
                      const fdk::FrameValueSet& times,
                      usg::Prim&                outputPrim,
                      const usg::Token&         targetPropertyName,
                      usg::Value::Type          targetPropertyType) const;

      //! Write an input Op connection as an AssetPath, typically for texture sources.
      bool  writeInputOpAsAssetPath(int32_t                   input,
                                    const fdk::FrameValueSet& times,
                                    usg::Prim&                outputPrim,
                                    const usg::Token&         targetPropertyName) const;

    };


//---------------------------------------------------------------------
//---------------------------------------------------------------------

template<typename T>
inline void AuthoringProvider::CopyValue(const T* valIn, double* valOut)
{
  *reinterpret_cast<T*>(valOut) = *valIn;
}

template<typename T>
inline void AuthoringProvider::CopyValue(const T* valInA, const T* valInB, float t, double* valOut)
{
  *reinterpret_cast<T*>(valOut) = (valInA == valInB) ? *valInA/*on key*/ : fdk::lerpUnclamped(*valInA, *valInB, t)/*between keys*/;
}

  }
}
#endif
