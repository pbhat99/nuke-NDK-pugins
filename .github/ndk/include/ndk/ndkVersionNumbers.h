/** \file ndkVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kNdkVersionInteger which you
  can use in a statement like "#if kNdkVersionInteger >= 52100"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _NdkVersionNumber_h_
#define _NdkVersionNumber_h_
#define kNdkVersion "16.0"
#define kNdkVersionInteger (10000 * 16 + 1000 * 0 + 9)
#define kNdkVersionMajorNum 16
#define kNdkVersionMinorNum 0
#define kNdkVersionReleaseNum 9
#define kNdkVersionReleaseStr "v9"
#define kNdkVersionPhaseNum 3
#define kNdkVersion64Bit true
#define kNdk_ABI_version Ndk_ABI_16_0_1
#ifndef kNdkBuildTargetOS
# define kNdkBuildTargetOS "win"
#endif
#endif
