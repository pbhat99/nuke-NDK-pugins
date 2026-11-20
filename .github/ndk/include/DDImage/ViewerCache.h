// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ViewerCache_H
#define DDImage_ViewerCache_H

#include "DDImage/DDImage_API.h"

#include "DDImage/AccountedVector.h"
#include "DDImage/Box.h"
#include "DDImage/Hash.h"
#include "Build/fnMacros.h"

#include <vector>

namespace DD {
  namespace Image {

    class ImageCacheReadI;
    class ChannelSet;

    //! Image cache for the viewer
    class DDImage_API ViewerCache
    {
      bool _frameRangeSet;
      int _firstFrame;
      int _lastFrame;

      static bool InternalBoxPresent(DD::Image::ImageCacheReadI* read, const DD::Image::Box& box, int downrez_y);

    public:
      enum StartType {
        START_PAUSED,
        START_NEVER,
        START_PLAYBACK,
        START_IF_NO_IMAGE,
        START_CONTEXT
      };
      
      //! Read into the memory cache (but don't try and actually load
      //! lines out of this file)
      void preload(DD::Image::Hash viewer_hash);

      /**
         Update the viewer from the cache file. Turn on image_done_ if all the
         visible data has been updated.
       */
      virtual void read_from_cache(DD::Image::Hash viewer_hash, StartType what);
      virtual void write_to_cache(DD::Image::Hash hash);
      
      static DD::Image::Hash MakeFileHash(DD::Image::Hash hash,
                                          int scale,
                                          DD::Image::ChannelSet channels);
      
      void deallocateAll();
      
      virtual ~ViewerCache() { }

      bool lineDone(int y) const
      {
        mFnAssert(y >= 0 && y < height);
        return !lineEnds.empty() && lineEnds[y].left <= roi.x() && lineEnds[y].right >= roi.r();
      }

      static void InitCache();      

      /**
       * Is the given box present in the memory cache for the viewer cache file identified by /hash/
       *
       * Only lines required for the passed-in downrez will be considered.
       */
      static bool BoxPresent(const DD::Image::Hash& hash, const DD::Image::Box& box, int downrez_y);

      const DD::Image::Box & get_roi() const { return roi; }
            
    protected:
      /*! One of these structures exists for each line of an image. */
      struct LineEnds
      {
        int left, right;
      };
      
      ViewerCache()
      {
        alloc_w = alloc_h = 0;
        width = height = 0;
        one_block = false;
        look_in_cache = true;
        viewer_downrez_y = 1;
        processing_downrez_y = 1;
        image_done_ = false;
        userRoiEnable = false;
        scale_ = 1;
        _frameRangeSet = false;
        _firstFrame = _lastFrame = 0;
      }
      
      void allocateLine(int y);
      void deallocateLine(int y);
      
      virtual double pixelAspect() const = 0;
      virtual DD::Image::Box bbox() const = 0;
      
      virtual void updateBBox(DD::Image::Box newBBox) { }
      virtual void readFromCacheDone(bool failed, bool allDone, bool skippedStuff) { }
      virtual void updateFrameRange(int first, int last) { }
      
      AccountedVector<U32*> lineBuffers;  //!< Array of line pointers
      AccountedVector<LineEnds> lineEnds; //!< Array of left/right ends
      
      int alloc_w, alloc_h;     //!< Size of image buffer
      
      int width, height;        //!< Size we are using
      bool one_block;           //!< Indicates a single allocation of line data
      
      DD::Image::Box roi;       //!< BBox of area to calculate
      int viewer_downrez_y;     //!< Viewer downrez factor (integer of 1/yzoom)
      int processing_downrez_y; //!< Processing downrez factor. Either the same as viewer_downrez_y or 1, if we're in full frame mode (processing all scanlines regardless of zoom level)
      bool userRoiEnable;       //!< True if user enabled roi
      DD::Image::Box userRoi;   //!< Roi selected by user
      
      bool image_done_;   //!< All visible lines are up to date
      bool look_in_cache; //!< False if we know there is nothing useful in cache file
      
      int scale_;   //!< Size of each channel of a pixel in image buffer

      void setFrameRange(int first, int last)
      {
        _frameRangeSet = true;
        _firstFrame = first;
        _lastFrame = last;
      }

      static int sAllocFudge;
    };
  }
}
    
#endif
