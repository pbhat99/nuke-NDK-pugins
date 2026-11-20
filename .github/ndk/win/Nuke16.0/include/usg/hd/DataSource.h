// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include <memory>
#include <vector>

#include "usg/api.h"
#include "usg/base/ArrayTypes.h"
#include "usg/base/Token.h"
#include "usg/base/Value.h"
#include "fdk/math/Vec3.h"

namespace usg::hd {

#define USG_HD_DECLARE_DATASOURCE_ABSTRACT(type)                                                   \
  using RefPtr = std::shared_ptr<type>;                                                            \
  static RefPtr Cast(const DataSource::RefPtr& v) { return std::dynamic_pointer_cast<type>(v); }

#define USG_HD_DECLARE_DATASOURCE(type)                                                            \
  USG_HD_DECLARE_DATASOURCE_ABSTRACT(type)                                                         \
  template <typename... Args> static RefPtr New(Args&&... args)                                    \
  {                                                                                                \
    auto refptr = RefPtr(new type(std::forward<Args>(args)...));                                   \
    refptr->allocateImplFromShared(refptr);                                                        \
    return refptr;                                                                                 \
  }

#define USG_HD_DECLARE_DATASOURCE_REFPTRS(type) using type##RefPtr = type::RefPtr;

  class USG_API DataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(DataSource)

    class Impl;         // Abstraction handle for implementation type
    class Handle;       // Abstraction handle for casting to HdDataSourceBase
    class HandleHandle; // Abstraction handle for casting to HdDataSourceBaseHandle

    virtual ~DataSource() = default;

    HandleHandle* getHdDataSourceBaseHandle();
    std::shared_ptr<Impl> impl();

  protected:
    std::shared_ptr<Impl> _pImpl;
    virtual void allocateImplFromShared(DataSource::RefPtr dsRefPtr);
    virtual Handle* newPxrDataSource(DataSource::RefPtr dsRefPtr) = 0;
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(DataSource)

  class USG_API ContainerDataSource : public DataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(ContainerDataSource)

    virtual DataSourceRefPtr get(const Token& name) = 0;
    virtual TokenArray getNames() = 0;

  protected:
    virtual Handle* newPxrDataSource(DataSource::RefPtr dsRefPtr) override;
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(ContainerDataSource)

  class USG_API SampledDataSource : public DataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(SampledDataSource)
    using Time = float;

    virtual Value getValue(Time shutterOffset) = 0;
    virtual bool getContributingSampleTimesForInterval(Time startTime, Time endTime,
                                                       std::vector<Time>* outSampleTimes) = 0;

  protected:
    virtual Handle* newPxrDataSource(DataSource::RefPtr dsRefPtr) override;
  };
  USG_HD_DECLARE_DATASOURCE_REFPTRS(SampledDataSource)

  template <typename T> class TypedSampledDataSource : public SampledDataSource
  {
  public:
    USG_HD_DECLARE_DATASOURCE_ABSTRACT(TypedSampledDataSource<T>)
    using Type = T;

    virtual T getTypedValue() = 0;
  };

  template <typename T> class RetainedTypedSampledDataSource : public TypedSampledDataSource<T>
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

    // Specialized in DataSource.cpp
    DataSource::Handle* newPxrDataSource(DataSourceRefPtr dsRefPtr) override;

  private:
    T m_value;
  };

  template <>
  USG_API DataSource::Handle*
  RetainedTypedSampledDataSource<Token>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSource::Handle*
  RetainedTypedSampledDataSource<bool>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSource::Handle*
  RetainedTypedSampledDataSource<Vec3fArray>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSource::Handle*
  RetainedTypedSampledDataSource<fdk::Mat4d>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSource::Handle*
  RetainedTypedSampledDataSource<IntArray>::newPxrDataSource(DataSourceRefPtr dsRefPtr);
  
  template <>
  USG_API DataSource::Handle*
  RetainedTypedSampledDataSource<FloatArray>::newPxrDataSource(DataSourceRefPtr dsRefPtr);

  template <>
  USG_API DataSource::Handle*
  RetainedTypedSampledDataSource<fdk::Vec3d>::newPxrDataSource(DataSourceRefPtr dsRefPtr);
  
  // clang-format off
  using BoolRetainedDataSource = RetainedTypedSampledDataSource<bool>;
  using BoolRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<bool>>;
  using IntArrayRetainedDataSource = RetainedTypedSampledDataSource<IntArray>;
  using IntArrayRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<IntArray>>;
  using FloatArrayRetainedDataSource = RetainedTypedSampledDataSource<FloatArray>;
  using FloatArrayRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<FloatArray>>;
  using Vec3fArrayRetainedDataSource = RetainedTypedSampledDataSource<Vec3fArray>;
  using Vec3fArrayRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<Vec3fArray>>;
  using MatrixRetainedDataSource = RetainedTypedSampledDataSource<fdk::Mat4d>;
  using MatrixRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<fdk::Mat4d>>;
  using TokenRetainedDataSource = RetainedTypedSampledDataSource<Token>;
  using TokenRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<Token>>;
  using Vec3dRetainedDataSource = RetainedTypedSampledDataSource<fdk::Vec3d>;
  using Vec3dRetainedDataSourceHandle = std::shared_ptr<RetainedTypedSampledDataSource<fdk::Vec3d>>;
  // clang-format on

} // namespace usg::hd
