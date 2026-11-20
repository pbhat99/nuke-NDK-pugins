/** \file ndkVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kNdkVersionInteger which you
  can use in a statement like "#if kNdkVersionInteger >= 52100"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _NdkVersionNumber_h_
#define _NdkVersionNumber_h_
#define kNdkVersion "17.1"
#define kNdkVersionInteger (10000 * 17 + 1000 * 1 + 2)
#define kNdkVersionMajorNum 17
#define kNdkVersionMinorNum 1
#define kNdkVersionReleaseNum 2
#define kNdkVersionReleaseStr "v2"
#define kNdkVersionPhaseNum 3
#define kNdkVersion64Bit true
#define kNdk_ABI_version Ndk_ABI_17_1_1
#ifndef kNdkBuildTargetOS
# define kNdkBuildTargetOS "win"
#endif
#endif
