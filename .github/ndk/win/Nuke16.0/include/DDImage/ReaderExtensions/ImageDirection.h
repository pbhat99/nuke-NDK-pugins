// Copyright (c) 2019 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDImage_ReaderExtensions_ImageDirection_H
#define DDImage_ReaderExtensions_ImageDirection_H

namespace DD
{
  namespace Image
  {
    
    //! Indicates whether the logical image stored in a data buffer (e.g. in a GenericImagePlane)
    //! is such that y increases with the address, i.e. the bottom of the image is stored at the start
    //! of the buffer.
    enum class ImageDirection
    {
      eIncreasingY, //<! The logical image's y-direction increases with increasing addresses.
      eDecreasingY  //<! The logical image's y-direction decreases with increasing addresses.
    };

  }
}

#endif  // DDImage_ReaderExtensions_ImageDirection_H
