// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef RADIO_KNOBI_H
#define RADIO_KNOBI_H

#include <DDImage/Enumeration_KnobI.h>

namespace DD
{
  namespace Image
  {

    /**
     * Abstract base class for Radio Knob-specific methods.
     */
    class Radio_KnobI : public Enumeration_KnobI
    {
    public:      
      ~Radio_KnobI() override
      {
      }
    };

  }
}

#endif
