// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepWriter_H
#define DDImage_DeepWriter_H

#include "DDImage/FileOp.h"

namespace DD {
  namespace Image {

    class DeepOp;

    /**
     * Helper class for DeepWriter
     */
    class DDImage_API DeepWriterOwner
    {
      friend class DeepWriter;

    public:
      /** return a pointer to the Op */
      virtual Op* op() = 0;
      
      /** return the DeepOp to write out */
      virtual DeepOp* input() = 0;

      /** return the filename to write to */
      virtual const char* filename() const = 0;

      /** return the channels to write */
      virtual const ChannelSet& channels() = 0;

      virtual ~DeepWriterOwner();
    };

    /**
     * Class for plugins that wish to write out deep data.  The DeepWriters
     * are selected by the DeepWrite node.
     */
    class DDImage_API DeepWriter : public FileHandler
    {
      std::string _tempName;

    protected:
      DeepWriterOwner* _owner;

    public:
      DeepWriter(DeepWriterOwner* owner);

      DeepWriter(const DeepWriter& other); /// left unimplemented 
      void operator=(const DeepWriter& other); /// left unimplemented

      /** get the deep op to use as the source of data for the write */
      DeepOp* input();

      /** open a temporary file and return it */
      FILE* openFile();

      /** close the temporary file and rename it to the full version */
      void closeFile(FILE* f);

      /** virtual function to be implemented.  should write out the Deep data
       * available on input() to the filename on _owner->filename().  Can use
       * helper functions openFile() and closeFile() for dealing with temporary
       * filenames.
       */
      virtual void execute() = 0;
      
      /**
       * Class for keeping track of the various available DeepWriters
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

        typedef DeepWriter* (*Constructor)(DeepWriterOwner*);

        /*! Make an instance of the writer.  This cannot fail, instead the
           execute() method should try to open the output file
           and produce any error messages. */
        Constructor constructor;

        /*! Constructor that fills all fields and allows a license check. */
        Description(const char* n, const char* l, Constructor c,
                    License * f = nullptr) :
          names(n), label(l), constructor(c) { license = f;
                                               ctor(add); }

        /*! Simpler constructor that sets names and label to same value. */
        Description(const char* n, Constructor c, License * f = nullptr) :
          names(n), label(n), constructor(c) { license = f;
                                               ctor(add); }

        /*! Return the i'th Writer::Description known about, or return a null pointer
          for the last one. */
        static const Description* find(int i);
      
        /*! Search all the defined DeepWriter::Descriptions for one whose name
          matches the passed string, or return NULL if none. This will also
          try to plugin_load("xyzWriterDeep") (where xyz is \a id) in order
          to find external file writing code.
          
          If this returns null, then plugin_error() will contain an error.
        */
        static const Description* find(const char* name);
      };
    };

  }
}

#endif
