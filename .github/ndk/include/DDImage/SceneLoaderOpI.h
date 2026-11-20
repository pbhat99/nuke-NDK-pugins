// Copyright (c) 2024 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_SCENELOADEROPI_H
#define DDIMAGE_SCENELOADEROPI_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"

#include "usg/base/Value.h" // for Value::Type
#include "usg/geom/Path.h"
#include "usg/geom/Stage.h"

namespace DD
{
  namespace Image
  {
    class Op;
    class AxisOp;
    class CameraOp;
    class LightOp;
    class GeomOp;
    class Iop;


    /*! \class DD::Image::SceneLoaderOpI

        Interface class for abstracting Ops which can import 3D scene prim data.
    */
    class DDImage_API SceneLoaderOpI
    {
    public:
      SceneLoaderOpI(bool importEnabled = false);
      virtual ~SceneLoaderOpI() {}

      //--------------------------------------------------------------------
      // Must implement these pure virtuals on the owner Op:
      //--------------------------------------------------------------------

      //! Return the Op the interface is attached to. Should return 'this'. Must implement.
      virtual Op*  getSceneLoaderOp() = 0;

      //! Allow subclasses to gain access to sibling functions.
      virtual SceneLoaderOpI*  asSceneLoader() { return nullptr; }

      //! Is loader enabled? Base class returns the state of the 'import_enabled' knob.
      virtual bool  isSceneLoaderEnabled();

      //! Return the node input to use for the scene importer source which is usually
      //! a GeomOp.
      //! -1 indicates an internal importer implementation with no input connection.
      //! Default implementation returns -1.
      virtual int32_t  sceneLoaderSourceInput() const { return -1; }

      //! Return the scene prim type to use when searching for a default to load - ie 'Camera', 'Xform', etc.
      virtual const char*   defaultSceneLoaderPrimType() { return "Xform"; }

      //---------------------------------------------------------------------------------
      // These methods should be implemented on a subclass depending on the Op's class:
      //---------------------------------------------------------------------------------

      //! If loader is attached to an AxisOp subclass return 'this'. Default returns nullptr.
      virtual AxisOp*   asAxisLoaderOp() { return nullptr; }

      //! If loader is attached to a CameraOp subclass return 'this'. Default returns nullptr.
      virtual CameraOp* asCameraLoaderOp() { return nullptr; }

      //! If loader is attached to a LightOp subclass return 'this'. Default returns nullptr.
      virtual LightOp*  asLightLoaderOp() { return nullptr; }

      //! If loader is attached to an Iop subclass return 'this'. Default returns nullptr.
      virtual Iop*      asGeomLoaderIop() { return nullptr; }


    protected:
      //--------------------------------------------------------------------
      // IMPORTANT: Call these methods from the owner Op otherwise importer
      //            will not update properly:
      //--------------------------------------------------------------------

      //! Call this from owner Op::_knobStorePost() which is called after Op::knobs() but
      //! before Op::_validate(), and offers the loader a chance to confgure its input
      //! source(s) early (ie EARLY_STORE) and load the scene data.
      //!
      //! If we end up loading a scene prim successfully there's a bunch of knobs
      //! who's contents will likely get changed, affecting the Op's local variables.
      //! But, since knobs() has already been called and the previously knobs stored
      //! at the prior values these vars are now out of date.
      //! This is a common Nuke issue when Knobs are changed outside the expected pattern,
      //! and is often experienced as an off-by-one delay in updating values when the 
      //! Viewer changes frame, since changing the frame will force the knobs to store
      //! with their new contents, causing an update 'snap'.
      //!
      //! The solution to this is to force the knobs to store after a known change
      //! has been applied, via Op::setKnobsToContext(force=true). That method will
      //! cause knobs() to be called again storing the new values, but that means
      //! Op::_knobStorePost() will also get called again, so we need to avoid a loop.
      //!
      //! \a _importerInvalid is used to note whether we're in that second knobs() update
      //! cycle and avoid re-calling the loader again.
      virtual void     _loadSceneData(const OutputContext& storeContext,
                                      Hash&                hash);

      //! Call this from owner Op::knob_changed().
      //! Updates loader gui and does node data reloads.
      virtual int32_t  _sceneLoaderKnobChanged(Knob*   k,
                                               int32_t callAgain = 0);

      //! Call this from owner Op::_validate().
      //! Sets an error on owner Op if there is a loader error.
      virtual void     _validateSceneLoader(bool for_real);


    public:
      struct ImportSourceOptions
      {
        int32_t      sourceMode;      //!< File path(0), scene node input(1), internal source(2)
        const char*  filePath;        //!< External path to scene file to use, if applicable
        int32_t      nodeInputIndex;  //!< Node input to get stage from, if applicable. Defaults to -1
      };

      struct ImportTimeOptions
      {
        bool    lockReadFrame;    //!< Whether to use the manually-set frame number
        double  readFrame;        //!< Manually-set frame number to read
        double  framesPerSecond;  //!< Output frames per second sampling rate
        double  frameOffset;      //!< Offset the incoming frame range (after any frame rate change)
        double  frameOrigin;      //!< Origin of the incoming frame range
      };

      struct ImportXformOptions
      {
        fdk::XformOrder  decomposeXformOrder;   //!< Preferred decompose xform order
        fdk::RotateOrder decomposeRotOrder;     //!< Preferred decompose rotation order
        bool             translationsEnable;    //!< Enable loading of translations
        bool             rotationsEnable;       //!< Enable loading of rotations
        bool             scalesEnable;          //!< Enable loading of scale
        bool             eulerFilterEnable;     //!< Enable euler filter on rotations
      };

      struct ImporterControls
      {
        std::string         primPath;       //!< Path + name for the prim to import
        ImportSourceOptions sourceOptions;  //!< Scene source options
        //
        ImportTimeOptions   timeOptions;    //!< Time handling options - if applicable
        bool                lockReadView;   //!< Whether to use the manually-set view
        int32_t             readView;       //!< Manually set the view to read
        //
        ImportXformOptions  xformOptions;   //!< Transform handling option - if applicable
        //
        int32_t             version;        //!< Incremented when reload button is pushed
        fdk::Hash           previousHash;   //!< Does the scene prim need importing?
        bool                error;          //!< Error was thrown attempting to import a scene prim
        std::string         errorMsg;       //!< Error msg saved
        //
        usg::Path           resolvedPrimPath;     //!< Fully expanded path, includes any parent connections
        fdk::Hash           resolvedPrimPathHash; //!< Does prim name and path need to be rebuilt?
      };


    protected:
      ImporterControls _importer;         //!< Scene prim importer controls & state
      bool             _importerEnabled;  //!< If true importer will extract prim attributes from scene source
      bool             _importerInvalid;  //!< If true any knobs set by the importer need to be re-stored


    public:
      //! Adds the scene import controls.
      //! \a filters sets a custom filter to use on the scene prim popup.
      //! If \a addXformOptions is true the knobs to control local parent extraction
      //! matrix decomposition options, etc are added.
      //! If \a addTimeOption is true the knobs to control frame offset and fps during
      //! import are added.
      //! Call this from owner Op::knobs().
      virtual void  addSceneLoaderKnobs(Knob_Callback      f,
                                        const std::string& filters,
                                        bool               addTimeOption,
                                        bool               addXformOptions = false);

      //! Add time control knobs.
      virtual void  addSceneLoaderTimeOptionKnobs(Knob_Callback f);

      //! Add xform decomposition control knobs.
      virtual void  addSceneLoaderXformOptionKnobs(Knob_Callback f);

      //! Enable/disable knobs filled in by the scene importer.
      virtual void  enableSceneLoaderKnobs(bool importEnabled);
      virtual void  enableSceneLoaderExtraKnobs(bool importEnabled);

      //! Enable/disable knobs filled in by the xform decomposer.
      virtual void  enableXformExtractKnobs(bool xformExtractEnabled);


      //! Check if the imported prim needs to be loaded or reloaded. If the prim needs
      //! reloading then importScenePrim() is called with the correct stage and prim path.
      //!
      //! If \a force is true then the prim will always be reloaded.
      //!
      //! Return codes:
      //! -1 - no prim data was loaded and no error (i.e. no change)
      //!  0 - prim data was loaded with no error
      //!  1 - there was an error and no prim data was loaded
      //!
      //! If error was returned (1) check the error state and message in ScenePrimImport.
      int32_t  updateScenePrim(bool force = false);


      //! Try to import prim data from the provided stage. Will return 1(error) if
      //! stage is invalid. If \a prim_path is empty then -1 will be returned indicating
      //! no change has occured, otherwise if \a prim_path is invalid or not present in
      //! stage then 1(error) will be returned.
      //!
      //! Return codes:
      //! -1 - no prim data was loaded and no error, usually due to empty prim path
      //!  0 - prim data was loaded with no error
      //!  1 - there was an error and no prim data was loaded
      virtual int32_t  importScenePrim(const usg::StageRef& stage,
                                       const std::string&   primPath) = 0;

      //! Retrieve the attribute values at times possibly modified by the importer
      //! time options. If there are no time samples in the attribute then only a
      //! single default-time value is returned, unaffected by the time options.
      //! If the output time of the samples have been affected the \a times array
      //! reflects the *output* time and not the time the attribute was sampled at.
      //!
      //! If the importer is set to frame lock then only a single value is set,
      //! sampled at the lock time, regardless of the animation status of the
      //! attribute.
      //!
      //! This method will always set at least one time and value if the attrib
      //! is valid.
      //!
      //! Returns the number of value/times set, or 0 if there was a problem.
      template <typename T>
      size_t  getAttribValuesTimeWarped(const usg::Attribute& attr,
                                        double                attr_fps,
                                        fdk::TimeValueList&   times,
                                        std::vector<T>&       values) const;

      //! Retrieve the attribute values at times possibly modified by the importer time
      //! options and save them into the target Knob.
      //! If \a force_keys is true then the Knob will have a keyframe set even if the
      //! sample attribute only has a default value (i.e. no time sample.)
      //! This likely only works for ArrayKnobs since they support animation.
      //! See Knob::set() for explanation of \a channel_start_offset and \a view options.
      //!
      //! Returns true on success.
      template <typename T>
      bool    copyAttribToKnobTimeWarped(const usg::Attribute& attr,
                                         double                attr_fps,
                                         Knob*                 knob,
                                         bool                  force_keys,
                                         int32_t               channel_start_offset,
                                         int32_t               view) const;

      //! Generate output time values from an input time list using the current importer
      //! settings to possibly time warp / offset them.
      //! If no time change needed the returned times will be the input times.
      //! \a input_fps is only needed if this is changing the frame rate of the input
      //! samples. If left at the 0.0 default then no time warping is applied.
      fdk::TimeValueList  getOutputSampleTimes(const fdk::TimeValueList& input_times,
                                               double                    input_fps = 0.0) const;


#if 0
      //! Enable the loader error state and fill in the error msg string.
      void setLoadError(const char* msg, ...);
      //! Disable the error state and clear the error message.
      void clearLoadError();
      //! Pop up a warning dialog the user must acknowledge.
      void showUserWarning(const char* msg, ...);

      //---------------------------------------------------------------------

      //! Returns the file path to the scene file, or an empty string if 'import_enabled' is off.
      std::string  getSceneFilePath();


      //! Returns the scene path for the selected node, or an empty string if 'import_enabled' is off.
      std::string  getSceneNodePath();

      //---------------------------------------------------------------------

      //! Get a list of node descriptions from the scene file.
      bool  getNodeDescriptions(const char*              file,
                                ndk::NodeDescriptionMap& nodeDescriptions,
                                bool                     debug = false);

      //!
      std::string  findDefaultNode(const char* file,
                                   bool        debug = false);
#endif


    protected:
      //! Subclass implementation of updateScenePrim().
      //! If subclass has local values that should contribute to the import hash state
      //! which indicates if the scene needs reloading, it can append them to \a importHash.
      //! Default implementation does nothing.
      virtual void  _updateScenePrim(fdk::Hash& importHash) {}
    };



    template <typename T>
    inline size_t
    SceneLoaderOpI::getAttribValuesTimeWarped(const usg::Attribute& attr,
                                              double                attr_fps,
                                              fdk::TimeValueList&   times,
                                              std::vector<T>&       values) const
    {
      times.clear();
      values.clear();
      if (!attr.isValid()) {
        return 0;
      }
      if (_importer.timeOptions.lockReadFrame) {
        times.push_back(_importer.timeOptions.readFrame);
        values.resize(1);
        attr.getValue(values[0], _importer.timeOptions.readFrame);
        return 1;
      }
      const size_t n = attr.getTimeSamples(times);
      if (n == 0) {
        // No time samples, get default value:
        fdk::setNotAnimated(times); // sets 1 default time entry
        values.resize(1);
        attr.getValue(values[0]);
        return 1;
      }
      values.resize(n);
      for (size_t i=0; i < n; ++i) {
        const double sample_time = times[i];
        attr.getValue(values[i], sample_time); // sample at original time
        // Possibly time-warp output time:
        const double input_time  = (sample_time - _importer.timeOptions.frameOrigin) / _importer.timeOptions.framesPerSecond;
        const double output_time = (input_time * attr_fps) + _importer.timeOptions.frameOrigin + _importer.timeOptions.frameOffset;
        times[i] = output_time;
      }
      return n;
    }


    template <typename T>
    inline bool
    SceneLoaderOpI::copyAttribToKnobTimeWarped(const usg::Attribute& attr,
                                               double                attr_fps,
                                               Knob*                 knob,
                                               bool                  force_keys,
                                               int32_t               channel_start_offset,
                                               int32_t               view) const
    {
      if (knob) {
        fdk::TimeValueList times;
        std::vector<T> values;
        if (getAttribValuesTimeWarped(attr, attr_fps, times, values) > 0) {
          if (force_keys && times.size() == 1 && fdk::isDefaultTimeValue(times[0])) {
            times[0] = _importer.timeOptions.frameOrigin; // force knob to create a keyframe
          }
          return knob->set(true/*clear_keys*/, values, times, channel_start_offset, view); // copy values into Knob
        }
      }
      return false;
    }

  } // namespace Image
} // namespace DD

#endif
