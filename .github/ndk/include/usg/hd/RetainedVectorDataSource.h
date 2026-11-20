// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include "usg/api.h"
#include "usg/hd/DataSource.h"

namespace usg::hd {

  class USG_API RetainedVectorDataSource : public VectorDataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(RetainedVectorDataSource)

    static VectorDataSource::Handle New();
    static VectorDataSource::Handle New(const std::vector<DataSourceHandleBase>& values);
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(RetainedVectorDataSource)

}
