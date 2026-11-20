/*
 * FileStream.h
 *
 * Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.
 *
 * Provides a cache-like system allowing caching to be diverted to use specified file locations instead of ImageCache.
 * Main usage is by the ParticleCache node.
 *
 */

#ifndef File_Stream_H
#define File_Stream_H

#include <fstream>
#include "DDImage/ImageCache.h"

namespace DD
{
  namespace Image
  {
    class FileStreamRead : public DD::Image::ImageCacheReadI
    {
    public:
      FileStreamRead(const std::string& filename);

      void   close() override;
      bool   error();
      size_t read(void* to, size_t n) override;

      void   seek(size_t to) override;
      void   seekRelative(off_t offset) override;

      int    uniqueID() const override;

    protected:
      std::ifstream _file;
    };

    ////////////////////////////////////////////////////////////////
    class FileStreamWrite : public DD::Image::ImageCacheWriteI
    {
    public:

      FileStreamWrite(const std::string& filename);

      void   close() override;
      bool   error();
      size_t write(const void* data, size_t n) override;


    protected:
      std::ofstream _file;
    };
  }
}
#endif

