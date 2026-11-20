// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_DEPRECATION_H
#define NDK_DEPRECATION_H

// The policy is to remove obsolete code in the next major version after it has
// been deprecated for at least a full major release cycle. As soon as the NDK
// is generated for a major release (i.e. the N.0v1 version is out), you should
// use the next major release's version (i.e. N+1) to mark any deprecated code.

// For example, as a Nuke developer, during the life cycle of Nuke 17.* (after
// 17.0v1), use mFnDeprecatedInNuke18 for any public API function. This will
// generate warnings on the standard output when one tries to build any code
// using these functions.

// The warnings are in place during the remainder of the Nuke 17 life cycle and
// during the whole life cycle of Nuke 18 to give enough time for customers to
// upgrade their code.

// The worst case scenario to justify this is that if we deprecate code in the
// last point release, this should not be deleted directly in the next major
// release. When we release Nuke 19.0v1 the code marked by the
// mFnDeprecatedInNuke18 macro should be deleted.

#if kNdkVersionMajorNum <= 17
#  ifdef NUKE_DISABLE_DEPRECATIONS
#    ifndef mFnDeprecatedInNuke18
#      define mFnDeprecatedInNuke18(msg)
#    endif
#  else
#    ifndef mFnDeprecatedInNuke18
#      define mFnDeprecatedInNuke18(msg) [[deprecated(msg)]]
#    endif
#  endif
#endif

#endif
