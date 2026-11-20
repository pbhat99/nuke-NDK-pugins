// MetaData.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef METADATA_H
#define METADATA_H

#include <DDImage/DDImage_API.h>

#include "DDImage/Matrix4.h"
#include "DDImage/Deprecation.h"

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <math.h>
#include <memory>
//------------------------------------------------------------------------------
// Forward declarations

namespace Foundry
{
  namespace Property
  {
    class Base;
    class Double;
    class Int;
    class String;
  }
}

//------------------------------------------------------------------------------

namespace DD
{

namespace Image
{

namespace MetaData
{

  static const char* const INPUT_PREFIX = "input/";

  /// the modified time of the actual file
  static const char* const MODIFIED_TIME = "input/mtime";

  /// the creation time of the original file
  static const char* const CREATED_TIME = "input/ctime";

  /// the size (in bytes) of the original file
  static const char* const FILESIZE = "input/filesize";

  /// the full path of the original file
  static const char* const FILENAME = "input/filename";

  /// the file reader used to read in the original file
  static const char* const FILEREADER = "input/filereader";

  /// the timecode.  expected to be in format "00:00:00:00"
  static const char* const TIMECODE = "input/timecode";

  /// the edgecode.
  static const char* const EDGECODE = "input/edgecode";

  /// pixel aspect ratio, as a number
  static const char* const PIXEL_ASPECT = "input/pixel_aspect";

  /// frame rate (in hertz)
  static const char* const FRAME_RATE = "input/frame_rate";

  /// frame rate (in hertz)
  static const char* const SHUTTER_ANGLE = "input/shutterangle";

  /// input frame
  static const char* const FRAME = "input/frame";

  /// width of original file in pixels
  static const char* const WIDTH = "input/width";

  /// height of original file in pixels
  static const char* const HEIGHT = "input/height";

  /// per-channel bit-depth of original file.  for floating
  /// point values can be "half", "float", "double" for 16-bit, 32-bit
  /// and 64-bit floating point numbers respectively.
  static const char* const DEPTH = "input/bitsperchannel";

  static const char* const DEPTH_DOUBLE = "64-bit double float";
  static const char* const DEPTH_FLOAT = "32-bit float";
  static const char* const DEPTH_HALF = "16-bit half float";

  inline std::string DEPTH_FIXED(int bits)
  {
    std::ostringstream os;
    os << bits;
    os << "-bit fixed";
    return os.str();
  }

  static std::string DEPTH_8 = DEPTH_FIXED(8);
  static std::string DEPTH_16 = DEPTH_FIXED(16);
  static std::string DEPTH_32 = DEPTH_FIXED(32);

  /// the name of the program which wrote out the original file
  /// writers need to disregard the
  static const char* const CREATOR = "input/creator";

  /// the creation date of the original file (from within the file, ie the time this was serialised)
  static const char* const FILE_CREATION_TIME = "input/header_ctime";

  /// the project the original file came from
  static const char* const PROJECT = "input/project";

  /// copyright information for the original file
  static const char* const COPYRIGHT = "input/copyright";

  /// comment field information for the original file
  static const char* const COMMENT = "input/comment";

  /// focal length of the camera from original file
  static const char* const FOCAL_LENGTH = "input/focal_length";

  /// fnumber from original file
  static const char* const FNUMBER = "input/fnumber";

  /// exposure time from original file (in seconds)
  static const char* const EXPOSURE = "input/exposure_time";

  /// shared between DPX and CIN
  static const char* const SLATE_INFO      = "input/slate_info";
  
  /// List of views in the input file, if specified. Each view name should
  /// be separated by a newline (with no newline required after the final view).
  static const char* const VIEW_NAMES = "input/view_names";

  /// Image element decsriptors
  static const char* const ELEMENT_DESCRIPTION[8]      =  {
                                  "image/description0",
                                  "image/description1",
                                  "image/description2",
                                  "image/description3",
                                  "image/description4",
                                  "image/description5",
                                  "image/description6",
                                  "image/description7"
  };

  namespace Nuke
  {
    static const char* const NUKE_PREFIX      = "nuke/";

    static const char* const NODE_HASH        = "nuke/node_hash";
    static const char* const VERSION          = "nuke/version";
    static const char* const FULL_LAYER_NAMES = "nuke/full_layer_names";
  }

  namespace DPX
  {
    static const char* const FRAMEPOS        = "dpx/frame_position";
    static const char* const SEQUENCE_LENGTH = "dpx/sequence_length";
    static const char* const HELD_COUNT      = "dpx/held_count";

    static const char* const FRAME_ID        = "dpx/frame_id";
    static const char* const TRANSFER        = "dpx/transfer";
    static const char* const INPUT_DEVICE    = "dpx/input_device";
    static const char* const FILE_NAME       = "dpx/file_name";
    static const char* const CREATION_TIME   = "dpx/creation_time";
    static const char* const INPUT_SN        = "dpx/input_SN";

    static const char* const TIME_CODE       = "dpx/time_code";
    static const char* const USER_BITS       = "dpx/user_bits";
    static const char* const INTERLACE       = "dpx/interlace";
    static const char* const FIELD_NUMBER    = "dpx/field_number";
    static const char* const VIDEO_SIGNAL    = "dpx/video_signal";
    static const char* const HORIZ_SAMPLE    = "dpx/horizontal_sample_rate";
    static const char* const VERT_SAMPLE     = "dpx/vertical_sample_rate";
    static const char* const FRAME_RATE      = "dpx/frame_rate";
    static const char* const TIME_OFFSET     = "dpx/time_offset";
    static const char* const GAMMA           = "dpx/gamma";
    static const char* const BLACK_LEVEL     = "dpx/black_level";
    static const char* const BLACK_GAIN      = "dpx/black_gain";
    static const char* const BREAK_POINT     = "dpx/break_point";
    static const char* const WHITE_LEVEL     = "dpx/white_level";
    static const char* const INTEGRATION_TIMES    = "dpx/integration_times";
  }

  namespace CIN
  {
    static const char* const ORIGINAL_FILE_CREATION_TIME = "cin/orig_ctime";
    static const char* const ORIGINAL_FILE_NAME          = "cin/orig_filename";
    static const char* const INPUT_DEVICE                = "cin/input_device";
    static const char* const INPUT_DEVICE_MODEL_NO       = "cin/input_device_model_no";
    static const char* const INPUT_DEVICE_SERIAL_NO      = "cin/input_device_serial_no";

    static const char* const LABEL = "cin/label";
  }

  namespace EXR
  {
    /// any unrecognised exr property gets put into the exr/ namespace
    /// we will write these out, stripping exr/ from them.
    static const char* const EXR_PREFIX = "exr/";

    /// this attribute will be present and set to 1 if the input exr is tiled
    static const char* const EXR_TILED = "exr/tiled";

    /// world to camera matrix
    static const char* const EXR_WORLD_TO_CAMERA = "exr/worldToCamera";

    /// world to NDC matrix
    static const char* const EXR_WORLD_TO_NDC = "exr/worldToNDC";
  }

  namespace DTEX
  {
    /// world to camera matrix
    static const char* const DTEX_NL = "dtex/nl";

    /// world to NDC matrix
    static const char* const DTEX_NP = "dtex/np";
  }

  namespace EXIF
  {
    /// all exif properties go into the exif/ namespace.
    /// it writes out these out.
    static const char* const EXIF_PREFIX = "exif/";
  }

  namespace TIFF
  {
    static const char* const TIFF_PREFIX = "tiff/";

    static const char* const TIFF_XRESOLUTION = "tiff/xresolution";
    static const char* const TIFF_YRESOLUTION = "tiff/yresolution";

    static const char* const TIFF_RESOLUTIONUNIT = "tiff/resolution_unit";
  }

  namespace QuickTime
  {
    static const char* const CODEC_ID                    = "quicktime/codec_id";
    static const char* const CODEC_NAME                  = "quicktime/codec_name";
    static const char* const CODEC_INFO                  = "quicktime/codec_info";
    static const char* const REEL                        = "quicktime/reel";

    static const char* const NCLC_PRIMARIES              = "quicktime/nclc_primaries";
    static const char* const NCLC_TRANSFER               = "quicktime/nclc_transfer_function";
    static const char* const NCLC_MATRIX                 = "quicktime/nclc_matrix";
    
    static const char* const QUICKTIME_PREFIX            = "quicktime/";
  }

  namespace FFmpegStream
  {
    static const char* const PIXEL_FORMAT = "stream/pixel_format";
  }

  /// a group of metadata, associated with keys to values.  values consist of Doubles, Ints, or Strings,
  /// or arrays of same
  class DDImage_API Bundle
  {
    class DDImage_API CompareKey {
    public:
      bool operator()(const std::string& a, const std::string& b) const;
    };

  public:

    /// opaque data type for the metadata properties themselves.  use helper functions isPropertyInt etc to
    /// access these
    typedef std::shared_ptr<const Foundry::Property::Base> PropertyPtr;

    /// all the metadata contained in the bundle
    typedef std::map<std::string, PropertyPtr, CompareKey> data_t;

  private:
    std::shared_ptr<data_t> _data;

    /// perform a deep copy of the data if necessary to detach it from other Metadata::Bundles
    void detachIfNecessary();

  public:

    /// iterator over all the metadata
    using const_iterator = data_t::const_iterator;

    /// create an empty bundle
    Bundle() : _data(std::make_shared<data_t>()) {
    }

    /// create a new bundle based upon other bundles. This performs a shallow copy.
    Bundle(const MetaData::Bundle & otherBundle);

    /// assign this bundle based upon another bundle. This performs a shallow copy.
    void operator=(const MetaData::Bundle& otherBundle);

    /// compare two bundles. Returns true if they are identical.
    bool operator==(const MetaData::Bundle& otherBundle) const;

    ~Bundle();

    /// how many metadata elements there are in this bundle
    size_t size() const
    {
      return _data->size();
    }

    /// get iterator to the first piece of metadata
    const_iterator begin() const
    {
      return _data->begin();
    }

    /// get iterator pointing past the last piece of metadata
    const_iterator end() const
    {
      return _data->end();
    }

    /// find a particular piece of metadata by key.  returns end() if not present.
    const_iterator find(const char* key) const
    {
      return find(std::string(key));
    }

    /// find a particular piece of metadata by key.  returns end() if not present.
    const_iterator find(const std::string& key) const;

    /// remove a particular piece of metadata. A deep copy may be performed if necessary
    /// return const_iterator following the last removed element or end() if it->first does not exist
    /// Note that existing iterators may be invalidated or no longer refer to this bundle including end()
    const_iterator erase(const_iterator it)
    {
      return erase(it->first);
    }

    /// remove a particular piece of metadata. A deep copy may be performed if necessary
    /// return const_iterator following the last removed element or end() if key does not exist
    /// Note that existing iterators may be invalidated or no longer refer to this bundle including end()
    const_iterator erase(const char* key)
    {
      return erase(std::string(key));
    }

    /// remove a particular piece of metadata. A deep copy may be performed if necessary
    /// return const_iterator following the last removed element or end() if key does not exist
    /// Note that existing iterators may be invalidated or no longer refer to this bundle including end()
    const_iterator erase(const std::string& key);

    /// return the underlying std::map for the metadata
    const data_t& meta() const
    {
      return *_data;
    }

    /// return the pointer to the named metadata element
    PropertyPtr getData(const char* key) const;

    /// return the pointer to the named metadata element
    PropertyPtr getData(const std::string& key) const;

    /// return the named metadata element as a double. this will convert strings to numbers.  XXX: need to convert rationals as well.
    double getDouble(const char* key) const;

    /// return the named metadata element as an unsigned int. this will convert strings to numbers.  XXX: need to convert rationals as well.
    unsigned char getUnsignedChar( const char* key ) const;

    /// return the named metadata as a string.  this involves converting ints and doubles to strings, and formatting arrays by inserting commas
    std::string getString(const char* key) const;

    /// set the given key to point at a copy of the given object
    mFnDeprecatedInNuke14("Use setData instead.")
    void setDataCopy(const std::string& key, const PropertyPtr& property);

    /// set the given key to point at the given object. A deep copy may be performed if necessary
    void setData(const std::string& key, const PropertyPtr& property);

    /// set the given key to the given number, if and only if number is not NAN. A deep copy may be performed if necessary
    void setDataIfNotNan(const std::string& key, double number);

    /// set the given key to the given string, if and only if data is not empty. A deep copy may be performed if necessary
    void setDataIfNotEmpty(const std::string& key, const std::string& data);

    /// set the given key to the given data. A deep copy may be performed if necessary
    void setData(const std::string& key, float data);

    /// set the given key to the given data. A deep copy may be performed if necessary
    void setData(const std::string& key, const float* data, unsigned count);

    /// set the given key to the given data. A deep copy may be performed if necessary
    void setData(const std::string& key, double data);

    /// set the given key to the given data. A deep copy may be performed if necessary
    void setData(const std::string& key, const double* data, unsigned count);

    /// set the given key to the given data. A deep copy may be performed if necessary
    void setData(const std::string& key, int data);

    /// set the given key to the given data. A deep copy may be performed if necessary
    void setData(const std::string& key, const int* data, unsigned count);

    /// set the given key to the given data. A deep copy may be performed if necessary
    template<class T>
    void setData(const std::string& key, const std::vector<T>& data)
    {
      setData(key, &data[0], data.size());
    }

    /// set the given key to the given data. A deep copy may be performed if necessary
    void setData(const std::string& key, unsigned int data);

    /// set the given key to the given data. A deep copy may be performed if necessary
    void setData(const std::string& key, const std::string& data);

    /// set the given key to the given data. A deep copy may be performed if necessary
    void setData(const std::string& key, const char* data);

    /// set the given key to the given, which is in the format of the timestamp
    /// this will reformat 'YYYY:MM:DD HH:MM:SS', 'YYYY:MM:DD:HH:MM:SS', and
    /// 'YYYY-MM-DD HH-MM:SS' as the ISO 8601-compliant 'YYYY-MM-DD HH:MM:SS'
    /// A deep copy may be performed if necessary
    void setTimeStamp(const std::string& key, const std::string& timestamp);

    /// copy all the data from the given metadata bundle
    /// Note that any existing keys will be replaced and a deep copy may be performed if necessary
    void copyFrom(const Bundle& otherBundle);
  };

  /// helper function to determine whether the opaque property is an int (or array of ints)
  DDImage_API bool isPropertyInt( const Bundle::PropertyPtr prop );

  /// helper function to determine whether the opaque property is a double (or array of doubles)
  DDImage_API bool isPropertyDouble( const Bundle::PropertyPtr prop );

  /// helper function to determine whether the opaque property is a string (or array of strings)
  DDImage_API bool isPropertyString( const Bundle::PropertyPtr prop );

  /// helper function to determine the size of an array property (1 if not an array)
  DDImage_API size_t getPropertySize( const Bundle::PropertyPtr prop );

  /// helper function to get int data at a particular index in the array (index should be 0 if not an array)
  DDImage_API int getPropertyInt( const Bundle::PropertyPtr prop, int index );

  /// helper function to get double data at a particular index in the array (index should be 0 if not an array)
  DDImage_API double getPropertyDouble( const Bundle::PropertyPtr prop, int index );

  /// helper function to get string data at a particular index in the array (index should be 0 if not an array)
  DDImage_API std::string getPropertyString( const Bundle::PropertyPtr prop, int index );

  /// helper function to get the array as a Matrix4
  DDImage_API DD::Image::Matrix4 getPropertyMatrix( const Bundle::PropertyPtr prop );

  /// Gets the string version of the property
  DDImage_API std::string propertyToString(const Bundle::PropertyPtr p);

  /// a built-in emptyBundle, which can be returned as a reference without the need to create and maintain one
  /// oneself.
  extern DDImage_API const Bundle& emptyBundle();
}

}

}

#endif
