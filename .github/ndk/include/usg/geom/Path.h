// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_GEOMPATH_H
#define USG_GEOM_GEOMPATH_H

#include "usg/api.h"
#include "usg/base/Array.h"
#include "usg/base/Token.h"

#include <iosfwd>
#include <functional>
#include <string>
#include <set>
#include <vector>

namespace fdk {
  class Hash;
}

namespace usg {
  //! A  wrapper for SdfPath
  class USG_API Path {
  public:
    class Handle; //!< Abstraction handle for casting to underlying implementation type

    Path();
    Path(const char* text);
    Path(const std::string& text);
    Path(const Path::Handle& handle);
    ~Path();

    // movable:
    Path(Path&& rhs) noexcept;
    Path& operator=(Path&& rhs) noexcept;

    // and copyable
    Path(const Path& rhs);
    Path& operator=(const Path& rhs);

    //---------------------------------------------------

    //! Return the last part of the path, usually the prim or attribute name.
    const std::string& name() const;

    //! Return the last part of the path, usually the prim or attribute name, as a token.
    Token nameToken() const;

    //! Return the parent portion of the path.
    Path parent() const;

    //! Clear the path to empty
    void clear();

    //! Return true if the path is empty
    bool empty() const;

    //! Return the path as a string.
    std::string asString() const;

    //! Return a const reference to the path string in persistent storage, which is built on demand.
    const std::string& string() const;

    //---------------------------------------------------

    //! Return a new path consisting of appending a child component to the path
    Path appendChild(const Token& suffix) const;

    //! Return a new path consisting of appending a child component to the path
    Path appendChild(const char* suffix) const;

    //! Return a new path consisting of appending a child component to the path
    Path appendChild(const std::string& suffix) const;

    //! Return a new path consisting of appending another path to this path
    Path appendPath(const Path& path) const;

    //! Return a new path consisting of appending a property component to the path
    Path appendProperty(const Token& property) const;

    //! Returns a new absolute path resolved against the anchor path
    Path makeAbsolutePath(const Path& anchor) const;

    //! Returns a new path relative to the anchor path
    Path makeRelativePath(const Path& anchor) const;

    //! Creates a path by stripping all relational attributes, targets,
    //! properties, and variant selections from the leafmost prim path.
    //! If the path is already a prim path, the same path is returned.
    Path getPrimPath() const;

    //! Return true if both this path and prefix are not empty and this path has prefix as a
    //! prefix.
    bool hasPrefix(const Path& prefix) const;

    //! Returns a path with all occurrences of the prefix path
    //! \p oldPrefix replaced with the prefix path \p newPrefix.
    Path replacePrefix(const Path& oldPrefix,
                       const Path& newPrefix,
                       bool fixTargetPaths = true) const;

    //! Fill prefixes with up to numPrefixes prefixes of this path. If 0 fill with all prefixes
    void getPrefixes(std::vector<Path>& prefixes, size_t numPrefixes=0) const;

    //---------------------------------------------------

    bool operator==(const Path& rhs) const;
    bool operator!=(const Path& rhs) const { return !(*this == rhs); }

    bool operator<(const Path& rhs) const;

    //! Get the underlying SdfPath
    Path::Handle&       getPath(int usd_version);
    const Path::Handle& getPath(int usd_version) const;

    //! Get the hash of the underlying SdfPath
    uint64_t hash() const;

    //! Add the path's hash to a fdk::Hash object.
    void append(fdk::Hash& hash) const;

    //! Checks if path contents are valid, and if not return false and pass the error string into the subsystem.
    bool isValidPath(std::string* errorMsg = nullptr) const;

    //! Returns true if the path is absolute
    bool isAbsolutePath() const;

    //! Checks if string is a valid Path, and if not return false and pass the error string into the subsystem.
    static bool IsValidPathString(const std::string& pathString, std::string* errorMsg = nullptr);

    static const Path& AbsoluteRootPath();
    
    //! Remove all elements of paths that are prefixed by other elements in paths.
    static void RemoveDescendentPaths(std::vector<Path>* paths);

  public:
    //! Returns the index of the underlying handle. For internal use only!
    int handleIndex() const { return _handle; }
    //! Assign from a pre-known handle index. For internal use only!
    explicit Path(int handleIndex) : _handle(handleIndex) {}

  private:
    int _handle;
  };

  //! Append the path to a hash
  USG_API fdk::Hash& operator<<(fdk::Hash& hash, const Path& path);

  //! Output the path to a stream
  USG_API std::ostream& operator<<(std::ostream& stream, const Path& path);

  using PathArray = usg::Array<Path>;
  using PathSet   = std::set<Path>;

}

template<>
struct std::hash<usg::Path>
{
  std::size_t operator()(const usg::Path& path) const noexcept
  {
    return path.hash();
  }
};

#endif
