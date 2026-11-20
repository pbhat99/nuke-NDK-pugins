// Copyright (c) 2026 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_UTILS_FILEFORMATUTILS_H
#define NDK_GEO_UTILS_FILEFORMATUTILS_H

#include "ndk/api.h"

#include <string>

namespace ndk {

  /// Returns true if \p path refers to a locally-readable file whose header
  /// identifies it as an HDF5-based Alembic file, which is not supported.
  ///
  /// Returns false in all other cases:
  ///   - The path cannot be opened as a local file (e.g. URLs, asset identifiers).
  ///     The caller should allow USD's asset resolver to attempt resolution.
  ///   - The file is readable but does NOT carry an HDF5 magic header.
  ///
  /// When true is returned, \p message is populated with a human-readable
  /// explanation suitable for display to the user.
  NDK_API bool IsUnsupportedAlembicFile(const std::string& path, std::string& message);

}  // namespace ndk

#endif  // NDK_GEO_UTILS_FILEFORMATUTILS_H
