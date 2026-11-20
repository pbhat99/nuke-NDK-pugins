// Copyright (c) 2024 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_SCENEPRIMEXPORTEROPI_H
#define DDIMAGE_SCENEPRIMEXPORTEROPI_H

#include "DDImage/GeometryProviderI.h"
#include "DDImage/Knobs.h"

#include "usg/geom/Layer.h"
#include "usg/geom/ShaderDesc.h"
#include "usg/engine/GeomState.h"

#include <memory>


namespace DD {
  namespace Image {
    class Op;

    /*! Helper interface class for non GeomOp based classes to produce scene geometry
        prims like GeomOps can output.

        IMPORTANT: In the owner Op's implementation (the Op subclass that's inheriting
        this interface) you MUST implement the Op::_knobStorePrep() and Op::_knobStorePost()
        methods and within those methods call _exporterResetScenePrimState() and
        _exporterUpdateScenePrimState() respectively. Otherwise the exporter will not
        update properly. This is due to the exporter needing to determine its export
        changes due to Knob value changes, so the exporter needs to know the state prior
        to any Knob changes, and the state after any Knob changes.

        For example AxisOp::_knobStorePrep() and AxisOp::_knobStorePost() are both
        implemented and call this interface, and this allows all AxisOp subclasses like
        CameraOp and LightOp to properly determine their export state.
    */
    class DDImage_API ScenePrimExporterOpI : public GeometryProviderI
    {
    public:
      //! Export sampling modes.
      enum {
        EXPORT_SAMPLE_ALL_SCRIPT_FRAMES,    //!< Sample root frame range at integer frame increments
        EXPORT_ADDL_TIMES,                  //!< Sample at additional times but do -NOT- clear the Layer
        EXPORT_SAMPLE_KEYFRAMES,            //!< Only sample at knob keyframes
      };

      //! Modes for building additional parent prims specified in 'prim_path'.
      enum {
        EXPORT_PARENT_XFORM,
        EXPORT_PARENT_SCOPE,
        EXPORT_PARENT_PLACEHOLDER_OVERRIDE,
        EXPORT_PARENT_IGNORE
      };

      //! Sets parent fill type to Xform(0).
      ScenePrimExporterOpI();

      virtual ~ScenePrimExporterOpI();

      //! Return the Op the interface is attached to. Should return 'this'. Must implement.
      virtual Op*  getScenePrimExporterOp() = 0;

      Op*  getScenePrimExporterOp() const { return const_cast<ScenePrimExporterOpI*>(this)->getScenePrimExporterOp(); }

      //! Allow subclasses to gain access to sibling functions.
      virtual ScenePrimExporterOpI*  asScenePrimExporter() { return nullptr; }

      //! Call this from owner Op::knobs().
      //! Adds the prim export option knobs.
      virtual void  addExportOptionsKnobs(Knob_Callback f);

      //! Called from owner Op, typically, to enable/disable the export knobs with some
      //! custom logic.
      //! Base class does nothing.
      virtual void  enableExportOptionsKnobs(bool exportEnabled);

      //! Return the value of the 'prim_path' knob.
      std::string  exportPrimPath() const;

      //! Return the mode value of the 'parent_prim_type' knob.
      int32_t  exportParentPrimMode() const;


      //! Return the full path of the -primary- prim and all its parents created by this node.
      //! This is the 'top' prim where any child prims are rooted under.
      //!
      //! For example if this is an AxisOp it's typically a single Xform prim, but a CameraOp
      //! or LightOp may have additional prims after the main Xform (ie for a stereo camera
      //! rig) so this connection point needs to be explicitly identified.
      //!
      //! Retrieve the base prim name by calling \a name() on the returned path.
      //!
      //! Must implement.
      virtual usg::Path  exportBasePrimPath() const = 0;

      //! The type of the \a exportBasePrimPath() base prim (ie the end prim returned by
      //! exportBasePrimPath().name().)
      //! EXPORT_PARENT_XFORM or EXPORT_PARENT_SCOPE are supported.
      // Must implement.
      virtual int32_t  exportBasePrimType() const = 0;


      //! Create (define) any parent prims in the \a exportBasePrimPath(). These are prims
      //! in the path -before- the base prim.
      //! Uses the values from \a exportPrimPath() and \a exportParentPrimMode().
      void  buildParentPrims(usg::LayerRef& exportLayer) const;

      //! Export prims into a scene layer. This returns fast if there's no change to the
      //! export data since last _validateScenePrimExporter() cycle.
      //! If \a exportMode is EXPORT_ADDL_TIMES then sample at the \a additionalTimes
      //! times but do -NOT- clear the Layer.
      //! If \a force is true then the export data is always regenerated and the Layer
      //! rebuilt.
      usg::LayerRef  exportScenePrims(int32_t                  exportMode,
                                      const fdk::TimeValueSet& additionalTimes = fdk::TimeValueSet(),
                                      bool                     force = false);

      //! Export prims to a layer, ignoring any current state of the exporter controls.
      //!
      //! This assumes \a exportLayer has been created, configured and valid. The list of sample
      //! frame times is a request to the exporting object and is not guaranteed to result in
      //! those exact time samples in the export layer. For example a Knob may not be animated
      //! and will only set the default sample, or the keyframes being output are sparse.
      //!
      //! Calls the virtual _exportScenePrims() method that subclasses implement.
      void  exportScenePrimsToLayer(usg::LayerRef&            exportLayer,
                                    int32_t                   exportMode,
                                    const fdk::FrameValueSet& sampleFrames);


      //----------------------------------------------

      //! Use this method if the Knob's value will affect the exported scenegraph topology by
      //! adding/removing Prims or Attributes, or by changing their names or paths in the
      //! output scenegraph.
      //!
      //! If the knob is just changing the contents of an existing attribute rather than
      //! explicitly creating that attribute, use KnobModifiesExportedAttribValues() instead.
      void  KnobDefinesExportedGeometry(const Knob_Closure& f);

      //! Use this method if the Knob's value is changing the contents of an existing
      //! attribute in an exported prim.
      //! This only applies if the exporter can support being called in EXPORT_ADDL_TIMES
      //! mode allowing attribute changes to be applied to an already defined prim.
      void  KnobModifiesExportedAttribValues(const Knob_Closure& f);

      //! Append arbitrary hash contributions to this exporter's local GeomState.
      //! These hash contribution sources are normally from non-Knob sources like inputs
      //! or some other Op value like frame or view.
      //! \a op_hash is provided as a convenience to allow the exporter to affect the Op's
      //! output hash at the same time without needing to reimplement the hash calcs in
      //! Op::append(), which will be called soon afterwards.
      //!
      //! This is commonly called from a GeomOp importing prims from this Op from its
      //! appendGeomState() method.
      //!
      //! Base class implementation does nothing.
      virtual void  appendExporterGeomState(DD::Image::Hash& opStoreHash);

      //----------------------------------------------------------------------------
      // GeometryProviderI handling
      //----------------------------------------------------------------------------

      //! GeometryProviderI:  Return the Op the interface is attached to.
      Op*  getGeometryProviderOp() override { return getScenePrimExporterOp(); }

      //! GeometryProviderI: Allow subclasses to gain access to sibling functions.
      GeometryProviderI*  asGeometryProvider() override { return this; }

      //! Returns the compose state of the exportLayer.
      fdk::Hash  geometryComposeState() const override;

      //! Returns the edit state of the exportLayer.
      fdk::Hash  geometryEditState() const override;

      //! Returns the animating state of the exportLayer.
      bool  geometryStateVaries() const override;

      //! Provide the export usg::Layer. If sampleTimes is not empty the the exporter
      //! may possibly be run for those additional times.
      usg::LayerRef  getGeometryLayer(const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet()) override;

      //! Provides a fully constructed usg::Stage via buildGeometryStage().
      bool canProvideGeometryStage() const override { return true; }

      //! Provide the cached exported stage. The cached stage is only created when
      //! this method is called. If the cached stage has the same composeState as
      //! the exporter then buildGeometryLayer() is called with any additional sample
      //! times but the cached stage itself is left untouched.
      //! \a stage is then set to the exporter's cached stage.
      void buildGeometryStage(usg::StageRef&           stage,
                              const usg::ArgSet&       requestArgs,
                              const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet()) override;


    protected:
      //--------------------------------------------------------------------
      //--------------------------------------------------------------------
      // IMPORTANT: Call _exporterResetScenePrimState() from the owner Op's
      //            implementation of Op::_knobStorePrep(), AND
      //            _exporterUpdateScenePrimState() from the owner Op's
      //            implementation of Op::_knobStorePost(), otherwise the
      //            exporter will not update properly:
      //--------------------------------------------------------------------

      //! Call this from the owner Op's implementation of Op::_knobStorePrep(), otherwise
      //! the exporter will not update properly.
      //! This resets the local GeomStateTargets which are built from knobs during the
      //! knobs() store pass when ExporterKnobDefinesGeometry() and
      //! ExporterKnobModifiesAttribValues() are used.
      void  _exporterResetScenePrimState(const OutputContext& opStoreContext);

      //! Call this from the owner Op's implementation of Op::_knobStorePost(), otherwise
      //! the exporter will not update properly.
      //! This method builds the exporter state hash by appending the exporter knob values
      //! and the owner Op's editState(), then calls _buildScenePrimExporterState() to
      //! allow subclasses to affect the state, then stores the result.
      //! On the next exportScenePrims() cycle if the state has changed the prim exporter
      //! will be run.
      void  _exporterUpdateScenePrimState(const OutputContext&   opStoreContext,
                                          const DD::Image::Hash& opStoreHash);

      //--------------------------------------------------------------------
      //--------------------------------------------------------------------

      //! Called by _exporterUpdateScenePrimState() when determining the exporter state hash.
      //! Subclasses can implement this to take any additional values into consideration
      //! when determining export state, like input hashes.
      //! Base class does nothing.
      virtual fdk::Hash  _buildScenePrimExporterState(const fdk::Hash& hash) { return hash; }

      //! Call this from owner Op::knob_changed().
      //! Updates exporter knobs and calls \a enableExportOptionsKnobs() to allow a subclass
      //! to perform some custom logic for enabling/disabling the knobs.
      int32_t  _exportScenePrimsKnobChanged(Knob*   k,
                                            int32_t callAgain);

      //! Call this from owner Op::_validate().
      //! Subclasses can implement this to do any additional work in preparation for prim
      //! exporting.
      //! Base class does nothing.
      virtual void  _validateScenePrimExporter(bool for_real);

      //! Subclass portion of exportScenePrims().
      //! Subclass should handle the creation of the actual scene prims containing data.
      //!
      //! Must implement.
      virtual void  _exportScenePrims(usg::LayerRef&            exportLayer,
                                      int32_t                   exportMode,
                                      const fdk::FrameValueSet& sampleFrames = fdk::FrameValueSet()) const = 0;


    protected:
      //! Note - \a opContext frame number is ignored if \a exportAllKeys is true.
      //! Returns true if value was exported / set, false if not exported due to error or
      //! knob value is at the default.
      bool  exportKnob(const char*               knobName,
                       int32_t                   exportMode,
                       const fdk::FrameValueSet& sampleFrames,
                       usg::Prim&                outputPrim,
                       const char*               targetPropertyName,
                       usg::Value::Type          targetPropertyTypeHint = usg::Value::Type::InvalidType) const;

      //! Same but controls the default value testing via the \a ignoreDefaultTest arg.
      //! This is needed when a Knob's default value does not match the default of the exported Prim
      //! attribute and must always be written.
      //! For example if a destination float prim attribute called 'foo' has a default of 2.0f but the
      //! source Op Knob 'foo' has a default of 0.0f, then this method can force always exporting the
      //! Knob's value even at the destination default.
      bool  exportKnob(const char*               knobName,
                       bool                      ignoreDefaultTest,
                       int32_t                   exportMode,
                       const fdk::FrameValueSet& sampleFrames,
                       usg::Prim&                outputPrim,
                       const char*               targetPropertyName,
                       usg::Value::Type          targetPropertyTypeHint = usg::Value::Type::InvalidType) const;

      //! Export an input Op connection as an asset path, usually for textures.
      bool  exportInputOpAsAssetPath(int32_t                   input,
                                     int32_t                   exportMode,
                                     const fdk::FrameValueSet& sampleFrames,
                                     usg::Prim&                outputPrim,
                                     const char*               targetPropertyName) const;


    protected:
      struct Impl;
      std::unique_ptr<Impl> _pImpl;   //!< Exporter instance is owned by firstOp
      //! For internal use only - returns the _pImpl from the firstOp which is unique per Node.
      Impl& _nodeExporterData() const;

      //! Groups of locally-created ShaderDescs
      usg::ShaderDescGroupPtrMap&  _shaderDescGroups();

      //! Calls exportScenePrims() with the appropriate mode.
      usg::LayerRef buildGeometryLayer(bool appendTo, const fdk::TimeValueSet& sampleTimes) override;

    };

  }
}

#endif
