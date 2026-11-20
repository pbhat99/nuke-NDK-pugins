// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_VALUEDICTIONARY_H
#define USG_GEOM_VALUEDICTIONARY_H

#include <cassert>
#include <memory>

#include "usg/base/Token.h"
#include "usg/base/Value.h"

namespace usg {
  class ValueDictionary;
  using ValueDictionaryRef = std::shared_ptr<ValueDictionary>;

  //! A variant-like class that can hold either a \ref usg::Value, or a \c ValueDictionaryRef object.
  class ValueDictionaryValue
  {
  public:
    explicit ValueDictionaryValue(ValueDictionaryRef ValueDictionary)
    : m_isValueDict(true)
    , m_valueDict(std::move(ValueDictionary))
    {
    }

    explicit ValueDictionaryValue(const Value& value)
    : m_isValueDict(false)
    , m_value(value)
    {
    }

    bool isValueDictionary() const { return m_isValueDict; }

    const ValueDictionaryRef& toValueDictionary() const
    {
      assert(m_isValueDict);
      return m_valueDict;
    }

    const Value& toValue() const
    {
      assert(!m_isValueDict);
      return m_value;
    }

  private:
    const bool m_isValueDict;
    const ValueDictionaryRef m_valueDict;
    const Value m_value;
  };

  using ValueDictionaryEntry = std::pair<Token, ValueDictionaryValue>;

  //! A key-value structure where each entry can be either a \ref usg::Value, or another
  //! \c ValueDictionaryRef key-value structure.
  //!
  //! This is a light wrapper for \c PXR_NS::VtDictionary that is built on \c std::vector. Lookup
  //! operations are not supported; insertion order is preserved.
  class ValueDictionary : public std::vector<ValueDictionaryEntry>
  {
  };
} // namespace usg

#endif
