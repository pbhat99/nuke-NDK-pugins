//------------------------------------------------------------------------------
// DDImage/SlrRayBundle.h
//
// Copyright (c) 2024 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef SLR_RAYBUNDLE_H
#define SLR_RAYBUNDLE_H

#include "ndk/geo/render/RayContext.h"

#include "DDImage/DDImage_API.h"

namespace slr
{
  class SlrShadingContext;

  //-----------------------------------------------------------------------------

  //! Predefined undefined/error/invalid RayContext returned by ray retrieval
  //! methods to identify an error condition such as an out of range ray request.
  DDImage_API const ndk::RayContext&  InvalidRay();

  //-----------------------------------------------------------------------------


  /*!
  */
  class DDImage_API SlrRayBundle
  {
  public:
    //! Is the bundle bound to valid data?
    bool     isValid() const { return (_sttx != nullptr); }

    //! Bundle index in the per-thread ray bundle list.
    int32_t  index() const;

    //! Starting index of bundle in the cache list.
    int32_t  startIndex() const { return _start; }

    //! Inclusive-end SlrShadingContext index in cache list. (ie i=startIndex(); i < endIndex())
    int32_t  endIndex() const { return (_start + _size); }

    //! Number of rays in bundle.
    int32_t  size() const { return _size; }

    //! Is \a rayIndex inside this bundle's range?
    bool     contains(int32_t rayIndex) const { return !(rayIndex < 0 || rayIndex >= _size); }

    //! Position of ray in shared ray storage list.
    int32_t  absoluteRayIndex(int32_t rayIndex) const { return _start + rayIndex; }

    //----------------------------------------------------------

    //! Index of the sample creating this ray bundle, usually the index of the primary, or camera ray
    int32_t  sampleIndex() const;

    //! Make this bundle be the active one.
    //! Same as calling SlrIntegratorContext::selectActiveRayBundle().
    void  makeActiveBundle() const;

    //! Type shared by all rays in bundle (camera, glossy, etc).
    ndk::RayContext::TypeMask  rayType() const { return _type; }

    //! Which ray in this bundle active, or -1 if this bundle is not the active one.
    int32_t  activeRay() const;

    //! Is the current active ray in this bundle? Same as calling contains(activeRay()).
    bool  hasActiveRay() const;

    //! Select the active intersection data cache to use during intersection testing and configure
    //! an empty bundle to be the active one and collect any intersection results.
    //!
    //! If this is not the active bundle it will be made the active one.
    //!
    //! InvalidRay() is returned if \a rayIndex is not within this rayBundle.
    const ndk::RayContext&  selectActiveRay(uint32_t rayIndex) const;

    //! If this bundle is the most recently created bundle 
    //! Add another RayContext to the bundle, returning a reference to it and incrementing
    //! the current bundle count. This method assumes the bundle currently 'owns' the
    //! RayContextList by a single thread and is always appending to the end of the list.
    ndk::RayContext&  addRay(const ndk::RayContext& rtx = ndk::RayContext()) const; // copy
    ndk::RayContext&  addRay(ndk::RayContext&& rtx) const; // move

          ndk::RayContext&  operator [] (int i);
    const ndk::RayContext&  operator [] (int i) const;

    //! Returns the first RayContext in the bundle. Caution - if this bundle is empty the returned
    //! RayContext may be from a previous bundle!
    ndk::RayContext&  front();
    //! Returns the last RayContext in the bundle. Caution - if this bundle is empty the returned
    //! RayContext may be from a previous bundle!
    ndk::RayContext&  back();

    ndk::RayContext*        array();
    const ndk::RayContext*  array() const;

    //! Iterator support
    typedef ndk::RayContextList::iterator       iterator;
    typedef ndk::RayContextList::const_iterator const_iterator;

    iterator        begin();
    const_iterator  begin() const;
    iterator        end();
    const_iterator  end() const;

    //----------------------------------------------------------

    friend DDImage_API std::ostream& operator << (std::ostream& o, const SlrRayBundle& b);


  private:
    friend class SlrThreadContext;
    class SlrThreadContext*   _sttx;      //!< Ray data owner
    ndk::RayContext::TypeMask _type;      //!< Type shared by all rays in bundle (camera, glossy, etc)
    class RayBundleData*      _data;      //!< Bundle data storage
    int32_t                   _start;     //!< Absolute starting index cache list
    int32_t                   _size;      //!< Absolute inclusive-end index (loop is index=start; index < end)

    SlrRayBundle() = default;
    SlrRayBundle(SlrThreadContext*, ndk::RayContext::TypeMask type, RayBundleData*, int32_t bundleIndex);
  };

  using SlrRayBundleList = std::vector<SlrRayBundle>;


} // namespace slr


#endif
