#ifndef IMAGEPLANE_H
#define IMAGEPLANE_H

#include "DDImage/Box.h"
#include "DDImage/ChannelSet.h"
#include "DDImage/RefCountedFloatBuffer.h"
#include "DDImage/ImagePlaneDescriptor.h"
#include "DDImage/Thread.h"
#include "DDImage/Deprecation.h"
#include "DDImage/PtrStridedIterator.h"
#include "Build/fnBuild.h"

#include <cstdint>

namespace RIP {
  namespace Image {
    class Image;
  }
}

namespace DD {
  namespace Image {

    class Row;

    using ImageTilePtr         = PtrStridedIterator<float>;
    using ImageTileReadOnlyPtr = PtrStridedIterator<const float>;

    /**
     * Class that represents an ImagePlane.  ImagePlanes can be of any size, have any set
     * of channels.
     *
     * The data is a reference-counted float buffer consisting of contiguous pixel data
     * for the defined bounding box and channels.
     *
     * This is a relatively low-level class intended for code that interfaces with
     * external code and has to operate in a particular.  More complex mechanisms
     * can be built upon this.
     */
    class DDImage_API ImagePlane : public ImagePlaneDescriptor
    {
      ReadWriteLock _lock;

    protected:
      RefCountedFloatBuffer _data;

    public:
      /// construct based upon bounds, packedness, mask and number of compenents
      ImagePlane(const DD::Image::Box& bounds, bool packed, DD::Image::ChannelSet mask, int nComps) :
        ImagePlaneDescriptor(bounds, packed, mask, nComps)
      {
      }

      /// construct based upon bounds, packedness, mask and number of components
      ImagePlane(const DD::Image::Box& bounds, bool packed, DD::Image::ChannelSet mask) :
        ImagePlaneDescriptor(bounds, packed, mask, mask.size())
      {
      }

      /// copy construct
      ImagePlane(const ImagePlane& ip) :
        ImagePlaneDescriptor(ip)
      {
        _data = ip._data;
      }

      /// Construct based upon BBox.
      ImagePlane(const DD::Image::Box& bounds) : ImagePlaneDescriptor(bounds, true, Mask_None, 0)
      {
      }

      /// empty constructor
      ImagePlane() : ImagePlaneDescriptor(Box(), true, Mask_None, 0)
      {
      }

      ImagePlane& operator=(const ImagePlane& ip)
      {
        ImagePlaneDescriptor::operator=(ip);
        _data = ip._data;
        return *this;
      }


      /// Allocate data if needed and return a non-const pointer to it.  The pointer
      /// is to element (x, y, z), where x = bounds().x(), y = bounds().y() and
      /// z is the lowest Channel in channels()
      float* writable()
      {
        makeWritable();
        return _data;
      }

      /// return a non-const pointer to the data.  The pointer is to element (x, y, z),
      /// where x = bounds().x(), y = bounds().y() and z is the lowest Channel in channels()
      const float* readable() const
      {
        return _data;
      }

      /// allocate data if needed. This locks the image plane to make sure only one thread
      /// will try to allocate the data buffer. Optionally align memory (also rounds up allocated amount to multiple of alignment)
      void makeWritable(size_t alignment = 0)
      {
        if (!_data) {

          WriteGuard guard(_lock);

          if (!_data) {
            _data = RefCountedFloatBuffer::allocate(_bounds, _nComps, rowStride(), colStride(), chanStride(), alignment);
          }

        }
      }

      /// make the buffer unique - copy the underlying data if this has more than one reference
      void makeUnique();

      /// copy the intersection in channels and bbox from the other plane to this one
      /// in the most efficient way possible.  Note that this may involve a pointer copy
      /// rather than a memory copy if the intersection is a subset of the other plane (or
      /// identical to it); unless forceDeep is set to true, in which case it will always
      /// do a deep copy.
      ///
      /// Allocates memory if needed, is (and preferred) valid to call it without having
      /// called writable().
      void copyIntersectionFrom(const ImagePlane& other, bool forceDeep = false);

      /// find out the index within this plane for the given Channel.  E.g. a Mask_RGBA plane
      /// will have Chan_Red = 0, Chan_Green = 1, Chan_Blue = 2, Chan_Alpha = 3, but a
      /// Mask_Green | Mask_Alpha plane will have Chan_Green = 0 and Chan_Alpha = 1.
      ///
      /// Returns -1 if the channel is not present.  This is the value that is passed as the
      /// last parameter of at() and would be multiplied by the chanStride().
      int chanNo(Channel z) const
      {
        int idx = 0;
        for (Channel chan = _mask.first();
             chan != Chan_Black;
             chan = _mask.next(chan))
        {
          if (chan == z)
            return idx;
          idx++;
        }

        return -1;
      }

      /// get a reference to the data at (x, y) with the channel number /z/.
      const float& at(int x, int y, int z) const
      {
        y -= _bounds.y();
        x -= _bounds.x();

        return _data[y * static_cast<int64_t>(rowStride()) + x * static_cast<int64_t>(colStride()) + z * chanStride()];
      }

      const float& at(int x, int y, Channel z)
      {
        return at(x, y, chanNo(z));
      }

      /// get a writable reference to the data at (x, y) with the channel
      /// number /z/.  you need to have called makeWritable() or similar first.
      float& writableAt(int x, int y, int z)
      {
        y -= _bounds.y();
        x -= _bounds.x();

        const int64_t offset = y * static_cast<int64_t>(rowStride()) + x * static_cast<int64_t>(colStride()) + z * chanStride();
        return _data[offset];
      }

      /// get a writable ImageTilePtr pointing at the data at (0, y) with the channel
      /// number /z/.  Like Row::operator[] and Row::writable() you must always have
      /// add the horizontal offset before deferencing this pointer.
      ///
      /// you need to have called makeWritable() or similar first.
      ImageTilePtr writableAt(int y, int z)
      {
        float* ptr = &writableAt(0, y, z);
        return ImageTilePtr(ptr, colStride());
      }

      ImageTileReadOnlyPtr readableAt(int y, int z) const
      {
        return ImageTileReadOnlyPtr(&at(0, y, z), colStride());
      }

      /// get the stride between columns.  Is the number of channels for packed planes.
      /// For unpacked planes is 1.
      int colStride() const {
        if (packed())
          return _nComps;

        return 1;
      }

      /// return the stride between rows.  Is the number of channels times the width
      /// for packed planes.  For unpacked planes is the width.
      int rowStride() const {
        if (packed())
          return _nComps * _bounds.w();

        return _bounds.w();
      }

      /// return the stride between channels.  Is 1 for packed planes.  For unpacked
      /// planes is the width times the height.
      int64_t chanStride() const {
        if (packed())
          return 1;

        return static_cast<int64_t>(_bounds.h()) * static_cast<int64_t>(_bounds.w());
      }

      size_t usage() const
      {
        return _data.usage();
      }

      void clear()
      {
        _data = RefCountedFloatBuffer();
      }

      const RefCountedFloatBuffer & memory() const
      {
        return _data;
      }

      void setMemory(RefCountedFloatBuffer& memory)
      {
        _data = memory;
      }

      /// fill a channel with /value/
      void fillChannel(Channel z, float value);

      /// fill a channel with /value/ (using multiple threads).
      /// /numThreads/ specifies the number of threads to use. Setting /numThreads/
      /// to 0 lets the function decide on the number of threads to use.
      void fillChannelThreaded(Channel z, float value, unsigned int numThreads = 0);

      /// copy channel /src/ into /dst/
      void copyChannel(Channel dst, Channel src);

      /// get a row into the DD scanline
      void getRow(int y, int x, int r, DD::Image::ChannelMask m, DD::Image::Row& row, bool customChannels);

    protected:

      RefCountedFloatBuffer buffer(int x, int y) const
      {
        y -= _bounds.y();
        x -= _bounds.x();

        return _data + ptrdiff_t(y * static_cast<int64_t>(rowStride()) + x * static_cast<int64_t>(colStride()));
      }

    private:

      void fillChannelStripe(Channel z, int yMin, int yMax, float value);

      static void fillChannelThreadedThreadFunction(unsigned int threadIndex, unsigned int nThreads, void* data);
    };


  }
}

#endif
