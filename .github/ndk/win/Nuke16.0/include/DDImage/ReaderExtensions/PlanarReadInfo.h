// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

//---------------------------------------------------------------------------------
//
// This file provides the definition and implementation of the DD::Image::PlanarReadInfo
// class, which is used by the read/decode interface to the Reader base class added
// by ReaderExtensions.h and ReaderExtensions.cpp.
//
// NOTE:
// Nuke itself makes no use of these extensions but it needs to compile with them
// so that Reader-derived plugins are compatible with Hiero. Similarly, thirdparty
// plugins built with these extensions included will be compatible with Hiero.
// Note that Reader-derived plugins do not need to provide their own implementation
// of the extensions API, even for use in Hiero, as Reader provides a default
// implementation in terms of its existing virtual engine() interface.
// 
//---------------------------------------------------------------------------------

#ifndef DDImage_ReaderExtensions_PlanarReadInfo_H
#define DDImage_ReaderExtensions_PlanarReadInfo_H

#include "DDImage/ReaderExtensions/GenericImagePlane.h"

namespace DD
{
  namespace Image
  {
    //! Information about a generic image plane, as opposed to a scan line, and how it can be read and decoded.
    class PlanarReadInfo
    {
    public:
      
      PlanarReadInfo(GenericImagePlaneDescriptor& desc, size_t readPassBufferSize, bool isDecodeThreadable, bool isValid)
      : _genericImagePlaneDescriptor(desc), _readPassBufferSize(readPassBufferSize), _isDecodeThreadable(isDecodeThreadable), _isValid(isValid)
      {}
      
      //! The default constructor creates an invalid PlanarReadInfo object. Since the interface is read-only the created object
      //! cannot be subsequently modified to become valid. This should be used in appropriate situations to indicate that no valid image
      //! plane is possible (e.g., if the user has, via planarReadInfo(), requested channels that are not present in the source data).
      PlanarReadInfo()
      : _genericImagePlaneDescriptor(ImagePlaneDescriptor(Box(0, 0, 0, 0), false, Mask_None, 0), DataInfo(eDataTypeNone), eColorCurveUnknown),
      _readPassBufferSize(0), _isDecodeThreadable(false), _isValid(false)
      {}
      
      const GenericImagePlaneDescriptor& genericImagePlaneDescriptor() const { mFnAssert(_isValid); return _genericImagePlaneDescriptor; }
      size_t readPassBufferSize() const { mFnAssert(_isValid); return _readPassBufferSize; }
      bool isDecodeThreadable() const { mFnAssert(_isValid); return _isDecodeThreadable; }
      bool isValid() const { return _isValid; }
      
      bool operator==(const PlanarReadInfo& rhs) const
      {
        return (_genericImagePlaneDescriptor == rhs._genericImagePlaneDescriptor) && (_readPassBufferSize == rhs._readPassBufferSize)
          && (_isDecodeThreadable == rhs._isDecodeThreadable) && (_isValid == rhs._isValid);
      }
      
      bool operator!=(const PlanarReadInfo& rhs) const
      {
        return !(operator==(rhs));
      }
      
    protected:
      GenericImagePlaneDescriptor _genericImagePlaneDescriptor; //!< Describes the image data format, size, etc.
      size_t _readPassBufferSize; //!< If decoupling read from decode, what is the size of the buffer to read into.
      bool _isDecodeThreadable;   //!< Can we run the decode in multiple threads?
      bool _isValid;
    };
    
  } // namespace Image
}   // namespace DD

#endif  // DDImage_ReaderExtensions_PlanarReadInfo_H

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.
