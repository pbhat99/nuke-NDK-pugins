#ifndef DDIMAGE_SCENEITEM_H
#define DDIMAGE_SCENEITEM_H

#include <tuple>
#include <vector>

#include "DDImage/Knobs.h"

namespace DD {
  namespace Image {
    //! Structure used to populate the SceneGraph_knob
    struct DDImage_API SceneItem
    {
      SceneItem() = default;

      SceneItem(std::string itemName, std::string itemType, bool isEnabled = true)
        :name(std::move(itemName)), type(std::move(itemType)), enabled(isEnabled)
      {
      }


      bool operator==(const SceneItem& item) const
      {
        if (name != item.name)
          return false;

        if (type != item.type)
          return false;

        return enabled == item.enabled;
      }

      explicit operator bool() const { return !(name.empty() && type.empty() && enabled == false); }

      bool operator!=(const SceneItem& item) const
      {
         return name != item.name || type != item.type || enabled != item.enabled;
      }

      std::string name{""};
      std::string type{""};
      bool enabled{false};
    };


    inline std::ostream& operator<<(std::ostream& os, const DD::Image::SceneItem& item)
    {
      os << "SceneItem('" << item.name << "', '" << item.type << "', " << std::boolalpha << item.enabled << ")";
      return os;
    }

    inline bool operator< (const SceneItem& a, const SceneItem& b) {
      return a.name < b.name;
    }

    typedef std::vector<SceneItem> SceneItems;
  }
}

#endif
