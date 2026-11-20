// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

//---------------------------------------------------------------------------------
//
// This file provides the definition for the ColorCurveEnum used by the read/decode
// interface to the Reader base class added by ReaderExtensions.h and ReaderExtensions.cpp.
//
// NOTE:
// Nuke itself makes no use of these extensions but it needs to compile with them
// so that Reader-derived plugins are compatible with Hiero. Similarly, thirdparty
// plugins built with these extensions included will be compatible with Hiero.
// Note that Reader-derived plugins do not need to provide their own implementation
// of the extensions API, even for use in Hiero, as Reader provides a default
// implementation in terms of its existing virtual engine() interface.
//
//---------------------------------------------------------------------------------

#ifndef DDImage_ReaderExtensions_ColorCurve_H
#define DDImage_ReaderExtensions_ColorCurve_H

namespace DD
{
  namespace Image
  {
    /// \brief Enum to label standard lookup curves applied to colours
    enum ColorCurveEnum
    {
      eColorCurveLinear,
      eColorCurveSRGB,
      eColorCurveRec709,
      eColorCurveLog,
      eColorCurveGamma18,
      eColorCurveGamma22,
      eColorCurveGamma24,
      eColorCurveGamma26,
      eColorCurvePanalog,
      eColorCurveREDLog,
      eColorCurveViperLog,
      eColorCurveAlexaV3LogC,
      eColorCurvePLogLin,
      eColorCurveSLog,
      eColorCurveSLog1,
      eColorCurveSLog2,
      eColorCurveSLog3,
      eColorCurveCLog,
      eColorCurveLog3G10,
      eColorCurveLog3G12,
      eColorCurveHybridLogGamma,
      eColorCurveProtune,
      eColorCurveBT1886,
      eColorCurveHDR2084,
      eColorCurveST2084,
      eColorCurveBMfilmgen5,

      eColorCurveUnknown
    };
  }
}

#endif  // DDImage_ReaderExtensions_ColorCurve_H

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

