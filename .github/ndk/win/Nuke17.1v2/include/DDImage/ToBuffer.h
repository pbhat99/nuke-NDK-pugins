// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ToBuffer_H
#define DDImage_ToBuffer_H

#include "NoIop.h"

namespace DD
{
  namespace Image
  {

    class LUT;

    /*! \class DD::Image::ToBuffer

       \brief Takes the output of an image tree and put it into an 8-bit
       buffer.

       Hides all the uglyness of calling DDImage and of multithreading.

       Potential RLE compressed output types should derive from this class.
     */
    class DDImage_API ToBuffer : public NoIop
    {
    public:
      typedef enum {
        NONE = 0, YFLIP = 1, XFLIP = 2, R180 = 3, TRANSPOSE = 4,
        R270 = 5, R90 = 6, TRANSPOSE1 = 7
      } Transpose;

    public:
      ToBuffer(Node * node = nullptr) : NoIop(node) {
        lut_ = nullptr;
        tpose_ = NONE;
        stride_ = 0;
        depth_ = 0;
      }

      const char* Class() const override;
      const char* node_help() const override;

      //! Returns nullptr to intentionally break concatenation.
      Transform* getTransform() override;

      LUT* lut() { return lut_; } //!< Get the transfer function for output conversion
      int stride() { return stride_; } //!< Get the number of pixels to next row
      int depth() { return depth_; } //!< Get image depth, i.e. number of channels
      Transpose transpose() { return tpose_; } //!< Get display orientation.

      void lut(LUT* l) { lut_ = l; } //!< Set the transfer function for output conversion
      void transpose(Transpose tpose) { tpose_ = tpose; } //!< Set the number of pixels to next row
      void stride(const int v) { stride_ = v; } //!< Set image depth, i.e. number of channels
      void depth(const int v) { depth_ = v; } //!< Set display orientation.

      /*!
         The master to buffer function.  The others simply set the
         necessary output attributes, and call this method.  This fills
         the provided \a buffer with data from the region specified to this
         class.
       */
      virtual void to_buffer(unsigned char* buffer);

      /*! Version producing 16-bit colors*/
      virtual void to_buffer(U16* buffer);

      // Next 6 functions are mostly for backcompatibility.

      /*! Read the output and put it into a set of buffers.  By cleverly
         setting the buffer pointers and the delta and stride you should
         be able to produce any interlace pattern, mirror the image, or
         file  rotate by right angles. */
      virtual void to_buffer(unsigned char* buffer,
                             int x, int y, int w, int h, ChannelMask channels,
                             int delta = 1, int stride = 0);
      virtual void to_buffer(U16* buffer,
                             int x, int y, int w, int h, ChannelMask channels,
                             int delta = 1, int stride = 0);

      /*! Put the first \a depth channels of the image into an interlaced \a buffer
         in OpenGL format.  Pass a negative \a depth to flip the byte order
         to BGR or BGRA. */
      virtual void to_buffer(unsigned char* buffer,
                             int x, int y, int w, int h, int depth,
                             int stride = 0);
      virtual void to_buffer(U16* buffer,
                             int x, int y, int w, int h, int depth,
                             int stride = 0);

      /*! Put the region 0, 0, \a w, \a h into \a buffer with simple
         rotations and reflections.  Legal values for \a transpose are
         NONE=0, YFLIP=1, XFLIP=2, R180=3, TRANSPOSE=4 (0,0 position corner
         stays the same), R270=5, R90=6, TRANSPOSE1=7 (upper-right corner
         ends up at 0,0).  Pass a negative \a delta to flip the
         byte order to BGR. */
      virtual void to_buffer(unsigned char* buffer,
                             int w, int h, int depth,
                             Transpose = NONE, int stride = 0);
      virtual void to_buffer(U16 * buffer,
                             int w, int h, int depth,
                             Transpose = NONE, int stride = 0);

    protected:
      LUT* lut_; // should be pointing at a singleton LUT instance
      int  stride_;
      int  depth_;
      Transpose tpose_;
      void* buffers[64];
    };
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
