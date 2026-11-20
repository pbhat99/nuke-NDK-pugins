// SubdivisiontContext.h
// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_SubdivisionContext_h
#define DDImage_SubdivisionContext_h

#include "DDImage/DDImage_API.h"
#include "DDImage/MatrixArray.h"
#include "DDImage/ChannelSet.h"
#include "DDImage/Channel3D.h"
#include "DDImage/Box.h"
#include "DDImage/Box3.h"

namespace DD
{
  namespace Image
  {

    class Iop;
          
    struct DisplacementCoefficients
    {

      enum SubdivisionMode {
        eUniform = 0,
        eScreen,
        eAdaptive,
      };

      int _maxSubdivision;
      int _mode;
      float _edgeLength;
      float _normalThreshold;
      float _displaceThreshold;
      float _edgeThreshold;

      DisplacementCoefficients()
        : _maxSubdivision(4)
        , _mode(eScreen)
        , _edgeLength(20.0f)
        , _normalThreshold(0.9f)
        , _displaceThreshold(0.01f)
        , _edgeThreshold(0.01f)
      {}
    };

    class SubdivisionContext
    {
    public:
      DisplacementCoefficients _coefficients;
      Box _requestedArea;
      Iop* _displacementShader;
      MatrixArray* _transforms;
    };

  }
}

#endif

// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.
