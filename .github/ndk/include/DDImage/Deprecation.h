#ifndef DDIMAGE_DEPRECATION_H
#define DDIMAGE_DEPRECATION_H

// The policy is to remove obsolete code in the next major version after it has
// been deprecated for at least a full major release cycle. As soon as the NDK
// is generated for a major release (i.e. the N.0v1 version is out), you should
// use the next major release's version (i.e. N+1) to mark any deprecated code.

// For example, as a Nuke developer, during the life cycle of Nuke 13.* (after
// 13.0v1), use mFnDeprecatedInNuke14 for any public API function. This will
// generate warnings on the standard output when one tries to build any code
// using these functions.

// The warnings are in place during the remainder of the Nuke 13 life cycle and
// during the whole life cycle of Nuke 14 to give enough time for customers to
// upgrade their code.

// The worst case scenario to justify this is that if we deprecate code in the
// last point release, this should not be deleted directly in the next major
// release. When we release Nuke 15.0v1 the code marked by the
// mFnDeprecatedInNuke14 macro should be deleted.

#if kDDImageVersionMajorNum <= 13
#define mFnDeprecatedInNuke13(msg) [[deprecated(msg)]]
#endif

#if kDDImageVersionMajorNum <= 17
#define mFnDeprecatedInNuke17(msg) [[deprecated(msg)]]
#endif

#if kDDImageVersionMajorNum <= 16
#  ifdef NUKE_DISABLE_DEPRECATIONS
#    define mFnDeprecatedInNuke16(msg)
#    define mFnDeprecatedDDImageHalfInNuke16(msg)
#    define mFnDeprecatedInNuke14(msg)
#    define mFnDeprecatedLegacy3DSystemInNuke14(msg)
#  else
#    define mFnDeprecatedInNuke16(msg) [[deprecated(msg)]]
#    define mFnDeprecatedInNuke14(msg) [[deprecated(msg)]]
#    ifdef NUKE_DISABLE_LEGACY3DSYSTEM_DEPRECATIONS
#      define mFnDeprecatedDDImageHalfInNuke16(msg)
#      define mFnDeprecatedLegacy3DSystemInNuke14(msg)
#    else
#      define mFnDeprecatedLegacy3DSystemInNuke14(msg) [[deprecated(msg)]]
#    endif
#    ifdef NUKE_DISABLE_DDIMAGEHALF_DEPRECATIONS
#      define mFnDeprecatedDDImageHalfInNuke16(msg)
#    else
#      define mFnDeprecatedDDImageHalfInNuke16(msg) [[deprecated(msg)]]
#    endif
#  endif
#endif

#endif
