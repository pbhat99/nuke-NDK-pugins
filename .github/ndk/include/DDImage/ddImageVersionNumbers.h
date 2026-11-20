/** \file ddImageVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kDDImageVersionInteger which you
  can use in a statement like "#if kDDImageVersionInteger >= 52100"
  to see if the version contains a given feature or bug fix.
*/
#ifndef _DDImageVersionNumber_h_
#define _DDImageVersionNumber_h_
#define kDDImageVersion "16.0"
#define kDDImageVersionInteger (10000 * 16 + 1000 * 0 + 9)
#define kDDImageVersionMajorNum 16
#define kDDImageVersionMinorNum 0
#define kDDImageVersionReleaseNum 9
#define kDDImageVersionReleaseStr "v9"
#define kDDImageVersionPhase DD::Image::Version::eRelease
#define kDDImageVersionPhaseNum 3
#define kDDImageVersion64Bit true
#define kDDImage_ABI_version DDImage_ABI_16_0_1
#ifndef kDDImageBuildTargetOS
# define kDDImageBuildTargetOS "win"
#endif
#endif // _DDImageVersionNumber_h_
