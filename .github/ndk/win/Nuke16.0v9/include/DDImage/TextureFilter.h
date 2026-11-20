// TextureFilter.h
// Copyright (c) 2014 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_TextureFilter_h
#define DDImage_TextureFilter_h

#include "DDImage/Filter.h"
#include "DDImage/Pixel.h"

namespace DD
{
  namespace Image
  {
    /*! \class DD::Image::TextureFilter
        Like DD::Image::Filter but specific to sampling textures in a 3d rendering context.       
    */
    class DDImage_API TextureFilter : public Filter
    {
    public:
      //! Built-in filter types.
      enum
      {
        Nearest = Filter::NumFilters, //!< Fastest and crudest, sample the nearest texel from the appropriate mipmap level
        Bilinear,     //!< Remove blockiness, sample and interpolate the four nearest texels from the appropriate mipmap level
        Trilinear,    //!< Smooth interpolation of texture according to the distance, bilinearly interpolates between two closest mipmap levels
        Anisotropic,  //!< Highest quality, gives better result when shading surfaces with a high angle relative to the camera
        //
        NumFilters
      };

      TextureFilter(int type = Filter::Cubic, bool enableMipType = false);

      void knobs(Knob_Callback, const char* name = "filter", const char* label = nullptr);

      //! Returns true if the filter instance supports a mipmap filtering type.
      bool isMipTypeEnabled() const { return _enableMipType; }

      //! Returns true if the filter needs mip levels.
      bool isMipMap() const { return IsMipMappingFilter(type_);}

      //! If not a mip map type calls Filter::initialize().
      void initialize();

      //! Filter's assigned type.
      int type() const { return type_; }

      //! Set the filter type.
      void type(int type);

      //! Does the filter type enum implement mip-mapping?
      //! A subclass can redefine this to support additional filter types that mipmap.
      virtual bool IsMipMappingFilter(int type) const { return type >= Nearest && type <= Anisotropic; }

    protected:
      bool _enableMipType;      
    };

    //------------------------------------------------------------------------

    //! Returns the appropriate mip Iop for the sample point and dU dV size from the
    //! list of Iops in the \a mipLevel array.
    //!
    //! \a mipLevels is a null-terminated list of Iop pointers, one for each mip level
    //! starting with level 0 which is full rez. Caution - the \a mipLevels array -must-
    //! end with a null.
    //! Ex. mipLevels = [level0Iop*, level1Iop*, level2Iop*, level3Iop*, nullptr]
    //!
    //! The first Iop in the array is usually the full rez image, and each level after
    //! is half the vertical and horizontal resolution of the previous Iop. In Nuke the
    //! specialized DownRez Iop class can be used to quickly produce these mip levels by
    //! chaining a series of them together, and when TextureMipSample() samples the mip
    //! Iop Nuke automatically produces the intermediate levels by pulling the data down
    //! through the DownRez chain.
    //!
    //! For example if mip level 3 is being sampled then Iop level 0, Iop level 1, and
    //! Iop level 2 are automatically executed. When another TextureMipSample() call wants
    //! level 1 it's highly likely the texel region has already been computed and cached
    //! in a Tile.
    DDImage_API
    Iop* GetMipLevelIop(float          cx,
                        float          cy,
                        const Vector2& dU,
                        const Vector2& dV,
                        Iop**          mipLevels);

    //! Get the mip area to filter, scaled to the resolution defined by \a format.
    DDImage_API
    float GetMipArea(const Vector2& dU,
                     const Vector2& dV,
                     const Format&  format);


    //! Sample a parallelogram of an iop with a mipmap technique.
    //!
    //! \a mipLevels is a null-terminated list of Iop pointers, one for each mip level
    //! starting with level 0 which is full rez. Caution - the \a mipLevels array -must-
    //! end with a null.
    //! Ex. mipLevels = [level0Iop*, level1Iop*, level2Iop*, level3Iop*, nullptr]
    //!
    //! The channels requested by \a get are sampled and put into \a out.
    DDImage_API
    void TextureMipSample(const Vector2&    center,
                          const Vector2&    dU,
                          const Vector2&    dV,
                          TextureFilter*    filter,
                          Iop**             mipLevels,
                          const ChannelSet& get,
                          Pixel&            out);

    //! Same except samples out.channels.
    DDImage_API
    void TextureMipSample(const Vector2& center,
                          const Vector2& dU,
                          const Vector2& dV,
                          TextureFilter* filter,
                          Iop**          mipLevels,
                          Pixel&         out);

  }
}

#endif

// Copyright (c) 2014 The Foundry Visionmongers Ltd.  All Rights Reserved.
