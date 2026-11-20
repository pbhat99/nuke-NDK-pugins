// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_ArrayKnobI_H
#define DD_Image_ArrayKnobI_H

#include <cstdint>
#include <set>
#include <stddef.h>
#include <vector>
#include <cstring>  // for std::memcpy

#include "DDImage/DDImage_API.h"
#include "DDImage/Deprecation.h"
#include "DDImage/ValueProvider.h"
#include "usg/base/Value.h"

#include "fdk/math/Vec3.h"

namespace DD {
  namespace Image {
    class Knob;
    class Knob_Closure;
    using Knob_Callback = Knob_Closure&;
    class OutputContext;

    class DDImage_API ArrayKnobI : public ValueProvidableI
    {
    public:
      virtual ~ArrayKnobI();

      //! Return the Knob this interface is on. Implementations should just "return this;"
      virtual Knob* baseKnob() = 0;

      //! Does this knob have an animation key at the given index and given time?
      //! "index" is the channel of the array knob that we are interested in. An index of -1
      //! indicates that we are not interested in any particular channel, and will return true
      //! if any of the channels have a key at this time.
      virtual bool has_key_at(double time, int index = -1) const = 0;

      //! Does this knob have an animation key at the given index?
      //! "index" is the channel of the array knob that we are interested in. An index of -1
      //! indicates that we are not interested in any particular channel, and will return true
      //! if any of the channels have a key at this time.
      virtual bool has_key(int index = -1) const = 0;

      //! get the number of elements stored in the knob
      virtual size_t array_size() const = 0;

      //! Change the dimensions of the knob
      virtual bool resize(int w, int h = 1) = 0;

      //! Return all fields on the ArrayKnob, evaluating expressions and suchforth for
      //! an optional specific OutputContext. If the requested OutputContext matches one
      //! already calculated and stored in the ArrayKnob's cache, the cached version is
      //! returned. Otherwise the new OutputContext is evaluated and added to the cache,
      //! and then returned.
      //! If OutputContext is null then Knob::uiContext() will be used.
      virtual const std::vector<double>& array_at(const OutputContext* context = nullptr) const = 0;

      //! Return the knob's default values.
      virtual const std::vector<double>& getDefaultValues() const = 0;

      //! Match all animation keys and expressions from another Array Knob to this one. This will completely
      //! replace this knob's current values, animation and views with those from the source knob.
      //! This will add and remove keys and views as appropriate
      virtual void matchAnimation(const ArrayKnobI* sourceKnob, int sourceKnobIndex, int thisKnobIndex) = 0;

    };



    //----------------------------------------------------------------


    struct DDImage_API ArrayKnobRef
    {
      Knob*       knob;       //!< Must be an ArrayKnob
      uint32_t    channel;    //!< Array channel index to target
      fdk::Vec3d  xform;      //!< Offset (xform.x), scale (xform.y), shift (xform.z) to apply to value

      ArrayKnobRef() : knob(nullptr), channel(0), xform(0.0, 1.0, 0.0) {}
      ArrayKnobRef(Knob* k, double scale = 1.0) : knob(k), channel(0), xform(0.0, scale, 0.0) {}
      ArrayKnobRef(Knob* k, const fdk::Vec3d& x) : knob(k), channel(0), xform(x) {}
      ArrayKnobRef(Knob* k, uint32_t chan, double scale) : knob(k), channel(chan), xform(0.0, scale, 0.0) {}
      ArrayKnobRef(Knob* k, uint32_t chan, const fdk::Vec3d& x) : knob(k), channel(chan), xform(x) {}

      //! Apply the offset/scale/shift to value as ((v + offset) * scale) + shift
      double  applyTransform(double v) const { return (v + xform.x) * xform.y + xform.z; }

      //! Extract a single ArrayKnob channel double at the optional context with transform applied.
      //! If the Knob, ArrayKnob, or the channel is invalid then \a defaultVal is returned.
      double  get(double defaultVal, const OutputContext* context = nullptr) const;

      //! Same, but extracts a single usg::Value from the Knob.
      //! \a valueType is required and defines the type of usg::Value returned, and the casting
      //! of the ArrayKnob double to the correct data type.
      usg::Value  getValue(usg::Value::Type valueType, double defaultVal, const OutputContext* context = nullptr) const;

    };

    using ArrayKnobRefList = std::vector<ArrayKnobRef>;


  }
}

#endif
