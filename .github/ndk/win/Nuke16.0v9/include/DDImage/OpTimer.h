// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_OpTimer_h
#define DDImage_OpTimer_h

#include "DDImage/DDImage_API.h"

namespace DD
{
  namespace Image
  {
    class Op;
    class Timer;

    /// Performance analysis helper.  Is created in Iop::get and similar functions to monitor
    /// runtime performance.
    class DDImage_API OpTimer 
    {
    public:
      enum Category {
        eStore,
        eValidate,
        eRequest,
        eEngine
      };
      static const int kNumCategories = 4;

      struct PerformanceInfo {
        long long _timeTakenCPU;
        long long _timeTakenWall;
        long long _callCount;
        void reset() { _timeTakenCPU = _timeTakenWall = _callCount = 0; }
        PerformanceInfo() : _timeTakenCPU(0), _timeTakenWall(0), _callCount(0) {}
      };

      OpTimer(DD::Image::Op* op, Category category);
      ~OpTimer();

    private:
      DD::Image::Op* _op;
      Timer* _timer;
      Category _category;
    };
  }
}


#endif
