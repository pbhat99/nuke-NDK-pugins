// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

//---------------------------------------------------------------------------------
//
// This file provides the definition for code encapsulating the data types used
// by the read/decode interface to the Reader base class added by ReaderExtensions.h
// and ReaderExtensions.cpp.
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

#ifndef DDImage_ReaderExtensions_DataInfo_H
#define DDImage_ReaderExtensions_DataInfo_H

#include "DDImage/DDImage_API.h"

namespace DD
{
  namespace Image
  {
    //! Enumeration for data types that DataInfo supports. 
    enum DataTypeEnum {
      eDataTypeNone,
      eDataTypeUInt8,
      eDataTypeUInt16,
      eDataTypeUInt32,
      eDataTypeFloat16,
      eDataTypeFloat32,
      eDataTypeUInt32_10bit,  // A special case where we want to return 10-bit RGB in a single 32-bit word.
    };
    
    inline size_t DataTypeSizeInBytes(DataTypeEnum dataType) {
      switch(dataType) {
        case eDataTypeNone: return 0;
        case eDataTypeUInt8: return 1;
        case eDataTypeUInt16: return 2;
        case eDataTypeUInt32: return 4;
        case eDataTypeFloat16: return 2;
        case eDataTypeFloat32: return 4;
        case eDataTypeUInt32_10bit: return 4; // Since we store our 10-bit data in a single 32-bit word.
        default: mFnAssert(false); return 0;
      }
    }

    /// \brief Class that provides information about component types
    class DDImage_API DataInfo
    {
    public :
      
      //! Constructor requiring full specification on the data info.
      DataInfo(DataTypeEnum dataType, bool clamps, int min, int max, int whitePoint)
      : _dataType(dataType), _clamps(clamps), _min(min), _max(max), _whitePoint(whitePoint)
      {}
      
      //! Constructor requuring only the data type, setting the other info to defaults typical for
      //! the specified type.
      DataInfo(DataTypeEnum dataType);
      
      DataTypeEnum dataType() const { return _dataType; }
      bool clamps() const { return _clamps; }
      int min() const { return _min; }
      int max() const { return _max; }
      int whitePoint() const { return _whitePoint; }
      
      bool operator==(const DataInfo& rhs) const
      {
        return (_dataType == rhs._dataType) && (_clamps == rhs._clamps) && (_min == rhs._min) && (_max == rhs._max) && (_whitePoint == rhs._whitePoint);
      }
      
    protected :
      DataTypeEnum _dataType; ///< the data type we are
      bool _clamps; ///< are min and max to be used or ignored, float formats should generally ignore them
      int  _min;    ///< the minimum legal value that can be stored, generally ignored by floats
      int  _max;    ///< the maximum legal value that can be store, generally ignored by floats
      int  _whitePoint; ///< the nominal white point, generally 1 for floats
    }; 
    
    
    inline DataInfo::DataInfo(DataTypeEnum dataType)
    : _dataType(dataType)
    {
      _clamps = false;
      _min = 0;
      _max = 0;
      _whitePoint = 0;
      
      switch (_dataType) {
        case eDataTypeNone:
          break;
        case eDataTypeUInt8:
          _clamps = true;
          _max = 0xff;
          break;
        case eDataTypeUInt16:
          _clamps = true;
          _max = 0xffff;
          break;
        case eDataTypeUInt32:
          _clamps = true;
          _max = 0xffffffff;
          break;
        case eDataTypeFloat16:
          _clamps = false;
          _max = 1;
          _whitePoint = 1;
          break;
        case eDataTypeFloat32:
          _clamps = false;
          _max = 1;
          _whitePoint = 1;
          break;
        // #mat: This is a special case where we want to return 10-bit RGB in a single 32-bit word - note that we don't use any alpha here, if we needed to accurately
        // represent the value range for the alpha then this wouldn't work (since the alpha is only 2 bits)
        case eDataTypeUInt32_10bit:
          _clamps = true;
          _max = (1 << 10) - 1;
          break;
        default:
          mFnAssert(false);
          break;
      }
    }

  }
}

#endif  // DDImage_ReaderExtensions_DataInfo_H

// Copyright (c) 2012 The Foundry Visionmongers Ltd.  All Rights Reserved.

