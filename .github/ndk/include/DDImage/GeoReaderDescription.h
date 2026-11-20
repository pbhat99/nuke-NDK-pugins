// GeoReader.h
// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_GeoReaderDescription_h
#define DDImage_GeoReaderDescription_h

#include "DDImage/ReaderDescription.h"
#include "DDImage/Reader.h"

#include <unordered_map>

#define USE_DEPRECATED_READ_GEO_CTOR 1

namespace DD
{
  namespace Image
  {

class ReadGeo;
class GeoReader;
class Description;
class GeoReaderFormat;

/*! 
    This structure describes a subclass of GeoReader. The constructor
    builds these into a list that ReadGeo operators search to find a way
    to read a filename given to them.

    Sample code for a plugin that defines a GeoReader:

    \code
   class MyReader : public GeoReader {
   static const Description d;
   ...
   };

   static GeoReader* build(int fd, const unsigned char* b, int n) {
   return new MyReader(fd, b, n);
   }

   static bool test(int fd, const unsigned char *block, int) {
   return (block[0] == MY_MAGIC_BYTE);
   }

   const GeoReader::Description MyReader::d("myf\0", "my file format", build, test);
    \endcode
 */
class DDImage_API GeoDescription : public DD::Image::ReaderDescription
{
public:
  typedef bool (* FilenameTester)(const std::string& filename);
private:
  static void add(DD::Image::Description*);
  static void addFilenameTester(const DD::Image::GeoDescription*, FilenameTester);

  /*! A map of descriptions to their filename tester functions. */
  static std::unordered_map<const DD::Image::GeoDescription*, FilenameTester> _descriptionTesters;
public:
  typedef GeoReader* (*Constructor)(ReadGeo *, int fd, const unsigned char* buf, int bufsize);
  typedef bool (* Tester)(int fd, const unsigned char* buf, int bufsize);

  /*! A list of pointers to all possible Descriptions to be
      used for instantiation of geo plugins. */
  static DD::Image::Description::DescriptionList _collection;

  /*! Make an instance of the reader.

     If test is not null then \a fd is an opened file, it is
     seeked to \a bufsize, and \a buf contains the first
     \a bufsize bytes of the file. You can use this data to
     store stuff into the info_ (however you cannot report
     error messages and filename() does not work yet). */
  Constructor _constructor;

  typedef GeoReaderFormat* (*GeoReaderFormatConstructor)(ReadGeo*);

  GeoReaderFormatConstructor _format_constructor;

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
  Tester _test;

  /**
   * whether or not the file descriptor is really needed (defaults to true)
   * 
   * if false then any open FD on the file will be closed before build is called,
   * and -1 passed in
   */
  bool _needFd;

#if USE_DEPRECATED_READ_GEO_CTOR
  /*! Constructor that fills all fields and allows a license check. */
  GeoDescription(const char* n, const char* l, Constructor c, Tester t = nullptr, License * f = nullptr, bool needFd = true)
    : ReaderDescription(n, l)
    , _constructor(c)
    , _format_constructor(nullptr)
    , _test(t)
    , _needFd(needFd)
  {
    license = f;
    ctor(add);
  }

  /*! Simpler constructor that sets names and label to same value. */
  GeoDescription(const char* n, Constructor c, Tester t = nullptr, License * f = nullptr, bool needFd = true)
    : ReaderDescription(n, n)
    , _constructor(c)
    , _format_constructor(nullptr)
    , _test(t)
    , _needFd(needFd)
  {
    license = f;
    ctor(add);
  }

#endif // USE_DEPRECATED_READ_GEO_CTOR

  /*! Simpler constructor that sets names and label to same value. */
  GeoDescription(const char* n, Constructor c, GeoReaderFormatConstructor fmtConstructor, Tester t = nullptr, License * f = nullptr, bool needFd = true)
    : ReaderDescription(n, n)
    , _constructor(c)
    , _format_constructor(fmtConstructor)
    , _test(t)
    , _needFd(needFd)
  {
    license = f;
    ctor(add);
  }

  /*! This constructor takes a FilenameTester parameter.
   * FilenameTester is a function pointer to test a file's
   * compatibility with the reader using the given filename.
   * This is an alternative to using GeoDescription::Tester which
   * requires a file descriptor.
  */
  GeoDescription(const char* n,
                 Constructor c,
                 GeoReaderFormatConstructor fmtConstructor,
                 FilenameTester ft,
                 License * f = nullptr,
                 bool needFd = true)
    : ReaderDescription(n, n)
    , _constructor(c)
    , _format_constructor(fmtConstructor)
    , _test(nullptr)
    , _needFd(needFd)
  {
    license = f;
    ctor(add);
    addFilenameTester(this, ft);
  }

  /*! Return the i'th GeoDescription known about, or return a null pointer
   * for the last one.
   */
  static const GeoDescription* find(int i);

  /*! Search all the defined GeoDescriptions for one whose name
   * matches the passed string, or return null pointer if none.
   */
  static const GeoDescription* find(const char* name);

  /*! Search all the defined FilenameTesters for one which is
   * related to the passed /p desc, or return null pointer if none.
   */
  static FilenameTester findFilenameTester(const GeoDescription* desc);
};


  } // namespace Image
} // namespace DD

#endif // DDImage_GeoReaderDescription_h
