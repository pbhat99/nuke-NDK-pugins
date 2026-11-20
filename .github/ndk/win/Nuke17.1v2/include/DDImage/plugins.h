// plugins.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/** \file plugins.h
    Code to load and manage plugins.
 */

#ifndef plugins_h
#define plugins_h

#include "DDImage_API.h"

#include <vector>
#include <string>

#ifdef FN_OS_LINUX
#include <string>
#endif

namespace DD
{

  namespace Image
  {

    /*! If this is set to true then it prints messages to stdout when it
       loads plugins. */
    extern DDImage_API bool plugin_verbose;

    /*! If this is set to true then it skips loading plugins (and menus)
       from ~/.nuke and NUKE_PATH. It also skips loading any Ofx plugins */
    extern DDImage_API bool plugin_safe_mode;

    /*!
       Return how many plugins have been loaded. This is incremented by
       every successful plugin_find().
     */
    DDImage_API int         plugin_count();

    /*!
       Return the name of the plugin being currently loaded. This is set
       when reading binary or tcl plugins. A blank string is returned if no
       such plugin is currently being loaded. The directory of this name
       will affect the locations searched by plugin_find().
     */
    DDImage_API const char* plugin_name();

    /*!
       Return the filename of the Nth plugin that was found.
     */
    DDImage_API const char* plugin_name(int n);

    /*!
       Find and return the filename of a plugin.
       If the file is found, the full filename is copied to permanent
       storage and that permanent copy is returned.  If the file is
       not found zero is returned.

       If name contains any slashes, then it is expanded relative to the
       directory that is in plugin_name(), and that single filename
       (with any possible extensions) is checked. Note that only forward
       slashes work.

       Otherwise every single directory in getPluginPath() is looked into.

       \a extensions is an optional null-terminated list of filename
       extensions to try. These should start with a period. Each of them
       is added to the filename in each directory and the first match
       is returned. This is *only* done if name does not contain a period
       after the last slash. If you need to know which one was used, look
       at the end of the returned string (I don't return the index in order
       to force calling code to check extensions that are supplied in the
       plugin name).

       The extension ".osx.bundle" is handled specially, if it is matched
       a whole lot more data is tacked on in order to produce the actual
       name of the shared object file that the plugin really is in.
     */
    DDImage_API const char* plugin_find(const char* name, const char* const* extensions = nullptr);

    //! load a plugin with the basename 'name' (or 'altname' if specified and nothing by name can be found)
    /*!
       Try to load a shared-object or tcl plugin.

       plugin_find() is done with the passed name. This means that if the
       name does not have any slashes in it, it is looked for in every
       directory in getPluginPath(). If it does have a slash in it, it
       is tried in one location, relative the the location of the current
       plugin if it does not start with slash.

       If there is no period in the name after the last slash, then it
       tries the following extensions:
       - ".$plugin_ext" (".so", ".dylib", ".dll" etc depending
       on platform). This is a compiled plugin.
       These extensions allow plugins compiled for different platforms to
       coexist in the same directory.
       - ".tcl" and ".gizmo". Extensions for interpreted nuke scripts

       If the plugin is found, it is loaded. If the resulting filename ends
       in ".tcl" or ".gizmo" then plugin_interpret() is called. Otherwise
       plugin_binary() is called. The filename found is returned. If the
       plugin fails to load due to some error you can retrieve the error
       message with plugin_error().

       If no matching filename is found, null is returned. plugin_error()
       is set to an appropriate error message.
     */
    DDImage_API const char* plugin_load(const char* name, const char* altname = nullptr);

    /*!
       Load a plugin and make sure it defines some object (such as a
       script command). The function test(arg) should search for the
       desired object. \a arg is typically the name of the object.

       This calls test(arg) first and if it returns non-zero that is returned.

       It then tries plugin_load(name). If that fails null is returned.

       If the plugin does load, test(arg) is called again. If it returns
       non-zero it returns that value. This will happen if some constructor
       exectued by loading the plugin added the new object to the correct
       tables.

       If test(arg) fails after loading the plugin, plugin_error() is set
       to a message with the filename of the plugin saying the plugin did
       not define \a symbol. If \a symbol is null then \a name is used.

       Thus when this returns, either a pointer to the desired object is
       returned, or null is returned and a useful error message is in
       plugin_error().
     */
    DDImage_API void*       plugin_load(const char* name, void*(*test)(const char*), const char* arg, const char* symbol = nullptr);

    /*!
       Load the plugin from every directory in getPluginPath(). This may
       load several plugins, only the name of the last one is returned.
       The directories in getPluginPath() are tried in backwards order, so that
       "earlier" plugins may override later ones.

       This is used by Nuke to load the <code>init.py</code> and
       <code>menu.py</code> plugins.

       The name of the last plugin found is returned, or NULL if no
       plugin was found.

       The last error produced by any of the plugins is reported by
       plugin_error(). However if no matching plugin was found this is
       not considered an error.
     */
    DDImage_API const char* plugin_load_all(const char* name);

    /*!
      Similar to plugin_load, except that plugin_addpath calls insert
      paths instead of adding them, like plugin_load_all.
    */
    DDImage_API const char* plugin_load_one(const char* name);

    /*!
       Old iterator for accessing items in getPluginPath(). Returns
       NULL if \a n is greater or equal to the size().
     */
    DDImage_API const char* plugin_path(int n);

    /*! Add a directory to getPluginPath().
        Nothing is done if the directory
        is already in the path (this is done with simple strcmp so an
        exact match is needed). \a name is copied to our own storage.

        Normally the new directory is added to the \e start of the path,
        ie the most recently added directory is searched first.

        However if we are currently inside a plugin_load_all/one() call the
        directory is inserted <i>before the directory we are currently
        reading from</i>. In addition if the filename does not start with
        a slash (or letter, colon, slash on NT) then it is appended to the
        current directory. This allows an init.tcl file to "plugin_addpath
        ./subdir" and make subdir be searched without knowing the current
        directory name. If there is an init.tcl in the subdirectory it
        will be sourced as well.
     */
    DDImage_API void        plugin_addpath(const char*);

    /*! Add a directory to the \e end of getPluginPath(), if not already
       in it. See plugin_addpath(). */
    DDImage_API void        plugin_appendpath(const char*);

    /*! Return any error reported by the last call to plugin_load(), or
        return NULL if none. The returned value points at a static buffer
        that is overwritten on each call.
     */
    DDImage_API const char* plugin_error();

    /*! Set an error message for the current plugin. */
    DDImage_API void        plugin_error(const char*);
    extern DDImage_API bool (* plugin_interpret_tcl)(const char* name);
    extern DDImage_API bool (* plugin_interpret_py)(const char* name);

    /*! plugin_load() calls this for a binary plugin. You can call it
        yourself to load dynamic objects in a portable way.

        This returns zero and sets plugin_error() if there is any
        problem loading the file as a plugin.

        If \a symbol is non-zero, it locates that symbol in the plugin and
        returns a pointer to it. If the symbol is not found this returns
        zero and sets plugin_error().

        If symbol is zero, this just returns a non-zero value if the plugin
        successfully loads. Stuff in the init portions of the object will
        be executed, this is how all DDImage plugins initialize themselves.
     */
    DDImage_API void*       plugin_binary(const char* name, const char* symbol = nullptr);

    DDImage_API const char* const* plugin_default_extensions();

    /*!
       Return the list of files in entry \e n in getPluginPath().
       \e n must be less than the size of the path.
     */
    DDImage_API const std::vector <const char*>& plugin_path_list(int n);

    ///! The standard extension used for plugins on this platform (e.g. ".so" or ".dll")
    extern DDImage_API const char* plugin_ext;

    DDImage_API void    setPluginHashCheck(bool lEnabled);

    /*!
       Returns a vector of the directories Nuke will look in to get
       the plugins. This is initially $HOME/.nuke, $NUKE_PATH, and
       directory(NukeExecutableName)/plugins.
     */
    DDImage_API const std::vector<const char*>& getPluginPath();  //!< Returns the entire plugin path as a vector

    /*!
     * Sets system-specific locations that Nuke will look in for third-party plugins.
     * Called during initialisation.
     */
    DDImage_API void setPluginInstallPaths(const std::vector<std::string>& paths);

    /*!
     * Gets system-specific locations that Nuke will look in for third-party plugins.
     */
    DDImage_API const std::vector<std::string>& getPluginInstallPaths();

    /*!
    * Return the plugin path of the application
    */
    DDImage_API const char* getApplicationPluginPath();

    /*!
    * Specify a function that will be called when plugins are unloaded.
    * This is primarily intended for making sure plugins are correctly
    * unregistered from various global tables.
    */
    using PluginUnloadedFunc = void (*)(const char*);
    DDImage_API void setPluginUnloadedCallback(PluginUnloadedFunc f);

    /*!
     * Return a vector of strings containing names for all the registered
     * plugin Ops.
     *
     * When Ops are bundled in a plugin, there is not a one to one relationship
     * between plugin files and Ops.  Loaded plugins (retrieved with
     * plugin_count/plugin_name) is a record of the plugin files which have been
     * loaded.
     *
     * Once a plugin file has been loaded, all of the Ops in the file are
     * registered automatically.  Use this function to query the registered
     * Ops from the loaded plugin files.
     */
    DDImage_API std::vector<std::string> getRegisteredPluginOps();
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
