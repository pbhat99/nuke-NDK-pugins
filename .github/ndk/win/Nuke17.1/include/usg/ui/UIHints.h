// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_UI_UIHINTS_H
#define USG_UI_UIHINTS_H

#include "usg/api.h"
#include "usg/geom/Attribute.h"
#include "usg/geom/Prim.h"
#include "usg/geom/Relationship.h"

#include <memory>

namespace usg {

class PrimHintsHandle;
class PropertyHintsHandle;
class AttributeHintsHandle;

namespace ui {

  class USG_API PrimHints {
  public:
    PrimHints();
    explicit PrimHints(const Prim& prim);
    PrimHints(const PrimHints& other);
    PrimHints& operator=(const PrimHints& other);
    ~PrimHints();

    bool validate();
    bool isValid() const;
    explicit operator bool() const { return isValid(); }

    bool setDisplayName(const std::string& value);
    std::string getDisplayName() const;

    bool setHidden(bool value);
    bool getHidden() const;

    bool setDisplayGroupExpanded(const std::string& groupName, bool expanded);
    bool getDisplayGroupExpanded(const std::string& groupName, bool defaultValue = false) const;

    bool setDisplayGroupShownIf(const std::string& groupName, const std::string& expression);
    std::string getDisplayGroupShownIf(const std::string& groupName) const;

  private:
    std::unique_ptr<PrimHintsHandle, void (*)(PrimHintsHandle*)> _pImpl;
  };

  class USG_API PropertyHints {
  public:
    PropertyHints();
    explicit PropertyHints(const Attribute& attribute);
    explicit PropertyHints(const Relationship& relationship);
    PropertyHints(const PropertyHints& other);
    PropertyHints& operator=(const PropertyHints& other);
    ~PropertyHints();

    bool validate();
    bool isValid() const;
    explicit operator bool() const { return isValid(); }

    bool setDisplayName(const std::string& value);
    std::string getDisplayName() const;

    bool setHidden(bool value);
    bool getHidden() const;

    bool setDisplayGroup(const std::string& value);
    std::string getDisplayGroup() const;

    bool setShownIf(const std::string& expression);
    std::string getShownIf() const;

  private:
    std::unique_ptr<PropertyHintsHandle, void (*)(PropertyHintsHandle*)> _pImpl;
  };

  class USG_API AttributeHints {
  public:
    AttributeHints();
    explicit AttributeHints(const Attribute& attribute);
    AttributeHints(const AttributeHints& other);
    AttributeHints& operator=(const AttributeHints& other);
    ~AttributeHints();

    bool validate();
    bool isValid() const;
    explicit operator bool() const { return isValid(); }

    bool setDisplayName(const std::string& value);
    std::string getDisplayName() const;

    bool setHidden(bool value);
    bool getHidden() const;

    bool setDisplayGroup(const std::string& value);
    std::string getDisplayGroup() const;

    bool setShownIf(const std::string& expression);
    std::string getShownIf() const;

    bool setValueLabelsOrder(const TokenArray& labels);
    TokenArray getValueLabelsOrder() const;

  private:
    std::unique_ptr<AttributeHintsHandle, void (*)(AttributeHintsHandle*)> _pImpl;
  };

}
}

#endif
