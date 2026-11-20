// Copyright (c) 2024 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_ABSTRACTPLUGIN_H
#define DDIMAGE_ABSTRACTPLUGIN_H

#include "DDImage/Description.h"
#include "fdk/base/StringTypes.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::PluginObject

        Abstract base class for object classes that load via a plugin. This can be used
        for non-Op functions like shaders, readers, writers, etc that add functionality
        but are not intended to be visible in the Nuke nodegraph like Ops are.

        A common example of this are the reader/writer plugins that the Read and Write
        Ops load to handle file types. They expose Knob functionality on the Read and
        Write UI panels but the plugin objects are invisible to the nodegraph and local
        to the Ops, which manage their allocation.
    */
    class DDImage_API PluginObject
    {
    public:
      //! Find all plugins matching the <prefix>Foo<suffix> naming style. This searches
      //! the current plugin paths and the list of built in (compiled in) plugins.
      //! The result of the search is cached so repeated calls with the same prefix and
      //! suffix is fast.
      //! Note - the names in the returned list are the pluginClass() strings, not the
      //! full name, since just the class name is typically wanted in menus.
      static const fdk::StringList& FindPlugins(const char* pluginPrefix,
                                                const char* pluginSuffix,
                                                bool        findBuiltins,
                                                bool        findExternals,
                                                bool        forceRefresh = false);


    public:
      //---------------------------------------------------------------------
      //! This structure creates a subclass of a general function plugin,
      //! possibly from loading a plugin file or a compiled-in Description.
      //---------------------------------------------------------------------
      class DDImage_API Description : public DD::Image::Description
      {
      protected:
        std::string_view _pluginClass;  //!< Name of plugin class from the ctor - *NOT* the plugin .so filename!
        std::string      _pluginName;   //!< Full name of plugin with prefix and suffix: '<prefix><_pluginClass><suffix>'

        //! Method type defined in Description.h: (*f)(Description*), called when the plugin .so
        //! is first loaded. This adds the plugin class to the map of loaded ones so that we don't
        //! need to search for or load the file again.
        static void pluginBuilderCallback(DD::Image::Description* desc);

      public:
        //! Constructors set name and label to same value.
        Description(const char*             className,
                    const std::string_view& classPrefix,
                    const std::string_view& classSuffix);

        //! Name of the plugin class without any leading prefix or suffix.
        const std::string_view& pluginClass() const { return _pluginClass; }

        //! Full name of plugin with prefix and suffix.
        const std::string& pluginName() const { return _pluginName; }

        //! Prefix and suffix strings prepended and appended to pluginClass() string to
        //! construct plugin filenames.
        //! Default implementation returns an empty string.
        virtual const std::string_view& classPrefix() const;
        virtual const std::string_view& classSuffix() const;

        //! Concatenates the prefix, class name and suffix.
        //! This name is used to find the plugin object to load.
        static std::string BuildPluginName(const std::string_view& className,
                                           const std::string_view& pluginPrefix,
                                           const std::string_view& pluginSuffix);

        //! Find a dso description by name. If it has been previously loaded this method quickly
        //! returns an existing cached PluginObject::Description.
        //!
        //! It uses BuildPluginName() to construct the full name of the Description. For example
        //! if className is 'Spherical', prefix is 'lens', and suffix is 'Abstract' then the full
        //! plugin name searched for is 'lensSphericalAbstract'.
        //!
        //! Returns nullptr if not found.
        static const Description* find(const std::string_view& className,
                                       const std::string_view& pluginPrefix = std::string_view(""),
                                       const std::string_view& pluginSuffix = std::string_view(""));
      };


    protected:
      //! Subclasses call this with their custom prefix & suffix.
      static const PluginObject::Description* _create(const std::string_view& className,
                                                      const std::string_view& pluginPrefix,
                                                      const std::string_view& pluginSuffix);

    public:
      virtual ~PluginObject() {}

    };

  } // namespace Image
} // namespace DD

#endif
