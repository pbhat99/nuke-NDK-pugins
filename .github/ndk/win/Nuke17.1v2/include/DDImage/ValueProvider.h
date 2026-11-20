// Copyright (c) 2025 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_ValueProvider_H
#define DD_Image_ValueProvider_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Deprecation.h"
#include <set>
#include <vector>
#include <string>
#include <cstring>

//! Interface to provide facility for 'output knobs', whose values are not
//! stored in the backend, but synthesised on demand from other values. 
//! To create an output knob, implement this interface, create a
//! Knob supporting value providers and call ->setValueProvider() on it pointing at your
//! implementation of the interface.
namespace DD {
  namespace Image {
    class Knob;
    class StringKnobI;
    class ArrayKnobI;
    class Knob_Closure;
    using Knob_Callback = Knob_Closure&;
    class OutputContext;

    class DDImage_API ValueProvider
    {
      public:
        virtual ~ValueProvider() = default;
        //! This function should return true if the knob is presently an output knob.
        //! This allows this functionality to be toggled on/off (with a bool knob for example)
        //! without having to call ->setValueProvider() after the initial setup.
        virtual bool provideValuesEnabled(const Knob* knob, const OutputContext& oc) const = 0;

        //! What kind of backing source is the provider using? This can be used to draw indicators
        //! or color the UI differently.
        //! Note that a 'backing source' is usually a value management system like USD or an
        //! asset manager, vs. a value being provided from an ad-hoc calculation.
        //! Current source types are: 0=ad-hoc calculation, 1=static value (no anim or expressions),
        //! 2=dynamic value (animation/expressions.) These types are coarsely defined for now and
        //! likely require further clarification.
        //! Default is 0, an ad-hoc calculation.
        virtual int providerSourceType(const Knob* knob, const OutputContext& oc) const { return 0; }

        //! Is the source of the provided value at default?
        virtual bool isDefault(const Knob* knob, const OutputContext& oc) const = 0;

        //! Does the provider produce animating values for the OutputContext?
        virtual bool isAnimated(const Knob* knob, const OutputContext& oc) const = 0;

        //! Populates the given set with animation times if there are any.
        //! Returns the number of new times inserted into the set.
        virtual int animationTimes(std::set<double>& times, const OutputContext& oc) { return 0; }


#if kDDImageVersionMajorNum <= 17
        //! DEPRECATED - Please use provideValues(double* values, ...) instead.
        //!
        //! This function should return the values to be displayed in the output knob.
        //! The vector must match the array_size() of the Array_Knob.
        mFnDeprecatedLegacy3DSystemInNuke14("Move to provideValues(double* values, ...) instead.")
        virtual std::vector<double> provideValues(const ArrayKnobI* arrayKnob, const OutputContext& oc) const = 0;

        //! This function should fill in the values to be displayed in the output knob.
        //! The \a values array is already created to \a nValues size, which usually matches the
        //! array_size() of the Array_Knob. Do not write more than \a nValues, no bounds checking
        //! is performed!
        virtual void provideValues(double* values, size_t nValues, const ArrayKnobI* arrayKnob, const OutputContext& oc) const
        { 
          std::vector<double> ret;
          ret = provideValues(arrayKnob, oc);
          mFnAssert(nValues == ret.size());
          std::memcpy(values, ret.data(), nValues*sizeof(double)); 
        }
#else
#error "Remove the above code block in Nuke 18 as they are deprecated in Nuke 17."
        virtual void provideValues(double* values, size_t nValues, const ArrayKnobI* arrayKnob, const OutputContext& oc) const {};
#endif

        //! This function should return the value to be displayed in the output knob.
        virtual void provideStringValue(std::string& value, const StringKnobI* knob, const OutputContext& oc) const {}
    };

    //! Knobs supporting a value provider should implement this interface.
    class DDImage_API ValueProvidableI
    {
      public:
        virtual ~ValueProvidableI() = default;

        //! Assign a custom value provider to the knob. The knob will call this when it needs
        //! to display or return a value.
        virtual void setValueProvider(ValueProvider* valueProvider, bool enabled = true) = 0;

        //! Global enable or disable the assigned ValueProvider. When enabled the knob cannot be
        //! edited and will only display or return a value.
        virtual void enableValueProvider(bool enable = true) = 0;

        //! Returns true if the value provider is assigned and globally enabled.
        //! If context is non-null the provider can test for view, graphscope enabling.
        virtual bool isValueProviderEnabled(const OutputContext* oc = nullptr) const = 0;

        //! Returns the value provider mode (<=0 means disable, 1 or above means enabled when not overriden by authoring)
        virtual int getValueProviderMode() const = 0;

        //! Returns the assigned ValueProvider, or nullptr if none is assigned.
        virtual ValueProvider* getValueProvider() const = 0;
    };

    //! set the valueProvider on the most recently made Knob.  if f.makeKnobs() is not true,
    //! or the that knob is not an ArrayKnobI/StringKnobI, is a no-op.  For calling from knobs(Knob_Callback)
    //! like calls to SetFlags()
    DDImage_API void SetValueProvider(Knob_Callback f, ValueProvider* valueProvider);
  }
}

#endif
