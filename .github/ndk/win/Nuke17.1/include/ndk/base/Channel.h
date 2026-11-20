// Copyright (c) 2024 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_BASE_CHANNEL_H
#define NDK_BASE_CHANNEL_H

#include "ndk/api.h"

#include <cstdint>
#include <iostream>
#include <limits>

namespace ndk {

  class ChannelManager;

  //-------------------------------------------------------------------------

  //! \class ndk::Channel
  //!
  //! Channel ID integer index used for indexing arrays of channel data.
  //! See also classes ndk::ChannelManager and ndk::ChannelSet.
  //!
  class NDK_API Channel
  {
  public:

    //! Channel::Type is a hint to the ordering and/or usage of a channel or layer. Some
    //! of the types apply to both layers or channels while others are just for channels.
    //! For example if a channel is a color component it's important to know if it is the
    //! red, green, or blue component regardless of the channel's actual name.
    //!
    //! When this can be confusing is when a channel name like 'y' is ambiguous and used
    //! in both color encoding (Yxy) and 3D position/vector encoding (xyz) and the expected
    //! ordered position of the 'y' channel in a layer is not obvious.
    //!
    enum Type : int32_t
    {
      AutoType = -1,  //!< Special value: Determine type from channel/layer name
      NoType = 0,     //!< Generic value: ordering=unimportant, blendable=unknown, interpolatable=unknown
      //
      // -------- Blendable -------
      Blendable,      //!< Generic blendable layer: ordering=unimportant, blendable=yes, interpolatable=yes
      //
      Color3,         //!< 3 channel color layer: ordering=unimportant, blendable=yes, interpolatable=yes
      Color4,         //!< 4 channel color layer: ordering=unimportant, blendable=yes, interpolatable=yes
      ColorRed,       //!< Color red  component: ordering=0, blendable=yes, interpolatable=yes
      ColorGreen,     //!< Color reen component: ordering=1, blendable=yes, interpolatable=yes
      ColorBlue,      //!< Color blue component: ordering=2, blendable=yes, interpolatable=yes
      ColorAlpha,     //!< Color alpha component: ordering=3, blendable=yes, interpolatable=yes
      //
      ColorComp0,     //!< Color 1st component: ordering=0, blendable=yes, interpolatable=yes
      ColorComp1,     //!< Color 2nd component: ordering=1, blendable=yes, interpolatable=yes
      ColorComp2,     //!< Color 3rd component: ordering=2, blendable=yes, interpolatable=yes
      ColorComp3,     //!< Color 4th component: ordering=3, blendable=yes, interpolatable=yes
      //
      Mask,           //!< Generic blendable mask: ordering=unimportant, blendable=yes, interpolatable=yes
      //
      // -------- Non-blendable (but still interpolatable) -------
      Data,           //!< Generic data layer: ordering=unimportant, blendable=no, interpolatable=yes
      DataDepth,      //!< Depth (Z) distance (vs. vector length): ordering=0, blendable=no, interpolatable=yes
      DataLength,     //!< Vector length: ordering=0, blendable=no, interpolatable=yes
      DataVec2,       //!< Vector 2: ordering=unimportant, blendable=no, interpolatable=yes
      DataVec3,       //!< Vector 3: ordering=unimportant, blendable=no, interpolatable=yes
      DataVec4,       //!< Vector 4: ordering=unimportant, blendable=no, interpolatable=yes
      DataVecX,       //!< Vector X component: ordering=0, blendable=no, interpolatable=yes
      DataVecY,       //!< Vector Y component: ordering=1, blendable=no, interpolatable=yes
      DataVecZ,       //!< Vector Z component: ordering=2, blendable=no, interpolatable=yes
      DataVecW,       //!< Vector W component: ordering=3, blendable=no, interpolatable=yes
      //
      // -------- Non-blendable, Non-interpolatable (mixing the value will destroy it) -------
      BitmaskData,        //!< Generic static data: ordering=unimportant, blendable=no, interpolatable=no
      BitmaskDataId,      //!< Id value layer: ordering=unimportant, blendable=no, interpolatable=no
      BitmaskDataId2,     //!< 2-value Id layer: ordering=unimportant, blendable=no, interpolatable=no
      BitmaskDataId3,     //!< 3-value Id layer: ordering=unimportant, blendable=no, interpolatable=no
      BitmaskDataId4,     //!< 4-value Id layer: ordering=unimportant, blendable=no, interpolatable=no
      BitmaskDataIdComp0, //!< Id value 1st component: ordering=0, blendable=no, interpolatable=no
      BitmaskDataIdComp1, //!< Id value 2nd component: ordering=1, blendable=no, interpolatable=no
      BitmaskDataIdComp2, //!< Id value 3rd component: ordering=2, blendable=no, interpolatable=no
      BitmaskDataIdComp3, //!< Id value 4th component: ordering=3, blendable=no, interpolatable=no
    };

    static bool  IsBlendableType(Type type);
    static bool  IsDataType(Type type);
    static bool  IsBitmaskType(Type type);

    //! Is the channel type part of a compound type? This is true for ColorRed but false for Color3.
    static bool  IsCompoundMemberType(Type type);

    //! Is the channel or layer a part of a color? This identifies only color components, not the alpha.
    static bool  IsColorType(Type type);

    //! Is the channel or layer a part of a data vector?
    static bool  IsVectorType(Type type);

    //! Human-readable string for Type debugging. Note - not intended for parsing!
    static const char*  TypeString(Type type);

    //! Returns ordering offset of a Type or -1 if ordering is not important.
    static int32_t      TypeOrdering(Type type);

    //! Return the 'purpose' of a channel subname. For example subnames 'r', 'R', 'red', and 'Red'
    //! all refer to a -red- color channel, formally identified in Nuke as Chan_R.
    //! Some subnames are not so clear cut in their 'purpose', for example 'y' and 'Y' which may
    //! refer to the 'Y' in an XYZ vector or the 'Y' in a Yxy color coordinate.
    //! See Channel::Type for more info.
    //! Note - this calls DefaultChannelManager()::getChannelTypeFromSubname(), so call a specific
    //! manager if you want its custom name handling.
    //! If a subname string is not recognized then Type::None is returned.
    static Type         TypeFromSubname(const char* subname);

    //! Returns the number of data elements for the Type. For example, 3 for a Color3 or DataVec3.
    //! Returns 1 if type is unspecific or non-compound (like Auto.)
    static size_t       TypeNumDataElements(Type type);

    //! Returns the nominal compound Type this type stores to. For example ColorRed will
    //! return Color3, DataVecZ will return DataVec3, etc. Types with no obvious compound
    //! organization like DataDepth will return the same type, ie DataDepth.
    //! \a sizeHint helps resolve compound size ambiguity, so if it's 2 and type is DataVecX
    //! then DataVec2 will be returned.
    static Type         TypeCompoundType(Type type, int32_t sizeHint = 0);

    //! Returns the nominal Value::Type this type stores to. For example ColorRed will return
    //! Value::Float, and Color3 will return Value::Color3f.
    //! Note this method returns an int to avoid including the usg headers, so casting to
    //! usg::Value::Type may be required.
    static int32_t      TypeValueType(Type type);

    //! Returns the nominal compound Value::Type this type stores to. For example ColorRed will
    //! return Value::Color3f, DataVecZ will return Value::Float3, etc. Types with no obvious
    //! compound organization like DataDepth will return their base data type, ie Value::Float.
    //! Note this method returns an int to avoid including the usg headers, so casting to
    //! usg::Value::Type may be required.
    //! \a sizeHint helps resolve compound size ambiguity, so if it's 2 and type is DataVecX
    //! then Float2 will be returned.
    static int32_t      TypeCompoundValueType(Type type, int32_t sizeHint = 0);

    //-------------------------------------------------------------------------

    //! Channel::Interpretation is an additional hint to Type for the handling and/or usage of
    //! a channel or layer.
    //! For example a DataVec3 Type can store a vector, position or normal which may affect the
    //! kinds of operations to be applied, such as a normal needing to multiplied by the inverse
    //! transpose of a matrix vs. a position which can be directly multiplied by a 4x4 matrix,
    //! vs. a vector which should only be rotated by a matrix.
    enum Interpretation : int32_t
    {
      AutoInterp = -1,  //!< Special value: Determine type from channel/layer name
      NoInterp = 0,     //!< Generic value: ordering=unimportant, blendable=unknown, interpolatable=unknown
      //
      ColorRGB,         //!< DataVec3 RGB color
      ColorXYZ,         //!< DataVec3 XYZ color coordinate
      ColorYxy,         //!< DataVec3 Yxy color coordinate
      //
      PositionXYZ,      //!< DataVec3 XYZ position coordinate
      VectorXYZ,        //!< DataVec3 XYZ direction vector
      NormalXYZ,        //!< DataVec3 XYZ normal direction vector
      //
      PositionXY,       //!< DataVec2 XY position coordinate
      VectorXY,         //!< DataVec2 XY direction vector
      //
      ParametricUV,     //!< DataVec2 uv/st surface coordinate
      ParametricUVW,    //!< DataVec3 uvw/stw surface coordinate (where W = depth coord)
      TextureUV,        //!< DataVec2 uv/st texture coordinate
      TextureUVW,       //!< DataVec3 uvw/stw texture coordinate (where W = perspective coord)
    };

    //! Human-readable string for Interpretation debugging. Note - not intended for parsing!
    static const char*  InterpretationString(Interpretation interp);


  public:
    //! Default ctor intentionally leaves junk in _index for performance reasons.
    Channel() = default;
    constexpr Channel(const Channel& b) : _index(b._index) {}
    constexpr Channel(int32_t i) : _index(i) {}

    explicit operator bool() const { return (_index > 0/*Chan_Empty*/); }

    constexpr int32_t   index() const { return _index; }
    constexpr operator  int32_t() const { return _index; }
    Channel&  operator = (int32_t i) { _index = i; return *this; }

    // Increment/decrement operators do no range checking!

    Channel&  operator += (const Channel& c) { _index += c; return *this; }
    Channel&  operator += (int32_t i) { _index += i; return *this; }
    Channel&  operator -= (const Channel& c) { _index -= c; return *this; }
    Channel&  operator -= (int32_t i) { _index -= i; return *this; }

    Channel&  operator ++ () { ++_index; return *this; } // ++prefix
    Channel   operator ++ (int) { return Channel(_index++); } // postfix++
    Channel&  operator -- () { --_index; return *this; } // --prefix
    Channel   operator -- (int) { return Channel(_index--); } // postfix--


  private:
    int32_t _index;
  };

  //! This prints the Channel index number only, use ChannelManager to print channel names.
  inline std::ostream& operator<<(std::ostream& o, Channel& c) { o << c.index(); return o; }

  //-------------------------------------------------------------------------

  /*! \relates ndk::ChannelManager

     Constant for the lowest valid Channel number that can be produced in a single
     ChannelManager. Note that Chan_Empty(0) is a reserved value and is not a valid
     Channel index.
  */
  static constexpr Channel Chan_Min = 1;

  /*! \relates ndk::ChannelManager

     Constant for the highest valid Channel number that can be produced in a single ChannelManager.
     This can be used to declare a fixed-size array that will be indexed by channel, see the
     Pixel class for an example.
     You can either declare the array to have a size of Chan_Max+1, or of a size Chan_Max
     and subtract 1 from the Channel to index it (thus removing the Chan_Empty entry at 0).

     The max channel count is quite large and you should try to make variable-sized arrays if
     possible when storing large amounts of channel data. Use ChannelSet::last() to find the
     last active channel number for that context.
  */
  static constexpr Channel Chan_Max = 1023;


  //
  // Standard color channel constants that all ChannelManagers share:
  //

  static constexpr Channel  Chan_Empty = 0;
  static constexpr Channel  Chan_R     = 1;
  static constexpr Channel  Chan_G     = 2;
  static constexpr Channel  Chan_B     = 3;
  static constexpr Channel  Chan_A     = 4;


  //-------------------------------------------------------------------------

  //! 32-bit mask used for initializing ChannelSets. Note that because of this hard mask size
  //! a ChannelSet cannot be default initialized past channel index 32. This is a legacy of the
  //! original Nuke channel list being limited to 32 channels and should be removed.
  using ChannelSetInit = uint32_t;

  //
  // Standard channel set constants that all ChannelManagers share:
  //
  // Important note - ChannelSetInit is 32-bits in size so Mask constants cannot be
  //                  assigned past bit 32.
  //

  static constexpr ChannelSetInit  Mask_None = 0x0;
  static constexpr ChannelSetInit  Mask_All  = 0xffffffff;  //!< Special value that makes a ChannelSet contain all possible channels
  //
  static constexpr ChannelSetInit  Mask_R = 1 << (Chan_R.index() - 1);
  static constexpr ChannelSetInit  Mask_G = 1 << (Chan_G - 1);
  static constexpr ChannelSetInit  Mask_B = 1 << (Chan_B - 1);
  static constexpr ChannelSetInit  Mask_A = 1 << (Chan_A - 1);
  //
  static constexpr ChannelSetInit  Mask_RGB  = Mask_R | Mask_G | Mask_B;
  static constexpr ChannelSetInit  Mask_RGBA = Mask_RGB | Mask_A;

  //! Logical AND a Channel into the set.
  inline ChannelSetInit operator & (ChannelSetInit a, Channel b) { return b ? ChannelSetInit(a & (1U << (b - 1))) : Mask_None; }

} // namespace ndk


namespace std
{
  template <>
  class numeric_limits<ndk::Channel>
  {
  public:

    static constexpr bool is_specialized = true;
    static constexpr bool is_signed  = true;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;

    static constexpr int radix  = 2;
    static constexpr int digits = 31;

    static ndk::Channel min() throw() { return ndk::Chan_Empty; }
    static ndk::Channel max() throw() { return ndk::Chan_Max; }

    //! Lowest active channel
    static ndk::Channel lowest() throw() { return ndk::Chan_Min; }
  };

} // namespace std

#endif
