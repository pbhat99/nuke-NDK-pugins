// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_BASE_PATHUTILS_H
#define FDK_BASE_PATHUTILS_H

#include "fdk/base/StringUtils.h"

#include <string>


namespace fdk
{


//--------------------------------------------------------------------------------
// Posix-style path handling
//--------------------------------------------------------------------------------


//! Build a path from two parts adding a '/' separator.
std::string
buildPath(const std::string& path0,
          const std::string& path1);

//! Split a path into the parent and last directory or filename.
void
splitPath(const std::string& path,
          std::string&       parent_path,
          std::string&       name);

//! Extract the filename (last) portion of the path.
std::string
fileNameFromPath(const std::string& path);


//--------------------------------------------------------------------------------
// Nuke node properties to paths:
//--------------------------------------------------------------------------------


/*! Convert a Nuke node path with '.' separators into a '/' separated path
    starting with '/'.
    Assumes no properties at end of path.
*/
std::string
nukeNodeToPath(const std::string& node_name);

/*! Split a property path into the parent prim path and the property name.
    Assumes Nuke property notation.
*/
inline void
splitPropertyPath(const std::string& path,
                  std::string&       parent_path,
                  std::string&       name);

/*! Split a property 'full' name into the namespace and property name.
    If there is no namespace it's left empty.
    Assumes Nuke property notation.
*/
inline void
splitPropertyName(const std::string& property_name,
                  std::string&       namespace_prefix,
                  std::string&       name);

/*! Extract the property portion of the path removing any leading path,
    namespace prefix or subelement suffix.
    Assumes Nuke property notation.
*/
inline std::string
propertyNameFromPath(const std::string& path);


/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*                   Inline Function Implementations                   */
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/

inline std::string
buildPath(const std::string& path0,
          const std::string& path1)
{
  if (path0.size() == 0) {
    return path1;
  }
  std::string out; out.reserve(path0.size() + path1.size() + 1);
  out = path0;
  if (path0[path0.size()-1] != '/') {
    out += '/';
  }
  out += path1;
  return out;
}

inline void
splitPath(const std::string& path,
          std::string&       parent_path,
          std::string&       name)
{
  const size_t a = path.rfind('/');
  if (a == 0) {
    name = path.substr(1, std::string::npos);
  }
  else if (a == std::string::npos) {
    name = path;
  }
  else {
    parent_path = path.substr(0, a);
    name = path.substr(a+1, std::string::npos);
  }
}

inline std::string
fileNameFromPath(const std::string& path)
{
  if (path.size() == 0) {
    return std::string();
  }
  const size_t a = path.rfind('/');
  if (a != std::string::npos) {
    return path.substr(a+1, std::string::npos);
  }
  else if (path[0] == '.') {
    return path;
  }
  return path; // no leading path
}

inline std::string
nukeNodeToPath(const std::string& node_name)
{
  std::string out = "/" + node_name;
  for (size_t i=0; i < out.size(); ++i) { if (out[i] == '.') { out[i] = '/'; } }
  return out;
}

inline void
splitPropertyPath(const std::string& path,
                  std::string&       parent_path,
                  std::string&       name)
{
  const size_t b = path.rfind(':');
  const size_t a = path.rfind('.', b);
  if (a == 0) {
    name = path.substr(1, std::string::npos);
  }
  else if (a == std::string::npos) {
    name = (b != std::string::npos) ? path.substr(b+1, std::string::npos) : path;
  }
  else
  {
    name = (b != std::string::npos) ? path.substr(b+1, std::string::npos) : path.substr(a+1, std::string::npos);
    // Do this last in case it's overwriting the same path string:
    parent_path = path.substr(0, a);
  }
}

inline void
splitPropertyName(const std::string& property_name,
                  std::string&       namespace_prefix,
                  std::string&       name)
{
  const size_t a = property_name.find(':');
  if (a == std::string::npos) {
    namespace_prefix.clear();
    name = property_name;
  }
  else {
    namespace_prefix = property_name.substr(0, a);
    name = property_name.substr(a+1, std::string::npos);
  }
}

inline std::string
propertyNameFromPath(const std::string& path)
{
  if (path.size() == 0) {
    return std::string();
  }
  size_t a = path.find_last_of(":/");
  if (a != std::string::npos) {
    const size_t b = path.find('.', a+1);
    return path.substr(a+1, (b != std::string::npos) ? (b-a-1) : std::string::npos);
  }
  if (path[0] == '.') {
    ++a;
  }
  const size_t b = path.rfind('.');
  return path.substr(a, (b != std::string::npos) ? (b-a-1) : std::string::npos);
}


} // namespace fdk

#endif

// Copyright (c) 2022 The Foundry Visionmongers Ltd.  All Rights Reserved.
