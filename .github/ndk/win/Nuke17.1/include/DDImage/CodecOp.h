/// CodecOp.h
/// Copyright (c) 2019 The Foundry Visionmongers Ltd. All Rights Reserved.
/// \file This file needs to be documented

#ifndef DDImage_CodecOp_H
#define DDImage_CodecOp_H

#include "DDImage/FileOp.h"

#include <string>
#include <memory>

namespace DD {
  namespace Image {

    class CodecHandler;

    /**
     * This class extends the functionality of the FileOp class
     * to allow codec-format-specific knobs to be substituted
     * in to a particular place more easily.
     * This is normally extended by the Read/Write nodes
     */
    class DDImage_API CodecOp : public FileOp
    {
    public:
      CodecOp();
      ~CodecOp() override;

      /**
       * return the codec format handler for the current knob settings.
       * this should return null if the handler does not support codecs.
       */
      virtual CodecHandler* codec_handler() const = 0;

      /**
       * insert in the codec specific knobs. subclasses should call this in their knobs() implementation
       */
      mFnDeprecatedInNuke14(
        "codec_options_knobs(Knob_Callback f) has been deprecated.")
        virtual void codec_options_knobs(Knob_Callback f);

      /**
       * If the id is different, replace the knobs by calling the above. Subclasses
       * should call this in e.g. knob_changed() after the handler has changed
       */
      mFnDeprecatedInNuke14(
        "add_codec_knobs(const void* id, const char* fileExt) has been deprecated.")
        void add_codec_knobs(const void* id, const char* fileExt);

      /**
       * get the number of codec knobs
       */
      mFnDeprecatedInNuke14("codec_knobs() has been deprecated.")
      int codec_knobs() const;

    private:
      // This pointer is deprecated too.
      std::unique_ptr<class CodecOpImpl> _pImpl;
    };

  } // namespace Image
} // namespace DD

#endif // DDImage_CodecOp_H
