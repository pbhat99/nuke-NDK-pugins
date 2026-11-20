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
#  ifdef NUKE_DISABLE_DEPRECATIONS
#    define mFnDeprecatedInNuke17(msg)
#    define mFnDeprecatedInNuke16(msg)
#    define mFnDeprecatedDDImageHalfInNuke16(msg)
#    define mFnDeprecatedInNuke14(msg)
#    define mFnDeprecatedLegacy3DSystemInNuke14(msg)
#    define mFnDeprecatedStlAllocatorInNuke17(msg)
#  else
#    define mFnDeprecatedInNuke17(msg) [[deprecated(msg)]]
#    define mFnDeprecatedInNuke16(msg) [[deprecated(msg)]]
#    define mFnDeprecatedInNuke14(msg) [[deprecated(msg)]]
#    ifdef NUKE_DISABLE_LEGACY3DSYSTEM_DEPRECATIONS
#      define mFnDeprecatedDDImageHalfInNuke16(msg)
#      define mFnDeprecatedLegacy3DSystemInNuke14(msg)
#      define mFnDeprecatedStlAllocatorInNuke17(msg)
#    else
#      define mFnDeprecatedLegacy3DSystemInNuke14(msg) [[deprecated(msg)]]
#      define mFnDeprecatedStlAllocatorInNuke17(msg) [[deprecated(msg)]]
#    endif
#    ifdef NUKE_DISABLE_DDIMAGEHALF_DEPRECATIONS
#      define mFnDeprecatedDDImageHalfInNuke16(msg)
#    else
#      define mFnDeprecatedDDImageHalfInNuke16(msg) [[deprecated(msg)]]
#    endif
#  endif
#endif

/// Macros to suppress deprecated-declaration warnings around a block of code.
///
/// Usage:
/// \code
///   FN_SUPPRESS_DEPRECATED_WARNINGS_BEGIN
///   some_deprecated_function();
///   FN_SUPPRESS_DEPRECATED_WARNINGS_END
/// \endcode
#ifdef __clang__
#  define FN_SUPPRESS_DEPRECATED_WARNINGS_BEGIN \
     _Pragma("clang diagnostic push") \
     _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#  define FN_SUPPRESS_DEPRECATED_WARNINGS_END \
     _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#  define FN_SUPPRESS_DEPRECATED_WARNINGS_BEGIN \
     _Pragma("GCC diagnostic push") \
     _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#  define FN_SUPPRESS_DEPRECATED_WARNINGS_END \
     _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#  define FN_SUPPRESS_DEPRECATED_WARNINGS_BEGIN \
     __pragma(warning(push)) \
     __pragma(warning(disable: 4996))
#  define FN_SUPPRESS_DEPRECATED_WARNINGS_END \
     __pragma(warning(pop))
#else
#  define FN_SUPPRESS_DEPRECATED_WARNINGS_BEGIN
#  define FN_SUPPRESS_DEPRECATED_WARNINGS_END
#endif

#endif  // DDIMAGE_DEPRECATION_H
