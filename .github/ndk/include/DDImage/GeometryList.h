// GeometryList.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_GeometryList_h
#define DDImage_GeometryList_h

#include "DDImage/GeoInfo.h"
#include "DDImage/Knob.h"
#include "DDImage/Hash.h"
#include <vector>
#include <stack>

namespace DD
{
  namespace Image
  {

    /*! A list of GeoInfos with state flags and out-of-dateness hashes.
     */
    class DDImage_API GeometryList
    {
      friend class Scene;
      friend class GeoOp;
    public:

      /*! Constructor.  Initializes the geometry hashes.
       */
      GeometryList();

      /*! Current object count. */
      unsigned objects() const { return current_range.objects; }
      /*! Absolute starting index in output list. */
      unsigned offset() const { return current_range.offset; }

      /*! The GeoOp that is currently manipulating this cache. */
      GeoOp* geo() const { return geo_; }

      /*! Object count.  Duplicate call to object() for convenience. */
      unsigned size() const { return objects(); }

      /* Writable Object Methods: */

      /*! Add a GeoInfo to the cache.
       */
      void add_object(int obj);

      /*! Delete all allocated geometry and clear the cache and output list
          of objects.
       */
      void delete_objects();

      /*! Synchronize the local cache object list with the current object list.
          Calling this repeatedly is cheap as it returns quickly if
          the cache's build_hash already matches the geo's build hash.
       */
      void synchronize_objects();

      /* Writable Primitive Methods: */

      /*! Create a local primitive allocation and append a primitive to it if
          \b p is non-NULL.  Returns the current size of the list.
       */
      int add_primitive(int obj, Primitive* p = nullptr);

      /*! Return a local writable primitive by index.
       */
      Primitive* writable_primitive(int obj, int prim);

      /* Writable Points Methods: */
      PointList* writable_points(int obj);

      /* Writable Attribute Methods: */
      Attribute* writable_attribute(int obj, GroupType group, const char* name, AttribType type);
      Attribute* writable_attribute(int obj, const AttribContext* attrib);
      AttribContext* writable_attribcontext(int obj, GroupType group, const char* name, AttribType type);
      AttribContext* writable_attribcontext(int obj, const AttribContext* attrib);
      
      void delete_attribcontext(int obj, GroupType group, const char* name, AttribType type);

      /* Modify Recursion Masks: */
      void set_recursion_mask(int obj, unsigned mask);
      void clear_recursion_mask(int obj, unsigned mask);

      /*! Returns GeoInfo \b n offset into the current stream. */
      const GeoInfo& operator[] (const size_t i) const { return list[i + current_range.offset]; }
      /*! Returns GeoInfo \b n offset into the current stream. */
      GeoInfo& operator[] (const size_t i) { return list[i + current_range.offset]; }
      /*! Returns GeoInfo \b n offset into the current stream. */
      GeoInfo& object(unsigned i) { return list[i + current_range.offset]; }
      /*! Returns the first GeoInfo in the current stream. */
      GeoInfo& object0() { return list[current_range.offset]; }

      /*! What groups need updating. */
      const GeometryMask& rebuild_mask() const { return rebuild_mask_; }
      bool rebuild(const GeometryMask& m = GeometryMask(Mask_All_Geometry)) const
      { return (rebuild_mask_ & m) != 0; }
      void set_rebuild(const GeometryMask& m) { rebuild_mask_ |= m; }

      /*! Geometry group out-of-dateness hashes. */
      const Hash& hash(int i) const { return geo_hash[i]; }

      /*! Compact the geometry data removing unused vertices and remapping all face indices */
      void compact(int obj);

      void push_range();
      void set_range(unsigned offset, unsigned objects);
      bool pop_range();

    private:
      void set_owner(GeoOp* geo) { geo_ = geo; }

      void insert(unsigned i = 0);
      void append();
      void clear_all();

    private:
      /*! The GeoOp that is currently manipulating this list. */
      GeoOp* geo_;
      
      /*! The list of GeoInfos. */
      std::vector<GeoInfo> list;

      struct DDImage_API ObjectRange
      {
        unsigned objects; //!< Object count
        unsigned offset; //!< Object starting index in \b list
      }
      current_range;
      std::stack<ObjectRange> range_stack;

      /*! Geometry group out-of-dateness hashes. */
      Hash geo_hash[Group_Last];

      /*! What groups need updating. */
      GeometryMask rebuild_mask_;

      /*! Current build pass, unique for each new scene being built. */
      int build_version;
    };


    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
    // INLINE METHODS:

    inline void GeometryList::push_range()
    {
      range_stack.push(current_range);
    }

    inline void GeometryList::set_range(unsigned offset, unsigned objects)
    {
      current_range.offset = offset;
      current_range.objects = objects;
    }

    inline bool GeometryList::pop_range()
    {
      if (range_stack.empty())
        return false;
      current_range = range_stack.top();
      range_stack.pop();
      return true;
    }

    //----------------------------------------------------------------------
    //----------------------------------------------------------------------

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
