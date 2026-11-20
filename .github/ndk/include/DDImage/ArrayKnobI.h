// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_ArrayKnobI_H
#define DD_Image_ArrayKnobI_H

#include <cstdint>
#include <set>
#include <stddef.h>
#include <vector>

#include "DDImage/DDImage_API.h"

namespace DD {
  namespace Image {
    class Knob;
    class Knob_Closure;
    using Knob_Callback = Knob_Closure&;
    class OutputContext;


    class DDImage_API ArrayKnobI
    {
    public:
      //! Interface to provide facility for 'output knobs', whose values are not
      //! stored in the backend, but synthesised on demand from other values. 
      //! To create an output knob, implement this interface, create an
      //! Array_Knob and call ->setValueProvider() on it pointing at your
      //! implementation of the interface.
      class DDImage_API ValueProvider
      {
      public:
        //! This function should return true if the knob is presently an output knob.
        //! This allows this functionality to be toggled on/off without (with a bool knob for example)
        //! without having to call ->setValueProvider() after the initial setup.
        virtual bool provideValuesEnabled(const ArrayKnobI* arrayKnob, const OutputContext& oc) const = 0;

        //! This function should return the values to be displayed in the output knob.
        //! The vector must match the array_size() of the Array_Knob.
        virtual std::vector<double> provideValues(const ArrayKnobI* arrayKnob, const OutputContext& oc) const = 0;

        virtual ~ValueProvider() { }
      };

      virtual void setValueProvider(ValueProvider* valueProvider) = 0;


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

      //! Match all animation keys and expressions from another Array Knob to this one. This will completely
      //! replace this knob's current values, animation and views with those from the source knob.
      //! This will add and remove keys and views as appropriate
      virtual void matchAnimation(const ArrayKnobI* sourceKnob, int sourceKnobIndex, int thisKnobIndex) = 0;

    };


    //! set the valueProvider on the most recently made Knob.  if f.makeKnobs() is not true,
    //! or the that knob is not an ArrayKnobI, is a no-op.  For calling from knobs(Knob_Callback)
    //! like calls to SetFlags()
    DDImage_API void SetValueProvider(Knob_Callback f, ArrayKnobI::ValueProvider* valueProvider);



  }
}

#endif
