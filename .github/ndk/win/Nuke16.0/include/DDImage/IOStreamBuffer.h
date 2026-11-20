#ifdef FN_OS_WINDOWS

#ifndef DDImage_IO_Stream_Buffer_h
#define DDImage_IO_Stream_Buffer_h

#include "DDImage/DDImage_API.h"
#include <sstream>
#include <windows.h>

namespace DD
{
  namespace Image
  {
  
    //! Windows class for writing a buffered stream to a file
    class DDImage_API IOStreamBuffer : public std::basic_streambuf<char, std::char_traits<char> >
    {
      public:
        IOStreamBuffer();
        ~IOStreamBuffer();

        /**
         * attempts to open a file
         * @param filename the filename to be opened
         * @return a flag indicating whether the operation was successfull or not
         */
        bool open( const wchar_t* filename );

        /**
         * attempts to open a file, will attempt to convert the UTF-8 encoded string into a wide char string
         * @param filename the filename to be opened
         * @return a flag indicating whether the operation was successful or not
         */
        bool open( const char* filename );

        /**
         * check whether the file is valid
         * @return a flag indicating whether the _file is valid
         */
        bool good() const;

        virtual std::basic_streambuf<char, std::char_traits<char> >::int_type overflow( std::basic_streambuf<char, std::char_traits<char> >::int_type t );

        void flush();

        void close();

      private:

        HANDLE _file;
        bool _async;
        char _firstBuffer[128 * 1024];
        char _secondBuffer[128 * 1024];
        char* _currentBuffer;
        OVERLAPPED _overlapped;
    };
  }
}

#endif // DDImage_IO_Stream_Buffer_h

#endif // FN_OS_WINDOWS
