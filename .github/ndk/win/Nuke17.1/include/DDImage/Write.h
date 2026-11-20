// Write.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Write_h
#define DDImage_Write_h

#include "DDImage/Iop.h"
#include "DDImage/Executable.h"
#include "DDImage/Read.h"
#include "DDImage/FileIop.h"

#include <unordered_set>

namespace DD
{
  namespace Image
  {

    class Writer;
    class MultiInterest;
    class WriteImpl;

    /*! \class DD::Image::Write
        This is a base class for a file writer.

        All the work is done by the various subclasses of Writer.
        Write just chooses what subclass to instantiate and returns the
        output from it, and correctly destroys and instantiates new ones
        as needed.

        Calling Iop::execute() initiates writing the file.
     */
    class DDImage_API Write : public FileIop, public Executable
    {
      friend class WriteImpl;

      //! Call to put the invisible iop on the write's input (iop must be valid).
      //! Returns false iff validation failed and the execution should stop.
      bool activateInvisibleIop();

      //! Call to remove the invisible iop from the write's input (iop must be valid)
      void deactivateInvisibleIop();

      Writer* writer;
      const void* writer_type; // points at a Writer::Description

      Op* op() override { return this; }

      FileHandler* handler() const override;

      const char* file_type_knob;
      const char* filename_;
      ChannelSet channels_;
      int colorspace_;
      bool raw_;
      bool premult_;
      MultiInterest* _interests;
      const char* fname_;

      LUT* _uiLUT;

      LUT* chosenLUT() const;

      void build_writer() const;

      /// views to write out according to the knobs
      std::set<int> _views;

      /// read from file rather than pass through
      bool _reading;
      /// what to do when a read error occurs
      enum { eError, eBlack, eCheckerBoard, eReadInput } _onReadError;
      int version; // for the reload button
      void reload(); // increments the version knob

      Read* _reader;
      OpTree* _pReaderTree;
      bool _readError;
      bool _readerShouldCheckHash;

      bool _executing;

      // Topdown rendering is called from the inputnget function. Since there will be multiple calls to 
      // inputnget during the writer execution cycle, this is used to determine if topdown rendering was
      // already run for a given input by checking whether an input index is present in the set.
      // Note that this set is cleared after the execution cycle is complete.
      std::unordered_set<int> _topdownComplete{};

      // Suggest whether or not the upstream reads should read frames line by line or all the lines in one go.
      // This updates the lineReadPreference of the OutputContext used by Read nodes higher up the tree and will
      // only have an effect on formats which support reading all lines.
      bool _readAllLines;

      Iop*        _pInvisibleIop;     //!< When valid, this iop is invisibly applied immediately prior to writing

      static bool _sWatermarkingEnabled; //!< When set, a watermark is created on construction as the invisible iop

      //!< Return the op that validate/request/engine ought to passthrough to, based on the outputContext.
      Iop& passthrough();

      int getPassthroughIndex() const;  //!< Returns the input index used by passthrough()

      void openReader();
      void closeReader();

      WriteImpl* _pImpl; //< private implementation

    protected:


      /*! Modify the info to show what is actually written to the file. This
          assumes the normal behavior that the file clips to the format and
          writes the channels packed together as the first N channels. It
          assumes that if you have set the mask to "all" then you
          are writing a file format that preserves channel names.
       */
      void _validate(bool) override;

      /*! Shuffles the channels according to which ones will be written. */
      void _request(int x, int y, int r, int t, ChannelMask, int count) override;

      /*! Shuffles the channels according to which ones will be written. */
      void engine(int y, int x, int r, ChannelMask, Row &) override;

      //!< Needed to allow different contexts to have different passthroughs
      void append(DD::Image::Hash& hash) override;

    public:

      Write(Node * node = nullptr);
      ~Write() override;

      Executable* executable() override;

      /*!
         This is not true if the input is not connected (to avoid writing black
         frames but without producing an error) and is not true if "reading" is
         turned on.
       */
      bool isWrite() const override;

      /*!
         Write the current frame. The Writer object is created if it does
         not exist, and then execute() is called on it. If the writer does
         not claim to be a movie file, finish() is called on it.
       */
      void execute() override;
      void beginExecuting() override;

      /*!
         Calls finish() on the Writer if it is writing a movie file.
         If reading is turned on it acts like user hit the Reload button.
       */
      void endExecuting() override;

      int minimum_inputs() const override { return 1; }
      int maximum_inputs() const override { return 1; }


      /// Returns WriteMany if the Writer object asks for more than one
      /// in Writer::split_input(), indicating that (most likely) more than
      /// one view is wanted.
      /// Returns WriteOne otherwise, indicating it is a waste of time to
      /// call this with different views. Nuke will detect if the filename
      /// changes when the view changes and will call this multiple times with
      /// different filenames, otherwise complain if the user asks for more
      /// than one view to execute.
      ExecMode execViewMode() override;

      /// Returns WriteMany if the Writer object returns true for movie().
      /// Returns WriteOne otherwise, indicating it is a waste of time to
      /// call this with different frames. Nuke will detect if the filename
      /// changes when the frame changes and will call this multiple times with
      /// different filenames, otherwise complain if the user asks for more
      /// than one frame to execute.
      ExecMode execFrameMode() override;
      const std::set<int>& views() override { return _views; }

      /*! Calls Writer::split_input(). */
      int split_input(int i) const override;

      /*! Calls Writer::inputContext() with the offset number. */
      const DD::Image::OutputContext& inputContext(int, int, OutputContext& c) const override;

      const char* filename() const override { return fname_ ? fname_ : filename_; }
      const char* getFilename() const override { return filename(); }
      const char* fname() const { return fname_; }
      void filename(const char* f) { filename_ = f; }
      static const char* const* file_types();


      /*!
         Return the type of file this will write. This is set by set_file_type()
         or set_file_type_from_filename() or by validate() if neither of these
         is done. This is actually the null-sepearted list of names from the
         DD::Image::Writer::Description object.
       */
      const char* file_type() const override;

      /*!
         Set the type of file the Writer will write. The type is a string like
         "cin" or "jpeg". Returns true if the new type is different than the
         previous one. Notice that if the type changes all settings on
         Writer-specific knobs are lost.

         If this type is not found or is zero then file_type() is set to zero.
         If the node is then executed it will attempt to figure out the type
         from the filename, and if that fails it will cause an error.

         file_types() returns a list of available names. You may also be able
         to set other names like "sgi16" that are not in this list, these
         typically initialize some knobs to non-default values.
       */
      bool set_file_type(const char*);

      /*!
         If the filename has "name:" near the start, try using that to set
         the file type. If that fails, try using the file extension.
         This is automatically done by execute() and validate() if this
         or set_file_type() has not been called.
       */
      bool set_file_type_from_filename(Knob* knob = nullptr);

      ChannelMask channels() const { return channels_; }
      void channels(ChannelMask m) { channels_ = m; }
      void depth(int d) { channels_ = depth2mask(d); }

      /*! Does Writer::depth(). Current implementation returns the number
          of channels turned on in the selector and in the input. */
      int depth() const;
      int num_channels() const { return depth(); }

      /*! Does Writer::channel() */
      Channel channel_written_to(int i) const;

      bool raw() const { return raw_; }
      void raw(bool v) { raw_ = v; }

      bool premult() const { return premult_; }
      void premult(bool v) { premult_ = v; }

      bool reading() const { return _reading; }
      const MetaData::Bundle& _fetchMetaData(const char* key) override;

      void input0get(int y, int x, int r, ChannelMask mask, Row& row) { return inputnget(0, y, x, r, mask, row); }

      /*! Get line from the input, and launch multiple threads to read other
         lines so they are ready to be returned when this is called next time. */
      void inputnget(int n, int y, int x, int r, ChannelMask, Row &, float fracCPU);
      void inputnget(int n, int y, int x, int r, ChannelMask, Row &);


      /*! Calls Op::error(), but prefixes "filename: " to the message. */
      void internalError(const char* fmt, ...);

      void filetype_knobs(Knob_Callback f);
      void knobs(Knob_Callback) override;
      int knob_changed(Knob*) override;

      const char* Class() const override;
      const char* node_help() const override;
      static const Description d;
      unsigned node_color() const override;

      Hash getHashOfInputs();

      void status(double f) { progressFraction(f, Op::StatusUseCallback); } // for back-compatibility

      void setOutputContext(const OutputContext& lrContext) override;

      // Expose access to the Writer this Write object owns so that it's
      // possible to use the Writer outside the operator framework.
      //
      // CAUTION: There's no guarantee the Write has a valid Writer
      Writer* getWriter() { return writer; }

      /*!
       * Cleans up the writer when the dag is detached.
       */
      void detach() override;
      //! Use this to set a watermark iop to be applied immediately before all writes, without showing it on the DAG.
      //! There is no function to turn this off again, as there should be no need.
      static void enableWatermarking();

      /*!
       * Update the colorspace knob so that the 'default' entry contains text
       * specifying what that default currently is
       */
      void updateDefaultLUT();

      //! Return hints to control when and how this op will be evaluated by the top-down system.
      OpHints opHints() const override;

      bool can_link() const override { return false; }
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
