// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include "usg/api.h"
#include "usg/base/Array.h"

#include <iosfwd>
#include <string>
#include <set>
#include <vector>

namespace usg {
  class Path;

  //! A wrapper for SdfPathExpression
  class USG_API PathExpr
  {
  public:
    class Handle; //!< Abstraction handle for casting to underlying implementation type

    PathExpr();
    PathExpr(const std::string& expression);
    PathExpr(const PathExpr::Handle& handle);
    ~PathExpr();

    // movable
    PathExpr(PathExpr&& rhs) noexcept;
    PathExpr& operator=(PathExpr&& rhs) noexcept;

    // and copyable
    PathExpr(const PathExpr& rhs);
    PathExpr& operator=(const PathExpr& rhs);

    /// Equality.
    bool operator==(const PathExpr& rhs) const;
    bool operator!=(const PathExpr& rhs) const { return !(*this == rhs); }

    //! Returns the text representation of the expression.
    //! Cannot return a reference since this is created on request
    //! and not returned by reference by the SdfPathExpression GetText.
    std::string asString() const;
    //! Return a new expression created by making any relative path prefixes in this expression
    //! absolute.
    PathExpr makeAbsolute(const Path& anchor) const;

    //! Get the underlying SdfPathExpression
    PathExpr::Handle&       getPathExpr(int usd_version);
    const PathExpr::Handle& getPathExpr(int usd_version) const;

  public:
    //! Returns the index of the underlying handle. For internal use only!
    int handleIndex() const { return _handle; }
    //! Assign from a pre-known handle index. For internal use only!
    explicit PathExpr(int handleIndex) : _handle(handleIndex) {}

  public:
      //! Lazily construct a PathExpr when requested. Primarily used for the static PathExpr 
      // structs.
      class Lazy {
      public:
        Lazy(const char* text) 
        : _pathExpr(nullptr)
        , _text(text)
        {}

        const PathExpr& pathExpr() const
        {
          if (!_pathExpr) {
            const_cast<PathExpr::Lazy*>(this)->_pathExpr = new PathExpr(_text);
          }
          return *_pathExpr;
        }

        operator const PathExpr&() const { return pathExpr(); }
        operator       PathExpr&() { return const_cast<PathExpr&>(pathExpr()); }

        //! Convenience to avoid writing 'MyStaticPathExprs.MyPathExpr.pathExpr().asString()'.
        std::string asString() const { return pathExpr().asString(); }

        PathExpr* _pathExpr;
        const char* _text;
      };

  private:
    int _handle;
  };

  //! Output the PathExpr string representation to a stream
  USG_API std::ostream& operator<<(std::ostream& stream, const PathExpr& path);

}

