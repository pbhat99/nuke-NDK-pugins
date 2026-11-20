// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef VIEWSET_H
#define VIEWSET_H

// Disable Windows DLL interface warning
#ifdef FN_OS_WINDOWS
  #pragma warning( push )
  #pragma warning(disable:4251)
#endif

#include "DDImage/DDImage_API.h"
#include <set>
#include <stddef.h>

namespace DD
{
  namespace Image
  {

    class DDImage_API ViewSet
    {
      bool _all;
      std::set<int> _views;

    public:
      ViewSet() : _all(0), _views() {
      }

      ViewSet(int view) : _all(0), _views() {
        _views.insert(view);
      }

      ViewSet(bool all) : _all(all), _views() { }

      ViewSet(const std::set<int> &views) : _all(0), _views(views) {
      }

      bool all() const
      {
        return _all;
      }

      bool operator>(size_t i)
      {
        if (_all)
          return true;

        return _views.size() > i;
      }

      bool any() const
      {
        return _all || _views.begin() != _views.end();
      }

      operator std::set<int>() const;

      const std::set<int>* operator->() const
      {
        return &_views;
      }

      std::set<int>* operator->()
      {
        return &_views;
      }

      void operator |=(const ViewSet& other)
      { if (other._all)
          _all = true;

        for (std::set<int>::const_iterator i = other._views.begin(); i != other._views.end(); i++)
        {
          _views.insert(*i);
        }
      }

      void operator &=(const ViewSet& other)
      {
        if (other._all)
          return;

        if (_all) {
          _views = other._views;
          _all = other._all;
        }

        _all = false;
        std::set<int> newviews;

        for (std::set<int>::iterator i = _views.begin(); i != _views.end(); i++) {
          if (other._views.find(*i) != other._views.end())
            newviews.insert(*i);
        }
        _views = newviews;
      }
    };
  }
}

#ifdef FN_OS_WINDOWS 
  // pop the warning removal for disabling the Windows DLL interface warning C4251
  #pragma warning( pop ) 
#endif

#endif
