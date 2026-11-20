// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef _DDImage_OrderedMap_h_
#define _DDImage_OrderedMap_h_

#include <algorithm>
#include <cassert>
#include <map>
#include <vector>
#include "Build/fnMacros.h"

namespace DD {
  namespace Image {


    //! A map-equivalent which records the insertion order of keys.
    //!
    //! Access to values is either through key lookup (via the get() method) or
    //! through position (via the getAt() method). The set() method will replace an
    //! existing item with the same key or, if that key isn't in use yet, will add
    //! a new item & record it as the last inserted item.
    //!
    //! This class deliberately avoids overloading the [] operator, because it
    //! could be confusing as to whether you're looking up by key or by index when
    //! your keys have an integral type.
    //!
    //! This implementation doesn't (yet!) provide any iterators. For now, if you
    //! want to iterate over the values, use a for loop and look up items by
    //! position. Sorry.
    template <typename Key, typename Value>
    class OrderedMap
    {
    public:
      typedef std::map<Key, Value> Map;
      typedef std::vector<Key> Order;

    public:
      OrderedMap() : _map(), _order() {
      }

      OrderedMap(const OrderedMap& that) : _map(that._map), _order(that._order) {
      }

      bool hasKey(const Key& key) const {
        return _map.find(key) != _map.end();
      }

      const Value& get(const Key& key) const {
        typename Map::const_iterator pos = _map.find(key);
        return (pos != _map.end()) ? pos->second : defaultVal();
      }

      Value& get(const Key& key) {
        typename Map::iterator pos = _map.find(key);
        mFnAssert(pos != _map.end());
        return pos->second;
      }

      void set(const Key& key, const Value& value) {
        if (!hasKey(key))
          _order.push_back(key);
        _map[key] = value;
      }

      size_t erase(const Key& key) {
        size_t numRemoved = _map.erase(key);
        if (numRemoved > 0)
          _order.erase(std::find(_order.begin(), _order.end(), key));
        return numRemoved;
      }

      Value getAt(size_t index) const {
        mFnAssert(index < size());
        return get(_order[index]);
      }

      Value& getAt(size_t index) {
        return get(_order[index]);
      }

      void setAt(size_t index, const Value& value) {
        mFnAssert(index < size());
        set(_order[index], value);
      }

      void eraseAt(size_t index) {
        mFnAssert(index < size());
        erase(_order[index]);
      }

      size_t size() const {
        return _map.size();
      }

      bool empty() const {
        return _map.empty();
      }

      void clear() {
        _map.clear();
        _order.clear();
      }

      size_t indexOf(const Key& key) const {
        typename Order::const_iterator pos = std::find(_order.begin(), _order.end(), key);
        if (pos != _order.end())
          return (size_t)(pos - _order.begin());
        mFnAssertMsg(false, "Given key doesn't exist in this OrderedMap");
        return static_cast<size_t>(-1);
      }

      const Key& getKeyAt(size_t index) const {
        mFnAssert(index < size());
        return _order[index];
      }

      bool setKeyAt(size_t index, Key newKey) {
        if (index >= _order.size())
          return false;

        Key oldKey = _order[index];
        if (oldKey == newKey)
          return true;
        else if (hasKey(newKey))
          return false;

        _map[newKey] = _map[oldKey];
        _map.erase(oldKey);
        _order[index] = newKey;
        return true;
      }

    protected:
      Map _map;
      Order _order;

    private:
      static const Value& defaultVal()
      {
        static const Value kDefaultVal = Value();
        return kDefaultVal;
      }
    };


  } // namespace Type
} // namespace Foundry

#endif // _DDImage_OrderedMap_h_
