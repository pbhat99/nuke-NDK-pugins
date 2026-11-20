/** \file fdkBaseVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kFdkVersionInteger which you
  can use in a statement like "#if kFdkVersionInteger >= 52100"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _FdkVersionNumber_h_
#define _FdkVersionNumber_h_
#define kFdkVersion "2.1.3"
#define kFdkVersionInteger (10000 * 2 + 1000 * 1 + 3)
#define kFdkVersionMajorNum 2
#define kFdkVersionMinorNum 1
#define kFdkVersionPatchNum 3
#define kFdk_ABI_version Fdk_ABI_2_1
#endif
