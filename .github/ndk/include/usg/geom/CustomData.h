// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_CUSTOMDATA_H
#define USG_GEOM_CUSTOMDATA_H

#include <cassert>
#include <memory>

#include "usg/base/Token.h"
#include "usg/base/Value.h"

namespace usg {
  class CustomData;
  using CustomDataRef = std::shared_ptr<CustomData>;

  //! A variant-like class that can hold either a \ref usg::Value, or a \c CustomDataRef object.
  class CustomDataValue
  {
  public:
    explicit CustomDataValue(CustomDataRef customData)
    : m_isCustomData(true)
    , m_customData(std::move(customData))
    {
    }

    explicit CustomDataValue(const Value& value)
    : m_isCustomData(false)
    , m_value(value)
    {
    }

    bool isCustomData() const { return m_isCustomData; }

    const CustomDataRef& toCustomData() const
    {
      assert(m_isCustomData);
      return m_customData;
    }

    const Value& toValue() const
    {
      assert(!m_isCustomData);
      return m_value;
    }

  private:
    const bool m_isCustomData;
    const CustomDataRef m_customData;
    const Value m_value;
  };

  using CustomDataEntry = std::pair<Token, CustomDataValue>;

  //! A key-value structure where each entry can be either a \ref usg::Value, or another
  //! \c CustomDataRef key-value structure.
  //!
  //! This is a light wrapper for \c PXR_NS::VtDictionary that is built on \c std::vector. Lookup
  //! operations are not supported; insertion order is preserved.
  class CustomData : public std::vector<CustomDataEntry>
  {
  };
} // namespace usg

#endif
