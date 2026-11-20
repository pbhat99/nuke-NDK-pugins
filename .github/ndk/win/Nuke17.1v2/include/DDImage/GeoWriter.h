// GeoWriter.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_GeoWriter_h
#define DDImage_GeoWriter_h

#include "DDImage/WriteGeo.h"

namespace DD
{
  namespace Image
  {

    /*! \relates FileWriter
       This type may need to be replaced on 64-bit file systems. */
    typedef unsigned long FILE_OFFSET;

    /*! \class DD::Image::GeoWriter
        Writes data to a geometry file.

        The WriteGeo creates one of these for every different filename it
        writes. Each different file type is a different subclass. Each
        subclass defines a Writer_Description object, all of these are
        searched to figure out what subclass to use.

        The implementation of the base class is in WriteGeo.C
     */
    class DDImage_API GeoWriter : public FileHandler
    {
      friend class WriteGeo;
      int frame_;

    protected:
      WriteGeo* geo; //!< Points at the WriteGeo that created this.

      /*! Frame number to write. */
      int frame() const { return frame_; }

      /*! The GeoOp whose output should be written. */
      GeoOp* input0() { return geo->input0(); }

      /*! The filename to write. */
      const char* filename() const { return geo->filename(); }

      /*! Call status to display percentage of the current frame done. */
      void status(float amount) const { geo->progressFraction(amount); }

      void* file; //!< Actually a FILE* but I don't want the <stdio.h> header!
      FILE_OFFSET offset; //!< Current offset into file

      /*! Open the output file.  This file is created with a temporary name
          so that the original file is not replaced until the write is
          successful.  This returns false and calls geo->error() if it
          can't open the file.
       */
      bool open();

      /*! Close the output file.  If there have not been any errors this will
         mv the temporary file to the correct place. */
      bool close();

      /*! Write \a length bytes to the file.  Call geo->error() on any errors
          and returns false.  Returns true on success.
       */
      bool write(const void*, FILE_OFFSET);

      /*! Same as "seek(off) && write(buf, length)", this writes \a length
          bytes to the file at \a offset and leaves the pointer after that. */
      bool write(FILE_OFFSET, const void*, FILE_OFFSET);

      /*! Move the write pointer to \a offset.  This is done efficiently if
          the pointer is already at that point.  Calls geo->error()
          on any errors and returns false.  Returns true on success. */
      bool seek(FILE_OFFSET);

      /*! \fn FILE_OFFSET GeoWriter::tell() const
          Returns where the pointer currently is.
       */
      FILE_OFFSET tell() const { return offset; }

    public:

      /*! This does not open the file. Instead a subclass should call open()
         as the first statement in execute() and then return if open fails. */
      GeoWriter(WriteGeo*);

      /*! Destruction closes the temporary file if execute() did not call
          close(). */
      ~GeoWriter() override;

      /*! Set the frame number to \a n.
       */
      virtual void frame(int);

      /*! Write the data in geo->input0().  The frame number may be retrieved
          with frame() and the filename from filename().  This may call
          geo->error() with any error messages.
       */
      virtual void execute(Scene& scene);

      /*! Return true if the destination can have an animation. If this is true then
          the writer is capable of writing objects with animation, and frame() and
          execute() may be called multiple times to write different objects.
          The default version returns false.
       */
      virtual bool animation() const;

      /*!
         Lets a writer override the Op::split_input() method of the including
         WriteGeo op. This is used by Writers that can write more than one view.
       */
      virtual int split_input(int i) const;

      /*! Override if the writer needs to do something to finish a sequence.
          This is mostly of use by animation writers that don't close out
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

      /*!
         Like the Op::knob_changed() method, this allows the Writers to handle
         changes in their own parameters.
       */
      int knob_changed(Knob* knob) override;
      
      //! Specialised GeoWriter Description
      /*! \class DD::Image::GeoWriter::Description

          Each subclass of GeoWriter should define at least one static instance
          of this structure. The constructor adds itself to a list that the
          WriteGeo operator searches to find the correct type of GeoWriter to use
          to write a given filename.

          Sample code for a plugin:

          \code
         class MyWriter : public GeoWriter {
         static const Description d;
         ...
         };

         static GeoWriter* build() {return new MyWriter();}

         const GeoWriter::Description MyWriter::d("myf\0", "my file type", build);
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
           method frame(n) on the writer should try to open the output file
           and produce any error messages. */
        GeoWriter* (*constructor)(WriteGeo*);

        /*! Constructor that fills all fields and allows a license check. */
        Description(const char* n, const char* l, GeoWriter * (*c)(WriteGeo*),
                    License * f = nullptr) :
          names(n), label(l), constructor(c) { license = f;
                                               ctor(add); }

        /*! Simpler constructor that sets names and label to same value. */
        Description(const char* n, GeoWriter * (*c)(WriteGeo*), License * f = nullptr) :
          names(n), label(n), constructor(c) { license = f;
                                               ctor(add); }


        /*! Return the i'th GeoWriter::Description known about, or return a null pointer
            for the last one. */
        static const Description* find(int i);

        /*! Search all the defined GeoWriter::Descriptions for one whose name
            matches the passed string, or return NULL if none. This will also
            try to plugin_load("xyzWriter") (where xyz is \a id) in order
            to find external file writing code.
         */
        static const Description* find(const char* name);
      };

    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
