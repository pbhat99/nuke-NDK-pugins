//------------------------------------------------------------------------------
// DDImage/AxisOp.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_AXISOP_H
#define DDIMAGE_AXISOP_H

#include "DDImage/Op.h"
#include "DDImage/SceneLoaderOpI.h"
#include "DDImage/ScenePrimExporterOpI.h"
#include "DDImage/KnobMixIns.h"

#include "DDImage/Knob.h"
#include "DDImage/ArrayKnobI.h"
#if kDDImageVersionMajorNum <= 16
#  include "DDImage/LookAt.h"
#  include "DDImage/Matrix4.h"
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif

#include "ndk/geo/camera/Camera.h"

#include "fdk/math/Mat4.h"

namespace usg {
  class XformablePrim;
}

namespace DD {
  namespace Image {
    class AxisOp;
    class Axis_KnobI;
    class GeomOp;


    //----------------------------------------------------------------------

#if kDDImageVersionMajorNum <= 16
    class DDImage_API Axis
    {
    public:
      // P is the ray location, X, Y and Z are axis normals:
      Vector3 p, x, y, z;

      mFnDeprecatedLegacy3DSystemInNuke14("Axis class has been deprecated, use Mat4 and its xAxis()/yAxis()/zAxis() methods instead")
      Axis() :
        p(0.0f, 0.0f, 0.0f),
        x(1.0f, 0.0f, 0.0f),
        y(0.0f, 1.0f, 0.0f),
        z(0.0f, 0.0f, 1.0f)  {}

      void print_info(const char* title);
    };
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif

    //----------------------------------------------------------------------


    /*! Provides a world-space 3D point.
    */
    class WorldPointProvider : public ArrayKnobI::ValueProvider
    {
      AxisOp* _axisOp;

    public:
      WorldPointProvider(AxisOp* axisOp) : _axisOp(axisOp) {}

      bool provideValuesEnabled(const ArrayKnobI* arrayKnob, const OutputContext&) const override;

      std::vector<double> provideValues(const ArrayKnobI* arrayKnob, const OutputContext&) const override;
    };


    /*! Provides values for the AxisOp \a 'world_matrix' knob.
    */
    class DDImage_API WorldMatrixProvider : public ArrayKnobI::ValueProvider
    {
      AxisOp* _axisOp;

    public:
      WorldMatrixProvider(AxisOp* axisOp) : _axisOp(axisOp) {}

      bool provideValuesEnabled(const ArrayKnobI* arrayKnob, const OutputContext&) const override;

      std::vector<double> provideValues(const ArrayKnobI* arrayKnob, const OutputContext&) const override;
    };


    //---------------------------------------------------------------------


    /*! \class DD::Image::AxisOp

        \brief AxisOp is the base class for all the 3D 'scene' objects like
        locators (usg::XformPrim), cameras (usg::CameraPrim) and lights (usg::tbd).

        It contains a local transform matrix and an optional local-parent transform
        matrix. The local-parent enables an arbitrary parenting hierarchy to be
        decomposed and contained within the AxisOp allowing the node to be disconnected
        or disassociated from a parenting network or scenegraph. The node is
        effectively localized and can be copied, pasted into other scripts, etc.
     */
    class DDImage_API AxisOp : public Op
                             , public SceneLoaderOpI
                             , public ScenePrimExporterOpI
#if kDDImageVersionMajorNum <= 16
                             , public LookAt
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif
    {
    protected:
      bool         _parentExtractEnable;    //!< Split the parent xform out from the local
      bool         _syncLocalParent;        //!< Local-parent read lock
      bool         _syncLocalAxis;          //!< Local-axis read lock
      //
      fdk::Vec3d   _localParentTranslate;   //!< Local-parent translation knob value
      fdk::Vec3d   _localParentRotate;      //!< Local-parent rotate knob value
      fdk::Vec3d   _localParentScale;       //!< Local-parent scale knob value
      fdk::Mat4d   _localParentMatrix;      //!< Parent transform derived from local parent TRS knobs
      //
      fdk::Mat4d   _localAxisMatrix;        //!< Local matrix that Axis_Knob fills in (no lookat rotations)
      //
      ndk::XformParameters _xformParams;    //!< Axis xform params

      DD::Image::Hash _parentHash;          //!< Does parent path or transform need to be updated?

#if kDDImageVersionMajorNum <= 16
      //----------------------------------------------------------------------------
      // Deprecation unions to retain backward-compatibility in Nuke 14
      // The old var names will disappear in a Nuke major rev after 16
      //----------------------------------------------------------------------------
      union {
        bool        _xformInversionValid;         //!< Is _xformParams.w2l valid?
        mFnDeprecatedLegacy3DSystemInNuke14("inversion_updated has been deprecated, use _xformInversionValid instead.")
        bool inversion_updated;    //!< Use _xformInversionValid instead
      };
      union {
        Axis_KnobI* _axisKnob;                    //!< Reference to the node Axis_Knob
        mFnDeprecatedLegacy3DSystemInNuke14("axis_knob has been deprecated, use _axisKnob instead.")
        Axis_KnobI* axis_knob;     //!< Use _axisKnob instead
      };
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
      bool         _xformInversionValid;          //!< Is _xformParams.w2l valid?
      Axis_KnobI*  _axisKnob;                     //!< Reference to the node Axis_Knob
#endif
      WorldMatrixProvider* _worldMatrixProvider;  //!< 'world_matrix' output knob
      //
      Knob*  _localParentTranslateKnob;   //!< If not null apply parent_translate
      Knob*  _localParentRotateKnob;      //!< If not null apply parent_rotate
      Knob*  _localParentScaleKnob;       //!< If not null apply parent_scale

      int   display3d_;                   //!< GUI display setting      - deprecate or translate to new system?
      bool  selectable_;                  //!< GUI selectable checkmark - deprecate or translate to new system?
      //
      bool  _editable;                    //!< Can the user change the knobs? (i.e. knobs not disabled)

      SnapKnobs _snapKnobs;

    public:
      AxisOp(Node* node,
             int   defaultLookAtAxis = LookAt::kAxisZPlus);

      virtual ~AxisOp();

      //! Cast this Op to an AxisOp. This is much cheaper and safer than using dynamic_cast.
      const AxisOp* axisOp() const override { return this; }
            AxisOp* axisOp()       override { return this; }

      //! Convenience that casts parent input 0 to an AxisOp.
      AxisOp* parentInputOp() const { return op_cast<AxisOp*>(Op::input(0)); }

      //! Return the node input to use for the parent axis source. Always defaults to input 0.
      int32_t          parentInput() const { return 0; }

      bool can_link() const override { return false; }

      //! Draw a circular node shape.
      const char*  node_shape() const override;
      unsigned     node_color() const override;

      //! Default is 2: axis and scene(optional)
      int          minimum_inputs() const override;
      //! Default is 2: axis and scene(optional)
      int          maximum_inputs() const override;
      //! Optional input index is the scene-importer input. This returns (maximum_inputs()-1).
      int          optional_input() const override;

      //! AxisOp and null works on 0, and GeomOp or null on input 1.
      bool         test_input(int node_input, Op* op) const override;

      //! Returns null for all inputs. Override this so Op::test_input() doesn't produce warnings.
      Op*          default_input(int node_input) const override;

      //!
      const char*  input_label(int node_input, char*) const override;


#if kDDImageVersionMajorNum <= 16
      /*! *** Nuke14 only *** virtual method is used to indicate that the AxisOp
          subclass is a legacy one or not.

          The new GeomOp-based AxisOp plugins return false, changing the behavior
          of the AxisOp base class methods like knobs(), _validate(), test_input(),
          minimum_inputs(), maximum_inputs(), etc so old plugins building against
          AxisOp get the same behavior.

          Existing AxisOp subclass plugins like 'Axis2', 'Axis3', 'Camera2', 'Camera3',
          'Light2', 'Light3', etc will retain their current behaviors through Nuke14,
          but those plugins will be removed in Nuke15, and existing scripts using them
          will no longer work.
      */
      mFnDeprecatedLegacy3DSystemInNuke14("isLegacyAxisOp() has been deprecated, port AxisOp sublass to new Nuke 14 api.")
      bool isLegacyAxisOp() const;
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif


      //----------------------------------------------------------------------------
      // SceneLoaderOpI scene prim importer handling
      //----------------------------------------------------------------------------

      //! SceneLoaderOpI: Return the Op the interface is attached to.
      Op*      getSceneLoaderOp() override { return this; }

      //! SceneLoaderOpI: If loader is attached to an AxisOp subclass return 'this'.
      AxisOp*  asAxisLoaderOp() override { return this; }

      //! SceneLoaderOpI: Return the node input to use for the scene importer. Default returns optional_input().
      int32_t  sceneLoaderSourceInput() const override { return optional_input(); }

      //! Convenience that casts input sceneImporterInput() to a GeomOp.
      GeomOp*  sceneInputOp() const { return op_cast<GeomOp*>(Op::input(sceneLoaderSourceInput())); }

      /*! SceneLoaderOpI: Try to import prim data from the provided stage.
          Will return 1(error) if stage is invalid. If \a prim_path is empty then -1
          will be returned indicating no change has occured, otherwise if \a prim_path
          is invalid or not present in stage then 1(error) will be returned.

          Return codes:
          -1 - no prim data was loaded and no error, usually due to empty prim path
           0 - prim data was loaded with no error
           1 - there was an error and no prim data was loaded
      */
      int32_t  importScenePrim(const usg::StageRef& stage,
                               const std::string&   primPath) override;


      //----------------------------------------------------------------------------
      // GeometryProviderI handling
      //----------------------------------------------------------------------------

      //! GeometryProviderI:  Return the Op the interface is attached to.
      Op*  getGeometryProviderOp() override { return this; }

      //! GeometryProviderI: Allow subclasses to gain access to sibling functions.
      GeometryProviderI*  asGeometryProvider() override { return this; }

      //! Cast to a GeometryProviderI interface. This is much cheaper and safer than using dynamic_cast.
            GeometryProviderI* geometryProvider()       override { return asGeometryProvider(); }
      const GeometryProviderI* geometryProvider() const override { return const_cast<AxisOp*>(this)->asGeometryProvider(); }


      //----------------------------------------------------------------------------
      // ScenePrimExporterOpI scene prim output handling
      //----------------------------------------------------------------------------

      //! ScenePrimExporterOpI:  Return the Op the interface is attached to.
      Op*  getScenePrimExporterOp() override { return this; }

      //! ScenePrimExporterOpI: Allow subclasses to gain access to sibling functions.
      ScenePrimExporterOpI*  asScenePrimExporter() override { return this; }

      //! Cast to a ScenePrimExporterOpI interface. This is much cheaper and safer than using dynamic_cast.
            ScenePrimExporterOpI* scenePrimExporterOpI()       override { return asScenePrimExporter(); }
      const ScenePrimExporterOpI* scenePrimExporterOpI() const override { return const_cast<AxisOp*>(this)->asScenePrimExporter(); }

      //! ScenePrimExporterOpI: Prepends the parent connection if there is one.
      usg::Path  exportBasePrimPath() const override;

      // ScenePrimExporterOpI: Returns 'Xform' for AxisOp base class.
      int32_t  exportBasePrimType() const override;


      //----------------------------------------------------------------------------
      // Knobs
      //----------------------------------------------------------------------------

      //!
      void  knobs(Knob_Callback f) override;

      //! Create the Axis Op Knobs and set a custom prim filter to use on the import popup.
      void createAxisOpKnobs(Knob_Callback      f,
                             bool               openImportGroup,
                             bool               openExportGroup,
                             const std::string& primFilters = "");

      //!
      int   knob_changed(Knob* k) override;

      //! Adds the OpenGL display option controls.
      virtual void  addDisplayOptionsKnobs(Knob_Callback f);

      /*! Add the local-parent TRS knobs. If \a addSyncOptions is true the
          import-sync options are added.
      */
      virtual void  addLocalParentingKnobs(Knob_Callback f,
                                           bool          addSyncOptions = true);

      /*! Adds the front panel local-axis transform knobs. If \a addSyncOptions is
          true the import-sync options are added.
      */
      virtual void  addLocalXformKnobs(Knob_Callback f,
                                       bool          addSyncOptions = true);

      /*! Adds the front panel local-parent & local-axis transform knobs. If
          \a addSyncOptions is true the import-sync options are added.
          Calls addLocalParentingKnobs() then addLocalXformKnobs(), adding dividers.
      */
      virtual void  addTransformKnobs(Knob_Callback f,
                                      bool          addSyncOptions = true);

      //! Adds the scene importer option knobs. A subclass can override this to
      //! include additional knobs within the import knob group.
      //! Default implementation calls addSceneLoaderKnobs(primFilters).
      virtual void addImportOptionsKnobs(Knob_Callback f, const std::string& primFilters = "");

      //! Adds the prim export option knobs.
      void addExportOptionsKnobs(Knob_Callback f) override;

      //! Adds addl front-panel knobs. Called after addTransformKnobs(). Base class adds nothing.
      virtual void  addExtraFrontPanelKnobs(Knob_Callback f);

      //-----------------------------

      //! Return internal 'transform' Axis_Knob for set/get internal transformation.
      Axis_KnobI*   getAxis();

      /*! Enable or disable axis knobs.
          \param on true to enable the axis knobs ('transform')
      */
      void  enableAxisKnobs(bool on);

      /*! Enable or disable knobs based on array of knobnames.
          \param knobNames knob names to enable or disable
          \param on true to enable the knobs
      */
      void  enableKnobs(const std::vector<std::string>& knobNames, bool on);

      //! SceneLoaderOpI: Enable/disable knobs filled in by the scene importer.
      void  enableSceneLoaderKnobs(bool importEnabled) override;
      void  enableSceneLoaderExtraKnobs(bool importEnabled) override;
      //! SceneLoaderOpI: Enable/disable knobs filled in by the xform decomposer.
      void  enableXformExtractKnobs(bool xformExtractEnabled) override;

      //! ScenePrimExporterOpI: Enable/disable knobs filled in by the scene prim exporter.
      void  enableExportOptionsKnobs(bool exportEnabled) override;

      //----------------------------------------------------------------------------
      // Transforms
      //----------------------------------------------------------------------------

      /*! Validate the parent axis first, if any, then concatenate the local transform
          with that.  If the resulting matrix has changed, update the inversion flag.
      */
      void _validate(bool for_real) override;

      // Input-parent transform if connected to a valid parent input source.
      const fdk::Mat4d&   inputParentTransform() const;

      //! Result of the local-parent TRS knobs, if enabled. Calculated in _validate().
      const fdk::Mat4d&   localParentTransform() const { return _localParentMatrix; }

      //! Result of the local-axis TRS Axis_Knob (*without* lookat rotations.) Calculated in knobs().
      const fdk::Mat4d&   localAxisTransform() const { return _localAxisMatrix; }

      //! Parent transform containing the input-parent and local-parent. Calculated in _validate().
      const fdk::Mat4d&   parentTransform() const { return _xformParams.parent; }

      //! Local object transform containing the local-axis TRS Axis_Knob *with* lookat rotations. Calculated in _validate().
      const fdk::Mat4d&   localTransform() const { return _xformParams.local; }

      //! World transform containing the input-parent, local-parent, local-axis, and lookat. Calculated in _validate().
      const fdk::Mat4d&   worldTransform() const { return _xformParams.l2w; }

      /*! Return the inverse world transform. If \a worldTransform has been marked invalid
          the inverse world transform will be calculated and updated now.
       */
      const fdk::Mat4d&   invWorldTransform();

      /*! Calculate the world-space transform of this node at a specific OutputContext.
          Calls Knob::store() on all the transform knobs but discards the results, so this
          method can be more expensive than creating an AxisOp at the OutputContext and
          reusing it.
      */
      virtual fdk::Mat4d  worldTransformAt(const OutputContext& context) const;

      /*! Replace the output matrix derived in _validate(), also invalidating the inverse
          world transform. Note this only persists until the next validate() pass
          overwrites the derived matrices.
      */
      void                setWorldTransform(const fdk::Mat4d&);

      //----------------------------------------------------------------------------

      int display3d() const { return display3d_; }
      int display3d()       { return display3d_; }
      // only constructors should call this, unless knobs() is not called:
      void setDisplay3d(int mode) { display3d_ = mode; }

      bool selectable() const { return selectable_; }
      void setSelectable(bool v) { selectable_ = v; }

      bool editable() const { return _editable; }
      void setEditable(bool v) { _editable = v; }

      //! Return true if a particular knob transformation is modifiable by user interaction in gui.
      virtual bool isGuiInteractive(Axis_KnobI::Transformation knob) const;

      //----------------------------------------------------------------------------

      //!
      HandlesMode  doAnyHandles(ViewerContext*) override;

      /*! This default version will always cause draw_handle() to be called
          when in 3D mode. Subclasses (i.e. the camera) should replace draw_handle
          with their own version to draw what they want.
      */
      void         build_handles(ViewerContext*) override;

      /*! Draws any geometry attached to this axis.
          Note that the Axis knob will draw the 3-arrow axis control at its
          pivot-translate location.
      */
      void         draw_handle(ViewerContext*) override;


    protected:
      //! SceneLoaderOpI: Change the import hash with local vars.
      void _updateScenePrim(fdk::Hash& importHash) override;

      //! Called by Nuke just BEFORE Op::knobs(EARLY_STORE).
      //! This is used to update the scene importer and exporter states.
      void _knobStorePrep(const OutputContext& opStoreContext) override;

      //! Called by Nuke immediately AFTER Op::knobs().
      //! This is used to update the scene importer and exporter states.
      void _knobStorePost(const OutputContext& opStoreContext, Hash& opHash) override;

      //! ScenePrimExporterOpI: If parent is connected include its hashes in the export state.
      fdk::Hash _buildScenePrimExporterState(const fdk::Hash& hash) override;

      //! ScenePrimExporterOpI: Subclass portion of exportScenePrims().
      void _exportScenePrims(usg::LayerRef&            exportLayer,
                             int32_t                   exportMode,
                             const fdk::FrameValueSet& sampleFrames = fdk::FrameValueSet()) const override;

      //! Export the transform data to an Xformable prim for all the specified times, or
      //! all keys, depending on the \a exportMode.
      //! A subclass can override this to do additional specialization.
      //!
      //! Base class implementation writes separate 'local_parent' and 'local_axis'
      //! matrix XformOps.
      virtual void _exportTransformOps(usg::XformablePrim&       xformable,
                                       int32_t                   exportMode,
                                       const fdk::FrameValueSet& sampleFrames = fdk::FrameValueSet()) const;

#if kDDImageVersionMajorNum <= 16
      /*! *** Nuke14 only *** virtual method is used to indicate that the AxisOp
          subclass is a legacy one or not.
      */
      mFnDeprecatedLegacy3DSystemInNuke14("isLegacyAxisOp() has been deprecated, port AxisOp sublass to new Nuke 14 api.")
      virtual bool _isLegacyAxisOp() const { return true; }

    protected:
      //----------------------------------------------------------------------------
      // Deprecated single-precision matrices - these are derived from the
      // new double-precision matrices. Will disappear in a Nuke major rev after 16
      //----------------------------------------------------------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("localtransform_ has been deprecated, use Mat4d _localTransform instead.")
        Matrix4 localtransform_;    //!< Local 'axis' matrix that Axis_Knob fills in - deprecated
      mFnDeprecatedLegacy3DSystemInNuke14("local_ has been deprecated, use Mat4d _localAxisMatrix instead.")
        Matrix4 local_;             //!< Local matrix after look at performed - deprecated
      mFnDeprecatedLegacy3DSystemInNuke14("matrix_ has been deprecated, use Mat4d _xformParams.l2w instead.")
        Matrix4 matrix_;            //!< Object matrix - local&parent - deprecated
      mFnDeprecatedLegacy3DSystemInNuke14("imatrix_ has been deprecated, use Mat4d _xformParams.w2l instead.")
        Matrix4 imatrix_;           //!< Inverse object matrix - deprecated

    public:
      //----------------------------------------------------------------------------
      // Deprecated functions - will disappear in a Nuke major rev after 16
      //----------------------------------------------------------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("Class() has been deprecated, implement on subclass plugin instead.")
        const char*  Class() const override { return "Axis"; }
      mFnDeprecatedLegacy3DSystemInNuke14("node_help() has been deprecated, implement on subclass plugin instead.")
        const char*  node_help() const override { return ""; }
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("selectable(bool mode) has been deprecated, use setSelectable() instead.")
        void  selectable(bool v) { setSelectable(v); }
      mFnDeprecatedLegacy3DSystemInNuke14("display3d(int mode) has been deprecated, use setDisplay3d() instead.")
        void  display3d(int mode) { setDisplay3d(mode); }
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("input0() has been deprecated, use parentInput() instead.")
        AxisOp*        input0() const { return op_cast<AxisOp*>(Op::input(0)); }
      mFnDeprecatedLegacy3DSystemInNuke14("lookAtEnabled() has been deprecated")
        virtual bool   lookAtEnabled() const;
      mFnDeprecatedLegacy3DSystemInNuke14("lookat_input() has been deprecated")
        Op*            lookat_input() const override;
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("matrix(const Matrix4&) has been deprecated, use setWorldTransform() instead.")
        void            matrix(const Matrix4&);
      mFnDeprecatedLegacy3DSystemInNuke14("local() has been deprecated, use localTransform() instead.")
        const Matrix4&  local() const;
      mFnDeprecatedLegacy3DSystemInNuke14("matrix() has been deprecated, use worldTransform() instead.")
        const Matrix4&  matrix() const;
      mFnDeprecatedLegacy3DSystemInNuke14("imatrix() has been deprecated, use invWorldTransform() instead.")
        const Matrix4&  imatrix();
      mFnDeprecatedLegacy3DSystemInNuke14("matrixAt() has been deprecated, use worldTransformAt() instead.")
        virtual void    matrixAt(const OutputContext&, Matrix4&) const;
      mFnDeprecatedLegacy3DSystemInNuke14("print_info() has been deprecated")
        virtual void    print_info(std::ostream&) const {}
      //----------------------------------------------------------------------------
      //----------------------------------------------------------------------------
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif

      //! Not implemented in the library but you can define it
      friend std::ostream& operator<<(std::ostream&, const AxisOp&);
    };

  }
}
#endif
