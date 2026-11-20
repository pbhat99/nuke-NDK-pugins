// BufferIop.h
// Copyright (c) 2017 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_BufferIop_h
#define DDImage_BufferIop_h

#include "DDImage/Iop.h"

namespace DD
{
  namespace Image
  {
    /// Base class for Iops which buffer the output of another Iop into an image plane.
    class DDImage_API BufferIop: public Iop
    {
    public:
      BufferIop();

      /// Get the name of this BufferIop, based on its Class and the name of the Op being buffered.
      std::string getName() const;

      /// Set the name of the Op being buffered. This is used to generate the name of this BufferIop.
      void setBufferedOpName(const std::string& opName) { _bufferedOpName = opName; }

      /// Get an ImagePlane for this BufferIop's data
      virtual DD::Image::ImagePlane getImagePlane(const DD::Image::ChannelSet& channels) = 0;

      /// get the address of the pixel componant at p = [x, y, z] for reading
      float* getReadableAddress(int x, int y, int z) const { return (float*)(_channelBaseAddress[z] +(std::int64_t)_rowStride *(std::int64_t)y +(std::int64_t)_pixelStride *(std::int64_t)x); }

      /// get the address of the pixel componant at p = [x, y, z] for writing
      float* getWritableAddress(int x, int y, int z) const { return (float*)(_channelWritableBaseAddress[z] +(std::int64_t)_rowWritableStride *(std::int64_t)y +(std::int64_t)_pixelWritableStride *(std::int64_t)x); }

      /// Helper function to return whether or not the iop passed in is a BufferIop.
      static bool IsBufferIop(const Iop& iop);

    protected:
      // These arrays hold the address of the bottom-left pixel of the buffered image data for each channel.
      char* _channelBaseAddress[Chan_Last +1];
      int _pixelStride;
      int _rowStride;

      char* _channelWritableBaseAddress[Chan_Last +1];
      int _pixelWritableStride;
      int _rowWritableStride;

    private:
      /// The name of the op being buffered.
      std::string _bufferedOpName;
    };

  } // DDImage
} // Foundry

#endif // DDImage_BufferIop_h
