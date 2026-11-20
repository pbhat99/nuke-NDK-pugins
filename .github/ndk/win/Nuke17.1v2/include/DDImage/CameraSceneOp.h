// Copyright (c) 2025 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_CAMERASCENEOP_H
#define DDIMAGE_CAMERASCENEOP_H

#include "DDImage/CameraOp.h"
#include "DDImage/SceneOpI.h"

#include "usg/geom/CameraPrim.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::CameraSceneOp
        Op that defines a 3D camera which can be imported and exported.
    */
    class DDImage_API CameraSceneOp : public CameraOp
                                    , public SceneOpI
    {
    public:
      //! Which component of the camera to provide.
      //! CameraProvider subclass can extend this to support additional custom
      //! components (stereo vars, rigging info, etc) to expose via output knobs.
      enum
      {
        PROJECTION = SceneOpI::LAST_XFORM_PARAM,
        WORLD_TO_METERS,
        STEREO_ROLE,
        HAPERTURE,
        VAPERTURE,
        FOCAL_LENGTH,
        FOCUS_DISTANCE,
        FSTOP,
        CLIP_NEAR,
        CLIP_FAR,
        WIN_TRANSLATE,
        WIN_SCALE,
        WIN_ROTATE_Z,
        SHUTTER_DURATION,
        SHUTTER_OFFSET_MODE,
        SHUTTER_OFFSET,
        SHUTTER_BIAS,
        DISTORT_A,
        DISTORT_B,
        DISTORT_C,
        //
        LAST_BASE_CAMERA_PARAM
      };

      // Extract transform and camera values.
      class DDImage_API CameraProvider : public AxisValsProvider
      {
      public:
        //! Assigns the camera component index.
        CameraProvider(Knob* knob, CameraSceneOp* owner, uint64_t componentId);

      protected:
              CameraSceneOp*  _cameraOwner()       { return _owner->asCameraSceneOp(); }
        const CameraSceneOp*  _cameraOwner() const { return _owner->asCameraSceneOp(); }

        //! Individual camera components can disable depending on options.
        bool  _isProviderEnabled() const override;

        //! Retrieve the default value for the assigned camera component.
        void  _getDefault(double* values, size_t nValues) const override;

        //! Return true if the source component is at its default.
        bool  _isDefault() const override;

        //! Return true if the source component is an animating attribute.
        bool  _isAnimated() const override;

        //! Subclass portion of provideValues().
        //! Default implementation extracts the value for the assigned camera component.
        bool  _provideValues(double* values, size_t nValues, const ArrayKnobI* arrayKnob, const OutputContext& context) const override;
      };

      //----------------------------------------------------------------------------

      CameraSceneOp(Node* node);
      ~CameraSceneOp();

      //! SceneOps are owned by the 'SceneOpNode' Node identifier.
      const char* nodeManagerClass() const override { return "SceneOpNode"; }

      //! Cast to a CameraSceneOp. This is much cheaper and safer than using dynamic_cast.
      const CameraSceneOp* cameraSceneOp() const override { return this; }
            CameraSceneOp* cameraSceneOp()       override { return this; }

      //! AxisOp: Return the node input to use for the parent axis source. Defaults to input 0.
      int32_t  parentInput() const override { return 0; }

      //! SceneOpI: Return the node input to use for the scene source. Defaults to input 1.
      int32_t  sceneInput() const override { return 1; }

      //! SceneOpI: Return the node input to use for the parent axis source. Defaults to input 2.
      int32_t  constrainInput() const override { return 2; }

      //-----------------------------------------------------------------

      //! SceneOpI: Return the parentInput() input cast to a SceneOpI, if possible.
      SceneOpI*  parentInputSceneOp() const override;

      //! SceneOpI: Return the sceneInput() input cast to a GeomOp, if possible.
      GeomOp*  sceneInputOp() const override;

      //! SceneOpI: Return the AxisOp::constrainInput() input cast to a SceneOpI, if possible.
      SceneOpI*  constrainInputSceneOp() const override;

      //! SceneOpI: Return the name of the primary USD schema this node is handling.
      const usg::Token&  primarySchemaType() const override { return usg::GeomTokens.types.Camera; }

      //! SceneOpI: Return the default filter string to apply in the prim path knob.
      const char*  primPathFilter() const override { return "filter:\"Type:Camera\""; }

      //-----------------------------------------------------------------

      //! Default is 3: axis, scene and constrain
      int          minimum_inputs() const override;
      //! Default is 3: axis, scene and constrain
      int          maximum_inputs() const override;
      //! Optional input index is the scene-importer input. This returns (maximum_inputs()-1).
      int          optional_input() const override;

      //! AxisOp and null works on 0, and GeomOp or null on input 1.
      bool         test_input(int node_input, Op* op) const override;

      //! Returns null for all inputs. Override this so Op::test_input() doesn't produce warnings.
      Op*          default_input(int node_input) const override;

      //!
      const char*  input_label(int node_input, char*) const override;

      //-----------------------------------------------------------------

      /*! Default knobs puts the transform controls on the front tab, and
          the projection, shutter, and lens controls on tab 'Projection'.
      */
      void knobs(Knob_Callback) override;

      //! Called by Nuke just BEFORE Op::knobs(EARLY_STORE).
      //! This is used to update the scene states.
      void  _knobStorePrep(const OutputContext& opStoreContext) override;

      //! Called by Nuke immediately AFTER Op::knobs().
      //! This is used to update the scene states.
      void  _knobStorePost(const OutputContext& opStoreContext, Hash& newOpHash, Hash& newOpEditVersionHash) override;

      //! Return the knob which provides projection selection.
      Knob*  projectionSelectorKnob() const override;

      //! Retrieves the class name from the projectionSelectorKnob()'s current value
      //! in the enumeration list.
      //! Default implementation supports either an EnumerationKnob's or StringKnob's
      //! current value. \a selectorKnob is optional to override the default retrieval
      //! of the selector Knob from projectionSelectorKnob().
      const char*  projectionFuncClassName(Knob* selectorKnob = nullptr) const override;

      //! Assigns AuthoringProviders to the shutter component knobs using the SHUTTER_*
      //! component ids.
      //! Always call this right after called ShutterControls.knobs() as the Knob* stored
      //! in \a shutter is only valid for this node during f.makeKnobs()==true mode.
      void  SetShutterKnobAuthoringProvider(Knob_Callback f, ShutterControls& shutter);

      //! Adds additional front-panel knobs, primarily the projection controls.
      //! Called before addTransformKnobs().
      void  addExtraFrontPanelKnobs(Knob_Callback f) override;

      //! Adds the frustum visualization controls.
      void  addDisplayOptionsKnobs(Knob_Callback f) override;

      //! Adds the default shutter controls.
      virtual void  addShutterKnobs(Knob_Callback);

      //! Adds the default projection controls.
      virtual void  addProjectionKnobs(Knob_Callback f);

      //! Adds the default aperture-window controls.
      virtual void  addWindowKnobs(Knob_Callback f);

      //! Adds the default lens controls.
      virtual void  addLensKnobs(Knob_Callback f);

      //-----------------------------------------------------------------

      void  _validate(bool for_real) override;

      int  knob_changed(Knob*) override;

      //-----------------------------------------------------------------

      //! Local parent axis transform matrix.
      const fdk::Mat4d&  localParentAxisTransform() const override { return _getSceneOpLocalParentAxisTransform(); }

      //! Set the local parent axis transform matrix. Note this only persists until the
      //! next knobs()/validate() pass.
      void  setLocalParentAxisTransform(const fdk::Mat4d& localParent) override { _xformComponents.localParentAxis = localParent; }

      //! Result of the local-axis TRS Axis_Knob (*without* lookat rotations.)
      //! Calculated in _validateTransforms().
      const fdk::Mat4d&  localAxisTransform() const override { return _getSceneOpLocalAxisTransform(); }

      //! Set the local axis transform matrix. Note this only persists until the
      //! next knobs()/validate() pass.
      void  setLocalAxisTransform(const fdk::Mat4d& localAxis) override { _xformComponents.localAxis = localAxis; }

      //! Does the local parent transform animate? This is only an indicator that the
      //! transform -MAY- vary in time, not that its value actually does.
      bool  localParentTransformVaries() const override { return _getLocalParentTransformAnimation(); }

      //! Does the local transform animate? This is only an indicator that the transform
      //! -MAY- vary in time, not that its value actually does.
      bool  localTransformVaries() const override { return _getLocalTransformAnimation(); }

      //-----------------------------------------------------------------

      //!
      HandlesMode  doAnyHandles(ViewerContext* ctx) override;

      //!
      void  build_handles(ViewerContext*) override;

      //! Draws a lovely camera, I think it is a Mitchell.
      void  draw_handle(ViewerContext*) override;

      //! Draw the camera's frustum outline solid or wireframe, dashed if 'dashed_lines'=true.
      virtual void  drawCameraIcon(bool solid,
                                   bool dashed_lines=false);


      //! Draw the node name at position 0,0,0 which will be the local center.
      void          drawNodeName(const char* subcam_name = "");

      //! Draw the node name at an xyz position \a P relative to the local center.
      void          drawNodeName(const fdk::Vec3d& P,
                                 const char*       subcam_name = "");

      //! Draw the aperture rectangle in millimeters, centered on xyz position \a P.
      void          drawAperture(const fdk::Vec3d& P,
                                 bool              dashed_lines = false);

      //! Draw the camera's frustum.
      void          drawSolidFrustum(const fdk::Vec3d& near_corner,
                                     const fdk::Vec3d& far_corner);
      //! Draw the camera's frustum outline, dashed if 'dashed_lines'=true.
      void          drawWireFrustum(const fdk::Vec3d& near_corner,
                                    const fdk::Vec3d& far_corner,
                                    bool              dashed_lines = false);

      //----------------------------------------------------------------------------
      // SceneOpI handling
      //----------------------------------------------------------------------------

      //! SceneOpI: Return the AxisOp the interface is attached to.
      AxisOp*  getAxisOp() override { return this; }

      //! Cast to a SceneOpI.
            SceneOpI*  sceneOpI()       override { return this; }
      const SceneOpI*  sceneOpI() const override { return this; }

      //----------------------------------------------------------------------------
      // GeometryProviderI scene prim output handling
      //----------------------------------------------------------------------------

      //! GeometryProviderI: Return the Op the interface is attached to.
      Op*  getGeometryProviderOp() override { return this; }

      //! Cast to a GeometryProviderI interface. This is much cheaper and safer than using dynamic_cast.
            GeometryProviderI*  geometryProvider()       override { return asGeometryProvider(); }
      const GeometryProviderI*  geometryProvider() const override { return const_cast<CameraSceneOp*>(this)->asGeometryProvider(); }

      //-----------------------------------------------------------------

      //! Builds a path from buildBasePrimPath(), appending the Node displayName() to use as
      //! the camera schema's prim name.
      //!
      //! For example if the Node class name is 'MyFisheyeCamera_v4' we want it to show up in the
      //! scenegraph as 'MyFisheyeCamera', while the camera schema name may be 'FisheyeCamera'.
      virtual usg::Path  buildCameraSchemaPrimPath() const;

      //! SceneOpI: Prepends the parent connection if there is one.
      usg::Path  buildBasePrimPath() const override;

      //----------------------------------------

      void getExtractKeyRange(SceneOpNodeI::SharedData& data, fdk::TimeValue& firstKey, fdk::TimeValue& lastKey) override;

      //! Retrieve the camera data for a single time sample at OutputContext.
      virtual ndk::Camera::Sample  getCameraDataAt(SceneOpNodeI::SharedData& data, const OutputContext& context) const;

      //! Return true if the component id is a base camera component shared by most cameras.
      //! Default implementation tests if id is within range PROJECTION...LAST_BASE_CAMERA_PARAM.
      virtual bool  isBaseCameraComponent(uint64_t componentId) const;

      //! Return true if the component id is a typed camera component.
      //! Default implmentation returns false.
      virtual bool  isTypedCameraComponent(uint64_t componentId) const { return false; }

      //! Retrieve the camera component value for a single time sample at OutputContext.
      //! Component value element count must not exceed nValues. If element count is less
      //! than nValues then fill the remaining elements with an appropriate default.
      virtual bool  getCameraComponentAt(const usg::StageRef&   stage,
                                         const usg::CameraPrim& cameraPrim,
                                         const OutputContext&   context,
                                         uint64_t               componentId,
                                         double*                values,
                                         size_t                 nValues) const;


    protected:
      //! Create the camera-specific providers.
      AuthoringProvider* _createAuthoringProvider(Knob* k, uint64_t componentId) override;

      //! Append camera knob values that affect scene load state.
      fdk::Hash buildSourceSceneState(const fdk::Hash& hash) override;

      //----------------------------------------

      //! Calculate the transform components of this node at a specific OutputContext.
      //! Subclass implementation of GetTransformComponentsAt(AxisOp), redirects to
      //! SceneOpI::_getSceneOpTransformComponentsAt().
      ndk::XformComponents _getTransformComponentsAt(const OutputContext& context) const override;

      //----------------------------------------

      //! Calculate the input parent transform of this node at a specific OutputContext.
      //! Subclass implementation of GetInputParentTransformAt(AxisOp), redirects to
      //! SceneOpI::_getSceneOpInputParentTransformAt().
      fdk::Mat4d _getInputParentTransformAt(const OutputContext& context) const override;

      //! Calculate the local parent transform of this node at a specific OutputContext.
      //! Subclass implementation of GetLocalParentAxisTransformAt(AxisOp), redirects to
      //! SceneOpI::_getSceneOpLocalParentAxisTransformAt().
      fdk::Mat4d _getLocalParentAxisTransformAt(const OutputContext& context) const override;

      //! Calculate the local axis transform of this node at a specific OutputContext.
      //! Subclass implementation of GetLocalAxisTransformAt(AxisOp), redirects to
      //! SceneOpI::_getSceneOpLocalAxisTransformAt().
      fdk::Mat4d _getLocalAxisTransformAt(const OutputContext& context) const override;

      //----------------------------------------

      //! Calculate the world transform of this node at a specific OutputContext.
      //! Subclass implementation of GetWorldTransformAt(AxisOp), redirects to
      //! SceneOpI::_getSceneOpWorldTransformAt().
      fdk::Mat4d _getWorldTransformAt(const OutputContext& context) const override;

      //----------------------------------------

      //! Calls _enableDefaultProjectionKnobs based on the projPlugin's needsDefaultCameraOpKnobs and 
      //! shows/hides a divider knob before the custom projection knobs based on if the 
      //! projPlugin has custom knobs to add.
      virtual void _updateDefaultKnobsForProjection(ProjectionFunc* projPlugin);

      //! Enables/Disables the default knobs on the projection tab for the CameraOp.
      //! This function is called from _updateDefaultKnobsForProjection.
      virtual void _enableDefaultProjectionKnobs(bool enable);

      //! Called from knob_changed when projection selection is changed via knob interaction.
      int _doProjectionFuncKnobChanged(Knob*, int callAgain);

      //! SceneOpI: Subclass portion of writeScenePrims().
      void  _writeScenePrims(usg::LayerRef&           layer,
                             bool                     appendTo,
                             const fdk::TimeValueSet& times = fdk::TimeValueSet()) override;

      //! Write win_translate knob values as USD horizontalApertureOffset/verticalApertureOffset.
      //! Applies the correct per-frame conversion:
      //!   apertureOffset = win_translate * (aperture_mm * mmToTenthSceneUnit / 2)
      //! which is the inverse of the import conversion performed in getCameraComponentAt().
      void  _writeWindowTranslateAsApertureOffsets(const fdk::TimeValueSet& times,
                                                   usg::CameraPrim&         camPrim,
                                                   double                   mmToTenthSceneUnit);


    public:
      //! For internal use only - called in addProjectionKnobs() to add ProjectionFunc dynamic knobs.
      void addProjectionFuncCustomKnobs(Knob_Callback);

    protected:
      bool _useSceneScaleMetadata;          //!< Use incoming scene world scale metadata for lens value conversions

    private:
      int32_t _projFuncCustomKnobsBeginIdx; //!< Knob signpost where ProjectionFunc dynamic knobs -start-
      int32_t _nProjFuncCustomKnobs;        //!< Count of added ProjectionFunc dynamic knobs

#if kDDImageVersionMajorNum <= 17
    protected:
      bool _isLegacyAxisOp() const override { return false; }
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif
    };

  }
}

#endif
