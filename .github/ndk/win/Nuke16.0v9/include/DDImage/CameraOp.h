//------------------------------------------------------------------------------
// DDImage/CameraOp.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_CAMERAOP_H
#define DDIMAGE_CAMERAOP_H

#include "DDImage/AxisOp.h"

#include "ndk/geo/camera/MultiProjectionCamera.h"

#include "fdk/math/Vec2.h"
#include "fdk/math/Mat4.h"
#include "fdk/math/types.h" // for Mat4dList

#if kDDImageVersionMajorNum <= 16
#include "DDImage/Shutter.h"
#include "DDImage/Vector2.h"
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif

namespace DD
{
  namespace Image
  {

    class ProjectionFunc;
    class Format;
#if kDDImageVersionMajorNum <= 16
    // For backwards-compatibility:
    class Scene;
    class MatrixArray;
    class Scene;
    class VArray;
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif

    //----------------------------------------------------------------------


    /*! \class DD::Image::CameraOp
       Op that defines a 3D camera.
     */
    class DDImage_API CameraOp : public AxisOp,
                                 public ndk::MultiProjectionCamera
    {
    protected:
#if kDDImageVersionMajorNum <= 16
      mFnDeprecatedLegacy3DSystemInNuke14("focal_length_ has been deprecated, use _params.optics.focalLength instead.")
      double focal_length_;     //!< Deprecated, use _params.optics.focalLength instead

      mFnDeprecatedLegacy3DSystemInNuke14("haperture_ has been deprecated, use _params.sensor.apertureSize.x instead.")
      double haperture_;        //!< Deprecated, use _params.sensor.apertureSize.x instead

      mFnDeprecatedLegacy3DSystemInNuke14("vaperture_ has been deprecated, use _params.sensor.apertureSize.y instead.")
      double vaperture_;        //!< Deprecated, use _params.sensor.apertureSize.y instead

      mFnDeprecatedLegacy3DSystemInNuke14("fstop_ has been deprecated, use _params.lensAperture.fStop instead.")
      double fstop_;            //!< Deprecated, use _params.lensAperture.fStop instead

      mFnDeprecatedLegacy3DSystemInNuke14("near_ has been deprecated, use _params.lensAperture.clipNearDistance instead.")
      double near_;             //!< Deprecated, use _params.lensAperture.clipNearDistance instead

      mFnDeprecatedLegacy3DSystemInNuke14("far_ has been deprecated, use _params.lensAperture.clipFarDistance instead.")
      double far_;              //!< Deprecated, use _params.lensAperture.clipFarDistance instead

      mFnDeprecatedLegacy3DSystemInNuke14("projection_mode_ has been deprecated, use projectionMode() instead.")
      int projection_mode_;     //!< Deprecated, use MultiProjectionCamera::projectionMode() instead

      mFnDeprecatedLegacy3DSystemInNuke14("focal_point_ has been deprecated, use _params.optics.focusDistance instead.")
      double focal_point_;      //!< Deprecated, use _params.optics.focusDistance instead

      mFnDeprecatedLegacy3DSystemInNuke14("win_translate_ has been deprecated, use _params.sensor.apertureTranslate instead.")
      Vector2 win_translate_;   //!< Deprecated, use _params.sensor.apertureTranslate instead

      mFnDeprecatedLegacy3DSystemInNuke14("win_scale_ has been deprecated, use _params.sensor.apertureScale instead.")
      Vector2 win_scale_;       //!< Deprecated, use _params.sensor.apertureScale instead

      mFnDeprecatedLegacy3DSystemInNuke14("win_roll_ has been deprecated, use _params.sensor.apertureRotate instead.")
      double win_roll_;         //!< Deprecated, use _params.sensor.apertureRotate instead

      //----------------------------------------------------------------------------

      mFnDeprecatedLegacy3DSystemInNuke14("lens_distort_a_, use _params.optics.opticalDistortionA instead.")
      double  lens_distort_a_;  //!< Deprecated, use, use _params.optics.opticalDistortionA instead

      mFnDeprecatedLegacy3DSystemInNuke14("lens_distort_b_, use _params.optics.opticalDistortionB instead.")
      double  lens_distort_b_;  //!< Deprecated, use, use _params.optics.opticalDistortionB instead

      mFnDeprecatedLegacy3DSystemInNuke14("lens_distort_c_, use _params.optics.opticalDistortionC instead.")
      double  lens_distort_c_;  //!< Deprecated, use, use _params.optics.opticalDistortionC instead

      mFnDeprecatedLegacy3DSystemInNuke14("lens_distort_D_ has been deprecated")
      double  lens_distort_D_;  //!< Deprecated

      mFnDeprecatedLegacy3DSystemInNuke14("lens_distortion_ has been deprecated")
      double  lens_distortion_; //!< Deprecated

      mFnDeprecatedLegacy3DSystemInNuke14("lens_power_ has been deprecated")
      double  lens_power_;      //!< Deprecated

      mFnDeprecatedLegacy3DSystemInNuke14("lens_distort_enabled_ has been deprecated")
      bool    lens_distort_enabled_;

      mFnDeprecatedLegacy3DSystemInNuke14("use_vertical_lock_ has been deprecated")
      bool    use_vertical_lock_; //!< Deprecated
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif
      int32_t         _projectionSelection; //!< Projection menu knob index
      //
      ShutterControls _shutter;             //!< Shutter controls which may be referenced by a renderer
      double          _shutterBias;         //!< Weights the shutter samples towards shutter close with a power function
      //
      bool            _glWorldSizeIcon;     //!< Always draw the camera icon at the 'real' world size (defined by worldToLens factor)
      bool            _glSolidFrustum;      //!< Draw the camera frustum as solid
      bool            _glInsideFrustum;     //!< Indicated whether to draw the frustum as an inside or outside box


    public:
      CameraOp(Node* node);
      ~CameraOp();

      //! Cast to a CameraOp. This is much cheaper and safer than using dynamic_cast.
      const CameraOp* cameraOp() const override { return this; }
            CameraOp* cameraOp()       override { return this; }

      /*! Return a default camera that can be used when there is no camera attached
          to your renderer. This camera is at the origin, looking along -Z, with a
          perspective projection set to a lens ratio (focal-length / film-width) of 1.
      */
      static CameraOp*  default_camera();

      //! Return hints to control when and how this op will be evaluated by the top-down system.
      OpHints opHints() const override;

      //!
      unsigned node_color() const override { return 0x71C67100; }

      //!
      void _validate(bool for_real) override;

      //-----------------------------------------------------------------

      /*! Default knobs puts the transform controls on the front tab, and
          the projection, shutter, and lens controls on tab 'Projection'.
      */
      void knobs(Knob_Callback) override;

      /*! AxisOp::knobs() calls this after addTransformKnobs(), inserting the
          'Projection' tab ahead of the 'Display' tab.
          The 'Projection' tab contains the projection, shutter, and lens controls.
      */
      void  addExtraFrontPanelKnobs(Knob_Callback f) override;

      //! Adds the frustum visualization controls.
      void  addDisplayOptionsKnobs(Knob_Callback f) override;

      //! Adds the default shutter controls.
      virtual void  addShutterKnobs(Knob_Callback);

      //! Adds the default projection controls.
      //! If \a enableDefaultKnobs is false then the standard projection-related
      //! knobs that are not needed by the current projection are disabled.
      virtual void  addProjectionKnobs(Knob_Callback f,
                                       bool          enableDefaultKnobs = true);

      //! Adds the default aperture-window controls.
      //! If \a enableDefaultKnobs is false then the standard projection-related
      //! knobs that are not needed by the current projection are disabled.
      virtual void  addWindowKnobs(Knob_Callback f,
                                   bool          enableDefaultKnobs = true);

      //! Adds the default lens controls.
      //! If \a enableDefaultKnobs is false then the standard projection-related
      //! knobs that are not needed by the current projection are disabled.
      virtual void  addLensKnobs(Knob_Callback f,
                                 bool          enableDefaultKnobs = true);

      //!
      int knob_changed(Knob*) override;

      //-----------------------------------------------------------------

      //! The currently-assigned projection plugin.
      virtual ProjectionFunc*  projectionPlugin() const { return _projectionPlugin; }

      //! Currently-assigned projection mode enum value. May be one of the pre-defined modes
      //! or a custom enum. Note - do not assume this value is within the hardcoded
      //! LensProjection set as a subclass may extend or completely replace the enum set!
      ndk::LensProjection  projectionMode() const override;

      //! Set the projection mode via a ProjectionFunc class name. If a matching
      //! ProjectionFunc plugin exists it will be loaded and an instance created for
      //! this camera (retrievable via projectionFunc() after _validate() occurs) and
      //! true is returned.
      //!
      //! Important note - setting the projection mode via this method does not update
      //! _projectionFunc immediately, it is updated during the next _validate() cycle
      //! called on this CameraOp. This is to avoid lens shaders from disappearing from
      //! renderer visibility until the next Op update cycle.
      //!
      //! If any error occurs the default 'Perspective' ProjectionFunc will be used as
      //! a fallback and false returned.
      bool  setProjection(const char* projectionClassName);

      //! Same but takes LensProjection enum which is converted to the matching
      //! ProjectionFunc class name.
      void  setProjectionMode(ndk::LensProjection mode);

      //! Return the class name of the selected lens projection which is used to find
      //! the matching ProjectionFunc plugin. Note that this class name is *NOT*
      //! the plugin name and will have a prefix and suffix added. See \a ProjectionFunc
      //! for more info.
      const char*  projectionFuncClassName() const;

      /*! Returns true if current camera projection can be described linearly with a
          4x4 matrix, based on the camera's current settings. This is usually only
          true for perspective and orthographic modes if no lens distortion is
          being done.

          A subclass should override this method if it's implementing custom
          projection calculations in projectionModeMatrix().

          Default implementation returns projectionFunc()->isLinear().
      */
      bool  isBaseProjectionLinear() const override;

      //-----------------------------------------------------------------

      static const char* projection_modes[];
      static const char* projection_modes_renderer[];

      /*! Return a linear camera projection matrix for the projection mode arg
          using the CameraOp's current lens values (ie its focal_length,
          horizontal_aperture, etc.)

          Only the predefined LensProjection enums are implemented in the base
          class implementation, but a subclass can override this method to
          implement custom projection calculations.

          This method should be used in conjunction with isBaseLinearProjection() to
          determine how to use the returned matrix, as not all projection modes
          can be supported by a linear matrix. For example uv and spherical, plus
          perspective/ortho projections become non linear if lens distortion is
          enabled.

          In non-linear cases the matrix returned is normally considered the 'base'
          projection for the camera based on its current lens settings.
          In other words although the projection mode may be 'perspective' and the
          matrix returned is a valid perspective projection, if lens distortion is
          enabled then the final projection is a combination of both the projection
          matrix and the distortion.
      */
      virtual fdk::Mat4d  projectionModeMatrix(ndk::LensProjection mode) const;

      //! Same as projectionMatrix() except it evaluates the matrix at an arbitrary OutputContext.
      virtual fdk::Mat4d  projectionModeMatrixAt(ndk::LensProjection  mode,
                                                 const OutputContext& context) const;

      //-----------------------------------------------------------------

      /*! Multiply the passed matrix by the necessary transformation to put
          x=-1 at the left edge of the \a format, +1 at the right, and to
          center it vertically, and compensate for any pixel aspect. The
          output z and w are left unchanged.

          Most Iops will need to do this using their output format to get the
          actual transformation from input space to output space. You would
          do this first, then multiply by the projection*local*parent.
       */
      static void  toFormat(fdk::Mat4f&, const Format*);

      /*! Opposite transformation of to_format(). Multiplies the passed
          matrix by a transformation that scales the input format to go from
          x = -1 to x = 1 and to center it vertically around y = 0, and to
          remove any pixel aspect ratio. You multiply by this matrix last.

          The 3DCard iop scales further so the image covers a range so that
          if the camera lens and "taking lens" are equal the image fills the
          field of view.
       */
      static void  fromFormat(fdk::Mat4f&, const Format*);

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

      //-----------------------------------------------------------------

      //! SceneLoaderOpI: If loader is attached to an AxisOp subclass return 'this'.
      CameraOp* asCameraLoaderOp() override { return this; }

      /*! SceneLoaderOpI:
          Try to import camera prim data from the provided stage. Will return 1(error) if
          stage is invalid. If \a prim_path is empty then -1 will be returned indicating
          no change has occured, otherwise if \a prim_path is invalid or not present in
          stage then 1(error) will be returned.

          This will call AxisOp::importScenePrim() to import the XformablePrim data
          then import the CameraPrim attributes.

          Return codes:
          -1 - no prim data was loaded and no error, usually due to empty prim path
           0 - prim data was loaded with no error
           1 - there was an error and no prim data was loaded
      */
      int32_t  importScenePrim(const usg::StageRef& stage,
                               const std::string&   prim_path) override;

    protected:
      //! Builds a path from AxisOp::exportBasePrimPath(), appending the Node displayName()
      //! to use as the camera schema's prim name. The camera schema's type name comes from
      //! \a exportCameraSchemaTypeName().
      //!
      //! For example if the Node class name is 'MyFisheyeCamera_v4' we want it to show up in the
      //! scenegraph as 'MyFisheyeCamera', while the camera schema name may be 'FisheyeCamera'.
      virtual usg::Path  exportCameraSchemaPrimPath() const;

      //! Return the schema type name to use for the prim defined by
      //! \a exportCameraSchemaPrimPath().
      //! Base class returns 'Camera'.
      virtual const char*  exportCameraSchemaTypeName() const;

      //! ScenePrimExporterOpI: Subclass portion of exportScenePrims().
      void  _exportScenePrims(usg::LayerRef&            exportLayer,
                              int32_t                   exportMode,
                              const fdk::FrameValueSet& sampleFrames = fdk::FrameValueSet()) const override;


#if kDDImageVersionMajorNum <= 16
    protected:
      //----------------------------------------------------------------------------
      // Deprecated single-precision matrices - these are derived from the
      // new double-precision matrices. Will disappear in a Nuke major rev after 16
      //----------------------------------------------------------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("projection_ has been deprecated, use Mat4d _projectionMatrix instead.")
        Matrix4 projection_;     //!< Camera's projection matrix - deprecated

    public:
      //----------------------------------------------------------------------------
      // Deprecated functions - will disappear in a Nuke major rev after 16
      //----------------------------------------------------------------------------
      enum
      mFnDeprecatedLegacy3DSystemInNuke14("Enumeration deprecated, use enum class ndk::LensProjection instead.")
      {
        LENS_PERSPECTIVE = 0, //!< Standard perspective projection
        LENS_ORTHOGRAPHIC, //!< Standard orthographic projection
        LENS_UV,          //!< Replace screen XY with UV coordinate
        LENS_SPHERICAL,   //!< 360x180 world spherically projected around camera
        //
        LENS_RENDER_CAMERA, //!< Renderer uses this option
        LENS_USER_CAMERA  = LENS_RENDER_CAMERA + 1//!< Camera user projection
      };
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("Class() has been deprecated, implement on subclass plugin instead.")
        const char* Class() const override { return "Camera"; }
      mFnDeprecatedLegacy3DSystemInNuke14("node_help() has been deprecated, implement on subclass plugin instead.")
        const char* node_help() const override { return ""; }
      mFnDeprecatedLegacy3DSystemInNuke14("projection_knobs() has been deprecated, use addProjectionKnobs() instead.")
        virtual void projection_knobs(Knob_Callback f) { addProjectionKnobs(f); }
      mFnDeprecatedLegacy3DSystemInNuke14("lens_knobs() has been deprecated, use addDistortionKnobs() instead.")
        virtual void lens_knobs(Knob_Callback f) { addLensKnobs(f); }
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("projection() has been deprecated, use projectionMatrix() instead")
        const Matrix4& projection() const;
      mFnDeprecatedLegacy3DSystemInNuke14("hfov(double) has been deprecated, use setHorizontalFov() instead")
        void hfov(double v);
      mFnDeprecatedLegacy3DSystemInNuke14("film_width() has been deprecated, use horizontalAperture() instead")
        double  film_width() const { return haperture_; }
      mFnDeprecatedLegacy3DSystemInNuke14("film_height() has been deprecated, use verticalAperture() instead")
        double  film_height() const { return vaperture_; }
      mFnDeprecatedLegacy3DSystemInNuke14("focal_length() has been deprecated, use focalLength() instead")
        double  focal_length() const { return focal_length_; }
      mFnDeprecatedLegacy3DSystemInNuke14("Near() has been deprecated, use nearPlaneDistance() instead")
        double  Near() const { return near_; }
      mFnDeprecatedLegacy3DSystemInNuke14("Far() has been deprecated, use farPlaneDistance() instead")
        double  Far() const { return far_; }
      mFnDeprecatedLegacy3DSystemInNuke14("focal_point() has been deprecated, use focusDistance() instead")
        double  focal_point() const { return focal_point_; }
      mFnDeprecatedLegacy3DSystemInNuke14("fstop() has been deprecated, use fStop() instead")
        double  fstop() const { return fstop_; }
      mFnDeprecatedLegacy3DSystemInNuke14("film_width(double) has been deprecated, use setHorizontalAperture() instead")
        void film_width(double v);
      mFnDeprecatedLegacy3DSystemInNuke14("film_height(double) has been deprecated, use setVerticalAperture() instead")
        void film_height(double v);
      mFnDeprecatedLegacy3DSystemInNuke14("focal_length(double) has been deprecated, use setFocalLength() instead")
        void focal_length(double v);
      mFnDeprecatedLegacy3DSystemInNuke14("Near(double) has been deprecated, use setNearPlaneDistance() instead")
        void Near(double v);
      mFnDeprecatedLegacy3DSystemInNuke14("Far(double) has been deprecated, use setFarPlaneDistance() instead")
        void Far(double v);
      mFnDeprecatedLegacy3DSystemInNuke14("focal_point(double) has been deprecated, use setFocusDistance() instead")
        void focal_point(double v);
      mFnDeprecatedLegacy3DSystemInNuke14("fstop(double) has been deprecated, use setfStop() instead")
        void fstop(double v);
      mFnDeprecatedLegacy3DSystemInNuke14("win_translate() has been deprecated, use windowTranslate() instead")
        const Vector2& win_translate() const { return win_translate_; }
      mFnDeprecatedLegacy3DSystemInNuke14("win_scale() has been deprecated, use windowScale() instead")
        const Vector2& win_scale() const { return win_scale_; }
      mFnDeprecatedLegacy3DSystemInNuke14("win_roll() has been deprecated, use windowRotateZ() instead")
        double win_roll() const { return win_roll_; }
      mFnDeprecatedLegacy3DSystemInNuke14("projection_mode() has been deprecated, use projectionMode() instead")
        int projection_mode() const { return projection_mode_; }
      mFnDeprecatedLegacy3DSystemInNuke14("projection_mode(int) has been deprecated, use setProjectionMode() instead")
        void projection_mode(int i);
      mFnDeprecatedLegacy3DSystemInNuke14("projection_is_linear(int) has been deprecated, use isProjectionModeLinear(ndk::LensProjection) instead")
        virtual bool projection_is_linear(int mode) const;
      mFnDeprecatedLegacy3DSystemInNuke14("projection(int) has been deprecated, use projectionModeMatrix(ndk::LensProjection) instead")
        virtual Matrix4 projection(int mode) const;
      mFnDeprecatedLegacy3DSystemInNuke14("projectionAt(OutputContext) has been deprecated, use projectionModeMatrixAt(ndk::LensProjection, OutputContext) instead")
        virtual Matrix4 projectionAt(const OutputContext&);
      //-----------------------------
      mFnDeprecatedLegacy3DSystemInNuke14("lens_distort_a() has been deprecated, use distortA() instead")
        double lens_distort_a() const { return lens_distort_a_; }
      mFnDeprecatedLegacy3DSystemInNuke14("lens_distort_b() has been deprecated, use distortB() instead")
        double lens_distort_b() const { return lens_distort_b_; }
      mFnDeprecatedLegacy3DSystemInNuke14("lens_distort_c() has been deprecated, use distortC() instead")
        double lens_distort_c() const { return lens_distort_c_; }
      mFnDeprecatedLegacy3DSystemInNuke14("lens_distort_D() has been deprecated")
        double lens_distort_D() const { return lens_distort_D_; }
      mFnDeprecatedLegacy3DSystemInNuke14("lens_distortion() has been deprecated")
        double lens_distortion() const { return lens_distortion_; }
      mFnDeprecatedLegacy3DSystemInNuke14("lens_power() has been deprecated")
        double lens_power() const { return lens_power_; }
      mFnDeprecatedLegacy3DSystemInNuke14("lens_distort_enabled() has been deprecated")
        bool lens_distort_enabled() const { return lens_distort_enabled_; }
      //-----------------------------
      typedef void (LensFunc)(Scene*, CameraOp*, MatrixArray* transforms,
                              const VArray& v, Vector4& out, void* data);
      mFnDeprecatedLegacy3DSystemInNuke14("lens_function(int mode) has been deprecated.")
        virtual LensFunc* lens_function(int mode) const;

      typedef void (LensNFunc)(Scene*, CameraOp*, MatrixArray* transforms,
                               VArray* vert_array, int n, void* data);
      mFnDeprecatedLegacy3DSystemInNuke14("lensNfunction(int mode) has been deprecated.")
        virtual LensNFunc* lensNfunction(int mode) const;

      mFnDeprecatedLegacy3DSystemInNuke14("to_format has been deprecated, use toFormat instead.")
      static void to_format(Matrix4&, const Format*);

      mFnDeprecatedLegacy3DSystemInNuke14("from_format has been deprecated, use fromFormat instead.")
      static void from_format(Matrix4&, const Format*);

      //----------------------------------------------------------------------------
      //----------------------------------------------------------------------------
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif

    public:
      //! For internal use only - called in addProjectionKnobs() to add ProjectionFunc dynamic knobs.
      void addProjectionFuncCustomKnobs(Knob_Callback);

    protected:
      //! Subclass implemention of \a validateCamera() to perform any custom configuration
      //! of the camera, called whenever the function has been changed.
      void _validateCamera() override;

      //! Called from knob_changed when projection selection is changed via knob interaction.
      int _doProjectionFuncKnobChanged(Knob*, int callAgain);
      //! Sets _newProjectionPlugin. _projectionPlugin gets updated in _validate().
      bool _setProjectionFuncPlugin(const char* projectionClassName);

    private:
      ProjectionFunc* _newProjectionPlugin; //!< New projection plugin selected, non-null if changed
      ProjectionFunc* _projectionPlugin;    //!< Projection func plugin to use, if applicable

      Knob*   _projFuncKnob;                //!< Projection selector knob
      Knob*   _projFuncDynamicDividerKnob;  //!< Knob signpost where ProjectionFunc dynamic knobs start
      int32_t _nProjFuncCustomKnobs;        //!< Count of added ProjectionFunc dynamic knobs
    };

  }
}

#endif
