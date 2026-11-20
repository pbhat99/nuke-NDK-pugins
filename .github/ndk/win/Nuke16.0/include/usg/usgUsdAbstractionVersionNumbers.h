/** \file usgUsdAbstractionVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kFnUsdAbstractionVersionInteger which you
  can use in a statement like "#if kFnUsdAbstractionVersionInteger >= 14000"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _usgUsdAbstractionVersionNumber_h_
#define _usgUsdAbstractionVersionNumber_h_

#define kFnUsdAbstractionVersion "0.6.2"
#define kFnUsdAbstractionVersionInteger (10000 * 0 + 1000 * 6 + 2)
#define kFnUsdAbstractionVersionMajorNum 0
#define kFnUsdAbstractionVersionMinorNum 6
#define kFnUsdAbstractionVersionPatchNum 2

#endif
