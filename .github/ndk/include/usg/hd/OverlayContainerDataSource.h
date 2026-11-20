// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include "usg/api.h"
#include "usg/hd/DataSource.h"

namespace usg::hd {

  //! A utility data source mirroring the functionality in HdOverlayContainerDataSource from
  //! pxr/imaging/hd/overlayContainerDataSource.h/cpp.
  class USG_API OverlayContainerDataSource : public ContainerDataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE(OverlayContainerDataSource)

    OverlayContainerDataSource(std::vector<DataSourceHandle<ContainerDataSource>> containers);
    OverlayContainerDataSource(const DataSourceHandle<ContainerDataSource>& src1,
                               const DataSourceHandle<ContainerDataSource>& src2);

    usg::TokenArray getNames() override;
    usg::hd::DataSourceHandleBase get(const usg::Token& name) override;

  private:
    std::vector<DataSourceHandle<ContainerDataSource>> _containers;
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(OverlayContainerDataSource)

}
