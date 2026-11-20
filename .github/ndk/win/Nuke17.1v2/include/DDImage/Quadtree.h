// Quadtree.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Quadtree_h
#define DDImage_Quadtree_h

#include "DDImage/DDImage_API.h"
#include "DDImage/Vector2.h"
#include "DDImage/DDMath.h"
#include "DDImage/Allocators.h"

#include <vector>

namespace DD
{
  namespace Image
  {

    /*! THIS SHOULD BE A TEMPLATE OF BOX!!!!! */
    class DDImage_API Box2f
    {
      Vector2 min_; // Lower-left
      Vector2 max_; // Upper-right
    public:
      Box2f() : min_(0, 0), max_(-1, 0) {}
      Box2f(float x, float y, float r, float t) { min_.x = x;
                                                  max_.x = r;
                                                  min_.y = y;
                                                  max_.y = t; }
      Box2f(const Vector2 p) { min_ = p;
                               max_ = p; }
      Box2f(const Vector2 p1, const Vector2 p2) { min_ = p1;
                                                  max_ = p2; }

      const Vector2& min() const { return min_; }
      const Vector2& max() const { return max_; }
      float x() const { return min_.x; }
      void x(float v) { min_.x = v; }
      float y() const { return min_.y; }
      void y(float v) { min_.y = v; }
      float r() const { return max_.x; }
      void r(float v) { max_.x = v; }
      float t() const { return max_.y; }
      void t(float v) { max_.y = v; }
      float w() const { return max_.x - min_.x; }
      float h() const { return max_.y - min_.y; }
      Vector2 center() const { return (min_ + max_) / 2.0f; }

      void set(float x, float y, float r, float t) { min_.set(x, y);
                                                     max_.set(r, t); }
      void set(Vector2 p1, Vector2 p2) { min_ = p1;
                                         max_ = p2; }
      void set(const Box2f& v) { *this = v; }
      void set_min(float x, float y) { min_.set(x, y); }
      void set_max(float x, float y) { max_.set(x, y); }

      /*! Return true if \b b is inside this box. */
      bool contains(const Box2f& b) const
      { return inside(b.min()) && inside(b.max()); }
      /*! Return true if the point is inside this box. */
      bool inside(const Vector2& p) const
      { return p.x >= min_.x && p.x <= max_.x && p.y >= min_.y && p.y <= max_.y; }
      /*! Return true if the point is inside this box. */
      bool inside(float x, float y) const
      { return x >= min_.x && x <= max_.x && y >= min_.y && y <= max_.y; }
    };

    //----------------------------------------------------------------------

    /*! Quadtree node structure contains the actual data stored in
        the tree, and an array of four possible child nodes. */
    template <class T>
    struct DDImage_API QuadtreeNode
    {
      std::vector< T, STL3DAllocator<T> > data;      //!< The actual data stored in the tree
      QuadtreeNode* child_nodes[4]; //!< Four possible child subnodes

      /*! Constructor clears the child node pointers. */
      QuadtreeNode() { memset(child_nodes, 0, 4 * sizeof(QuadtreeNode*)); }
      /*! Destructor deletes all allocated child nodes. */
      ~QuadtreeNode() { for (int i = 0; i < 4; ++i)
                          delete child_nodes[i];}
      
      /* Required new/delete operator overload */
      inline void* operator new(size_t size);
      inline void  operator delete(void* mem);
      inline void* operator new[](size_t size);
      inline void  operator delete[](void* mem);
    };

    //----------------------------------------------------------------------

    /*! This template class provides spatial subdivision functionality of
        a 2D rectangular area and methods to add to or return its contents.
     */
    template <class T>
    class DDImage_API Quadtree
    {
    public:
      Quadtree(const Box2f &bbox, int max_depth = 16) :
        _bbox(bbox), _max_depth(max_depth) {}

      const Box2f& bbox() const { return _bbox; }
      int max_depth() const { return _max_depth; }
      const QuadtreeNode<T>& root() const { return _root; }

      /*! Add an object with its bounding box to all nodes in the tree that
         \b obj_bbox intersects, adding subdivisions until \b obj_bbox
         is larger than the subdivision size, or the maximum recursion level
         is reached.
       */
      void add(const T& object, const Box2f& obj_bbox, float min_D = 0.0f)
      {
        _add(&_root, _bbox, object, obj_bbox,
             MAX(obj_bbox.min().distanceSquared(obj_bbox.max()), min_D * min_D), 0);
      }

      /*! Add the object to the root with no further tests. */
      void add_to_root(const T& object) { _root.data.push_back(object); }

      /*! Find the last node in the tree that \b xy intersects, returning a
         pointer to the node's data vector. */
      const std::vector< T, STL3DAllocator<T> >* find(float x, float y) const
      { return _bbox.inside(x, y) ? _find(&_root, _bbox, x, y) : 0; }

      /*! Find all nodes in the tree that intersect \b bbox and add them to
         /b node_list. */
      int intersections(const Box2f&                         bbox,
                        std::vector<const QuadtreeNode<T>*>& node_list) const
      { return _intersections(&_root, _bbox, bbox, node_list); }

    protected:
      /*! Recursive version of add() that finds a node to add object to. */
      inline void _add(QuadtreeNode<T>* node, const Box2f& node_bbox,
                       const T& object, const Box2f& obj_bbox, float D, int depth);
      /*! Recursive version of find() that steps through the tree. */
      inline const std::vector< T , STL3DAllocator<T> >* _find(const QuadtreeNode<T>* node,
                                         const Box2f& node_bbox,
                                         float x, float y) const;
      /*! Recursive version of intersections() that steps through the tree. */
      inline int _intersections(const QuadtreeNode<T>*               node,
                                const Box2f&                         node_bbox,
                                const Box2f&                         bbox,
                                std::vector<const QuadtreeNode<T>*>& node_list) const;

    private:
      Box2f _bbox;       //!< The quadtree's 2D extent
      int _max_depth;    //!< Maximum recursion depth
      QuadtreeNode<T> _root; //!< Tree's start node
    };


    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
    // INLINE METHODS:

    /*! Recursive version of add() that finds a node to add object to.
     */
    template <class T>
    inline void Quadtree<T>::_add(QuadtreeNode<T>* node, const Box2f& node_bbox,
                                  const T& object, const Box2f& obj_bbox,
                                  float D, int depth)
    {
      // Check if we can add the object to the current node:
      if (depth >= _max_depth || node_bbox.min().distanceSquared(node_bbox.max()) < D) {
        if (node->data.size() && node->data.size() >= node->data.capacity())
          node->data.reserve(node->data.capacity() * 2);
        node->data.push_back(object);
        return;
      }

      // Check which child nodes the object bbox intersect:
      const Vector2 mid = node_bbox.center();
      bool over[4];
      over[0] = over[2] = (obj_bbox.x() <= mid.x);
      over[1] = over[3] = (obj_bbox.r() >  mid.x);
      over[0] &= (obj_bbox.y() <= mid.y);
      over[1] &= (obj_bbox.y() <= mid.y);
      over[2] &= (obj_bbox.t() >  mid.y);
      over[3] &= (obj_bbox.t() >  mid.y);

      // Add child nodes which have an intersection flag set:
      ++depth;
      Box2f cb;
      for (int i = 0; i < 4; ++i) {
        if (!over[i])
          continue;
        if (!node->child_nodes[i])
          node->child_nodes[i] = new QuadtreeNode<T>;
        // Bbox for child node:
        if (i & 1) {
          cb.x(mid.x);
          cb.r(node_bbox.r());
        }
        else {
          cb.x(node_bbox.x());
          cb.r(mid.x);
        }
        if (i & 2) {
          cb.y(mid.y);
          cb.t(node_bbox.t());
        }
        else {
          cb.y(node_bbox.y());
          cb.t(mid.y);
        }
        // Add the child node:
        _add(node->child_nodes[i], cb, object, obj_bbox, D, depth);
      }
    }

    /*! Recursive version of find() that steps through the tree.
     */
    template <class T>
    inline const std::vector< T, STL3DAllocator<T> >* Quadtree<T>::_find(const QuadtreeNode<T>* node,
                                                    const Box2f& node_bbox,
                                                    float x, float y) const
    {
      // Find the child node xy is inside:
      const Vector2 mid = node_bbox.center();
      int i = (x > mid.x ? 1 : 0) + (y > mid.y ? 2 : 0);
      // If no child node exist, we're done:
      if (!node->child_nodes[i])
        return &node->data;

      // Bbox for child node:
      Box2f cb;
      if (i & 1) {
        cb.x(mid.x);
        cb.r(node_bbox.r());
      }
      else {
        cb.x(node_bbox.x());
        cb.r(mid.x);
      }
      if (i & 2) {
        cb.y(mid.y);
        cb.t(node_bbox.t());
      }
      else {
        cb.y(node_bbox.y());
        cb.t(mid.y);
      }

      // Continue looking on the child node:
      return _find(node->child_nodes[i], cb, x, y);
    }

    /*! Recursive version of intersections() that steps through the tree.
     */
    template <class T>
    inline int Quadtree<T>::_intersections(const QuadtreeNode<T>*               node,
                                           const Box2f&                         node_bbox,
                                           const Box2f&                         bbox,
                                           std::vector<const QuadtreeNode<T>*>& node_list) const
    {
      // Add this node if it has objects:
      int count = int(node->data.size());
      if (count)
        node_list.push_back(node);

      // Check which child nodes the bbox intersect:
      const Vector2 mid = node_bbox.center();
      bool over[4];
      over[0] = over[2] = (bbox.x() <= mid.x);
      over[1] = over[3] = (bbox.r() >  mid.x);
      over[0] &= (bbox.y() <= mid.y);
      over[1] &= (bbox.y() <= mid.y);
      over[2] &= (bbox.t() >  mid.y);
      over[3] &= (bbox.t() >  mid.y);

      // Recurse into any child nodes which have intersections:
      Box2f cb;
      for (int i = 0; i < 4; ++i) {
        const QuadtreeNode<T>* child = node->child_nodes[i];
        if (!over[i] || !child)
          continue;
        // Bbox for child node:
        if (i & 1) {
          cb.x(mid.x);
          cb.r(node_bbox.r());
        }
        else {
          cb.x(node_bbox.x());
          cb.r(mid.x);
        }
        if (i & 2) {
          cb.y(mid.y);
          cb.t(node_bbox.t());
        }
        else {
          cb.y(node_bbox.y());
          cb.t(mid.y);
        }
        // Call the child node:
        count += _intersections(node->child_nodes[i], cb, bbox, node_list);
      }
      return count;
    }

    //----------------------------------------------------------------------
    //----------------------------------------------------------------------

    template <class T>
    inline void* QuadtreeNode<T>::operator new(size_t size)
    {
      return DD::Image::Allocators::g3DAllocator->allocate(size);
    }

    template <class T>
    inline void QuadtreeNode<T>::operator delete(void* mem)
    {
      return DD::Image::Allocators::g3DAllocator->deallocate(mem);
    }

    template <class T>
    inline void* QuadtreeNode<T>::operator new[](size_t size)
    {
      return DD::Image::Allocators::g3DAllocator->allocate(size);
    }

    template <class T>
    inline void QuadtreeNode<T>::operator delete[](void* mem)
    {
      return DD::Image::Allocators::g3DAllocator->deallocate(mem);
    }

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
