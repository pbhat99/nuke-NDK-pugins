// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepReader_h
#define DDImage_DeepReader_h

#include "DDImage/DDImage_API.h"

#include "DDImage/DeepInfo.h"
#include "DDImage/MetaData.h"
#include "DDImage/ReaderDescription.h"
#include "DDImage/FileOp.h"

namespace DD {
  namespace Image {

    class DeepReaderImpl;
    class DeepOutputPlane;
    class Op;

    /*
     * This class is used to provide file-format specific 
     * knobs for DeepReaders.  It is different from DeepReader
     * as DeepReader needs to be per-file, whereas a DeepReaderFormat
     * applies for the entire deep sequence.
     */
    class DDImage_API DeepReaderFormat : public FileHandler
    {
    public:
      ~DeepReaderFormat() override;
      virtual void append(Hash& hash) = 0;
    };

    class DDImage_API DeepReaderOwner
    {
    protected:
      virtual ~DeepReaderOwner();

    public:
      virtual FileHandler* handler() const = 0;

      /// preferred format to use (assuming the bbox matches)
      virtual const Format* formatHint() const = 0;

      /// the context to be used for scaling
      virtual const DD::Image::OutputContext& readerOutputContext() const = 0;

      /// calls internalError on the fileop
      virtual void readerInternalError(const char* fmt, ...) = 0;

      virtual DD::Image::Op* op() = 0;
    };

    /**
     * Subclasses of these are implemented by file-format specific
     * deep image readers.  This interface is not yet finalised.
     */
    class DDImage_API DeepReader
    {
      std::unique_ptr<DeepReaderImpl> _pImpl;
      friend class DeepRead;

    protected:
      DeepReaderOwner* _owner;
      Op* _op;

      DeepInfo _deepInfo;
      MetaData::Bundle _metaData;

      /// set the _deepInfo to be a box from (0, 0) - (width, height), with matching formats,
      /// and set the channels.
      void setInfo(int width,
                   int height,
                   const DD::Image::OutputContext& oc,
                   const DD::Image::ChannelSet& channels,
                   double aspect = 0.0);

    public:

      const DeepInfo& deepInfo()
      {
        return _deepInfo;
      }
      
      const MetaData::Bundle& metaData() const
      {
        return _metaData;
      }
      
      DeepReader(DeepReaderOwner* op);

      virtual bool doDeepEngine(DD::Image::Box, const DD::Image::ChannelSet&, DD::Image::DeepOutputPlane&) = 0;

      virtual ~DeepReader();

      class DDImage_API Description : public DD::Image::ReaderDescription
      {
        static void add(DD::Image::Description*);
      public:
        typedef DeepReader* (*Constructor)(DeepReaderOwner*, const std::string& filename);

        /*! Make an instance of the reader. */
        Constructor constructor;

        typedef DeepReaderFormat* (*FormatConstructor)(DeepReaderOwner*);

        /*! Make an instance of the reader format */
        FormatConstructor formatConstructor;

        /*! Constructor that fills all fields and allows a license check. */
        Description(const char* n, const char* l, Constructor c, FormatConstructor formatConstructor = nullptr,
                    License * f = nullptr) :
          ReaderDescription(n, l), constructor(c), formatConstructor(formatConstructor) { license = f;
	    ctor(add); }

        /*! Simpler constructor that sets names and label to same value. */
        Description(const char* n, Constructor c, FormatConstructor formatConstructor = nullptr, License * f = nullptr) :
          ReaderDescription(n, n), constructor(c), formatConstructor(formatConstructor) { license = f;
	    ctor(add); }

        static const Description* find(int i);
        static const Description* find(const char* name);
      };
    };
  }
}

#endif
