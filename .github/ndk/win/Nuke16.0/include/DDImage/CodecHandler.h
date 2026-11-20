#ifndef DDImage_CodecHandler_H
#define DDImage_CodecHandler_H

#include "DDImage/Knobs.h"

namespace DD {
  namespace Image {

    /**
     * This is a codec format handler interface. This provides a custom
     * labeled section of knobs to the reader formats that implement it.
     * It is usually recommended for the reader formats to extend from
     * \class DD::Image::CodecReaderFormat, which is a subclass of this.
     * 
     * Subclasses should implement codec_knobs() just as an Op would.
     */
    class DDImage_API CodecHandler
    {
    public:
      virtual void codec_knobs(Knob_Callback cb) = 0;

      /** Get the current codec name associated with the format container */
      virtual std::string codec_name() const = 0;

      virtual ~CodecHandler() = default;
    };

  }
}

#endif // DDImage_CodecHandler_H