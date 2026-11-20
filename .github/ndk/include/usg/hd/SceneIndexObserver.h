// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include <memory>
#include <string>

#include "usg/api.h"
#include "usg/geom/Path.h"
#include "usg/hd/DataSourceLocatorSet.h"

namespace usg::hd {

  class SceneIndex;

  class SceneIndexObserverImpl;
  class USG_API SceneIndexObserver
  {
  public:
    struct AddedPrimEntry {
      Path primPath;
      Token primType;

      AddedPrimEntry() {}
      AddedPrimEntry(const Path& primPath, const Token& primType)
      : primPath(primPath)
      , primType(primType)
      {
      }
    };
    using AddedPrimEntries = std::vector<AddedPrimEntry>;
    struct RemovedPrimEntry {
      Path primPath;

      RemovedPrimEntry() {}
      RemovedPrimEntry(const Path& primPath)
      : primPath(primPath)
      {
      }
    };
    using RemovedPrimEntries = std::vector<RemovedPrimEntry>;
    struct DirtiedPrimEntry {
      Path primPath;
      DataSourceLocatorSet dirtyLocators;

      DirtiedPrimEntry() {}
      DirtiedPrimEntry(const Path& primPath, const DataSourceLocatorSet& dirtyLocators)
      : primPath(primPath)
      , dirtyLocators(dirtyLocators)
      {
      }
    };
    using DirtiedPrimEntries = std::vector<DirtiedPrimEntry>;
    struct RenamedPrimEntry {
      Path oldPrimPath;
      Path newPrimPath;

      RenamedPrimEntry() = default;
      RenamedPrimEntry(const Path& oldPrimPath, const Path& newPrimPath)
      : oldPrimPath(oldPrimPath)
      , newPrimPath(newPrimPath)
      {
      }
    };
    using RenamedPrimEntries = std::vector<RenamedPrimEntry>;

    class Handle;

    SceneIndexObserver(SceneIndexObserver&) = delete; // Not copyable.
    SceneIndexObserver& operator=(SceneIndexObserver&) = delete; // Not assignable.
    virtual ~SceneIndexObserver();

    SceneIndexObserver::Handle* getHdSceneIndexObserverPtr(int usd_version);

    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    //! Returns the handle to the wrapped object.
    SceneIndexObserverImpl* impl() { return _pImpl.get(); }
    const SceneIndexObserverImpl* impl() const { return _pImpl.get(); }

    virtual void primsAdded(const SceneIndex& sender, const AddedPrimEntries& entries) = 0;
    virtual void primsRemoved(const SceneIndex& sender, const RemovedPrimEntries& entries) = 0;
    virtual void primsDirtied(const SceneIndex& sender, const DirtiedPrimEntries& entries) = 0;

  protected:
    SceneIndexObserver();
    std::unique_ptr<SceneIndexObserverImpl, void(*)(SceneIndexObserverImpl*)> _pImpl;
  };
  using SceneIndexObserverPtr = std::weak_ptr<SceneIndexObserver>;

}
