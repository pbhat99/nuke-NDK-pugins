// Copyright (c) 2018 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef COLORSPACE_KNOBI_H
#define COLORSPACE_KNOBI_H

namespace DD
{
  namespace Image
  {
    /**
     * Abstract base class for Colorspace Knob-specific methods.
     */
    class Colorspace_KnobI
    {

    public:

      /** Signal the Knob that the OCIO Config has changed .

          This will cause the knob to repopulate its Roles and Colorspace menus.

          Pass setToDefault=true if you want the Knob setting back to its default
          Role or Colorspace after updating.
      **/
      virtual void configChanged(bool setToDefault) = 0;

      virtual ~Colorspace_KnobI()
      {
      }
    };

  }
}

#endif
