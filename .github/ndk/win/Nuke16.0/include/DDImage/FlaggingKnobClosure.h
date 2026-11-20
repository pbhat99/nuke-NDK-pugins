// FlaggingKnobClosure.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/** \file FlaggingKnobClosure.h
 * \brief Closure to flag all knobs when creating them.
 */

#ifndef FlaggingKnobClosure_H
#define FlaggingKnobClosure_H

#include "Knobs.h"

// we must fool Doxygen a little
#ifdef DOXYGEN
  #error 'DOXYGEN' must not be defined when compiling!
#endif

namespace DD
{
  namespace Image
  {

#ifndef DOXYGEN

    class DDImage_API FlaggingKnob_Closure : public Knob_Closure
    {
    private:
      Knob_Callback _f;
      Knob::FlagMask _flags;

    public:

      static const Knob::FlagMask gDisabledKnobEdit = DD::Image::Knob::DISABLED | DD::Image::Knob::READ_ONLY | DD::Image::Knob::READ_ONLY_IN_SCRIPTS;

      FlaggingKnob_Closure(Knob_Callback f, Knob::FlagMask flags);

      ~FlaggingKnob_Closure() override;

      Knob* call(int type, StoreType datatype, void* pointer, const char* name, const char* label, const void* d) override;

      void constructed( Knob* k ) override{_f.constructed(k);}
      size_t getKnobCount() const override { return _f.getKnobCount(); }
    };

#endif
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
