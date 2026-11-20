// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

// This file describes the Foundry::Version object.
// Please include ddImageVersionNumbers.h for the library version number

#ifndef _fnVersion_h_
#define _fnVersion_h_

// FIXME BJN - MOVE THIS INTO A CORE COMPONENT FOR 5.1 and remove DD references

#include "DDImage_API.h"
#include <string>
#include <iostream>

// Turn this on to compile in the testing functions
//#define FOUNDRY_VERSION_TEST

namespace DD
{

  namespace Image
  {

    /// Class that wraps up version numbers and logic
    ///
    /// We have a hierarchy of three release numbers, these are...
    ///   - major - represents the product series, eg "5"
    ///   - minor - represents a specific release in the series, all
    ///             versions with the same minor number are compatible, eg "5.0"
    ///   - release - represents a specific release of a minor number typically
    ///             to fix bugs. eg "5.0.0"
    ///
    /// With this logic 5.0.0 and 5.0.1 are compatible versions that work the
    /// same way, but with 5.0.1 having bugs fixed and so on.
    ///
    /// To manage alpha/beta builds we also have a 'phase', which indicates
    /// whether the software is in development, alpha, beta or release. If the software
    /// is not in release, we have to have a fourth number indicating the
    /// alpha/beta number.
    ///
    /// The final number is the build number, which represents the current
    /// repository version.
    ///
    /// We also have logic to turn a version into and out of a string as
    /// well a comparison to see which release is greater than another.
    ///
    /// So,
    ///    - an alpha build will look like "5.0v3.000004a"
    ///    - an beta build will look like "5.0v3.000004b"
    ///    - a release build will look like "5.0v3"
    ///
    /// The string that represents the version number fits
    /// the following regex...
    /// (0..9)+"."(0..9)+"v"(0..9)+[(d|a|b|p)(0..9)+]
    ///
    /// NOTE: there is special case for version comparisons for dev builds.
    /// A dev build is considered not less than or greater than any other build for the same major and
    /// minor version, but not equal to either.  This is because dev versions can (in theory) proceed
    /// right through from v1a1 to full release of v99 and still not provide incompatibilities, and
    /// any incompatibility still wouldn't be detected by the current system of having dev builds at v0.
    /// e. g. 5.2v0d0 < 5.2v1 is false; 5.2v0d0 > 5.2v1 is false; 5.2v0d0 == 5.2v1 is false.
    ///       Still, 5.2v0d0 < 5.3v1 is true, etc.
    class DDImage_API Version
    {
    public:

      /// enum representing the phase of the version
      enum PhaseEnum {
        eDevelopment,
        eAlpha,
        eBeta,
        eRelease
      };

    protected:
      std::string  _version; ///< product version (11.0dev)
      unsigned int _major; ///< product major version
      unsigned int _minor; ///< product minor version
      unsigned int _release; ///< product release version
      PhaseEnum    _phase; ///< the phase the product is in
      unsigned int _phaseNumber; ///< if the phase is alpha or beta, what is number for that
      std::string  _buildDate; ///< the date the object's constructor was compiled on
      bool         _is64Bit;   ///< object was compiled as 64 bit
      unsigned int _buildNumber;

    public:

      /// default ctor, sets to the minimum version
      
      /// make an empty which defaults to 0.0v0a1
      ///
      /// NEVER OVERRIDE THE DATE. THIS IS THE MECHANISM WE
      /// USE TO BE SURE THE BUILD DATE IS THE DATE THE
      /// OBJECT WAS COMPILED, NOT THE DATE THE LIBRARY WAS
      /// COMPILED. (Yes I am shouting.)
      Version(const char* buildDate = __DATE__);

      /// make the version

      /// NEVER OVERRIDE THE DATE. THIS IS THE MECHANISM WE
      /// USE TO BE SURE THE BUILD DATE IS THE DATE THE
      /// OBJECT WAS COMPILED, NOT THE DATE THE LIBRARY WAS
      /// COMPILED. (Yes I am shouting.)
      Version(const char* version,
              unsigned int major,
              unsigned int minor,
              unsigned int release,
              PhaseEnum phase,
              unsigned int phaseNumber,
              bool is64bit,
              unsigned int buildNumber,
              const char* date = __DATE__);

      /// make it from a std::string. This breaks it down into fields.
      /// NOTE: must be an explicit std::string, as passing a const char* will use a different constructor.
      Version(const std::string & v, const char* date = __DATE__);

      /// make it from a std::string. This breaks it down into fields
      void parseFromString(const std::string& lrSource, const char* lpDate = __DATE__);

      /// get major number
      unsigned int majorNumber() const { return _major; }

      /// get  minor number
      unsigned int minorNumber() const { return _minor; }

      /// get  release number
      unsigned int releaseNumber() const { return _release; }

      /// get build version string
      std::string getVersionString() const { return _version;  }
      
      /// get build number (jenkins build number, or 0)
      unsigned int buildNumber() const { return _buildNumber; }

      /// get the phase
      PhaseEnum phase() const { return _phase; }

      /// is this a release
      bool isRelease() const { return _phase == eRelease; }

      /// get the phase as a (0- or 1-letter) string
      const char* phaseNameString() const;

      /// get the phase as a string
      static const char* phaseNameString(PhaseEnum phase);

      /// get phase number (the "build number"; this is zero for any final release)
      unsigned int phaseNumber() const { return _phaseNumber; }

      /// Returns the phase string with letter and number (e. g. "001234b"), or a blank string if none
      std::string phaseStringFull() const;

      /// get the build date, which returns the date passed into the ctor
      const std::string& buildDate(void) const { return _buildDate; }

      /// 64 bit bool flag
      bool is64Bit() const { return _is64Bit; }

      /// get the 32/64 bit flag as a string
      const char* getBitString() const;

      /// get the architecure string ("arm64" or "x86_64")
      std::string getArchitectureString() const;

      /// is the other version compatible with this version
      /// - ie: are the major & minor numbers the same
      bool isCompatible(const Version& v) const
      {
        return v.majorNumber() == majorNumber() && v.minorNumber() == minorNumber();
      }

      /// Return equivalent of string passed to constructor

      /// Return the string description of major, minor, release, and phase.
      /// Does not include date or bits
      std::string string() const;

      /// Returns the major and minor numbers in a double-style string
      /// - e. g. 5.1 for 5.1v*
      std::string majorMinorString() const;

      /// Returns the major, minor, and release numbers in a string
      /// - e. g. 5.1v2 for 5.1v2*
      std::string majorMinorReleaseString() const;

      /// Returns the release and phase identifiers in a string
      /// - e. g. v1.000007b for ?.?v1.000007b
      std::string releasePhaseString() const;

      /// Returns a string version of majorMinorReleasePhaseDouble().
      /// 5.1v2b3 returns "5.102000003".
      /// Deprecated.  Please avoid using double strings.
      std::string doubleString() const;

      /// Return an integer representation of the major, minor, release,
      /// and build. This number will increase for a "newer" version,
      /// and is the value of the kDDImageVersionInteger macro.
      /// 5.1v2.000003b returns 5102000003.
      /// Note however that 5.1v2 returns 51200, so direct integer comparisons
      /// of version numbers are not completely reliable.
      /// Deprecated.  Please use the class comparison methods instead.
      long long majorMinorReleasePhaseInteger() const;

      /// Returns a double version of majorMinorReleasePhaseInteger().
      /// 5.1v2.000003b returns 5.102000003.
      /// Deprecated.  Please use the class comparison methods instead.
      double majorMinorReleasePhaseDouble() const;

      /// Simple testing function - check by reading output
  #ifdef FOUNDRY_VERSION_TEST
      static void Test();
  #endif

      /// same as string(); for back-compatibility
      std::string majMinRelPhaseStr(void) const { return string(); }

    };

    /// put to a stream. Doesn't persist the date yet
    DDImage_API std::ostream& VersionStreamPut(std::ostream& s, const DD::Image::Version& v);

    /// get from a stream. The thing should be in the regex format above. Doesn't persist the date yet
    DDImage_API std::istream& VersionStreamGet(std::istream& s, DD::Image::Version& v);

    DDImage_API bool VersionLessThan(const Version& a, const Version& b);
    DDImage_API bool VersionCompare(const Version& a, const Version& b);

  } // namespace Image

} // namespace DD

/// comparison operator
inline bool operator < (const DD::Image::Version& a, const DD::Image::Version& b)
{
  return DD::Image::VersionLessThan(a, b);
}

/// comparison operator
inline bool operator > (const DD::Image::Version& a, const DD::Image::Version& b)
{
  return b < a;
}

/// comparison operator: are they exactly equal
inline bool operator == (const DD::Image::Version& a, const DD::Image::Version& b)
{
  return VersionCompare(a, b);
}

/// comparison operator: is 'a' greater than or exactly equal to 'b'
inline bool operator >= (const DD::Image::Version& a, const DD::Image::Version& b)
{
  return a > b || a == b;
}

/// comparison operator: is 'a' less than or exactly equal to 'b'
inline bool operator <= (const DD::Image::Version& a, const DD::Image::Version& b)
{
  return a < b || a == b;
}

/// put to a stream. Doesn't persist the date yet
inline std::ostream& operator << (std::ostream& s, const DD::Image::Version& v)
{
  return VersionStreamPut(s, v);
}

/// get from a stream. The thing should be in the regex format above. Doesn't persist the date yet
inline std::istream& operator >> (std::istream& s, DD::Image::Version& v)
{
  return VersionStreamGet(s, v);
}

#endif  // _fnVersion_h_
