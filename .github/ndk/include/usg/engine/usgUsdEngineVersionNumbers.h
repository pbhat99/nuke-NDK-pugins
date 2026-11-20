/** \file usgUsdEngineVersionNumbers.h 
  Library version numbers.
  These are compile-time constants that source code can use in compiler directives.

  The most useful symbol defined is kFnUsdEngineVersionInteger which you
  can use in a statement like "#if kFnUsdEngineVersionInteger >= 12000"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _usgUsdEngineVersionNumber_h_
#define _usgUsdEngineVersionNumber_h_

#define kFnUsdEngineVersion "0.6."
#define kFnUsdEngineVersionInteger (10000 * 0 + 1000 * 6 + 2)
#define kFnUsdEngineVersionMajorNum 0
#define kFnUsdEngineVersionMinorNum 6
#define kFnUsdEngineVersionPatchNum 2

/* #undef kUsdAbstractionPythonEnabled */

#endif
