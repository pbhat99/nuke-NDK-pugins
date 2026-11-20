// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_IMAGING_IMAGEINTERFACE_H
#define USG_IMAGING_IMAGEINTERFACE_H

#include "usg/base/Token.h"
#include "usg/geom/Attribute.h"

#include "fdk/math/Box2.h"

namespace usg
{

  /*! Abstract wrapper for a HioImage subclass specifically for Nuke's
      internal use connecting an in-memory DDImage Op/Iop to a Hydra texture
      reader.

      Image orienation matches Y-up Nuke - lower-left corner = 0,0 and the endpoint-exclusive
      top-right corner = width(),height().

      Only the methods needed for DDImage to interface with this class
      are exposed.
  */
  class USG_API ImageInterface
  {
  public:
    /*!
    */
    class USG_API Description
    {
    public:
      //! Constructor method definition used for builder methods in ImageInterface plugins.
      typedef ImageInterface* (*InterfaceBuilder)(const std::string& nodepath);

      const char*      image_identifier;    //!< Image identifier string ('nuke', etc)
      InterfaceBuilder interface_builder;   //!< Call this to construct a ImageInterface object.

      //! Add a ImageInterface builder for a specific image identifier.
      Description(const char*      identifier,
                  InterfaceBuilder builder);
    };


  public:
    //!
    ImageInterface(const std::string& nodepath);
    //!
    virtual ~ImageInterface();


    /*! Find an ImageInterface description by image indentifier name.
        This does not currently support searching for plugin files, it
        assumes a static Description was compiled into an existing plugin.
    */
    static const Description*  find(const char* identifier);


    //! Path string identifying a Nuke Node by name and context.
    virtual const std::string&  nodepath() const { return _nodepath; }

    //----------------------------------------------------------

    /*! Total image width with left edge assumed to be at 0 and the endpoint-exclusive
        right edge equal to width().
    */
    virtual int32_t  width()  const { return 0; }
    /*! Total image height with bottom edge assumed to be at 0 and the endpoint-exclusive
        top edge equal to height().
    */
    virtual int32_t  height() const { return 0; }
    /*! Channel count can be 0,1,2,3 or 4 only. ie max number of channels is 4.
    */
    virtual int32_t  numChannels()     const { return 0; }
    /*! The UV offset for this image in UV space.
    */
    virtual fdk::Vec2f uvTranslate() const { return fdk::Vec2f(0.0f, 0.0f); }
    /*! The UV scaling factor for this image in UV space.
    */
    virtual fdk::Vec2f uvScale() const { return fdk::Vec2f(1.0f, 1.0f); }

    //----------------------------------------------------------

    //! Is the image configured and a valid size? Default version tests that W/H/D is non-zero.
    virtual bool     isValid() const { return (width() > 0 && height() > 0 && numChannels() > 0); }

    //----------------------------------------------------------

    //!
    virtual bool  getMetadata(const Token& key, Value* value) const;
    //!
    //virtual bool  getSamplerMetadata(PXR_NS::HioAddressDimension dim,
    //                                 PXR_NS::HioAddressMode*     param) const override { return false; }

    //----------------------------------------------------------

    /*! Fill the interleaved(packed) image data buffer which is allocated based on the
        width()
        Note that 'active_bbox' is in standard Nuke DD::Image::Box notation(endpoint-exclusive)
        and will be clamped to [0 0 width() height()].
        Returns true if no error.
    */
    virtual bool read(const fdk::Box2i& active_bbox,
                      int32_t num_chans,
                      int32_t bytes_per_channel,
                      bool signed_type,
                      bool apply_srgb_correction,
                      bool mirrorX,
                      bool mirrorY,
                      void* buffer);


  protected:
    std::string _nodepath;    //!< Usually assigned in ctor

  };


} // usg

#endif
