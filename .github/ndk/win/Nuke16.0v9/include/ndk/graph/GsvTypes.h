// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GRAPH_GSVTYPES_H
#define NDK_GRAPH_GSVTYPES_H

#include "fdk/base/Hash.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ndk {

  using GsvFlattenedSet = std::map<std::string, std::shared_ptr<std::string>>;
  using GsvFlattenedAllDataSet = std::map<std::string, std::vector<std::shared_ptr<std::string>>>;
  
  using GsvSet = std::map<std::string, std::string>;
  using GsvSets = std::map<std::string, GsvSet>;

  class GraphScope;
  using GraphScopePtr = std::shared_ptr<GraphScope>;
  using GraphScopeId = fdk::Hash;

  class GraphScopeRepository;
  using GraphScopeRepositoryPtr = std::shared_ptr<GraphScopeRepository>;

} // namespace ndk

#endif
