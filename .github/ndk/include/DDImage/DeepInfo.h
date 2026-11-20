// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepInfo_H
#define DDImage_DeepInfo_H

#include "DDImage/Box.h"
#include "DDImage/ChannelSet.h"
#include "DDImage/Format.h"
#include "DDImage/GeneralInfo.h"
#include "DDImage/Info2D.h"
#include "DDImage/IopInfo.h"

namespace DD {
  namespace Image {

    /**
     * DeepInfo class.  Analogous to IopInfo.  Keeps just a box and a channelset,
     * and two formats.
     */
    class DDImage_API DeepInfo : public Info2D
    {
      DD::Image::ChannelSet _channels;

    public:

      /// default constructor
      DeepInfo() { }
      
      /// construct based on formats, box, and channels
      DeepInfo(DD::Image::FormatPair formats, DD::Image::Box box, DD::Image::ChannelSet channels)
        : _channels(channels)
      {
        setBox(box);
        setFormats(formats);
      }

      /// construct based on an existing IopInfo
      DeepInfo(const IopInfo& iopInfo)
        : Info2D(iopInfo)
        , _channels(iopInfo.channels())
        {
          setBox(iopInfo);
          setFormats(iopInfo.formats());
        }

      const Format* format() const
      {
        return &Info2D::format();
      }

      const Format* fullSizeFormat() const
      {
        return &Info2D::full_size_format();
      }

      /// get the channels
      ChannelSet channels() const
      {
        return _channels;
      }

      /// merge some other DeepInfo's boxes and channels into this one
      void merge(const DeepInfo& other);
    };
  
  }
}

#endif
