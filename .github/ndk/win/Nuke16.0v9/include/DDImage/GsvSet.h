// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifndef DDIMAGE_GSVSET_H
#define DDIMAGE_GSVSET_H

#include "fdk/base/Hash.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace DD {
  namespace Image {
    using GsvSet = std::map<std::string, std::string>;
    using GsvFlattenedSet = std::map<std::string, std::shared_ptr<std::string>>;
    using GsvFlattenedAllDataSet = std::map<std::string, std::vector<std::shared_ptr<std::string>>>;
    using GsvSets = std::map<std::string, GsvSet>;

    class GraphScope;
    using GraphScopePtr = std::shared_ptr<GraphScope>;
    using GraphScopeId = fdk::Hash;

    class GraphScopeRepository;
    using GraphScopeRepositoryPtr = std::shared_ptr<GraphScopeRepository>;
  }
}

#endif
