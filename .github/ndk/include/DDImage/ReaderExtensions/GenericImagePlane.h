// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

//---------------------------------------------------------------------------------
//
// This file provides the definition and implementation of the DD::Image::GenericImagePlane
// class, and the assocaited GenericImagePlaneDescriptor, which is used by the
// read/decode interface to the Reader base class added by ReaderExtensions.h and
// ReaderExtensions.cpp.
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

#ifndef DDImage_ReaderExtensions_GenericImagePlane_H
#define DDImage_ReaderExtensions_GenericImagePlane_H

#include "DDImage/Box.h"
#include "DDImage/Format.h"
#include "DDImage/ImagePlaneDescriptor.h"
#include "DDImage/Memory.h"
#include "DDImage/ReaderExtensions/ColorCurve.h"
#include "DDImage/ReaderExtensions/DataInfo.h"
#include "DDImage/ReaderExtensions/Handle.h"
#include "DDImage/ReaderExtensions/ImageDirection.h"

#include <fdk/math/half.h>

#include <vector>

namespace DD
{
  namespace Image
  {
    //! \class DD::Image::GenericImagePlaneDescriptor
    //! \brief Describes a generic image plane which extends DataInfo, a ColorCurve,
    //!        an ImageDirection and ChannelSwizzling to an ImagePlaneDescriptor
    class DDImage_API GenericImagePlaneDescriptor : public ImagePlaneDescriptor
    {
    public:

      GenericImagePlaneDescriptor(const ImagePlaneDescriptor &planeDesc,
                                  DataInfo dataInfo,
                                  ColorCurveEnum colorCurve,
                                  ImageDirection imageDirection = ImageDirection::eIncreasingY,
                                  std::vector<int> channelSwizzling = std::vector<int>())
        : ImagePlaneDescriptor(planeDesc)
        , _dataInfo(dataInfo)
        , _colorCurve(colorCurve)
        , _imageDirection(imageDirection)
        , _channelSwizzling(channelSwizzling)
      {
      }
      
      const DataInfo& dataInfo() const { return _dataInfo;}
      ColorCurveEnum colorCurve() const { return _colorCurve; }
      ImageDirection imageDirection() const { return _imageDirection; }
      const std::vector<int>& channelSwizzling() const { return _channelSwizzling; }

      bool operator==(const GenericImagePlaneDescriptor& rhs) const
      {
        return  (_bounds == rhs._bounds) &&
                (_mask == rhs._mask) &&
                (_packed == rhs._packed) &&
                (_nComps == rhs._nComps) &&
                (_dataInfo == rhs._dataInfo) &&
                (_colorCurve == rhs._colorCurve) &&
                (_imageDirection == rhs._imageDirection) &&
                (_channelSwizzling == rhs._channelSwizzling);
      }
      
    protected:
      
      //! Info describing the data type, limits, etc, in each channel. 
      DataInfo _dataInfo;
      
      //! The type of color curve in which the image data is stored.
      ColorCurveEnum _colorCurve;

      //! Indicates the logical y-direction of image is stored in a GenericImagePlane's data buffer.
      ImageDirection _imageDirection { ImageDirection::eIncreasingY };

      //! Optionally specifies how channels are swizzled in a GenericImagePlane's data buffer.
      //! For example, client code expects RGBA data in that order, so if the buffer contains BGRA then
      //! this should be set to 2, 1, 0, 3.
      //! If this is not empty then it should have the same number of entries as in our ChannelSet (in the
      //! ImagePlaneDescriptor base class).
      std::vector<int> _channelSwizzling;
    };
    
    /// @brief Class to wrap up an image plane of arbitrary type. This
    /// is used by the readers to return image data with the minimal 
    /// amount of conversion needed.
    ///
    /// TODO - refactor ImagePlane and it's floatingpoint refcounted thing
    /// to be compatible with this (via assigment and CC both ways, with
    /// checks for types being right).
    class DDImage_API GenericImagePlane 
    {
    public:
      /// ctor, relying on our own allocation
      GenericImagePlane(const GenericImagePlaneDescriptor &desc);

      //! Constructor using a specific chunk of memory in which we want the image data stored.
      //!
      //! \note Make sure the specified memory is large enough to hold the image plane described
      //! by the specified descriptor (bounds, bit depth and channel count).
      GenericImagePlane(const GenericImagePlaneDescriptor &desc, HandlePtr handle);
      
      const GenericImagePlaneDescriptor& desc() const { return _desc; }
      
      ptrdiff_t rowStrideBytes() const { return _rowStrideBytes; }
      ptrdiff_t colStrideBytes() const { return _colStrideBytes; }
      ptrdiff_t chanStrideBytes() const { return _chanStrideBytes; }
      
      /// Get a reference to the data at (x, y) with the channel number /z/.
      template <class T>
      const T& at(int x, int y, int z) const
      {
        CheckType(T(0));  // TODO: Wrap this in suitable ifdef to guarantee it's removed from retail builds.
                          //       Could use the return variable.
        
        return * reinterpret_cast<T *>(address(x, y, z));
      }

      /// Get a reference to the data at (x, y) with the channel number /z/.
      template <class T>
      T& writableAt(int x, int y, int z)
      {
        CheckType(T(0));  // TODO: Wrap this in suitable ifdef to guarantee it's removed from retail builds.
                          //       Could use the return variable.

        const char *addrc = address(x, y, z);
        char *addr = const_cast<char *>(addrc);
        return *reinterpret_cast<T *>(addr);
      }
      
      //! Clears the image by writing 0 into the entire buffer.
      void clearImage()
      {
        if (_data)
          memset(_data, 0, _handle->getDataSize());
      }

    protected:
      
      //! Helper function to calculate the various strides given the information in the GenericImagePlaneDescriptor member _desc,
      //! which must have been set before the call to SetStrides().
      void SetStrides();
      
      const char *address(int x, int y, int z) const
      {
        y -= _desc.bounds().y();
        x -= _desc.bounds().x();
        
        return _data + (y * rowStrideBytes() + x * colStrideBytes() + z * chanStrideBytes());
      }
      
      GenericImagePlaneDescriptor _desc;
      
      HandlePtr _handle;
      
      char *_data; ///< base address of our image, kept as a char for address maths
      
      //! The various CheckType() functions are used to assert that the type on which at() or writableAt() have been
      //! templatised corresponds to the enumerated type of the descriptor's data info, which is determined at
      //! runtime.
      void CheckType(unsigned char testVariable) { mFnAssert(_desc.dataInfo().dataType() == eDataTypeUInt8); }
      void CheckType(unsigned short testVariable) { mFnAssert(_desc.dataInfo().dataType() == eDataTypeUInt16); }
      void CheckType(unsigned int testVariable) { mFnAssert(_desc.dataInfo().dataType() == eDataTypeUInt32 || _desc.dataInfo().dataType() == eDataTypeUInt32_10bit); }
      void CheckType(fdk::half testVariable) { mFnAssert(_desc.dataInfo().dataType() == eDataTypeFloat16); }
      void CheckType(float testVariable) { mFnAssert(_desc.dataInfo().dataType() == eDataTypeFloat32); }
      
      // Note:
      // ImagePlane stores the stides in terms of pointer arithmetic rather than byte arithmetic.
      // This doesn't matter, GenericImagePlane and ImagePlane aren't part of the same class
      // hierarchy, but might cause confusion for the unwary.
      
      //! The stride between rows in *bytes*. This depends on the channel data type and the number
      //! of channels and/or image width, according to whether the image plane is packed or not
      //! (as specified by GenericImagePlaneDescriptor::_packed).
      ptrdiff_t _rowStrideBytes;
      
      //! The stride between columns in *bytes*.
      ptrdiff_t _colStrideBytes; 
      
      //! The stride between channels in *bytes*.
      ptrdiff_t _chanStrideBytes;

    };
    
    inline GenericImagePlane::GenericImagePlane(const GenericImagePlaneDescriptor &desc)
    : _desc(desc)
    {
      SetStrides();     
      
      // Determine the total size of the image data from the descriptor information.
      size_t dataSize = _desc.bounds().w() * _desc.bounds().h() * DataTypeSizeInBytes(_desc.dataInfo().dataType()) * _desc.nComps();
      
      // Creating the Handle will allocate the image data memory internally.
      _handle = Handle::Make(dataSize);
      _data = static_cast<char*>(_handle->getAllocatedData());
    }
    
    inline GenericImagePlane::GenericImagePlane(const GenericImagePlaneDescriptor &desc, HandlePtr handle)
    : _desc(desc), _handle(handle)
    {
      SetStrides();
      _data = static_cast<char*>(_handle->getAllocatedData());
    }

    inline void GenericImagePlane::SetStrides()
    {
      size_t dataTypeSize = DataTypeSizeInBytes(_desc.dataInfo().dataType());
      
      // Unpacked images are stored as, for a 4x2 rgb image:
      //
      //  rrrr rrrr
      //  gggg gggg
      //  bbbb bbbb
      //
      // Packed images are stored as, given the same example:
      //
      //  rgb rgb rgb
      //  rgb rgb rgb
      //
      if (_desc.packed()) {
        _colStrideBytes = dataTypeSize * _desc.nComps();
        _rowStrideBytes = dataTypeSize * _desc.nComps() * _desc.bounds().w();
        _chanStrideBytes = dataTypeSize;
      }
      else {
        _colStrideBytes = dataTypeSize;
        _rowStrideBytes = dataTypeSize * _desc.bounds().w();
        _chanStrideBytes = dataTypeSize * _desc.bounds().w() * _desc.bounds().h();
      }
    }

  } // namespace Image
}   // namespace DD

#endif  // DDImage_ReaderExtensions_GenericImagePlane_H

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

