// Writer.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Writer_h
#define DDImage_Writer_h

#include "DDImage/Write.h"
#include "DDImage/Reader.h"

#include <utility>

namespace DD
{
  namespace Image
  {
    class WriterImpl;
    class LUT;

    /*! \class DD::Image::Writer
        Writes data to an image file (or other image destination).

        The Write creates one of these for every different filename it
        writes. Each different file type is a different subclass. Each
        subclass defines a Writer_Description object, all of these are
        searched to figure out what subclass to use.

        For most image file formats you will want to use a subclass of
        the FileWriter subclass.

        The implementation of the base class is in Write.C
     */
    class DDImage_API Writer : public FileHandler
    {
    public:
      typedef U64 FlagMask;

    private:
      // Be sure to change this symbol if the ABI changes.
#define DDImage_Writer_ABI abi10_0_1
      virtual int DDImage_Writer_ABI();

      friend class Write;
      friend class WriterImpl;
      LUT* lut_;
      FlagMask _flags;

      /// noncopy and nonassign
      Writer& operator=(const Writer&);
      Writer(const Writer &);

    public:
      /// tell the Write to suppress the validation of input0() and checking that it
      /// has channels.  Used for the exrWriter and other stereo-aware writers, which
      /// might not be using input0.  If this is set then the writer needs to perform
      /// its own checking that channels exist.
      static const FlagMask DONT_CHECK_INPUT0_CHANNELS = 0x00000001;

      /// Turn the flags in /newFlags/ on or off, depending
      /// upon newValue.  Leaves flags not in /newFlags/ unaltered.
      /// It is expected that the Writer's constructor should call this.
      void setFlags(FlagMask flags, bool newValue = true)
      {
        if (newValue)
          _flags |= flags;
        else
          _flags &= ~flags;
      }

      void clearFlags(FlagMask flags)
      {
        setFlags(flags, false);
      }

      /// test whether the Writer has any of the given flags
      bool anyFlags(FlagMask newFlags) const
      {
        return (_flags & newFlags) != 0;
      }

      /// test whether the Writer has all of the given flags
      bool allFlags(FlagMask newFlags) const
      {
        return (_flags & newFlags) == newFlags;
      }

    protected:

      Write* iop; //!< Points at the Write that created this.
      std::string _hash;

      LUT* lut() { return lut_; }

      /*!
         Return the value that should be used in lut_ if the user picks
         default. The default version returns the LUT::INT8. This function is
         needed if the default changes in response to controls defined in knobs().
       */
      virtual LUT* defaultLUT() const;

      //! if the value of the knob passed in contributes to the 
      //! value returned from defaultLUT you should return true
      //! from this function to ensure that the default LUT is 
      //! correctly updated in the panel when you change that
      //! knob
      virtual bool isDefaultLUTKnob(DD::Image::Knob* knob) const {
        return false;
      }

      /*! The info() going into the Write */
      const Iop::Info& info() const { return iop->info(); }

      /*! The width of image to write. */
      int width() const { return info().format().width(); }

      /*! The height of image to write. */
      int height() const { return info().format().height(); }

      /*! The pixel aspect ratio of image to write. */
      double getPixelAspect() const { return info().format().pixel_aspect(); }

      /*! The pixel aspect ratio of image to write represented as X/Y resolution values */
      std::pair<int, int> getPixelAspectAsResolution() const;

      /*! Does the format force a clip to the format, or can it write
          out the area outside the format (the overscan), too? 

          Also used to indicate whether the channels are represented
          incompletely in the file, so that if RB is written out then
          it will read back in as RG.
      */
      virtual bool clipToFormat() const
      {
        return true;
      }

      /*!
         Lets a writer override the Op::split_input() method of the including
         Write op. This is used by Writers that can write more than one view.
       */
      virtual int split_input(int i) const;

      /*!
         Lets a writer override the Op::inputContext() method of the including
         Write op. This is used by Writers that can write more than one view.
         Note that \a c is \e scratch space, it is not a usable return value!
       */
      virtual const OutputContext& inputContext(int n, OutputContext& c) const;

      /*! Many file formats can write a certain \e number of channels, but
         cannot identify them in any useful way other than by index. This call
         will return a recommended number of channels to write, based on the
         set the user selected for the Write operator. You may want to clamp
         this to the legal range (such as 1-4) for your file format.
       */
      int depth() const;

      int num_channels() const { return depth(); }

      /*! For simple file writers that use channel(int) to get their inputs,
          this returns a channel mask to use to call open() on the input.
       */
      ChannelSet channel_mask(int num_channels) const;

      /*! For file formats that just write a \e number of channels (rather
          than any identifying information) you should use this to turn an
          index (starting at zero) into the channel to write. You can call
          this with any number, including numbers larger than depth() returns.
          This may return Chan_Black to indicate that no channel should be
          written.
       */
      Channel channel(int index) const;

      /*! The premult() setting from the Write */
      bool premult() const { return iop->premult(); }

      /*! The Iop whose output should be written. */
      Iop& input0() { return iop->input0(); }

      /*! The Iop whose output should be written. */
      const Iop&  input0() const { return iop->input0(); }

      /*! Get a line from the input, and start parallel threads. */
      void get(int y, int x, int r, ChannelMask channels, Row& row)
      {
        iop->input0get(y, x, r, channels, row);
      }

      /*! The filename to write. */
      const char* filename() const { return iop->filename(); }

      std::string createFileHash();

      std::string getHash() const 
      {
#ifdef FN_OS_WINDOWS
        return _hash; 
#else
        return filename();
#endif // FN_OS_WINDOWS
      }

      void renameFromHash();

      /*!    Convert to bytes from floating point.

          \a from should point at an array of \a W floats, spaced 1 apart.

          These are converted and placed into \a to[0], to[delta], to[2*delta],...

          \a z is the channel index <i>(warning this is one less than the
          channel number)</i>. If \a z >= 2 then linear (multiply by 255)
          conversion is done.

          Otherwise the lut() is called to do a normal conversion.

          If premult() is on and \a alpha is not null, it should
          point at an array of \a W bytes for an alpha channel, spaced \a
          delta apart. The lut() is then called to do an unpremult-convert
          of the values.

       */
      void to_byte(int z, uchar*, const float*, const float*, int W, int delta = 1);

      /*! Same as to_byte() except the destination is shorts in the range
         0 through (2<<bits)-1. */
      void to_short(int z, U16*, const float*, const float*, int W, int bits = 16, int delta = 1);

      /*! Same as to_byte() except the destination is an array of floats.
          Linear conversion will leave the numbers unchanged.
       */
      void to_float(int z, float*, const float*, const float*, int W, int delta = 1);

      /*! Call to display percentage of the current frame done. */
      void progressFraction(double f) { iop->progressFraction(f, Op::StatusUseCallback); }
      void progressFraction(int a, int b) { iop->progressFraction(a, b, Op::StatusUseCallback); }

      std::set<int> executingViews() const
      {
        return iop->views();
      }

    public:
      Writer(Write * i);

      /*! The destructor should close any open files (this may be used for
          movie formats that keep the file open between calls to execute().
       */
      ~Writer() override;

      /*! Return true if the destination is a "movie".  If this is true then
          the writer is capable of writing multiple frames, and frame() and
          execute() may be called multiple times to write different images.
          The default version returns false.
       */
      virtual bool movie() const;

      /*! Write the data in iop->input0().  The frame number may be retrieved
          with frame() and the filename from filename().  This may call
          iop->error() with any error messages.
       */
      virtual void execute();

      /*! Override if the writer needs to do something to finish a sequence.
          This is mostly of use to movie writers that don't close out
          the destination on a frame-by-frame basis.
       */
      virtual void finish();

      /*!
         Like the Op::knobs() method, this allows the Writers to create
         format-specific knobs for format-specific parameters. At present,
         the Writers are tasked with adding the knobs themselves after the
         'file_type' knob of the parent Write.
       */
      void knobs(Knob_Callback cb) override;

      //! Allows a writer to check directly if its op has been aborted
      bool aborted() const;

      /*! Set LUT. This is virtual to allow overrides in movWriter
          which performs additional processing
      */
      virtual void setLUT(LUT* lut) { lut_ = lut; }

      /*! Private implementation
       */
      WriterImpl* getImpl() { return _pImpl; }
      WriterImpl* getImpl() const { return _pImpl; }

      static void flip(U16* p, int n) { Reader::flip(p, n); }
      static void flip(U32* p, int n) { Reader::flip(p, n); }
      /*! Flips the array or does nothing, depending on your machine. */
      static void tolsb(U32* p, int n) { Reader::fromlsb(p, n); }
      static void tomsb(U32* p, int n) { Reader::frommsb(p, n); }
      static void tolsb(U16* p, int n) { Reader::fromlsb(p, n); }
      static void tomsb(U16* p, int n) { Reader::frommsb(p, n); }
      
      //! Specialised Writer Description
      /*! \class DD::Image::Writer::Description

          Each subclass of Writer should define at least one static instance
          of this structure. The constructor adds itself to a list that the
          Write operator searches to find the correct type of Writer to use
          to write a given filename.

          Sample code for a plugin:

          \code
         class MyWriter : public FileWriter {
         static const Description d;
         ...
         };

         static Writer* build() {return new MyWriter();}

         const Writer::Description MyWriter::d("myf\0", "my file type", build);
          \endcode
       */
      class DDImage_API Description : public DD::Image::Description
      {
        static void add(DD::Image::Description*);
      public:
        /*! Null separated list (as in "sgi\0rgb\0") of identifiers for this
           file type.

           If "xyz" in in this list then a filename of "*.xyz" or "xyz:*" is
           considered to be this type.
         */
        const char* names;

        /*! User-friendly version of the name. */
        const char* label;

        /*! Make an instance of the writer.  This cannot fail, instead the
           execute() method should try to open the output file
           and produce any error messages. */
        Writer* (*constructor)(Write*);

        /*! Constructor that fills all fields and allows a license check. */
        Description(const char* n, const char* l, Writer * (*c)(Write*),
                    License * f = nullptr) :
          names(n), label(l), constructor(c) { license = f;
                                               ctor(add); }

        /*! Simpler constructor that sets names and label to same value. */
        Description(const char* n, Writer * (*c)(Write*), License * f = nullptr) :
          names(n), label(n), constructor(c) { license = f;
                                               ctor(add); }

        /*! Return the i'th Writer::Description known about, or return a null pointer
            for the last one. */
        static const Description* find(int i);

        /*! Search all the defined Writer::Descriptions for one whose name
            matches the passed string, or return NULL if none. This will also
            try to plugin_load("xyzWriter") (where xyz is \a id) in order
            to find external file writing code.

            If this returns null, then plugin_error() will contain an error.
         */
        static const Description* find(const char* name);
      };

    private:
      WriterImpl* _pImpl; //< private implementation
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
