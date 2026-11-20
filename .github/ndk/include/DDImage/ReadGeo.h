// ReadGeo.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ReadGeo_h
#define DDImage_ReadGeo_h

#include "DDImage/SourceGeo.h"
#include "DDImage/FileRead.h"
#include "DDImage/FileOp.h"
#include "DDImage/Reader.h"

namespace DD
{
  namespace Image
  {

    class GeoReader;
    class GeoDescription;
    class DDImage_API GeoReaderFormat : public ReaderFormat {};

    /*!
       Returns geometry from the outside world. The geometry is identified
       by the filename (which may just be a string interpreted by one of
       the reader implementations and not an actual file on the disk).

       All the work is done by the various subclasses of GeoReader.
       ReadGeo just chooses what subclass to instantiate and returns the
       output from it, and correctly destroys and instantiates new ones
       as needed.
     */
    class DDImage_API ReadGeo : public SourceGeo,
                                public FileRead,
                                public FileOp
    {
    private:

      const GeoDescription* format_for_name(const char* fname);
      GeoReader* _reader;
      GeoReader* make_reader(const char* fname);
      int reader_frame;

      const char* filename_;
      const char* _file_type;
      int fd_; // << put this in the GeoReader class

      int version;
      Hash filehash;
      const char* fname_;
      int frame_range[2];

      int update_mode_, prev_update_mode;
      GeoReader* full_geometry_reader;
      int full_geometry_frame_;
      char* full_geometry_filename;
      int full_geometry_version;
      Hash full_geometry_filehash;
      GeoReaderFormat* _fileformat;
      const GeoDescription* _reader_type;
      std::string _errorMessage;
    protected:
      // last find() sets this to name of plugin it loaded:
      static const char* _tried_plugin;
      
      void       make_format();

      // local matrix that Axis_Knob fills in
      Matrix4 _local;
      bool _readTextureWCoord;
      Knob* _pAxisKnob;

    protected:

      /*!    validate() will destroy any previous GeoReader and open a new one if
          the filename has changed or the version number has changed (the
          version number is incremented by the refresh button).

          After opening the file, it copies the info_ from the Reader object.
          This will have been initialized with expected_first_frame(),
          expected_last_frame(), expected_format(), and expected_full_size_format(),
          but the Reader subclass may have changed this information.

          For Nuke, this may be called with \a for_real set to false. If
          expected_format() and expected_full_size_format() are not null
          then no attempt is made to open the file and the the previous
          info_ is set to those formats and returned otherwise unchanged.
          This is done to avoid the overhead of opening image files that
          will not be used. Later when data is needed this is called again
          with \a for_real true, the file is really opened and the real info
          is determined.

       */
      void _validate(bool) override;

      //! Apply the concat matrix to all the GeoInfos.
      void geometry_engine(Scene&, GeometryList& out) override;

      /*! Changes geometry hash by the filename.  Use file date as well..?
       */
      void get_geometry_hash() override;

      /*! Calls GeoReader::geometry_engine()
       */
      void create_geometry(Scene&, GeometryList& out) override;

      /*! Call this to close any current file. Besides freeing up resources,
          this can be used to make validate() check for the existence of a new
          file and prevent an error if the dimensions change.
       */
      void _close() override;

      ReadGeo* firstRead()
      {
        return dynamic_cast<ReadGeo*>(firstOp());
      }

      const ReadGeo* firstRead() const
      {
        return dynamic_cast<const ReadGeo*>(firstOp());
      }

    public:

      static const char* kFileKnobName;
      static const char* kReloadKnobName; 
      static const char* kVersionKnobName;

      ReadGeo(Node * node);
      ~ReadGeo() override;

      // FileOp
      Op* op() override { return static_cast<Op*>(this); }
      FileHandler* handler() const override;
      const char* getFilename() const override { return fname();}
      const char* file_type() const override { return _file_type; }
      // ~FileOp

      void close() override
      {
        SourceGeo::close();
      }

      bool can_link() const override { return false; }

      void filename(const char* f) { filename_ = f; }
      const char* filename() const { return fname_ ? fname_ : filename_; }
      virtual const char* fname() const;

      /*! read texture texture w coord */
      virtual bool readTextureWCoord() { return _readTextureWCoord; }

      int update_mode() const { return update_mode_; }
      int full_geometry_frame() const { return full_geometry_frame_; }

      int expected_first_frame() const { return frame_range[0]; }
      void expected_first_frame(int v) { frame_range[0] = v; }
      int expected_last_frame() const { return frame_range[1]; }
      void expected_last_frame(int v) { frame_range[1] = v; }

      void knobs(Knob_Callback) override;
      //! The function will handle the knob changes.
      int knob_changed(Knob* k) override;

      /*! Normally calls Op::error(), but prefixes "filename: " to the message.
       *
       * \note
       * The last error message is preserved and can be obtained via \ref getErrorMessage().
      */
      void internalError(const char* fmt, ...);
      /*! reset the error */
      void clearError() { _errorMessage.clear(); }
      /*! return true if an error is reported */
      bool hasError()const { return !_errorMessage.empty(); }
      /*! Return the last error message reported via internalError().
       * An empty string is returned if the error hasn't been reported or was cleared.
       */
      const std::string& getErrorMessage() const { return _errorMessage; }

      const char* Class() const override;
      const char* node_help() const override;
      static const Description d;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
