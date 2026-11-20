#ifndef IMAGEPLANE_DESCRIPTOR_H
#define IMAGEPLANE_DESCRIPTOR_H

#include "DDImage/DDImage_API.h"

#include "DDImage/Box.h"
#include "DDImage/ChannelSet.h"

#include <algorithm>

namespace DD {
  namespace Image {

    //! Class to describe an image plane.  Contains a bounding box, a set of channels,
    //! and whether or not the image plane is packed or not.
    class DDImage_API ImagePlaneDescriptor {
    protected:
      DD::Image::Box _bounds;
      bool _packed;
      ChannelSet _mask;
      int _nComps;

    public:
      ImagePlaneDescriptor(const DD::Image::Box& bounds,
                           bool packed,
                           DD::Image::ChannelSet mask,
                           int nComps) : _bounds(bounds), _packed(packed), _mask(mask), _nComps(nComps)
      {
      }

      DD::Image::Box bounds() const
      {
        return _bounds;
      }

      ChannelSet channels() const
      {
        return _mask;
      }

      bool packed() const
      {
        return _packed;
      }

      int nComps() const {
        return _nComps;
      }

      //! whether this image plane intersects in channels and in bounding box 
      //! with the other.
      bool intersects(const ImagePlaneDescriptor& other) const
      {
        if ((other._mask & _mask) == Mask_None)
          return false;

        DD::Image::Box intersection(std::max(_bounds.x(), other._bounds.x()),
                                    std::max(_bounds.y(), other._bounds.y()),
                                    std::min(_bounds.r(), other._bounds.r()),
                                    std::min(_bounds.t(), other._bounds.t()));
        
        if (intersection.r() <= intersection.x()) // no width
          return false;
        
        if (intersection.t() <= intersection.y()) // no height
          return false;

        return true;
      }

      bool intersects(int y, int t) const
      {
        int intY = std::max(_bounds.y(), y);
        int intT = std::min(_bounds.t(), t);
        
        if (intT <= intY)
          return false;

        return true;
      }

      bool satisfiedBy(const ImagePlaneDescriptor& candidate /*, PlaneMatchFlags flags*/)
      {
        //        if (!(flags & ePlaneMatchAllowAnyPacked))
        //          if (_packed != candidate._packed)
        //            return false;
        //
        //        if (!(flags & ePlaneMatchAllowExtraChannels))
        //          if (_mask != candidate._mask)
        //            return false;
        //
        //        if (!(flags & ePlaneMatchAllowExtraWidth))
        //          if (_bounds.x() != candidate._bounds.x() || _bounds.r() != candidate._bounds.r())
        //            return false;
        //
        //        if (!(flags & ePlaneMatchAllowExtraHeight))
        //          if (_bounds.y() != candidate._bounds.y() || _bounds.t() != candidate._bounds.t())
        //            return false;

        if (_bounds.x() < candidate._bounds.x() || _bounds.r() > candidate._bounds.r())
          return false;

        if (_bounds.y() < candidate._bounds.y() || _bounds.t() > candidate._bounds.t())
          return false;

        if ((_mask & candidate._mask) != _mask)
          return false;

        return true;
      }
    };

  }
}

#endif
