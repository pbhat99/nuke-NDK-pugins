// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_PRIMVAR_H
#define USG_GEOM_PRIMVAR_H

#include "usg/api.h"

#include "usg/base/ArrayTypes.h"

#include "usg/geom/Attribute.h"
#include "usg/geom/Prim.h"

namespace usg {

  //! A class representing a primvar attribute. This has extra methods for dealing with indices, flattening values and so on.
  class USG_API Primvar {
  public:
    Primvar();

    //! Construct primvar with the existing attribute named by the given name
    Primvar(const Prim& prim, const Token& name);

    //! Construct primvar with the given attribute
    Primvar(const Prim& prim, const Attribute& attr);

    //! Create primvar with the given name and type
    Primvar(Prim& prim, const Token& name, const Value::Type& typeName);

    //! Returns true if the primvar is valid
    bool isValid() const;
    explicit operator bool() const { return isValid(); }

    //! Returns the primvar attribute
    Attribute& attribute() { return _attr; }

    //! Block the indices that were previously set
    void blockIndices();

    //! Returns the primvar indices attribute if it exists
    Attribute& indicesAttribute() { return _indicesAttr; }

    //! Sets the interpolation for this attribute
    bool setInterpolation(const Token& interpolation);

    //! Gets the interpolation for this attribute
    Token getInterpolation() const;

    //! Sets the elementSize for this attribute
    bool setElementSize(const int elementSize);

    //! Gets the elementSize for this attribute
    int getElementSize() const;

    //! Gets the attribute name from primvar name like "displayColor" -> "primvars:displayColor"
    static Token makeNamespaced(const Token& name);

    //! Computes the flattened primver attributes from the attribute and its indices. If there are no indices, just returns the straight values.
    template<class T> bool computeFlattened(Array<T>& flattenedValues, fdk::TimeValue time = fdk::defaultTimeValue()) const;

  protected:
    Prim _prim;
    Attribute _attr;
    Attribute _indicesAttr;
  
  private:
    //! Sets invalid index if empty or blocked. Used in constructors
    void _setupIndices();

  };


  template<class T> inline bool Primvar::computeFlattened(Array<T>& flattenedValues, fdk::TimeValue time) const
  {
    if (_attr) {
      flattenedValues.clear();
      if (_indicesAttr) {
        IntArray indices;
        Array<T> values;
        _indicesAttr.getValue(indices, time);
        _attr.getValue(values, time);
        flattenedValues.resize(indices.size());
        for (size_t i = 0; i < indices.size(); ++i) {
          flattenedValues[i] = values[indices[i]];
        }
      }
      else {
        _attr.getValue(flattenedValues, time);
      }
      return true;
    }
    return false;
  }

}

#endif
