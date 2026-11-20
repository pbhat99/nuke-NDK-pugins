// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include <memory>
#include <string>

#include "usg/api.h"
#include "usg/geom/Path.h"
#include "usg/hd/DataSource.h"
#include "usg/hd/SceneIndexObserver.h"

namespace usg::hd {

  struct USG_API SceneIndexPrim {
    Token primType;
    ContainerDataSourceRefPtr dataSource;
  };

  class SceneIndexInterface
  {
  public:
    virtual ~SceneIndexInterface() = default;

    virtual SceneIndexPrim getPrim(const usg::Path& primPath) = 0;
    virtual PathArray getChildPrimPaths(const usg::Path& primPath) = 0;
  };
  using SceneIndexInterfaceRefPtr = std::shared_ptr<SceneIndexInterface>;

  class SceneIndexImpl;
  class USG_API SceneIndex
  {
  public:
    class Handle;

    explicit SceneIndex(const SceneIndexInterfaceRefPtr& interface);
    virtual ~SceneIndex();
    SceneIndex(SceneIndex&) = delete;            // Not copyable.
    SceneIndex& operator=(SceneIndex&) = delete; // Not assignable.

    SceneIndexPrim getPrim(const usg::Path& path);
    PathArray getChildPrimPaths(const usg::Path& path);

    void addObserver(const SceneIndexObserver* observer);

    void sendPrimsAdded(const SceneIndexObserver::AddedPrimEntries& entries);
    void sendPrimsRemoved(const SceneIndexObserver::RemovedPrimEntries& entries);
    void sendPrimsDirtied(const SceneIndexObserver::DirtiedPrimEntries& entries);

    SceneIndex::Handle* getHdSceneIndexBaseRefPtr(int usd_version);

    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    SceneIndex(const SceneIndexImpl& impl);
    //! Returns the handle to the wrapped object.
    SceneIndexImpl* impl() { return _pImpl.get(); }
    const SceneIndexImpl* impl() const { return _pImpl.get(); }

  protected:
    std::unique_ptr<SceneIndexImpl, void (*)(SceneIndexImpl*)> _pImpl;
  };
  using SceneIndexRefPtr = std::shared_ptr<SceneIndex>;

} // namespace usg::hd
