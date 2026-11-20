// Read.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Read_h
#define DDImage_Read_h

#include "DDImage/Iop.h"
#include "DDImage/Knob.h"
#include "DDImage/MetaData.h"
#include "DDImage/FileIop.h"
#include "DDImage/CodecOp.h"
#include "DDImage/Reader.h"
#include "DDImage/FileRead.h"
#include "DDImage/PlanarIop.h"
#include "DDImage/DopeItemFlags.h"

namespace DD
{
  namespace Image
  {

    class Reader;
    class ReaderFormat;
    class ReadImpl;
    class CodecHandler;

    /**
     * Interface used by Read_File_knob for access to its owning Op.
     */
    class DDImage_API ReadFileKnobOwner
    {
    public:
      virtual ~ReadFileKnobOwner();

      virtual void expectedFrameRange(int first, int last) = 0; 
      virtual void setReadFormat(const DD::Image::Format* format) = 0;
      virtual void setFullSizeFormat(const DD::Image::Format* fullSizeFormat) = 0;
      virtual void setFormat(const DD::Image::Format* realFormat) = 0;
      
      virtual DD::Image::FormatPair& getFormats() = 0;
      virtual const DD::Image::Format* getReadFormat() const = 0;

      virtual bool videosequence() const = 0;

      virtual void force_near_frame(bool) = 0;
      virtual void frame_for_reader(int n) = 0;
      virtual void output_type(int n) = 0;

      enum {
        IMAGE = 0,
        BLACK = 1,
        CHECKERBOARD = 2,

        TYPE_MASK = 0xf,

        IGNORE_MISSING_CLIP = 0x10,
      }; // types of output
    };

    /*! \class DD::Image::Read

       Return an image from the outside world. The image is identified by
       the filename (which may just be a string interpreted by one of the
       reader implementations and not an actual file on the disk), and by
       the frame() if it is a movie file.

       If the filename is null a black frame of the expected_format() (or
       video if no format is given) is produced. This is done so that a new
       Read in Nuke will not produce an error.

       All the work is done by the various subclasses of Reader.  Read just
       chooses what subclass to instantiate and returns the output from it,
       and correctly destroys and instantiates new ones as needed.

       Read extends from CodecOp, which extends from FileOp, therefore the Read
       nodes are capable of handling both the container and codec specific knobs
       for those reader formats that inherit from CodecReaderFormat.
     */
    class DDImage_API Read : public PlanarIop, public CodecOp, public FileRead, public ReadFileKnobOwner
    {
    public:

      // Provides a handler to the reader specific knobs. The returned
      // object is usually a ReaderFormat.
      FileHandler* handler() const override;
      
      // Provides a handler to the codec specific knobs. The returned
      // object is usually a CodecReaderFormat.
      CodecHandler* codec_handler() const override;

      void close() override
      {
        Iop::close();
      }

      const char* getFilename() const override
      {
        return filename();
      }
      
      // Expose access to the Reader this Read object owns so that it's possible 
      // to use the Reader outside the operator framework.
      //
      // CAUTION: There's no guarantee the Read has a valid Reader
      Reader* getReader() { return _reader; }
      const Reader* getReader() const { return _reader; }

      DD::Image::DopeItemFlags::Mask getDopeItemFlags() const override;
      bool shouldHideInDopeSheet(Knob* knob) const override;

      // Set the type of file the Read will read. The type is a string like
      // "cin" or "jpeg". Doing this will create all the type specific knobs.
      void set_file_type(const char* fileType);

      const char* file_type() const override { return file_type_; }

      bool can_link() const override { return false; }

    protected:
      Reader* _reader;
      ReaderFormat* _fileformat;
      const Reader::Description* _reader_type;

      Op* op() override { return this; }

      const DD::Image::Description* _readerdesc;

      // Information needed by validate() from reader, copied here so the file
      // does not need to be opened again:
      Iop::Info _readerInfo;
      bool _errorOnConstructor;
      bool _readerInfoOk;
      bool videosequence_;
      bool force_near_frame_;
      bool force_format_;
      Hash _filehash;
      bool _ignoreCachedErrors;

      Reader* create_reader(const char* fname) const;

      /*!
         Open the file and create the Reader object.
       */
      void make_format(const char* fileName);
      void make_format();
      void make_reader();
      void make_reader_base();    // for internal use

      Read* firstRead()
      {
        return dynamic_cast<Read*>(firstOp());
      }

      const Read* firstRead() const
      {
        return dynamic_cast<const Read*>(firstOp());
      }

      const Reader::Description* format_for_name(const char*);

      // Info for the reader to look at:
      int frame_for_reader_;
      bool frame_for_reader_valid_; // Has frame_for_reader_ been initialized to at least first_frame of sequence?
      int view_for_reader_;
      const char* filename_;
      const char* file_type_;

      // Knob settings:
      int colorspace_;
      bool raw_;
      bool premult_;
      bool autoAlpha_;
      int _outputType;
      int _frameRange[2];

      // autoscaling setup by validate:
      bool _proxyscale;
      float _sx, _sy, _tx, _ty;
      void asengine(int y, int x, int r, ChannelMask channels, Row& row);
      Format _localFormat;

      MetaData::Bundle _meta;
      const MetaData::Bundle& _fetchMetaData(const char* key) override;

      FormatPair _formats;
      const Format* _readFormat;

      DD::Image::RecursiveLock _makeReaderLock;

    protected:

      /*!
         validate() will destroy any previous Reader and open a new one if
         the filename has changed or the version number has changed (the
         version number is incremented by the reload button).

         After opening the file, it copies the info_ from the Reader object.
         This will have been initialized with expected_first_frame(),
         expected_last_frame(), expected_format(), and
         expected_full_size_format(), but the Reader subclass may have
         changed this information.

         For Nuke, this may be called with \a for_real set to false. In
         this case it does not try to open the file. Instead it assumes
         everything is at the expected settings. Anything that actually
         needs the data will open the file.
       */
      void _validate(bool) override;

      /*!
         Calls open() on the reader object.
       */
      void _open() override;

      /*! Calls Reader::engine() */
      void engine(int y, int x, int r, ChannelMask, Row &) override;

      void _request(int x, int y, int r, int t, ChannelMask chans, int count) override;

      /**
       * implementation of PlanarIop::useStripes()
       */
      bool useStripes() const override;
      
      /**
       * implementation of PlanarIop::stripeHeight()
       */
      size_t stripeHeight() const override;

      /**
       * implementation of PlanarIop::packedPreference()
       */
      PackedPreference packedPreference() const override;

      /**
       * implementation of PlanarIop::renderStripe
       */
      void renderStripe(ImagePlane& imagePlane) override;

      /**
       * implementation of Iop::packedPreference().  Calls
       * Iop::doFetchPlane() or PlanarIop::doFetchPlane() depending
       * on whether the reader supports planar.
       */
      void doFetchPlane(ImagePlane& imagePlane) override;

      /**
       * implementation of PlanarIop::getPlanarI
       */
      PlanarI* getPlanarI() override;

      /**
       * implementation of PlanarI::getPlaneFromChannel
       */
      PlanarI::PlaneID getPlaneFromChannel(Channel chan) override;
      
      /*!
         Call this to close any current file. This will free up resources. Nuke calls this
         automatically after a timeout.

         open() will reopen the file, but it assumes the file is the same one. You must call
         invalidate() if there is a possibility the existence, readability, or dimensions
         of the file has changed.
       */
      void _close() override;

    public:
      void setReadFormat(const Format* fmt) override;
      void setFullSizeFormat(const Format* fmt) override;
      void setFormat(const Format* fmt) override;

      /*!
         Set the flag that controls the value update propagation for the file type.
         If the flag is reset, the value refreshing of the file type knob is blocked.
      */
      void setUpdateFileTypeKnobFlag(const bool updateFileTypeKnobFlag);

      FormatPair& getFormats() override;
      const FormatPair& getFormats() const;
      const DD::Image::Format* getReadFormat() const override;

      /*! \fn const Format* Read::expected_full_size_format() const

         This is the Format the Read will output as full_size_format().  If
         it is NULL or is equal to expected_read_format() then the format() is
         returned.
       */
      const Format* expected_full_size_format() const { return _formats.fullSizeFormat(); }

      /*! \fn const Format* Read::expected_format() const
         If not NULL then this is the proxyfied version of the expected_full_size_format
         and will be used as the output format(). If this is NULL then the read
         format is used and you better not try to set any proxy scaling.
       */
      const Format* expected_format() const { return _formats.format(); }

      /*! \fn const Format* Read::expected_read_format() const
         This is the format to assume file is in if it has the correct dimensions.
         If the dimensions are wrong then Format::find() is used to locate or
         create a correct format, but the pixel_aspect from this format is used
         as a hint.

         If this is NULL (the default value) then Format::find() is always
         called with a pixel aspect of 1.
       */
      const Format* expected_read_format() const { return _readFormat ? _readFormat : _formats.format(); }
      const Format& read_format() const { return _readerInfo.format(); }

      bool videosequence() const override { return videosequence_; }

      Read(Node * node = nullptr);
      ~Read() override;

      /*!
         Indicate that the frame number, view, autoAlpha, and LUT may affect the output.
       */
      void append(Hash&) override;

      void retrieveInfo();
      
      //! Setter for ignoreCachedErrors flag which is used by the renderFrames() function
      //! to force-invalidate read ops which depend on previous writes in the op tree.
      void setIgnoreCachedErrors(bool ignoreCachedErrors);

      void frame_for_reader(int n) override { frame_for_reader_ = n; frame_for_reader_valid_ = true; }
      int frame_for_reader() const { return frame_for_reader_; }
      void filename(const char* f);
      const char* filename() const;

      void view_for_reader(int view) { view_for_reader_ = view; }
      int view_for_reader() const { return view_for_reader_; }

      ViewSet splitForViews() const override;

      int current_view(const DD::Image::OutputContext* oc) const;

      /*!
         Set the frame range returned if the image file itself does not contain
         a frame range.
       */
      void expectedFrameRange(int first, int last) override;
      int expected_first_frame() const { return _frameRange[0]; }
      int expected_last_frame() const { return _frameRange[1]; }

      int first_frame() const { return _readerInfo.first_frame(); }
      int last_frame() const  { return _readerInfo.last_frame();  }

      int colorspace() const { return colorspace_; }

      /*! \fn void Read::colorspace(int v)

         Make the Reader use LUT::builtin(v) to convert the data, unless \a v
         is zero in which case the Reader chooses the LUT itself (for most
         Readers the default LUT is sRGB). Zero is the default value.
       */
      void colorspace(int v) { colorspace_ = v; }
      bool raw() const { return raw_; }

      /*! \fn void Read::raw(bool v)

         Sets the LUT to LUT::LINEAR (overriding the colorspace() setting).
         Some Reader types may examine this setting and use it to turn off other
         processing, such as conversion from YUV space.
       */
      void raw(bool v) { raw_ = v; }
      bool premult() const { return premult_; }

      /*! \fn void Read::premult(bool v)

         Indicates that data should be divided by the alpha channel before being
         run through the LUT, and then multiplied again afterwards. This will
         convert data that was premultiplied in the file's colorspace into
         data that is premultiplied in linear colorspace.

         This makes a difference if the LUT is anything other than
         LUT::LINEAR for any pixels where the alpha is greater than zero and
         less than one.
       */
      void premult(bool v) { premult_ = v; }
      bool autoAlpha() const { return autoAlpha_; }
      void autoAlpha(bool v) { autoAlpha_ = v; }

      void knobs(Knob_Callback) override;
      int knob_changed(Knob*) override;
      int knob_change_finished(Knob* knob, bool changeByUser) override;
      bool invalidate(Hash hash);
      bool invalidate() { return Iop::invalidate(); }

      int output_type() const { return _outputType & TYPE_MASK; }
      bool ignore_missing_clip() const { return (_outputType & IGNORE_MISSING_CLIP) != 0; }

      /*! \fn void Read::output_type(int v)

         Sets exactly what is in the pixels that the Read produces. This is
         used by Nuke to produce results with the correct size and channels
         for missing frames, such as out-of-range frame numbers, or missing
         files in the middle of the range.  This is done by setting the
         filename to an actual existing file, but using this to make the read
         produce black or a checkerboard.

         - IMAGE returns the data from the file.
         - BLACK returns zero in all channels and sets the bounding box to 1x1.
         - CHECKERBOARD produces a box pattern for all channels and sets the
         bounding box to the format.

       */
      void output_type(int v) override { _outputType = v; }

      /*! Force the read node to use near frame in case of error !*/
      void force_near_frame(bool b) override { force_near_frame_ = b; }

      /*! Force the read node to output at the expected format !*/
      void force_format(bool b) { force_format_ = b; }

#ifdef FN_OS_MAC
      /*! Disable os level file caching for this read. By default on OSX all files are cached by the OS which can drastically increase
       the amount of memory being used.
       * !*/
      void disable_OSX_caching() { _disable_OSX_caching = true; }
#endif //FN_OS_MAC

      /*!
         Return hints to control when and how this op will be evaluated by the top-down system.
      */
      OpHints opHints() const override;

      /*!
        Update our UI in response to a path change on an asset-aware knob
      */
      void _updateKnobsFromAsset(Knob* assetKnob, const AssetData& assetData) override;
      
    private:
      ReadImpl* getImpl();  // Return the private implementation instance 

      // Return the list of names of the supported Reader file types
      static const char* const* File_types();

      // Helper function to refresh the value of the file type knob
      void update_filetype_knob();

      // Helper function to update a colorspace knob selection from an asset's trait data
      void updateColorspaceKnobFromAsset(const AssetData& assetData);

    public:
      const char* Class() const override;
      const char* node_help() const override;
      static const Description d;

    private:
      ReadImpl* _pImpl; //< private implementation

      // Bug 41789 - Nuke performance on OSX suffers massively when OS file caching is disabled.
      // Hence the ability to control it needs to be exposed, so Nuke reads can have it enabled
      // while other reads can disable it.
      bool _disable_OSX_caching;

      bool _updateFileTypeKnobFlag;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
