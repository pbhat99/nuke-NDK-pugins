// SceneGraph_Knob.h
// Copyright (c) 2020 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef SCENEREADERPLUGIN_H
#define SCENEREADERPLUGIN_H

#include <DDImage/FileOp.h>

#include <typeinfo>
#include <memory>
#include <string>

namespace DD {
namespace Image {

  class SceneReader;

  class DDImage_API SceneReaderPlugin {
  public:
    virtual ~SceneReaderPlugin() { }
    /*! called during the op call with the Store_Callback as the Knob_Creator callback isn't guaranteed to be
     * available
     *\param cb Store_Callback
     */
    virtual void knobs(Knob_Callback cb) {}
    /*! handle file specific format knobs
    * \param Knob* k the knob that has changed
    * \return true if the knob has been handled by the callback, false otherwise
    */
    virtual bool knob_changed(SceneReader* reader, Knob* k) { return false; }
    /// check if the file can be used by this plugin
    virtual bool isValid(const std::string& filename) = 0;
    /*! generate the list of knobs that's enable/disable state is linked to the read_from_file checkbox
     *  in the SceneReader
     * \return vector of pointers to knobs to enable/disable
     */
    virtual std::vector<Knob*> getFileDependentKnobs(SceneReader* reader) { return std::vector<Knob*>(); }
    /*! request data from plugins
     * \param in input parameters to read from
     * \param out output stream to write to
     * \return true if the request succeeded, false otherwise
     */
    virtual bool query(std::istream& in, std::ostream &out) const { return false; }
    //! This will validate the plugin
    virtual void _validate(SceneReader& reader, bool for_real) {};
  };

namespace SceneReaders {
  /// function signature used to create the SceneReaderPlugin
  typedef std::unique_ptr<SceneReaderPlugin>(*constructorFunction)();
  /// default template function for allocating and creating SceneReaderPlugins
  template<class T>
  std::unique_ptr<SceneReaderPlugin> Constructor() { return std::unique_ptr<SceneReaderPlugin>(new T()); };
  /*! create a new instance of SceneReaderPlugin based on the Class() of the op and the filename in the "file" parameter of the op
   * \param opClass the Class() of the op that the plugin is registered to
   * \param filename the file to load
   * \return a new SceneReaderPlugin created for the node and filename
   */
  DDImage_API std::unique_ptr<SceneReaderPlugin> ConstructPlugin(const std::string& opClass, const std::string& filename);
  /// Creating an instance of this will register the new SceneReaderPlugin constructor for the class with the list of extensions
  struct DDImage_API PluginDescription {
    PluginDescription(const std::string& opClass, const std::vector<std::string>& extensions, constructorFunction constructor);
  };
}
}
}

#endif // SCENEREADERPLUGIN_H
