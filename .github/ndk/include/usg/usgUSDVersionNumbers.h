/** \file usgUSDVersionNumbers.h 
  USD package version numbers.

  These are compile-time constants that source code can use in compiler directives
  to check the version numbers of the USD package this library was built against.

  The most useful symbol defined is kFnUsgUSDVersion which you can use in a statement
  like "#if kFnUsgUSDVersion >= 2205" to see if the version contains a given feature
  or bug fix.
*/

#ifndef _usgUSDVersionNumber_h_
#define _usgUSDVersionNumber_h_

#define kFnUsgUSDVersion "2405"
#define kFnUsgUSDMajorVersion 0
#define kFnUsgUSDMinorVersion 24
#define kFnUsgUSDPatchVersion 5

#endif
