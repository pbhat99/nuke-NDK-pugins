// Copyright (c) 2025 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_AXISSCENEOP_H
#define DDIMAGE_AXISSCENEOP_H

#include "DDImage/AxisOp.h"
#include "DDImage/SceneOpI.h"
#include "DDImage/KnobMixIns.h"

#include "usg/geom/GeomTokens.h"

namespace DD {
  namespace Image {

    /*! \class DD::Image::AxisSceneOp

        \brief AxisSceneOp is the base class for all the 3D 'scene' objects like
        locators (usg::XformPrim), cameras (usg::CameraPrim) and lights which are
        translatable to USD prims.

        It contains a local transform matrix and an optional local-parent transform
        matrix. The local-parent enables an arbitrary parenting hierarchy to be
        decomposed and contained within the AxisOp allowing the node to be disconnected
        or disassociated from a parenting network or scenegraph. The node is
        effectively localized and can be copied, pasted into other scripts, etc.
    */
    class DDImage_API AxisSceneOp : public AxisOp
                                  , public SceneOpI
    {
    public:
      AxisSceneOp(Node* node);
      ~AxisSceneOp();

      //! SceneOps are owned by the 'SceneOpNode' Node identifier.
      const char* nodeManagerClass() const override { return "SceneOpNode"; }

      //! Cast to a AxisSceneOp. This is much cheaper and safer than using dynamic_cast.
      const AxisSceneOp* axisSceneOp() const override { return this; }
            AxisSceneOp* axisSceneOp()       override { return this; }

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
      const usg::Token&  primarySchemaType() const override { return usg::GeomTokens.types.Xform; }

      //! SceneOpI: Return the default filter string to apply in the prim path knob.
      const char*  primPathFilter() const override { return "filter:\"Type:Xform\""; }

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

      //!
      void  knobs(Knob_Callback) override;

#if kDDImageVersionMajorNum <= 17
      void  addDisplayOptionsKnobs(Knob_Callback) override;
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif

      //! Called by Nuke just BEFORE Op::knobs(EARLY_STORE).
      //! This is used to update the scene importer and exporter states.
      void  _knobStorePrep(const OutputContext& opStoreContext) override;

      //! Called by Nuke immediately AFTER Op::knobs().
      //! This is used to update the scene importer and exporter states.
      void  _knobStorePost(const OutputContext& opStoreContext, Hash& newOpHash, Hash& newOpEditVersionHash) override;

      //-----------------------------------------------------------------

      int  knob_changed(Knob* k) override;

      void  _validate(bool for_real) override;

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
      HandlesMode  doAnyHandles(ViewerContext*) override;

      //! This default version will always cause draw_handle() to be called when in 3D mode.
      void  build_handles(ViewerContext*) override;

      //! Draws any geometry attached to this axis.
      //! Note that the Axis knob will draw the 3-arrow axis control at its pivot-translate
      //! location.
      void  draw_handle(ViewerContext*) override;


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
      const GeometryProviderI*  geometryProvider() const override { return const_cast<AxisSceneOp*>(this)->asGeometryProvider(); }

      //-----------------------------------------------------------------

      //! SceneOpI: Prepends the parent connection if there is one.
      usg::Path  buildBasePrimPath() const override;

      //! Builds a path from buildBasePrimPath(), appending the Node displayName() to use as
      //! the Xform schema's prim name.
      //!
      //! For example if the Node class name is 'MyCustomXform_v4' we want it to show up in the
      //! scenegraph as 'MyCustomXform', while the custom schema name may be 'ProjectionXform'.
      virtual usg::Path  buildXformSchemaPrimPath() const;


    protected:
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

      //! SceneOpI: Subclass portion of writeScenePrims().
      void  _writeScenePrims(usg::LayerRef&           layer,
                             bool                     appendTo,
                             const fdk::TimeValueSet& times = fdk::TimeValueSet()) override;


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
