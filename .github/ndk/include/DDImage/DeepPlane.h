// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepPlane_h
#define DDImage_DeepPlane_h

#include "DDImage/ChannelSet.h"
#include "DDImage/DeepPixel.h"
#include "DDImage/Box.h"

#include <vector>

namespace DD {
  namespace Image {

    struct DeepPlaneData;

    /**
     * Deep image plane class.  Represents deep data on certain channels for a certain box.
     * The actual deep data is stored in a separate reference-counted structure, so that
     * copying is cheap.  This class is read-only.  If you wish to write to a DeepPlane,
     * use the DeepOutputPlane subclass.
     */
    class DDImage_API DeepPlane
    {
    protected:
      /** The channels this plane is for */
      DD::Image::ChannelMap _channels;
      
      /** The bounding box this plane is for */
      DD::Image::Box _box;
      
      /** The ordering in which the samples are stored within the pixels for this plane */
      DeepPixel::Ordering _ordering;
      
      /** Pointer to a reference-counted object that contains the actual memory allocations */
      DeepPlaneData* _sharedData;
      
      /**
       * Create an empty deep plane for a given set of channels and bounding box.  addPixel() or
       * addHole() must be called for each position in the box for the object to be valid for
       * returning from doDeepEngine().
       */
      DeepPlane(DD::Image::ChannelSet channels, DD::Image::Box box, DeepPixel::Ordering ordering = DeepPixel::eUnordered);

      /**
       * Get the actual number of pixels in this plane
       */
      size_t getPixelCount() const;

    public:

      /** Create an uninitialised plane. */
      DeepPlane();
            
      /**
       * Construct one deep plane based on another.  The data is shared.
       */
      DeepPlane(const DeepPlane&);
      
      /**
       * Copy one deep plane to another.  The data is shared.
       */
      void operator=(const DeepPlane& other);

      /** 
       * Destroy this plane.
       */
      ~DeepPlane();
      
      /**
       * Get the channels available on this plane
       */
      const ChannelMap& channels() const
      {
        return _channels;
      }
      
      /**
       * Get the bounding box available for this plane
       */
      const DD::Image::Box& box() const
      {
        return _box;
      }

      /**
       * Get the ordering of the samples for this plane
       */
      DeepPixel::Ordering ordering() const
      {
        return _ordering;
      }
      
      /*
       * The number of pixels in the bounding box (width * height)
       */
      int pixels() const
      {
        return _box.w() * _box.h();
      }
      
      /*
       * The offset of pixel (y, x) within the data
       */
      int positionToOffset(int y, int x) const
      {
        y -= _box.y();
        x -= _box.x();
        return y * _box.w() + x;
      }
      
      /*
       * get the DeepPixel at the given coordinates
       */
      DeepPixel getPixel(int y, int x) const;
      
      /*
       * get the DeepPixel at the given coordinates
       */
      DeepPixel getPixel(DD::Image::Box::iterator it) const
      {
        return getPixel(it.y, it.x);
      }

      /**
       * Returns true if this DeepPlane covers the given box and channels
       */
      bool covers(DD::Image::Box box, DD::Image::ChannelSet cs) const
      {
        DD::Image::Box newBox = _box;
        newBox.merge(box.x(), box.y());
        newBox.merge(box.r()-1, box.t() - 1);
        if (newBox != _box) {
          return false;
        }

        return _channels.containsAll(cs);
      }

      /**
       * Returns true if this DeepPlane covers a particular pixel
       */
      bool covers(int y, int x) const
      {
        return x >= _box.x() && x < _box.r() && y >= _box.y() && y < _box.t();
      }
      
      /**
       * return the total memory used for this plane
       */
      size_t memUsage() const;


      /**
      * return the number of samples at planes[x,y]
      */
      size_t getSampleCount(int y, int x) const;
      /**
      * return the number of samples at "it"
      */
      size_t getSampleCount(const DD::Image::Box::iterator& it) const
      {
        return getSampleCount(it.y, it.x);
      }

      /**
      * get total sample count
      */
      size_t getTotalSampleCount() const;
    };

    /**
     * Class used for construction of output pixels to be passed to DeepOutputPlane::addPixel()
     */
    class DDImage_API DeepOutPixel
    {
      std::vector<float> _data;
      int _idx;
      bool _var;
    public:

      /// initialise the output pixel for filling with data of a known size.
      DeepOutPixel(const size_t size) : _data(size)
      {
        _idx = 0;
        _var = false;
      }

      /// initialise the output pixel to a variable size
      DeepOutPixel()
      {
        _idx = 0;
        _var = true;
      }

      /// clear the output pixel of data.  calls to push_back() will insert 
      /// from the start again
      void clear()
      {
        _idx = 0;
        if (_var)
          _data.resize(0);
      }


      /// reserve m elements of data
      void reserve(size_t m)
      {
        if (_var)
          _data.reserve(m);
        else
          _data.resize(m);
      }

      /// reserve an additional m elements of data
      void reserveMore(size_t m)
      {
        _data.reserve(m + _data.size());
      }

      /// push the element /f/ to the end of the vector, or assign it to the
      /// next available position
      void push_back(float f)
      {
        if (_var) {
          _data.push_back(f);
        } else {
          _data[_idx] = f;
          _idx++;
        }
      }
      
      /// get a const pointer to the data
      const float* data() const
      {
        return _data.data();
      }

      /// return the number of elements that have been used
      size_t size() const
      {
        return _var ? _data.size() : _idx;
      }

      DeepPixel getPixel(const ChannelMap& channels, DeepPixel::Ordering ordering) const
      {
        return DeepPixel(channels, data(), size(), ordering);
      }
    };
    
    /**
     * DeepOutputPlane is a subclass of DeepPlane that allows the
     * pixel data to be altered.  It is used as the output parameter
     * for deepEngine.
     * 
     * Pattern of use should be: create a DeepOutputPlane with the
     * required channels, box and ordering, and then call addHole()
     * or addPixel() for each position within that box.  This results
     * in a valid plane that can then be accessed with the accessor
     * methods above.
     */
    class DDImage_API DeepOutputPlane : public DeepPlane
    {
    public:
      DeepOutputPlane() : DeepPlane()
      {
      }
      
      /**
       * Create a new output plane
       */
      DeepOutputPlane(DD::Image::ChannelSet channels,
                      DD::Image::Box box,
                      DeepPixel::Ordering ordering = DeepPixel::eUnordered)
        : DeepPlane(channels, box, ordering)
      {
      }

      /** add a 'hole' (ie zero samples) at the next pixel */
      void addHole();
      
      /** add data at the next pixel.  The parameter should consist of
         a number of floats equal to sampleCount * channelCount, and
         is arranged 
         
           sample 0, channel 0
           sample 0, channel 1
           sample 0, channel 2
           sample 1, channel 0
           sample 1, channel 1
           sample 1, channel 2

         addHole and addPixel start at the bottom-left of the box,
         and should be called left-to-right, bottom-to-top.  The box
         iterator returns the coordinates in the correct order for this.
       */
      void addPixel(const DeepOutPixel& f);

      /** add data at the next pixel, copying from the input pixel. */
      void addPixel(const DeepPixel& f);
    };
    
    /**
     * DeepInPlaceOutputPlane is a subclass of DeepOutputPlane that allows the
     * pixel data to be altered in place.
     * 
     * Pattern of use should be: create a DeepInPlaceOutputPlane with the
     * required channels, box and ordering, and then call reserveSamples() to
     * allocate storage for the samples then setSampleCount() and getPixel()
     * at each position within that box from top left to bottom right (see
     * Box::iterator).  This results in a valid plane that can then be accessed
     * with the accessor methods above.
     *
     * Overallocated planes may be trimmed to size using reviseSamples().
     * setSampleCount() will increase the allocation if required but doing so
     * is expensive.
     */
    class DDImage_API DeepInPlaceOutputPlane : public DeepOutputPlane
    {
    public:
      /**
       * Create a new output plane
       */
      DeepInPlaceOutputPlane(DD::Image::ChannelSet channels,
                      DD::Image::Box box,
                      DeepPixel::Ordering ordering = DeepPixel::eUnordered);


      /**
      reserve memory in DeepPlaneData for pixel sample data
      */
      void reserveSamples(size_t sampleCount);

      /**
      * set the newSampleCount amount of samples at pixel,
      * Initialises Data for direct access
      *
      * Note that sample counts should be set sequentially.
      * setSampleCount() invalidates all pixels after (see Box::iterator) this pixel.
      * After setting the sample count for a pixel _all_ subsequent pixels
      * are invalid.  Their sample count and pixel data must be reset.
      */
      void setSampleCount(int y, int x, size_t newSampleCount);

      /**
      * set the newSampleCount amount of samples at pixel 'it',
      * Initialises Data for direct access
      *
      * Note that sample counts should be set sequentially.
      * setSampleCount() invalidates all pixels after (see Box::iterator) this pixel.
      * After setting the sample count for a pixel _all_ subsequent pixels
      * are invalid.  Their sample count and pixel data must be reset.
      */
      void setSampleCount(DD::Image::Box::iterator it, size_t newSampleCount)
      {
        setSampleCount(it.y, it.x, newSampleCount);
      }

      /**
      * get a modifiable pixel, at 'it'
      *
      * Note that this data is only valid after the sample count has been set
      * using setSampleCount().
      */
      DeepOutputPixel getPixel(const DD::Image::Box::iterator& it)
      {
        return getPixel(it.y, it.x);
      }

      /**
      * get a modifiable pixel, at y,x
      *
      * Note that this data is only valid after the sample count has been set
      * using setSampleCount().
      */
      DeepOutputPixel getPixel(int y, int x);

      /**
      * Reduces overallocated data.
      */
      void reviseSamples();

      /**
      * Low overhead verification of a complete plane.
      */
      bool isComplete() const;

      /**
      * Error checking to debug invalid use.
      */
      bool isInError() const { return _inError; }

    private:
      /* Replace the DeepOutputPlane interface */
      void addHole();
      void addPixel(const DeepOutPixel& f);
      void addPixel(const DeepPixel& f);

      bool _inError;
      size_t  _validPixels;
     };
  }
}

#endif
