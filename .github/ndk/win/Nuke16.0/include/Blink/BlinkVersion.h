// Copyright (c) 2014 The Foundry Visionmongers Ltd.  All Rights Reserved.

/** \file BlinkVersion.h
 * \brief The main header file for Blink Library versioning.
 */

#ifndef Blink_Library_BlinkVersion_h
#define Blink_Library_BlinkVersion_h

#include "BlinkBuild.h"
#include "BlinkVersionNumbers.h"

#define BlinkVersionFunction kBlinkLibrary_ABI_version

extern "C" int BLINK_SHARED_API BlinkVersionFunction();



namespace Blink
{

  class Version
  {
  public:
    Version()
    {
      BlinkVersionFunction();
    }
  };

} // namespace Blink

static Blink::Version blinkVersion;

#endif // Blink_Library_BlinkVersion_h
