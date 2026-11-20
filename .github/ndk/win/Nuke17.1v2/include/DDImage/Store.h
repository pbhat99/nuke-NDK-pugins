//------------------------------------------------------------------------------
// DDImage/Store.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_STORETYPE_H
#define DDIMAGE_STORETYPE_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Deprecation.h"

#include <string>
#include <vector>
#include <map>

namespace DD
{
  namespace Image
  {

    using IntPair          = std::pair<int32_t, int32_t>;
    using IntIntMap        = std::map<int32_t, int32_t>;
    using StringVecVec     = std::vector<std::vector<std::string> >;
    using StlStringIntPair = std::pair<std::string, int32_t>;
    using StlStringVec     = std::vector<std::string>;

    enum StoreType {
      DoublePtr,                // double
      FloatPtr,                 // float
      StringPtr,                // const char*
      IntPtr,                   // int32_t
      UnsignedIntPtr,           // uint32_t
      BoolPtr,                  // bool
      //
      ConvolveArrayPtr,         // DD::Image::ConvolveArray
      ChannelSetPtr,            // DD::Image::ChannelSet
      ChannelPtr,               // DD::Image::Channel
      FormatPairPtr,            // DD::Image::FormatPair
#if kDDImageVersionMajorNum <= 17
      MatrixPtr mFnDeprecatedInNuke14("Use FloatPtr/DoublePtr instead + Knob::numElements()"),
#else
#error "Remove the above code block in Nuke 17 as they are deprecated in Nuke 14."
#endif
      LookupCurvesPtr,          // DD::Image::LookupCurves
      //
      IntSetPtr,                // std::set<int32_t>
      IntIntMapPtr,             // std::map<int32_t, int32_t>
      IntPairPtr,               // std::pair<int32_t, int32_t>
      StringVecVecPtr,          // std::vector<std::vector<std::string> >
      StlStringPtr,             // std::string
      StlStringIntPairPtr,      // std::pair<std::string, int32_t>
      StlStringVecPtr,          // std::vector<std::string>
      //
      Custom,                   // undefined type
      //
      Invalid                   // StoreType is a not useable value (for error conditions)
    };


    //! Get type name string from StoreType enum.
    DDImage_API const std::string&  StoreTypeName(StoreType storetype);

    /*! Return the nominal size, in bytes, of the base type pointed at by a StoreType
        pointer.

        For example a FloatPtr will return sizeof(float), DoublePtr returns
        sizeof(double), ConvolveArrayPtr returns sizeof(DD::Image::ConvolveArray),
        and StlStringIntPairPtr returns sizeof(std::pair<std::string, int32_t>).

        Note - this is only accurate for the StoreType's base data size and does not
        take into account if a Knob is storing an array of the StoreType data. Knobs
        should implement the virtual numElements() method so that, in conjunction
        with StoreTypeBaseSize(), the total size of the data block is deduceable.
    */
    DDImage_API size_t     StoreTypeBaseSize(StoreType storetype);

    /*! Return a compatible StoreType for an usg::Value::Type.
        This only handles the base usg::Value data types like String, Int, Bool, etc.

        Note this method takes an int to avoid including the ndk headers.
    */
    DDImage_API StoreType  StoreTypeForValueType(int valuetype);

  }
}

#endif
