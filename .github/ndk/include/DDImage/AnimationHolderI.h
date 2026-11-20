
// Copyright (c) 2019 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_AnimationHolderI_H
#define DD_Image_AnimationHolderI_H

#include "DDImage/DDImage_API.h"

namespace DD {
  namespace Image {

    //! This is for internal use only.
    //! Only to be used by knobs that are also memory holders (e.g. AnimationKnob).
    class DDImage_API AnimationHolderI
    {
    public:
      virtual ~AnimationHolderI() = 0;
    };

  }
}

#endif
