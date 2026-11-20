// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_BASE_PLUGINREGISTRY_H
#define FDK_BASE_PLUGINREGISTRY_H

#include "fdk/api.h"
#include "fdk/base/Hash.h"
#include "fdk/base/StringTypes.h"

#include <filesystem>
#include <vector>
#include <string>

namespace fdk
{

using PluginHandle = void*;


class FDK_API PluginRegistry
{
public:
  struct Directory
  {
    std::string path;           //!< Directory's parent path
    std::string name;           //!< Directory's name
    bool        loaded = false; //!< Has the directory's file list been loaded?
    bool        ok = true;      //!< Is directory valid? Will be false if load failed for any reason.
    StringSet   contents;       //!< Set of child names.

    Directory() {}
    //! Splits \a filepath to derive the directory parent path and name.
    Directory(const std::filesystem::path& filepath);
    //! Directly set the path and name of the directory.
    Directory(const std::string& _path,
              const std::string& _name);

    explicit operator bool() const { return (loaded && ok); }

    //! Returns true if searchTerm is in this directory's contents list.
    //! If the directory has not been loaded yet, do it now.
    bool find(const std::string& searchTerm) const;

    //! Load the contents of the directory.
    void load(bool force = false);

    //! Force the directory to reload its contents.
    void reload() { load(true/*force*/); }
  };

  using DirectoryList = std::vector<Directory>;


public:

  //! Adds a search path to the beginning of the path list.
  void                  prependSearchPath(const char*);

  //! Adds a search path to the end of the path list.
  void                  appendSearchPath(const char*);

  //! Returns the number of total search path strings, which is also the total number of Directories.
  size_t                numSearchPaths() const { return _searchPaths.size(); }

  //! Returns the list of search paths.
  const StringList&     searchPaths() const;

  //! Returns the list of directories the registry will search to find a plugin.
  //! The indexing of this list will match searchPaths().
  const DirectoryList&  directories() const;

  //! Retrieve the Directory entry.
  const Directory&      directory(size_t i) const { return _directories[i]; }

  //! Returns the Directory containing the file in \a filepath if it exists in the registry,
  //! otherwise an empty(invalid) Directory is returned.
  const Directory&      findDirectoryForPath(const char* filepath) const;

  //! Returns the Directory matching \a directory_name if it exists in the registry,
  //! otherwise a new Directory is added with that name.
  //! If \a name_length is non-zero then only that portion of the \a directory_name
  //! string is used as for the name search.
  const Directory&      findOrAddDirectory(const char* directory_name,
                                           size_t      name_length = 0);

  //-------------------------------------------------------------------------

  //! List of default extensions that findPlugin() will append to the plugin name and search
  //! for, in priority order.
  const StringList&     pluginExtensions() const { return _extensions; }

  //! Attempt to find a plugin matching \a plugin_name with each possible extension inside each
  //! of the search directories, returning the path to the plugin file.
  //! If the \a extensions list is explicitly provided they are used instead of the current list
  //! in the registry.
  std::filesystem::path  findPlugin(const char*       plugin_name,
                                    const StringList* extensions = nullptr);

  //! Attempt to find and load a plugin matching \a plugin_name with each possible extension
  //! inside each of the search directories.
  //! If \a plugin_name starts with a '\' or '/' then it's assumed to be an absolute file path
  //! and the plugin file directly loaded, skipping the search and extension matching.
  //! If the \a extensions list is explicitly provided it's used instead of the current list
  //! in the registry.
  //! If the plugin file is loaded without error it is added to the end of the loadedPlugins()
  //! list and the pointer to it returned, otherwise the error message from the failed load
  //! is retrieved via lastLoadedError().
  PluginHandle  loadPlugin(const char* plugin_name,
                           const StringList* extensions = nullptr);

  //! Returns the path of the most recently loaded plugin file, or empty if error has occurred,
  //! found in lastLoadedError().
  const std::string&  lastLoadedPlugin() const;

  bool                haveLoadError() const { return (!_lastLoadedError.empty()); }
  const std::string&  lastLoadedError() const { return _lastLoadedError; }
  void                setLastLoadedError(const char* error_message);


  //! Returns the list of paths to all successfully loaded plugins, last loaded at end.
  const StringList&   loadedPlugins() const { return _loadedPlugins; }


protected:
  PluginRegistry(const PluginRegistry&) = delete;
  PluginRegistry& operator= (const PluginRegistry&) = delete;
 ~PluginRegistry() = default;

  //! Called the first time the registry is asked for the list of paths.
  //! A subclass should add the plugin search paths needed for basic functionality such as
  //! directories relative to the application or library install location.
  //! Must implement.
  virtual void  _initializeSearchPaths() = 0;

  //! Check that a loaded plugin file object is ok for the client dcc. Can be used for
  //! binary validation, checking for symbols, or license checks.
  //! Called when a plugin file has successfully loaded inside loadPlugin(), but before
  //! it's been added to the _loadedPlugins list.
  //! If this method returns false the plugin is unloaded and loadPlugin() returns a
  //! null PluginHandle.
  //! A clent dcc can fille in setLastLoadedError() with whatever it needs to communicate
  //! the reason why the plugin was rejected.
  //! Default implementation returns true.
  virtual bool  _isValidPlugin(const char*                  /*plugin_name*/,
                               const std::filesystem::path& /*plugin_filepath*/,
                               PluginHandle                 /*plugin_handle*/) const { return true; }


protected:
  StringList    _searchPaths;       //!< List of directory paths in search order, starting with 0
  DirectoryList _directories;       //!< List of directory contents in search order (matching _searchPaths)
  StringList    _loadedPlugins;     //!< Names of plugins already loaded, last loaded at end.
  std::string   _lastLoadedError;   //!< Contains the error message from the last loaded plugin, or empty if no error.
  StringList    _extensions;        //!< List of possible extensions to check, in priority order (with 0 = highest)

  PluginRegistry();

};


} // namespace fdk


#endif
