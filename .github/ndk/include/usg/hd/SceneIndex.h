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
    ContainerDataSource::Handle dataSource;
  };

  class SceneIndex;
  using SceneIndexRefPtr = std::shared_ptr<SceneIndex>;
  class SceneIndexImpl;
  class SceneIndexWrapperHandle;

  using SceneIndexWrapperHandleFunc_dtor = void (*)(SceneIndexWrapperHandle*);
  using SceneIndexFunc_dtor = void (*)(SceneIndexImpl*);

  using SceneIndexWrapperHandlePtr =
    std::unique_ptr<SceneIndexWrapperHandle, SceneIndexWrapperHandleFunc_dtor>;

  // Scene Index Handle
  // We make use of a handle class to manage the lifetime of a scene index instance. 
  // A scene index instance should remain alive as long as the following holds true:
  // The USG scene index is being held by some client code (Nuke, Katana).
  // OR
  // The PXR scene index is being held by USD.
  // The purpose of this handle class is to manage the lifetime of handles held by client
  // code, while avoiding circular references. 
  class USG_API SceneIndexHandleBase
  {
  public:
    SceneIndexHandleBase();
    SceneIndexHandleBase(const SceneIndexRefPtr& sceneIndex,
                         SceneIndexWrapperHandle* wrapperHandlePtr,
                         void (*wrapperHandleDtor)(SceneIndexWrapperHandle*));
    explicit SceneIndexHandleBase(const SceneIndexHandleBase&);

    virtual ~SceneIndexHandleBase();

    SceneIndexHandleBase& operator=(const SceneIndexHandleBase&);

    bool isValid() const { return _sceneIndex && _wrapperHandle; }
    explicit operator bool() const { return isValid(); }

    const SceneIndexWrapperHandlePtr& wrapperHandle() const { return _wrapperHandle; }
    SceneIndexRefPtr sceneIndex() const { return _sceneIndex; }
    SceneIndex* operator->() const { return _sceneIndex.get(); }
    SceneIndex& operator*() const { return *_sceneIndex; }

  protected:
    SceneIndexRefPtr _sceneIndex;
    SceneIndexWrapperHandlePtr _wrapperHandle;
  };

  // A convenience class for interfacing with a handle of a particular scene index type.
  template <class SceneIndexT> class SceneIndexHandle : public SceneIndexHandleBase
  {
  public:
    SceneIndexHandle()
    : SceneIndexHandleBase()
    {}

    SceneIndexHandle(const SceneIndexRefPtr& sceneIndex, SceneIndexWrapperHandle* wrapperHandlePtr,
                     void (*wrapperHandleDtor)(SceneIndexWrapperHandle*))
    : SceneIndexHandleBase(sceneIndex, wrapperHandlePtr, wrapperHandleDtor)
    {}

    SceneIndexHandle(const SceneIndexHandleBase& other) : SceneIndexHandleBase(other)
    {
      if (!std::dynamic_pointer_cast<SceneIndexT>(_sceneIndex)) {
        _sceneIndex.reset();
        _wrapperHandle.release();
      }
    }

    explicit SceneIndexHandle(const SceneIndexHandle<SceneIndexT>& other)
    : SceneIndexHandleBase(other)
    {}

    virtual ~SceneIndexHandle() = default;
    SceneIndexHandle<SceneIndexT>& operator=(const SceneIndexHandleBase& rhs)
    {
      SceneIndexHandleBase::operator=(rhs);
      if (!std::dynamic_pointer_cast<SceneIndexT>(_sceneIndex)) {
        _sceneIndex.reset();
        _wrapperHandle.release();
      }
      return *this;
    }
    SceneIndexHandle<SceneIndexT>& operator=(const SceneIndexHandle<SceneIndexT>& rhs) = default;

    std::shared_ptr<SceneIndexT> sceneIndex() const
    {
      return std::dynamic_pointer_cast<SceneIndexT>(_sceneIndex);
    }
    SceneIndexT* operator->() { return std::dynamic_pointer_cast<SceneIndexT>(_sceneIndex).get(); }
    SceneIndexT& operator*() { return *std::dynamic_pointer_cast<SceneIndexT>(_sceneIndex); }
  };

  class USG_API SceneIndex
  {
  public:
    class Handle;

    // All creation of scene indices MUST go through this factory method. It ensures all 
    // required components are created and references setup correctly.
    template <class SceneIndexT, typename... ArgsT>
    static SceneIndexHandle<SceneIndexT> New(ArgsT... args)
    {
      auto sceneIndex = std::make_shared<SceneIndexT>(args...);
      const SceneIndexHandle<SceneIndexT> handle{ sceneIndex, sceneIndex->createWrapperHandle(),
                                                  sceneIndex->getWrapperHandleDtor() };
      sceneIndex->initImpl(handle);
      return handle;
    }

    virtual ~SceneIndex();
    SceneIndex(SceneIndex&) = delete;            // Not copyable.
    SceneIndex& operator=(SceneIndex&) = delete; // Not assignable.

    virtual SceneIndexPrim getPrim(const usg::Path& path) = 0;
    virtual PathArray getChildPrimPaths(const usg::Path& path) = 0;

    virtual SceneIndexWrapperHandle* createWrapperHandle();
    virtual SceneIndexWrapperHandleFunc_dtor getWrapperHandleDtor();

    void addObserver(const SceneIndexObserver* observer);

    void sendPrimsAdded(const SceneIndexObserver::AddedPrimEntries& entries);
    void sendPrimsRemoved(const SceneIndexObserver::RemovedPrimEntries& entries);
    void sendPrimsDirtied(const SceneIndexObserver::DirtiedPrimEntries& entries);

    SceneIndex::Handle* getHdSceneIndexBasePtr(int usd_version);

    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    SceneIndex();
    //! Returns the handle to the wrapped object.
    SceneIndexImpl* impl() { return _pImpl.get(); }
    const SceneIndexImpl* impl() const { return _pImpl.get(); }

  protected:
    std::unique_ptr<SceneIndexImpl, void (*)(SceneIndexImpl*)> _pImpl;

    SceneIndex(SceneIndexImpl*, void (*)(SceneIndexImpl*));
    SceneIndex(const SceneIndexImpl& impl);
    void initImpl(const SceneIndexHandleBase& handle);
  };

  // In cases where a USD scene index is returned, with no USG-derived subclass,
  // we'll get one of these, which forwards any calls to `getPrim` and `getChildPrimPaths`
  // to the paired USD scene index.
  // These should probably only be created by the shim.
  class USG_API RawSceneIndex final : public SceneIndex
  {
    friend class SceneIndexImpl;

  public:
    explicit RawSceneIndex(const SceneIndexImpl& impl)
    : SceneIndex(impl)
    {
    }
    SceneIndexPrim getPrim(const usg::Path& path) override;
    PathArray getChildPrimPaths(const usg::Path& path) override;
  };
  using RawSceneIndexRefPtr = std::shared_ptr<RawSceneIndex>;

} // namespace usg::hd
