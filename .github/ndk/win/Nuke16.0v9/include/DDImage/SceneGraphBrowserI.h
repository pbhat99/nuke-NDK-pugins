// Copyright (c) 2020 The Foundry Visionmongers Ltd.  All Rights Reserved.
#ifndef SCENEGRAPHBROWSERI_H
#define SCENEGRAPHBROWSERI_H

#include "DDImage/DDImage_API.h"
#include "DDImage/SceneItem.h"
#include <string>
#include <vector>
#include <map>

namespace DD {
namespace Image{
  /// type of primitive represented by the scene
  class SceneGraph_KnobI;

  namespace SceneGraph {

  static const std::string kSceneGraphPluginClass = "kSceneGraphPluginClass";
  static const std::string kSceneGraphKnobCommand = "command:scene_graph";
  static const std::string kSceneGraphTypeCommand = "command:op_type";

  /*! Deserialise string map values stored in a stringstream
   * \param stream stream to deserialise
   *    no of keys | key2 | no of items for key1 | items for key1 ... | key2 | no of items for key2 | items for key2...
   *    For example, a stringstream that had two objects, with type `sphere, cube` and name `a, b` and an
   *    enabled state on and off looks like:
   *    "3""name""2""a""b""type""2""sphere""cube""enabled""2""1""0"
   * \return collection of scene items
   */
  DDImage_API DD::Image::SceneItems DeserialiseSceneData(std::stringstream& stream);

  /*! Write scene data to the output stream
   * \param data serialise this data into the stream
   * \param out stream to serialise into
   */
  DDImage_API void SerialiseSceneData(const DD::Image::SceneItems& data, std::ostream &out);

   /*! Build the scene browser from provided information about primitives and show it
    * \param fileName the name of the file to load
    * \param pSceneGraphKnob the scene graph knob
    * \param items collection of scene graph items to insert into the tree
    * \return true if primitives were selected false if the browser failed to load or nothing was selected
    */
   DDImage_API bool createBrowser(const char* fileName, DD::Image::SceneGraph_KnobI* pSceneGraphKnob, const DD::Image::SceneItems& items);
  }
}
}

#endif // SCENEGRAPHBROWSERI_H
