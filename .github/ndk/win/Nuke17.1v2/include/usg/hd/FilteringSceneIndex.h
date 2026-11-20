// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include <memory>

#include "usg/api.h"
#include "usg/hd/SceneIndex.h"
#include "usg/hd/SceneIndexObserver.h"

namespace usg::hd {
  class USG_API FilteringSceneIndex : public SceneIndex
  {
  public:
    FilteringSceneIndex();

    virtual std::vector<SceneIndexRefPtr> getInputScenes() const;

    SceneIndexWrapperHandle* createWrapperHandle() override;
    SceneIndexWrapperHandleFunc_dtor getWrapperHandleDtor() override;

  protected:
    explicit FilteringSceneIndex(const SceneIndexImpl& impl);
    explicit FilteringSceneIndex(const SceneIndexRefPtr& sceneIndex);
    FilteringSceneIndex(SceneIndexImpl* impl, void (*dtor)(SceneIndexImpl*))
    : SceneIndex(impl, dtor)
    {
    }
  };
  using FilteringSceneIndexRefPtr = std::shared_ptr<FilteringSceneIndex>;

  class USG_API SingleInputFilteringSceneIndex : public FilteringSceneIndex
  {
  public:
    SingleInputFilteringSceneIndex(const SceneIndexRefPtr& inputScene);

    virtual void _primsAdded(const SceneIndexRefPtr& sender,
                             const SceneIndexObserver::AddedPrimEntries& entries) = 0;

    virtual void _primsRemoved(const SceneIndexRefPtr& sender,
                               const SceneIndexObserver::RemovedPrimEntries& entries) = 0;

    virtual void _primsDirtied(const SceneIndexRefPtr& sender,
                               const SceneIndexObserver::DirtiedPrimEntries& entries) = 0;

    // Base implementation converts prim removed messages.
    virtual void _primsRenamed(const SceneIndexRefPtr& sender,
                               const SceneIndexObserver::RenamedPrimEntries& entries);

    std::vector<SceneIndexRefPtr> getInputScenes() const final;
    SceneIndexRefPtr _getInputSceneIndex() const;

    SceneIndexWrapperHandle* createWrapperHandle() override;
    SceneIndexWrapperHandleFunc_dtor getWrapperHandleDtor() override;

  protected:
    explicit SingleInputFilteringSceneIndex(const SceneIndexImpl& impl);

  private:
    SceneIndexRefPtr _inputScene;
  };
  using SingleInputFilteringSceneIndexRefPtr = std::shared_ptr<SingleInputFilteringSceneIndex>;
} // namespace usg::hd
