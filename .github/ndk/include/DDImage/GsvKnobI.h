// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_GSVKNOBI_H
#define DDIMAGE_GSVKNOBI_H

#include "DDImage/DDImage_API.h"
#include "DDImage/GsvSet.h"

#include <optional>
#include <string>

class Gsv_Knob;

namespace DD {
  namespace Image {
    class DDImage_API GsvKnobI
    {
    public:
      virtual ~GsvKnobI();

      //! Gets a Graph Scope Variable node value. This needs to differentiate
      //! between "empty value" and "request node does not exist", so this
      //! returns an optional. Otherwise, the path syntax is the same as for
      //! setting the value, i.e. the delimiter between the nodes is dot.
      virtual std::optional<std::string> getGsvValue(const std::string& path) const = 0;

      //! Sets the value of a Graph Scope Variable node. The delimiter is dot,
      //! so the example use would be this:
      //!   path: __default__.var
      //!   value: val
      //! The path is optional, e.g. this would create the variable under the
      //! "__default__" variable set.
      //!   path: var
      //!   value: val
      //! Returns true if successful, otherwise false.
      virtual bool setGsvValue(const std::string& path, const std::string& value) = 0;

      //! Returns the GsvSets stored in the knob.
      virtual const GsvSets& getValue() const = 0;

      //! Sets the knob data. This will set the model if the group that owns
      //! this knob is attached and its entry exists in the model. If the
      //! groups isn't yet attached, the knob values will be set when the
      //! attachment happens.
      virtual bool setValue(const GsvSets& data) = 0;

      //! Removes a GSV with the given path.
      virtual bool removeGsv(const std::string& path) = 0;

      //! Adds a GsvSet with the given path.
      virtual bool addGsvSet(const std::string& path) = 0;

      //! Removes a GsvSet with the given path.
      virtual bool removeGsvSet(const std::string& path) = 0;

      //! Renames a Gsv with the given \a path to the provided \a name.
      virtual bool renameGsv(const std::string& path, const std::string& name) = 0;

      //! Renames a GsvSet with the given \a path to the provided \a name.
      virtual bool renameGsvSet(const std::string& path, const std::string& name) = 0;

      //! Returns true if \a path is exists relative to the knob. This function
      //! does not make distinction between the element types for the given \a
      //! path i.e. the path must include the GsvSet name when checking for a
      //! variable in the __default__ variable set.
      virtual bool contains(const std::string& path) const = 0;
    };
  }
}

#endif
