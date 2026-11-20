
// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ConvertibleToIop_H
#define DDImage_ConvertibleToIop_H

#include "DDImage/Op.h"
#include "DDImage/Iop.h"

#include <vector>

namespace DD {
  namespace Image {

  /// Helper class for converting non Iop based classes to Iop
  class DDImage_API ConvertibleToIop
  {
  public:
    virtual ~ConvertibleToIop();

    /// return the name of the helper node that should be used for converting
    /// this op to an Iop for viewing.
    virtual const char* conversionHelperNodeClass() const = 0;

    /// return the format for this op after were it to be converted to an Iop
    /// via an instance of the node returned by conversionHelperNodeClass.
    virtual const Format* convertibleFormat() const = 0;

    /// if it is possible just to view another Iop in place of this one in 2D then return it
    virtual Iop* conversionPassThroughIop() const { return nullptr; }
  };
  }
}

#endif
