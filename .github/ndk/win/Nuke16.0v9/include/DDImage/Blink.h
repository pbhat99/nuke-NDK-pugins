// Blink.h
// interface between DDImage and Blink

#ifndef DDImage_Blink_h
#define DDImage_Blink_h

#include "DDImage_API.h"

namespace Blink {
  class Image;
}


namespace DD {
  namespace Image {
    class ImagePlane;

    namespace Blink {

      /**
        * Wrap an ImagePlane into the provided Blink::Image.
        *
        * ImagePlane must have 4 components or less and a non-null buffer associated with it.
        * 
        * If the call returns true blinkImage is assigned a Blink::Image that has the same
        * underlying buffer as imagePlane. If blinkImage is already referencing an image
        * it will be released in the same manner as other Blink::Image assignments. blinkImage
        * can then be used like any other Blink::Image created on the CPU device.
        * imagePlane is not modified. blinkImage retains the underlying memory independently of
        * imagePlane so can be used beyond the lifespan of imagePlane.
        *
        * If the call returns false both imagePlane and blinkImage are unchanged.
        */
      DDImage_API bool ImagePlaneAsBlinkImage(ImagePlane& imagePlane, ::Blink::Image& blinkImage);

    } // namespace Blink
  } // namespace Image
} // namespace DD

#endif // RIPContext_h

