#ifdef FN_OS_WINDOWS

#ifndef DDImage_IO_F_Stream
#define DDImage_IO_F_Stream

#include "DDImage/DDImage_API.h"
#include "DDImage/IOStreamBuffer.h"
#include <stdio.h>
#include <sstream>
#include <windows.h>
#include <assert.h>

namespace DD
{
  namespace Image
  {
    class DDImage_API IOFStream : public std::ostream
    {
      public:
        IOFStream( const char* filename );
        IOFStream( const wchar_t* filename );
        ~IOFStream();

        bool good() const;

        bool open( const char* filename );
        bool open( const wchar_t*  filename );

        void close();

        void flush();

      private:
        template <typename T> bool openStreamBuffer(T filename);
        IOStreamBuffer _streamBuffer;
    };
  }
}


#endif // DDImage_IO_F_Stream

#endif // FN_OS_WINDOWS
