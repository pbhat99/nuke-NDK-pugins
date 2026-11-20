// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_BASE_STRINGTYPES_H
#define FDK_BASE_STRINGTYPES_H

#include "fdk/api.h"

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <iosfwd>
#include <string>

namespace fdk
{

  using KeyValueMapSorted = std::map<std::string, std::string>;
  using KeyValueMap       = std::unordered_map<std::string, std::string>;
  using KeyValuePtrMap    = std::unordered_map<std::string, const std::string*>;

  using StringList        = std::vector<std::string>;
  using StringPtrList     = std::vector<const std::string*>;
  using StringSet         = std::set<std::string>;

} // namespace fdk

#endif
