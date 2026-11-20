// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include <memory>
#include <vector>

#include "usg/api.h"
#include "usg/base/ArrayTypes.h"
#include "usg/base/Token.h"
#include "usg/base/Value.h"
#include "usg/geom/Path.h"
#include "fdk/math/Vec3.h"

namespace usg::hd {

#define USG_HD_DECLARE_DATASOURCE_ABSTRACT(type)                                                   \
  using RefPtr = std::shared_ptr<type>;                                                            \
  using Handle = usg::hd::DataSourceHandle<type>;                                                  \
  static Handle Cast(const usg::hd::DataSourceHandleBase& v) { return Handle{v}; }                 \
  static RefPtr _Cast(const usg::hd::DataSourceRefPtr& v) {                                        \
    return std::dynamic_pointer_cast<type>(v); }

#define USG_HD_DECLARE_DATASOURCE(type)                                                            \
  USG_HD_DECLARE_DATASOURCE_ABSTRACT(type)                                                         \
  template <typename... Args> static Handle New(Args&&... args)                                    \
  {                                                                                                \
    auto dataSource = RefPtr(new type(std::forward<Args>(args)...));                               \
    const Handle handle{                                                                           \
      dataSource, dataSource->newPxrDataSource(dataSource)};                                       \
    dataSource->initImpl(handle);                                                                  \
    return handle;                                                                                 \
  }

#define USG_HD_DECLARE_DATASOURCE_REFPTRS(type) using type##RefPtr = type::RefPtr;

// Ensure we have visible RTTI information for the templated data sources
#if !defined(_WIN32)
#define USD_HD_VISIBILITY_DEFAULT __attribute__ ((visibility("default")))
#else
#define USD_HD_VISIBILITY_DEFAULT
#endif

  class DataSourceWrapperHandle;
  class DataSource;
  class DataSourceImpl;
  using DataSourceRefPtr = std::shared_ptr<DataSource>;

  using DataSourceWrapperHandleFunc_dtor = void (*)(DataSourceWrapperHandle*);

  using DataSourceWrapperHandlePtr =
    std::unique_ptr<DataSourceWrapperHandle, DataSourceWrapperHandleFunc_dtor>;

  // DataSource Handle
  // We make use of a handle class to manage the lifetime of a data source instance. 
  // A data source instance should remain alive as long as the following holds true:
  // The USG data source is being held by some client code (Nuke, Katana).
  // OR
  // The PXR data source is being held by Hydra.
  // The purpose of this handle class is to manage the lifetime of handles held by client
  // code, while avoiding circular references.
  class USG_API DataSourceHandleBase
  {
  public:
    class PxrHandleHandle; // Abstraction handle for casting to HdDataSourceBaseHandle

    DataSourceHandleBase();
    DataSourceHandleBase(const DataSourceRefPtr& dataSource,
                         DataSourceWrapperHandle* wrapperHandlePtr);
    DataSourceHandleBase(const DataSourceHandleBase&);
    DataSourceHandleBase(DataSourceHandleBase&&) = default;

    virtual ~DataSourceHandleBase();

    DataSourceHandleBase& operator=(const DataSourceHandleBase&);

    bool isValid() const { return _dataSource && _wrapperHandle; }
    explicit operator bool() const { return isValid(); }

    DataSourceRefPtr dataSource() const { return _dataSource; }
    DataSource* operator->() const { return _dataSource.get(); }
    DataSource& operator*() const { return *_dataSource; }
    
    //------------------------------------------------------
    // For internal use only...
    //------------------------------------------------------

    /// @brief Internal. Get an opaque pointer to a Hydra handle for the data source. 
    ///        Pointer is valid _at least_ for the lifetime of this handle.
    /// @return A pointer to the PxrHandleHandle, or nullptr if not available.
    PxrHandleHandle* getHdDataSourceBaseHandle() const;

    /// @brief Internal. Get an opaque pointer to the Shim counterpart for this handle.
    /// @return A pointer to the wrapper handle, or nullptr if not available.
    const DataSourceWrapperHandlePtr& wrapperHandle() const { return _wrapperHandle; }

  protected:
    DataSourceRefPtr _dataSource;
    DataSourceWrapperHandlePtr _wrapperHandle;
  };

  // A convenience class for interfacing with a handle of a particular data source type.
  template <class DataSourceT> 
  class DataSourceHandle : public DataSourceHandleBase
  {
  public:
    using DataSourceHandleBase::DataSourceHandleBase;

    DataSourceHandle(const DataSourceHandleBase& other) : DataSourceHandleBase(other)
    {
      if (!std::dynamic_pointer_cast<DataSourceT>(_dataSource)) {
        _dataSource.reset();
        _wrapperHandle.release();
      }
    }

    DataSourceHandle(const DataSourceHandle<DataSourceT>& other)
    : DataSourceHandleBase(other)
    {}

    virtual ~DataSourceHandle() = default;

    DataSourceHandle<DataSourceT>& operator=(const DataSourceHandleBase& rhs)
    {
      DataSourceHandleBase::operator=(rhs);
      if (!std::dynamic_pointer_cast<DataSourceT>(_dataSource)) {
        _dataSource.reset();
        _wrapperHandle.release();
      }
      return *this;
    }
    DataSourceHandle<DataSourceT>& operator=(const DataSourceHandle<DataSourceT>& rhs) = default;

    std::shared_ptr<DataSourceT> dataSource() const
    {
      return std::dynamic_pointer_cast<DataSourceT>(_dataSource);
    }
    DataSourceT* operator->() const { return std::dynamic_pointer_cast<DataSourceT>(_dataSource).get(); }
    DataSourceT& operator*() const { return *std::dynamic_pointer_cast<DataSourceT>(_dataSource); }
  };

  class USG_API DataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(DataSource)

    using Impl = DataSourceImpl; // Abstraction handle for implementation type
    using PxrHandleHandle = DataSourceHandleBase::PxrHandleHandle;

    DataSource();
    virtual ~DataSource() = default;

    //------------------------------------------------------
    // For internal use only...
    //------------------------------------------------------
    std::shared_ptr<Impl> impl();

  protected:
    std::shared_ptr<Impl> _pImpl;
    void initImpl(const DataSourceHandleBase& handle);
    virtual DataSourceWrapperHandle* newPxrDataSource(DataSource::RefPtr dsRefPtr) = 0;
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(DataSource)

  class USG_API ContainerDataSource : public DataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(ContainerDataSource)

    virtual DataSourceHandleBase get(const Token& name) = 0;
    virtual TokenArray getNames() = 0;

  protected:
    virtual DataSourceWrapperHandle* newPxrDataSource(DataSource::RefPtr dsRefPtr) override;
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(ContainerDataSource)

  class USG_API VectorDataSource : public DataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(VectorDataSource)

    virtual DataSourceHandleBase getElement(size_t) = 0;
    virtual size_t getNumElements() = 0;

  protected:
    virtual DataSourceWrapperHandle* newPxrDataSource(DataSource::RefPtr dsRefPtr) override;
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(VectorDataSource)

  class USG_API SampledDataSource : public DataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(SampledDataSource)
    using Time = float;

    virtual Value getValue(Time shutterOffset) = 0;
    virtual bool getContributingSampleTimesForInterval(Time startTime, Time endTime,
                                                       std::vector<Time>* outSampleTimes) = 0;

  protected:
    virtual DataSourceWrapperHandle* newPxrDataSource(DataSource::RefPtr dsRefPtr) override;
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(SampledDataSource)

  template <typename T> class USD_HD_VISIBILITY_DEFAULT TypedSampledDataSource : public SampledDataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(TypedSampledDataSource<T>)
    using Type = T;

    virtual T getTypedValue() = 0;
  };


  class RetainedSampledDataSource : public SampledDataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE(RetainedSampledDataSource)
    explicit RetainedSampledDataSource(const Value& value)
    : m_value(value)
    {
    }

    Value getValue([[maybe_unused]] SampledDataSource::Time shutterOffset) override
    {
      return m_value;
    }

    bool getContributingSampleTimesForInterval(
      [[maybe_unused]] SampledDataSource::Time startTime,
      [[maybe_unused]] SampledDataSource::Time endTime,
      [[maybe_unused]] std::vector<SampledDataSource::Time>* outSampleTimes) override
    {
      return false;
    }

  private:
    Value m_value;
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(RetainedSampledDataSource)

  template <typename T> class USD_HD_VISIBILITY_DEFAULT RetainedTypedSampledDataSource : public TypedSampledDataSource<T>
  {
  public:
    USG_HD_DECLARE_DATASOURCE(RetainedTypedSampledDataSource<T>)
    explicit RetainedTypedSampledDataSource(const T& value)
    : m_value(value)
    {
    }

    Value getValue([[maybe_unused]] SampledDataSource::Time shutterOffset) override
    {
      return Value(m_value);
    }

    bool getContributingSampleTimesForInterval(
      [[maybe_unused]] SampledDataSource::Time startTime,
      [[maybe_unused]] SampledDataSource::Time endTime,
      [[maybe_unused]] std::vector<SampledDataSource::Time>* outSampleTimes) override
    {
      return false;
    }

    T getTypedValue() override { return m_value; }

  protected:
    // Specialized in DataSource.cpp
    DataSourceWrapperHandle* newPxrDataSource(DataSourceRefPtr dsRefPtr) override;

  private:
    T m_value;
  };

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<Token>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<bool>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<int>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<Vec2fArray>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<Vec3fArray>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<fdk::Mat4d>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<IntArray>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<float>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<FloatArray>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<fdk::Vec2f>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<fdk::Vec3d>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<Path>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<PathArray>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSourceWrapperHandle*
  RetainedTypedSampledDataSource<AssetPath>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  // clang-format off
  using BoolRetainedDataSource = RetainedTypedSampledDataSource<bool>;
  using BoolRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<bool>>;
  using IntRetainedDataSource = RetainedTypedSampledDataSource<int>;
  using IntRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<int>>;
  using IntArrayRetainedDataSource = RetainedTypedSampledDataSource<IntArray>;
  using IntArrayRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<IntArray>>;
  using FloatRetainedDataSource = RetainedTypedSampledDataSource<float>;
  using FloatRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<float>>;
  using FloatArrayRetainedDataSource = RetainedTypedSampledDataSource<FloatArray>;
  using FloatArrayRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<FloatArray>>;
  using Vec2fArrayRetainedDataSource = RetainedTypedSampledDataSource<Vec2fArray>;
  using Vec2fArrayRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<Vec2fArray>>;
  using Vec3fArrayRetainedDataSource = RetainedTypedSampledDataSource<Vec3fArray>;
  using Vec3fArrayRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<Vec3fArray>>;
  using MatrixRetainedDataSource = RetainedTypedSampledDataSource<fdk::Mat4d>;
  using MatrixRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<fdk::Mat4d>>;
  using TokenRetainedDataSource = RetainedTypedSampledDataSource<Token>;
  using TokenRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<Token>>;
  using Vec2fRetainedDataSource = RetainedTypedSampledDataSource<fdk::Vec2f>;
  using Vec2fRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<fdk::Vec2f>>;
  using Vec3dRetainedDataSource = RetainedTypedSampledDataSource<fdk::Vec3d>;
  using Vec3dRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<fdk::Vec3d>>;
  using PathRetainedDataSource = RetainedTypedSampledDataSource<Path>;
  using PathRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<Path>>;
  using PathArrayRetainedDataSource = RetainedTypedSampledDataSource<PathArray>;
  using PathArrayRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<PathArray>>;
  using AssetPathRetainedDataSource = RetainedTypedSampledDataSource<AssetPath>;
  using AssetPathRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<AssetPath>>;
  // clang-format on

} // namespace usg::hd
