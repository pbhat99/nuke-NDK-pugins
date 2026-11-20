// Reader.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Reader_h
#define DDImage_Reader_h

#include "DDImage/FileIop.h"
#include "DDImage/ReaderDescription.h"
#include "DDImage/ReaderExtensions/PlanarReadInfo.h"
#include "DDImage/PlanarI.h"
#include "DDImage/CodecHandler.h"

namespace DD
{
  namespace Image
  {

    class Read;
    class LUT;
    class AudioReaderI;

    typedef Read ReaderOwner; // for back-compatibility with 5.1 plugin source

    class DDImage_API ReaderFormat : public FileHandler
    {
    public:
      ~ReaderFormat() override;
      virtual void append(Hash& hash) = 0;
    };
    
    /*! \class DD::Image::CodecReaderFormat
       A reader format capable of handling both container and codec specific knobs.

       Codec centric readers may want to implement this to expose codec specific knobs
       under a separate section with a custom name returned from codec_name method. An
       example is arriReader plugin, which is exposing arri specific settings (knobs).
       A good situation to use this format is when the codec reader (such as Arri) is
       initialised from a container reader (such as Mxf Reader). In that situation the
       container sepecific settings are isolated from the codec specific settings.
       
       Subclasses should implement codec_knobs() just as an Op would.
     */
    class DDImage_API CodecReaderFormat : public ReaderFormat, public CodecHandler
    {
    public:
      ~CodecReaderFormat() override;
    };
    
    class ReaderImpl;

    /*! \class DD::Image::Reader
       Reads data from a single image file (or other image source).

       The Read creates one of these for each file it reads. Each
       different file type is a different subclass. Each subclass defines
       a Reader_Description object, all of these are searched to figure
       out what subclass to use.

       For most image file types you will want to use the FileReader
       subclass, which provides multi-threaded access to a Unix file.

       The base class for Reader both provides default implementations and
       serves as the "NullReader" that is used when the type cannot be
       figured out.
     */
    class DDImage_API Reader
    {
      // Be sure to change this symbol if the ABI changes.
  #define DDImage_Reader_ABI abi10_0_1
      virtual int DDImage_Reader_ABI();

      friend class Read;
      LUT* make_lut();

      /// noncopy and nonassign
      Reader& operator=(const Reader&);
      Reader(const Reader &);

    protected:

      Read* iop;

      LUT* lut_;
      Iop::Info info_;

      /// This should be overriden by videosequence file formats who
      /// wish to set frame-varying metadata.  Set the metadata from
      /// this, rather than in the constructor, which would be
      /// used for other metadata.
      virtual void prefetchMetaData() { }

      virtual const MetaData::Bundle& fetchMetaData(const char* key);

      enum PlanarPreference
      {
        ePlanarNever,
        ePlanarAllow,
        ePlanarAlways
      };
      
      /**
       * Return a value to indicate the Reader's preference as to 
       * planar access.
       *
       * It can return ePlanarNever if it never wants fetchPlane() 
       * to be called; ePlanarAlways if it wants engine() to be called.
       *
       * Otherwise, it should return ePlanarAllow to allow both planar
       * access or row-based access depending upon what the using code
       * thinks appropriate (for example single Row fetching will use engine()
       * but GeneralTile would use fetchPlane)
       */
      virtual PlanarPreference planarPreference() const
      {
        return ePlanarNever;
      }

      /**
       * This should be overriden by subclasses if they want striping.
       */
      virtual bool useStripes() const
      {
        return false;
      }

      /**
       * This should be overriden by subclasses to return the desired stripe
       * height.
       */
      virtual size_t stripeHeight() const
      {
        return info_.h();
      }

      /**
       * Override this if you want your planes to be different to the default.
       *
       * By default Nuke will try and decode all the channels at once
       */
      virtual PlanarI::PlaneID getPlaneFromChannel(Channel chan)
      {
        return info_.channels();
      }

      /*!
         This is called before the first call to engine(). It should do any
         expensive setup that is not needed for setting the info(), such as
         reading in pointer tables. If this is a movie file format you should
         check frame() and prepare to read in that frame number.
       */
      virtual void open();

      /*!
         Read a line from the file and convert it to floats.

         Warning: \a row.x may be greater than zero, \a row.r may be less
         than the image width, and \a channels may be less than the number of
         channels.  Usually uncompression code is difficult to write with
         such clipping. To expand the size of the buffers in the row to the
         full width, and to read all the channel, do this:

         \code
         row.range(0, width());
         for (int z = 0; z < 4; z++)
          read_channel(y, z, row.writable(z));
         \endcode

         The floating point buffer returned by row.writable() may be used
         with a cast as a temporary buffer to store 8 or 16 bit data.
       */
      virtual void engine(int y, int x, int r, ChannelMask, Row &);

      /*!
       * If you override planarPreference to return anything other than 
       * ePlanarNever, you should implement this
       */
      virtual void fetchPlane(ImagePlane& imagePlane);

      /*!
         Convert bytes to floating point.

         \a from should point at a set of \a W bytes, spaced \a delta apart.
         These are converted and placed into \a to (1 apart).

         \a z is the channel number. If \a z >= 3 then linear (divide by 255)
         conversion is done.

         Otherwise the lut() is called to do a normal conversion.

         If premult() is on and \a alpha is not null, it should point at an
         array of \a W bytes for an alpha channel, spaced \a delta
         apart. The lut() is then called to do an unpremult-convert of the
         values.
       */
      void from_byte(Channel z, float*, const uchar*, const uchar*, int W, int delta = 1);

      /*!
         Same as from_byte() but the source data is an array of shorts in the
         range 0 to (2<<bits)-1.
       */
      void from_short(Channel z, float*, const U16*, const U16*, int W, int bits, int delta = 1);

      /*!
         Same as from_byte() but the source is floating point data.
         Linear conversion will leave the numbers unchanged.
       */
      void from_float(Channel z, float*, const float*, const float*, int W, int delta = 1);

      /*!
         Set all the fields in the info_ correctly for an image file
         that is the given \a width and \a height in pixels and has
         \a depth channels of color data. This sets the bounding box
         to 0,0,width,height and sets the channels so the first \a depth
         bits are turned on. This will try to use the Read::expected_format()
         if it is the right size (so user choices for pixel
         aspect ratio and image area are preserved), otherwise it will find
         or generate a new format that matches.

         The option \a pixel_aspect value forces the pixel aspect to be the given
         value. Zero means that any format that matches the width & height
         is used, and the pixel aspect defined for that is used.

         If your image file has more information such as an actual bounding
         box or real channel assignments, you should modify the fields of
         info_ directly after (or instead of) calling this.
       */
      void set_info(int width, int height, int depth, double aspect = 0);

    public:

      /*! Subclasses should have constructors that match
         Reader_Description::constructor. */
      Reader(Read * iop);

      /*!
         The destructor must close any files (even though the Read may
         have opened them).
       */
      virtual ~Reader();

      virtual bool supports_stereo() const { return false; }
      virtual bool fileStereo() const { return false; }
      virtual bool videosequence() const { return false; }

      //! Rows must be padded to be multiples of this value, in bytes. 0 means no padding.
      int rowPadMultiple() const noexcept;
      //! Rows must be padded to be multiples of this value, in bytes. 0 means no padding.
      void setRowPadMultiple(int rowPadMultiple) noexcept;

      //! Allows a reader to check directly if its op has been aborted
      bool aborted() const;
      
      const Iop::Info& info() const { return info_; }
      int x() const { return info_.x(); }
      int y() const { return info_.y(); }
      int r() const { return info_.r(); }
      int t() const { return info_.t(); }
      int w() const { return info_.w(); }
      int h() const { return info_.h(); }
      bool is_constant() const { return info_.is_constant(); }
      int clampx(int x) const { return info_.clampx(x); }
      int clampy(int y) const { return info_.clampy(y); }
      const Format& format() const { return info_.format(); }
      const Format& full_size_format() const { return info_.full_size_format(); }
      int ydirection() const { return info_.ydirection(); }
      ChannelMask channels() const { return info_.channels(); }
      int first_frame() const { return info_.first_frame(); }
      int last_frame() const { return info_.last_frame(); }

      /*! Returns the width value passed to iop->set_info() */
      int width() const { return format().width(); }

      /*! Returns the height value passed to iop->set_info() */
      int height() const { return format().height(); }

      /*! The filename (this is set before frame() is called) */
      const char* filename() const;

      /*! The frame number from the Read, rounded to an integer.
         If you want the floating point value or the shutter value you
         should use iop->frame() and iop->shutter(). */
      int frame() const;

      /*! The premult() setting from the Read */
      bool premult() const;

      /*! Calls Iop::channel(), turn a name into a channel number assignment. */
      Channel channel(const char* name) { return DD::Image::Iop::channel(name); }
      /*! Calls Iop::channel_name(), turn channel number into name */
      const char* channel_name(Channel c) { return DD::Image::Iop::channel_name(c); }

      /*! Change the format. Use one with the same width and height or it
         will screw up the width() and height() methods which most file
         readers rely on. */
      void format(const Format& f) { info_.format(f); }
      /*! Change the full-size format. */
      void full_size_format(const Format& f) { info_.full_size_format(f); }

      //! Set LUT. This is virtual to allow override in movReader
      //! which performs additional processing
      virtual void setLUT(LUT* lut) { lut_ = lut; }

      //! Get LUT. This is virtual to allow override in movReader
      //! which performs additional processing
      virtual LUT* lut() { return lut_; }

      /*! Overrides lut() with a specific colorspace name (if non-empty). 
          If the colorspace name isn't found the Node will error and the 
          user will have to choose an acceptable alternative.
       */
      virtual std::string getDefaultColorspace() const { return ""; }

      //! Get private implementation
      ReaderImpl* getImpl() { return _pImpl; }
      ReaderImpl* getImpl() const { return _pImpl; }


#if defined(__BIG_ENDIAN__) && __BIG_ENDIAN__
      static U32 lsb32(const unsigned char* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24); }
      static U32 msb32(const unsigned char* p) { return *(U32*)p; }
      static U16 lsb16(const unsigned char* p) { return p[0] | (p[1] << 8); }
      static U16 msb16(const unsigned char* p) { return *(U16*)p; }
  #else
      /*! Get a 32-bit value stored low byte first in a buffer */
      static U32 lsb32(const unsigned char* p) { return *(U32*)p; }
      /*! Get a 32-bit value stored high byte first in a buffer */
      static U32 msb32(const unsigned char* p) { return p[3] | (p[2] << 8) | (p[1] << 16) | (p[0] << 24); }
      /*! Get a 16-bit value stored low byte first in a buffer */
      static U16 lsb16(const unsigned char* p) { return *(U16*)p; }
      /*! Get a 16-bit value stored high byte first in a buffer */
      static U16 msb16(const unsigned char* p) { return p[1] | (p[0] << 8); }
  #endif

      /*! For convenience and speed when reading file formats defined by
          C data structures, you can use this to flip adjacent locations
          of the same type into machine alignment.  This should only be
          done to lock() data if you are sure you will only do this once
          and that no other thread will attempt to do it at the same time.
       */
      static void flip(U16* p, unsigned n);

      /*! Same as previous flip but for an array of 32-bit and 64-bit objects. */
      static void flip(U32* p, unsigned n);
      static void flip(U64* p, unsigned n);

 #if defined(__BIG_ENDIAN__) && __BIG_ENDIAN__
      static void fromlsb(U64* p, unsigned n) { flip(p, n); }
      static void frommsb(U64* p, unsigned n) {}
      static void fromlsb(U32* p, unsigned n) { flip(p, n); }
      static void frommsb(U32* p, unsigned n) {}
      static void fromlsb(U16* p, unsigned n) { flip(p, n); }
      static void frommsb(U16* p, unsigned n) {}
  #else
      /*! Calls flip() if necessary to convert an array of 64 bit values
         stored low-byte first so that they can be read by your machine. */
      static void fromlsb(U64* p, unsigned n) {}
      /*! Calls flip() if necessary to convert an array of 64 bit values
         stored high-byte first so that they can be read by your machine. */
      static void frommsb(U64* p, unsigned n) { flip(p, n); }
      /*! Calls flip() if necessary to convert an array of 32 bit values
         stored low-byte first so that they can be read by your machine. */
      static void fromlsb(U32* p, unsigned n) {}
      /*! Calls flip() if necessary to convert an array of 32 bit values
         stored high-byte first so that they can be read by your machine. */
      static void frommsb(U32* p, unsigned n) { flip(p, n); }
      /*! Calls flip() if necessary to convert an array of 16 bit values
         stored low-byte first so that they can be read by your machine. */
      static void fromlsb(U16* p, unsigned n) {}
      /*! Calls flip() if necessary to convert an array of 16 bit values
         stored high-byte first so that they can be read by your machine. */
      static void frommsb(U16* p, unsigned n) { flip(p, n); }
  #endif
      
      static void setInPreviewMode(bool preview);
      static bool inPreviewMode();
      
#     include "DDImage/ReaderExtensions/ReaderExtensions.h"

      /*! For internal use only.
          If the reader supports audio decoding, it can implement this. */
      virtual AudioReaderI* audioReader();

      /*! \class DD::Image::Reader::Description

         This structure describes a subclass of Reader. The constructor
         builds these into a list that Read operators search to find a way
         to read a filename given to them.

         Sample code for a plugin that defines a Reader:

         \code
         class MyReader : public FileReader {
         static const Description d;
         ...
         };

         static Reader* build(Read* r, int fd, const unsigned char* b, int n) {
           return new MyReader(r, fd, b, n);
         }

         static bool test(int fd, const unsigned char *block, int) {
           return (block[0] == MY_MAGIC_BYTE);
         }

         const Reader::Description MyReader::d("myf\0", "my file format", build, test);
         \endcode
       */
      class DDImage_API Description : public DD::Image::ReaderDescription
      {
        static void add(DD::Image::Description*);
      public:
        /*! Make an instance of the reader.

           If test is not null then \a fd is an opened file, it is
           seeked to \a bufsize, and \a buf contains the first
           \a bufsize bytes of the file. You can use this data to
           store stuff into the info_ (however you cannot report
           error messages and filename() does not work yet). */
        typedef Reader* (*ReaderConstructor)(Read*, int fd, const unsigned char* buf, int bufsize);

        ReaderConstructor constructor;

        typedef ReaderFormat* (*ReaderFormatConstructor)(Read*);

        ReaderFormatConstructor format_constructor;

        /*! If this pointer is non-null, it indicates a named Unix file is
           used to store the image.  In this case Read will open the file
           and call this function.  If it returns non-zero then the constructor
           will be called.  This allows Read to poll all the Readers to
           see which one can read an unknown file format.

           If this is null then zero is passed as the fd to the constructors.

           \a buf points at the first \a bufsize characters of the file.
           \a bufsize is at least 512.  If necessary you can mess with the
           \a fd, but be sure to seek it back to \a bufsize.
         */
        typedef bool (*Tester)(int fd, const unsigned char* buf, int bufsize);

        Tester test;

        /*! Constructor that fills all fields and allows a license check. */
        Description(const char* n, const char* l, ReaderConstructor c, Tester t = nullptr,
                    License * f = nullptr) :
          ReaderDescription(n, l), constructor(c), test(t) { license = f;
                                                        ctor(add); }

        /*! Constructor that fills all fields and allows a license check. */
        Description(const char* n, const char* l, ReaderConstructor c, Tester t,
                    ReaderFormatConstructor format, License * f = nullptr) :
          ReaderDescription(n, l), constructor(c), format_constructor(format), test(t) { license = f;
                                                                                    ctor(add); }

        /*! Simpler constructor that sets names and label to same value. */
        Description(const char* n, ReaderConstructor c, Tester t = nullptr, License * f = nullptr) :
          ReaderDescription(n, n), constructor(c), test(t) { license = f;
                                                        ctor(add); }

        /*! Constructor that fills all fields and allows a license check. */
        Description(const char* n, ReaderConstructor c, Tester t,
                    ReaderFormatConstructor format, License * f = nullptr) :
          ReaderDescription(n, n), constructor(c), format_constructor(format), test(t) { license = f;
                                                                                    ctor(add); }

        /*!
           Return the i'th Description known about, or return a null pointer
           for the last one.
         */
        static const Description* find(int i);

        /*!
           Search all the defined Descriptions for one whose name
           matches the passed string, or return NULL if none. This will also
           try to plugin_load("xyzReader") (where xyz is \a id) in order
           to find external file reader code.
         */
        static const Description* find(const char* name);
      };

    private:
      ReaderImpl* _pImpl; //< private implementation of Reader objects

    };

  }
}

#endif

#include "DDImage/Read.h"

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
