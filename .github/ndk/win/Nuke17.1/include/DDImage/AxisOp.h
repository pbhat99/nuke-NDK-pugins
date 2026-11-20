// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_AXISOP_H
#define DDIMAGE_AXISOP_H

#include "DDImage/Op.h"
#include "DDImage/KnobMixIns.h"
#include "DDImage/ConstrainOpI.h"

#include "DDImage/Knob.h"
#include "DDImage/ArrayKnobI.h"
#include "DDImage/AxisVals.h"
#if kDDImageVersionMajorNum <= 17
#  include "DDImage/LookAt.h"
#  include "DDImage/Matrix4.h"
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
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

#if kDDImageVersionMajorNum <= 17
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
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif

    //----------------------------------------------------------------------


    //! Provides a world-space 3D point.
    class WorldPointProvider : public ValueProvider
    {
      AxisOp* _axisOp;
    public:
      WorldPointProvider(AxisOp* axisOp) : _axisOp(axisOp) {}

      bool provideValuesEnabled(const Knob* knob, const OutputContext&) const override;
      int  providerSourceType(const Knob* knob, const OutputContext&) const override;
      bool isDefault(const Knob* knob, const OutputContext&) const override;
      bool isAnimated(const Knob* knob, const OutputContext&) const override;

      std::vector<double> provideValues(const ArrayKnobI* arrayKnob, const OutputContext&) const override;
    };


    //! Provides values for the AxisOp \a 'world_matrix' knob.
    class DDImage_API WorldMatrixProvider : public ValueProvider
    {
      AxisOp* _axisOp;
    public:
      WorldMatrixProvider(AxisOp* axisOp) : _axisOp(axisOp) {}

      bool provideValuesEnabled(const Knob* knob, const OutputContext&) const override;
      int  providerSourceType(const Knob* knob, const OutputContext&) const override;
      bool isDefault(const Knob* knob, const OutputContext&) const override;
      bool isAnimated(const Knob* knob, const OutputContext&) const override;

      std::vector<double> provideValues(const ArrayKnobI* arrayKnob, const OutputContext&) const override;
    };


    //---------------------------------------------------------------------


    /*! \class DD::Image::AxisOp

        \brief AxisOp is the base class for all the 3D 'scene' objects like locators,
        cameras and lights.

        AxisOp contains a world transform matrix sampled at the Op's OutputContext
        and nothing more. This base class is intentionally kept simple and without
        transform Knobs or input connections to allow AxisOp subclasses to be filled
        in via other OutputContext sampling methods such as direct conversion from
        USD prims. The CameraOp and LightOp base classes have been similarily
        simplified for the same reason.

        AxisSceneOp is the Op class that now contains all the transform Knobs and
        parenting/lookat input connections to other SceneOp nodes like CameraSceneOps
        and LightSceneOps. See those classes for more info.
     */
    class DDImage_API AxisOp : public Op
#if kDDImageVersionMajorNum <= 17
                             , public LookAt
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif
    {
    protected:
      ndk::XformParameters _xformParams;    //!< Axis xform params

#if kDDImageVersionMajorNum <= 17
      //----------------------------------------------------------------------------
      // Deprecation unions to retain backward-compatibility in Nuke 14
      // The old var names will disappear in a Nuke major rev after 16
      //----------------------------------------------------------------------------
      union {
        bool        _xformInversionValid;         //!< Is _xformParams.w2l valid?
        mFnDeprecatedLegacy3DSystemInNuke14("inversion_updated has been deprecated, use _xformInversionValid instead.")
        bool inversion_updated;    //!< Use _xformInversionValid instead
      };
      mFnDeprecatedLegacy3DSystemInNuke14("axis_knob has been deprecated")
      Axis_KnobI* axis_knob;

      int   display3d_;     //!< GUI display setting      - deprecate or translate to new system?
      union {
        bool  selectable_;  //!< GUI selectable checkmark - deprecate or translate to new system?
        bool  _selectable;
      };
      bool  _editable;      //!< Can the user change the knobs? (i.e. knobs not disabled)

      WorldMatrixProvider* _worldMatrixProvider;  //!< 'world_matrix' output knob
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
      bool         _xformInversionValid;          //!< Is _xformParams.w2l valid?
#endif

    public:
      AxisOp(Node* node
#if kDDImageVersionMajorNum <= 17
             , int   defaultLookAtAxis = LookAt::kAxisZPlus
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif
            );

      virtual ~AxisOp();

      //! Cast this Op to an AxisOp. This is much cheaper and safer than using dynamic_cast.
      const AxisOp* axisOp() const override { return this; }
            AxisOp* axisOp()       override { return this; }

      //! Retrieve a version of this AxisOp at a specific OutputContext.
      AxisOp* getAxisOpAt(const OutputContext& context) { return BuildOpAt(this, context)->axisOp(); }

      bool can_link() const override { return false; }

      //! Draw node as a circular shape.
      const char*  node_shape() const override;
      unsigned     node_color() const override;

      //----------------------------------------------

#if kDDImageVersionMajorNum <= 17
      //! Convenience that casts parent input 0 to an AxisOp.
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
      //! Return the parentInput() input cast to an AxisOp, if possible. This can be further
      //! cast to a SceneOp (or directly to an AxisSceneOp/CameraSceneOp/LightSceneOp)
      //! using 'op_cast()' like so:
      //! 'SceneOpI* sceneOp = op_cast<SceneOpI*>(axisOp->parentInputOp())'.
      //! See AxisSceneOp for the subclass version that supports input connections.
#endif
      virtual AxisOp* parentInputOp() const;

#if kDDImageVersionMajorNum <= 17
      //! Return the node input to use for the parent axis source, or -1 if no parent input.
      //! Default returns input 0.
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
      //! Return the node input to use for the parent axis source, or -1 if no parent input.
      //! Default returns -1, no parent input. See AxisSceneOp for the subclass version that
      //! supports input connections.
#endif
      virtual int32_t parentInput() const;

      //! Return the constrainInput() input cast to an AxisOp, if possible.
      virtual AxisOp* constrainInputOp() const;

      //! Return the node input to use for the constrain source, or -1 if no constrain input.
      //! Default returns -1, no constraint input. See AxisSceneOp for the subclass version
      //! that supports input connections.
      virtual int32_t constrainInput() const;

      //----------------------------------------------

#if kDDImageVersionMajorNum <= 17
      //! Default is 2: axis and scene(optional)
      int          minimum_inputs() const override;
      //! Default is 3: axis, constraint and scene
      int          maximum_inputs() const override;
      //! Optional input index
      int          optional_input() const override;

      //! AxisOp and null works on 0, GeomOp or null on input 1 and  AxisOp or null on input 2
      bool         test_input(int node_input, Op* op) const override;

      //! Returns null for all inputs. Override this so Op::test_input() doesn't produce warnings.
      Op*          default_input(int node_input) const override;

      //!
      const char*  input_label(int node_input, char*) const override;

      /*! *** Nuke14 only *** virtual method is used to indicate that the AxisOp
          subclass is a legacy one or not.

          The new GeomOp-based AxisOp plugins return false, changing the behavior
          of the AxisOp base class methods like knobs(), _validate(), test_input(),
          minimum_inputs(), maximum_inputs(), etc so old plugins building against
          AxisOp get the same behavior.

          Existing AxisOp subclass plugins like 'Axis2', 'Axis3', 'Camera2', 'Camera3',
          'Light2', 'Light3', etc will retain their current behaviors through Nuke17,
          but those plugins will be removed in Nuke18, and existing scripts using them
          will no longer work.
      */
      mFnDeprecatedLegacy3DSystemInNuke14("isLegacyAxisOp() has been deprecated, port AxisOp sublass to AxisSceneOp.")
      bool isLegacyAxisOp() const;

      //!
      void  knobs(Knob_Callback f) override;

      //!
      int   knob_changed(Knob* k) override;
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif

#if kDDImageVersionMajorNum <= 17
      //! Validate the parent axis first, if any, then concatenate the local transform
      //! with that. If the resulting matrix has changed, update the inversion flag.
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
      //! Build the concatenated world transform matrix from the XformParameters and
      //! if changed update the inversion flag.
#endif
      void _validate(bool for_real) override;


      //----------------------------------------------------------------------------
      // Transforms
      //----------------------------------------------------------------------------

      //! Does the local parent transform animate? This is only an indicator that the
      //! transform -MAY- vary in time, not that its value actually does.
      //! A subclass can determine this from any of its knobs animation state and any
      //! input Ops which may affect the local parent transform.
      //! Default implementation returns false.
      virtual bool  localParentTransformVaries() const { return false; }

      //! Does the local transform animate? This is only an indicator that the transform
      //! -MAY- vary in time, not that its value actually does.
      //! A subclass can determine this from any of its knobs animation state and any
      //! input Ops which may affect the local transform.
      //! Default implementation returns false.
      virtual bool  localTransformVaries() const { return false; }

      //------------------------------------

      //! Transform components.
      //! Default implementation returns an empty XformComponents struct.
      virtual const ndk::XformComponents&  transformComponents() const;

      //! Calculate the transform components of \a axisOp at a specific OutputContext.
      static ndk::XformComponents  GetTransformComponentsAt(const AxisOp* axisOp, const OutputContext& context);

      //------------------------------------

      //! Input parent transform, if any, otherwise the identity.
      //! Default implementation returns an identity matrix.
      virtual const fdk::Mat4d&  inputParentTransform() const;

      //! Set the input parent transform matrix. Note that depending on the subclass
      //! implementation this may only persists until the next knobs()/validate() pass.
      //! Default implementation does nothing.
      virtual void  setInputParentTransform(const fdk::Mat4d&);

      //! Calculate the input parent transform of \a axisOp at a specific OutputContext.
      static fdk::Mat4d  GetInputParentTransformAt(const AxisOp* axisOp, const OutputContext& context);

      //------------------------------------

      //! Local parent axis transform, if any, otherwise the identity.
      //! Default implementation returns an identity matrix.
      virtual const fdk::Mat4d&  localParentAxisTransform() const;

      //! Set the local parent axis transform matrix. Note that depending on the subclass
      //! implementation this may only persists until the next knobs()/validate() pass.
      //! Default implementation does nothing.
      virtual void  setLocalParentAxisTransform(const fdk::Mat4d&);

      //! Calculate the local parent axis transform of \a axisOp at a specific OutputContext.
      static fdk::Mat4d  GetLocalParentAxisTransformAt(const AxisOp* axisOp, const OutputContext& context);

      //------------------------------------

      //! Local 'axis' transform, if any, otherwise the identity. Typically this is the
      //! TRS Axis_Knob results separate from additional constraints or stereo offsets.
      //! Default implementation returns an identity matrix.
      virtual const fdk::Mat4d&  localAxisTransform() const;

      //! Set the local axis transform matrix. Note that depending on the subclass
      //! implementation this may only persists until the next knobs()/validate() pass.
      //! Default implementation does nothing.
      virtual void  setLocalAxisTransform(const fdk::Mat4d&);

      //! Calculate the local axis transform of \a axisOp at a specific OutputContext.
      static fdk::Mat4d  GetLocalAxisTransformAt(const AxisOp* axisOp, const OutputContext& context);

      //------------------------------------

      //! Parent transform, if any, otherwise the identity. Should include
      //! localParentAxisTransform() and any constraint transform applied.
      const fdk::Mat4d&  parentTransform() const { return _xformParams.parent; }

      //! Set the parent transform matrix. Note that depending on the subclass
      //! implementation this may only persists until the next knobs()/validate() pass.
      void  setParentTransform(const fdk::Mat4d&);

      //------------------------------------

      //! Local object transform, if any, otherwise the identity. Should include
      //! localAxisTransform() and any constraint, stereo or any other local
      //! transform applied.
      const fdk::Mat4d&  localTransform() const { return _xformParams.local; }

      //! Set the local transform matrix. Note that depending on the subclass
      //! implementation this may only persists until the next knobs()/validate() pass.
      void  setLocalTransform(const fdk::Mat4d&);

      //------------------------------------

      //! World transform containing the concatenated parent and local transforms.
      //! Calculated in _validate() and overridable via setWorldTransform().
      const fdk::Mat4d&  worldTransform() const { return _xformParams.l2w; }

      //! Return the inverse world transform. If \a worldTransform has been marked invalid
      //! the inverse world transform will be calculated and updated now.
      const fdk::Mat4d&  invWorldTransform();

      //! Replace the output matrix derived in _validate(), also invalidating the inverse
      //! world transform. Note this only persists until the next validate() pass
      //! overwrites the derived matrices.
      void  setWorldTransform(const fdk::Mat4d&);

      //! Calculate the world transform of \a axisOp at a specific OutputContext.
      static fdk::Mat4d  GetWorldTransformAt(const AxisOp* axisOp, const OutputContext& context);

      //----------------------------------------------------------------------------

      int display3d() const { return display3d_; }
      int display3d()       { return display3d_; }

      // only constructors should call this, unless knobs() is not called:
      void setDisplay3d(int mode) { display3d_ = mode; }


    protected:
      //----------------------------------------

      //! Calculate the transform components of this node at a specific OutputContext.
      //! Subclass implementation of GetTransformComponentsAt(AxisOp).
      //! Base class does nothing and returns an empty XformComponents struct.
      virtual ndk::XformComponents _getTransformComponentsAt(const OutputContext& context) const;

      //----------------------------------------

      //! Calculate the input parent transform of this node at a specific OutputContext.
      //! Subclass implementation of GetInputParentTransformAt(AxisOp).
      //! Base class does nothing and returns \a inputParentTransform().
      virtual fdk::Mat4d _getInputParentTransformAt(const OutputContext& context) const;

      //! Calculate the local parent transform of this node at a specific OutputContext.
      //! Subclass implementation of GetLocalParentAxisTransformAt(AxisOp).
      //! Base class does nothing and returns \a localParentAxisTransform().
      virtual fdk::Mat4d _getLocalParentAxisTransformAt(const OutputContext& context) const;

      //! Calculate the local axis transform of this node at a specific OutputContext.
      //! Subclass implementation of GetLocalAxisTransformAt(AxisOp).
      //! Base class does nothing and returns \a localAxisTransform().
      virtual fdk::Mat4d _getLocalAxisTransformAt(const OutputContext& context) const;

      //----------------------------------------

      //! Calculate the world transform of this node at a specific OutputContext.
      //! Subclass implementation of GetWorldTransformAt(AxisOp).
      //! Base class does nothing and returns \a worldTransform().
      virtual fdk::Mat4d _getWorldTransformAt(const OutputContext& context) const;


#if kDDImageVersionMajorNum <= 17
    public:
      /*! *** Nuke14-17 only *** virtual method is used to indicate that the AxisOp
          subclass is a legacy one or not.
      */
      mFnDeprecatedLegacy3DSystemInNuke14("isLegacyAxisOp() has been deprecated, port AxisOp sublass to AxisSceneOp.")
      virtual bool _isLegacyAxisOp() const { return true; }

      mFnDeprecatedLegacy3DSystemInNuke14("worldTransformAt() deprecated on AxisOp, use the static AxisOp::GetWorldTransformAt(AxisOp) "
                                          "method instead. If implementing an override of this method implement _getWorldTransformAt().")
      virtual fdk::Mat4d worldTransformAt(const OutputContext& context) const;

      mFnDeprecatedLegacy3DSystemInNuke14("getAxis() deprecated on AxisOp, now implemented on AxisSceneOp::getLocalAxisKnob().")
      Axis_KnobI* getAxis();

      mFnDeprecatedLegacy3DSystemInNuke14("enableAxisKnobs() deprecated on AxisOp, now implemented on AxisSceneOp.")
      void enableAxisKnobs(bool on);

      mFnDeprecatedLegacy3DSystemInNuke14("enableKnobs() deprecated on AxisOp, now implemented on AxisSceneOp.")
      void enableKnobs(const std::vector<std::string>& knobNames, bool on);

      mFnDeprecatedLegacy3DSystemInNuke14("doAnyHandle() deprecated on AxisOp, implement on AxisSceneOp instead.")
      HandlesMode doAnyHandles(ViewerContext*) override;

      mFnDeprecatedLegacy3DSystemInNuke14("draw_handle() deprecated on AxisOp, implement on AxisSceneOp instead.")
      void build_handles(ViewerContext*) override;

      mFnDeprecatedLegacy3DSystemInNuke14("draw_handle() deprecated on AxisOp, implement on AxisSceneOp instead.")
      void draw_handle(ViewerContext*) override;

      bool selectable() const { return selectable_; }
      void setSelectable(bool v) { selectable_ = v; }

      mFnDeprecatedLegacy3DSystemInNuke14("editable() deprecated on AxisOp, implement on AxisSceneOp instead.")
      bool editable() const { return _editable; }
      mFnDeprecatedLegacy3DSystemInNuke14("setEditable() deprecated on AxisOp, implement on AxisSceneOp instead.")
      void setEditable(bool v) { _editable = v; }

      mFnDeprecatedLegacy3DSystemInNuke14("isGuiInteractive() deprecated on AxisOp, implement on AxisSceneOp instead.")
      virtual bool isGuiInteractive(Axis_KnobI::Transformation knob) const;


    protected:
      //----------------------------------------------------------------------------
      // Deprecated single-precision matrices - these are derived from the
      // new double-precision matrices. Will disappear in a Nuke major rev after 16
      //----------------------------------------------------------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("localtransform_ has been deprecated, use Mat4d AxisSceneOp::_localAxisMatrix instead.")
        Matrix4 localtransform_;    //!< Local 'axis' matrix that Axis_Knob fills in - deprecated
      mFnDeprecatedLegacy3DSystemInNuke14("local_ has been deprecated, use Mat4d _xformParams.local instead.")
        Matrix4 local_;             //!< Local matrix after look at performed - deprecated
      mFnDeprecatedLegacy3DSystemInNuke14("matrix_ has been deprecated, use Mat4d _xformParams.l2w instead.")
        Matrix4 matrix_;            //!< Object matrix - local&parent - deprecated
      mFnDeprecatedLegacy3DSystemInNuke14("imatrix_ has been deprecated, use Mat4d _xformParams.w2l instead.")
        Matrix4 imatrix_;           //!< Inverse object matrix - deprecated

    public:
      //----------------------------------------------------------------------------
      // Deprecated functions - will disappear in a Nuke major rev after 17
      //----------------------------------------------------------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("Class() has been deprecated, implement on subclass plugin instead.")
        const char*  Class() const override { return "Axis"; }
      mFnDeprecatedLegacy3DSystemInNuke14("node_help() has been deprecated, implement on subclass plugin instead.")
        const char*  node_help() const override { return ""; }
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("selectable(bool mode) has been deprecated, use AxisSceneOp::setSelectable() instead.")
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
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 14."
#endif

      //! Not implemented in the library but you can define it
      friend std::ostream& operator<<(std::ostream&, const AxisOp&);
    };


  }
}
#endif
