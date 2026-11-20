// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include "usg/api.h"
#include "usg/hd/DataSource.h"

namespace usg::hd {

  class USG_API RetainedContainerDataSource : public ContainerDataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(RetainedContainerDataSource)

    static DataSourceHandle<ContainerDataSource> New();
    static DataSourceHandle<ContainerDataSource> New(const std::vector<usg::Token>& names,
                                         const std::vector<DataSourceHandleBase>& values);
    static DataSourceHandle<ContainerDataSource> New(const usg::Token& name1, const DataSourceHandleBase& value1);
    static DataSourceHandle<ContainerDataSource> New(const usg::Token& name1, const DataSourceHandleBase& value1,
                                         const usg::Token& name2, const DataSourceHandleBase& value2);
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(RetainedContainerDataSource)

}
