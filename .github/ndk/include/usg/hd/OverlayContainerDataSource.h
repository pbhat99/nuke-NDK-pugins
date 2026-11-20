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

    OverlayContainerDataSource(size_t count, ContainerDataSourceRefPtr* containers);
    OverlayContainerDataSource(const ContainerDataSourceRefPtr& src1,
                               const ContainerDataSourceRefPtr& src2);

    usg::TokenArray getNames() override;
    usg::hd::DataSourceRefPtr get(const usg::Token& name) override;

  private:
    std::vector<usg::hd::ContainerDataSourceRefPtr> _containers;
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(OverlayContainerDataSource)

}
