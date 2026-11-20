// Copyright (c) 2024 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_PROJECTIONFUNC_H
#define DDIMAGE_PROJECTIONFUNC_H

#include "DDImage/PluginObject.h"
#include "DDImage/Knobs.h"

#include "ndk/geo/camera/LensProjection.h" // for plugin building convenience

namespace DD
{
  namespace Image
  {

    class CameraOp;

    /*! \class DD::Image::ProjectionFunc
        Abstract interface class for building LensProjectionFunction plugins.
        Typically interface is attached to a LensProjectionFunction subclass as a secondary
        inheritance.
        The subclass must implement \a lensProjectionFunction() to provide access to the
        projection object.
    */
    class DDImage_API ProjectionFunc : public DD::Image::PluginObject
    {
    public:
      static constexpr std::string_view pluginCategory = "LensProjections"; //!< Category can be used to organize plugins within a subdirectory.
      static constexpr std::string_view pluginPrefix   = "lens";            //!< The name to use as the plugin name prefix.
      static constexpr std::string_view pluginSuffix   = "Projection";      //!< The name to use as the plugin name suffix.

      //! Return the LensProjectionFunction object this interface is attached to. Should return 'this'.
      //! Must implement.
      virtual ndk::LensProjectionFunction*  lensProjectionFunction() = 0;

      //! Return the class identifier name which is usually Description::projectionClass() of the
      //! subclass's local Description. Note that this is the class identifier name -without- the
      //! prefix & suffix.
      //! Must implement.
      virtual const char*  projectionClass() const = 0;

      //! Find plugins matching the prefix and suffix of this class.
      //! See PluginObject::FindPlugins() for more info.
      static const fdk::StringList&  FindPlugins(bool findBuiltins,
                                                 bool findExternals,
                                                 bool forceRefresh = false);

      //-----------------------------------------------------------------

      //! CameraOp specific support - does this projection utilize the CameraOp's
      //! standard CameraParameters knobs or should they be disabled/hidden?
      //!
      //! This is used by CameraOp to enable or disable the knobs in its panel while
      //! keeping the knobs presence constant so that scripts will not break.
      //!
      //! Default implementation returns false.
      virtual bool  needsDefaultCameraOpKnobs() const { return false; }

      //! Return true if this ProjectionFunc wants to create any additional custom
      //! Knobs to control internal parameters.
      //!
      //! Nodes that support the option for dynamic knobs, like CameraOp, should
      //! first test this to decide whether to configure any custom Knob groups
      //! tabs, etc for the projection knobs to be placed in. The calling node
      //! can then organize the knobs by the sub categories provided via the
      //! various virtual knob methods - \a addCameraLensControlKnobs(), etc.
      //!
      //! Must implement.
      virtual bool  providesCustomKnobs() const = 0;

      //! Add any lens-specific controls like focal length, fstop, etc.
      virtual void  addLensControlKnobs(Knob_Callback f) {}

      //! Add any aperture-window specific controls like stereo offsets, roll, etc.
      virtual void  addApertureControlKnobs(Knob_Callback f) {}

      //! Perform any custom knob changed callback logic.
      //! \a callAgain is usually passed from the Op::knob_changed() method that owns
      //! this plugin, setting to 1 if the knob wants to be handled again.
      //! Default implementation does nothing and returns \a callAgain unchanged.
      virtual int knob_changed(DD::Image::Knob* knob, int callAgain = 0) { return callAgain; }


    public:
      //------------------------------------------------------------------------------------------
      /*! This structure creates a subclass instance of ProjectionFunc, possibly from loading
          a plugin file or a compiled-in Description.
      */
      class DDImage_API Description : public DD::Image::PluginObject::Description
      {
      public:
        //! Constructor method definitions used for 'build()' methods in plugins.
        typedef ProjectionFunc* (*ProjectionFuncBuilder)(void);
        ProjectionFuncBuilder builderFunc;  //!< Call this to construct a ProjectionFunc object.

      public:
        //! ProjectionFunc specific builder.
        Description(const char*           projectionClassName,
                    ProjectionFuncBuilder projectionBuilder);

        //! Prefix and suffix strings prepended and appended to pluginClass() string to
        //! construct plugin filenames.
        const std::string_view&  classPrefix() const override { return ProjectionFunc::pluginPrefix; }
        const std::string_view&  classSuffix() const override { return ProjectionFunc::pluginSuffix; }

        //! Name of the ProjectionFunc class without any leading prefix or or trailing suffix.
        const char*  projectionClass() const { return pluginClass().data(); }

        //! Full name of plugin with prefix and suffix.
        const std::string&  projectionPluginName() const { return _pluginName; }
      };
      //------------------------------------------------------------------------------------------

      //! Create a ProjectionFunc instance from a projection class name. Note that the
      //! class name is -without- the prefix & suffix used to build the ProjectionFunc
      //! plugin name. For example a projection class named 'MyFisheye' will be
      //! combined with \a pluginPrefix and \a pluginSuffix to find a plugin description
      //! with the name 'lensMyFisheyeProjection'.
      //!
      //! Calling code takes ownership of returned pointer.
      static ProjectionFunc*  create(const std::string_view& projectionClassName);

      //! Create a ProjectionFunc instance from a description.
      //! Calling code takes ownership of returned pointer.
      static ProjectionFunc*  create(const ProjectionFunc::Description& projectionDescription);

    };


    //! Return a plugin description for one of the built in camera projection functions.
    //! See ndk::LensProjection for the list.
    //! If the returned description is valid create an instance of the ProjectionFunc by
    //! calling ProjectionFunc::create(*description).
    DDImage_API
    const ProjectionFunc::Description*  CameraProjectionFuncDescription(ndk::LensProjection mode);

    //! Find the list of available projection plugin names in the -current- plugin directories.
    //! The built in projection plugins will always be first in the list so it should never
    //! be empty. This is the list of strings used to populate a projection selector menu.
    //!
    //! Note that repeat calls to this will not rescan the directories by default so use \a forceRescan
    //! if this is required.
    DDImage_API
    std::vector<const char*>& ProjectionPluginsList(bool forceRescan = false);

    //! Projection function to use by default, which always exists, which is the standard
    //! Perspective function. 
    DDImage_API
    ProjectionFunc* DefaultProjectionFunc();

  } // namespace Image
} // namespace DD

#endif
