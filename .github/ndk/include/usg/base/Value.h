// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_BASE_VALUE_H
#define USG_BASE_VALUE_H

#include "usg/api.h"
#include "usg/base/Array.h"
#include "usg/base/Token.h"
#include "usg/geom/Path.h"
#include "usg/geom/AssetPath.h"
#include "usg/geom/PathExpr.h"

#include <memory>

namespace usg
{

  //! An object which can hold many types of value.
  class USG_API Value
  {
  public:
    class Handle; //!< Abstraction handle for casting to underlying implementation type


    // Base data types
    enum DataType
    {
      STRING_DATA, TOKEN_DATA,  ASSETPATH_DATA, PATHEXPRESSION_DATA,
      INT8_DATA,   INT16_DATA,  INT32_DATA,  INT64_DATA,
      HALF_DATA,   FLOAT_DATA,  DOUBLE_DATA,
    };

    //! Supported POD & compound value types. Each value type has an array variant.
    //!
    //! Some types are interpretation aliases (a role) overlaying the base type.
    //! For example Normal, Normal3f, Color, Color3f, Point, Point3f, Vector and
    //! Vector3f are all aliases for the Float3 base type.
    //!
    //! Shader properties only support an explicit subset of these types and
    //! should only be declared with the following types:
    //!   Int        IntArray
    //!   String     StringArray
    //!   Float      FloatArray
    //!
    //!   Color3f    Color3fArray
    //!   Point3f    Point3fArray
    //!   Normal3f   Normal3fArray
    //!   Vector3f   Vector3fArray
    //!
    //!   Matrix4d   Matrix4dArray
    //!
    //!   Struct     StructArray,
    //!   Terminal   TerminalArray
    //!   Vstruct    VstructArray
    enum Type
    {
      //---------------------------------------
      DefaultType = -1,    //!< Special value used as the 'default' indicator for an argument
      //
      //---------------------------------------
      // Base data types:
      //---------------------------------------
      String = 0, StringArray,
      Token,      TokenArray,
      Path,       PathArray,
      AssetPath,  AssetPathArray,
      PathExpr,
      //----------------
      Bool,       BoolArray,
      Int,        IntArray,
      Int64,      Int64Array,
      UChar,      UCharArray,
      UInt,       UIntArray,
      UInt64,     UInt64Array,
      //----------------
      Half,       HalfArray,
      Float,      FloatArray,
      Double,     DoubleArray,
      TimeCode,   TimeCodeArray,
      //----------------
      Int2,       Int2Array,
      Half2,      Half2Array,
      Float2,     Float2Array,
      Double2,    Double2Array,
      //----------------
      Int3,       Int3Array,
      Half3,      Half3Array,
      Float3,     Float3Array,
      Double3,    Double3Array,
      //-----------------
      Int4,       Int4Array,
      Half4,      Half4Array,
      Float4,     Float4Array,
      Double4,    Double4Array,
      //-----------------
      Matrix2d,   Matrix2dArray,
      Matrix3d,   Matrix3dArray,
      Matrix4d,   Matrix4dArray,
      //-----------------
      Quath,      QuathArray,
      Quatf,      QuatfArray,
      Quatd,      QuatdArray,
      //-----------------
      //
      //---------------------------------------
      // Type aliases (roles, interpretations):
      //---------------------------------------
      Channel,    ChannelArray,     // Int, IntArray
      //-----------------
      Color,      ColorArray,       // Float3
      Color3h,    Color3hArray,     // Half3
      Color3f,    Color3fArray,     // Float3
      Color3d,    Color3dArray,     // Double3
      Color4h,    Color4hArray,     // Half4
      Color4f,    Color4fArray,     // Float4
      Color4d,    Color4dArray,     // Double4
      //-----------------
      Normal,     NormalArray,      // Float3
      Normal3h,   Normal3hArray,    // Half3
      Normal3f,   Normal3fArray,    // Float3
      Normal3d,   Normal3dArray,    // Double3
      //-----------------
      Point,      PointArray,       // Float3
      Point3h,    Point3hArray,     // Half3
      Point3f,    Point3fArray,     // Float3
      Point3d,    Point3dArray,     // Double3
      //-----------------
      Vector,     VectorArray,      // Float3
      Vector3h,   Vector3hArray,    // Half3
      Vector3f,   Vector3fArray,    // Float3
      Vector3d,   Vector3dArray,    // Double3
      //-----------------
      TexCoord2h, TexCoord2hArray,  // Half2
      TexCoord2f, TexCoord2fArray,  // Float2
      TexCoord2d, TexCoord2dArray,  // Double2
      TexCoord3h, TexCoord3hArray,  // Half3
      TexCoord3f, TexCoord3fArray,  // Float3
      TexCoord3d, TexCoord3dArray,  // Double3
      //-----------------
      Matrix,     MatrixArray,      // Matrix4d
      Frame4d,    Frame4dArray,     // Matrix4d
      //-----------------
      //
      //---------------------------------------
      // Abstract types:
      //---------------------------------------
      Struct,     StructArray,
      Terminal,   TerminalArray,
      Vstruct,    VstructArray,
      //-----------------
      //
      InvalidType,    //!< Special value typically used as a return argument indicating an error or unhandled type
      NumValueTypes = InvalidType
    };

    //! Gets Type enum from string.
    static Type                getTypeFromName(const std::string& type_name);

    //! Gets type name from Type enum.
    static const std::string&  getTypeName(Type type);

    //! Returns this Type's non-aliased Type enumeration (ie base type of a Color3f is Float3)
    static Type                getBaseType(Type type);

    //! Returns this Type's base DataType enumeration (ie data type of a Color3f is FLOAT_DATA)
    static DataType            getBaseDataType(Type type);

    //! Returns the number of data elements for the Type. For example, 3 for a Vector3 or 16 for a Matrix4.
    static size_t              getNumDataElements(Type type);

    //! Returns the number of bytes for a data element of the Type. For example, 4 for a float or 8 for a double.
    static size_t              getBytesPerDataElement(Type type);

    //! Returns a value for the default 'empty' state of the Type. ie 0 for int, 0.0f for float, '' for string, etc.
    static Value               getDefaultValueForType(Type type);


  public:
    //! Creates an empty (invalid) value.
    Value();
    //! Copy constructors
    Value(const Value&);
    Value& operator = (const Value&);


    //------------------------------------------------------
    // Set/construct the value:

    //! Construct from a value source
    template <typename T>
    explicit Value(const T& data);

    //! Construct from a value Array source
    template <typename T>
    explicit Value(const usg::Array<T>& array);

    //! Construct from a std::vector source
    template <typename T>
    explicit Value(const std::vector<T>& vector);

    // Convenience for const char* (stores as Type::String)
    explicit Value(const char* data);

    //------------------------------------------------------

    //! True if Value::Impl is valid (has a valid object binding.)
    bool      isValid() const;
    operator  bool()    const { return this->isValid(); }

    //! Return a pointer to the underlying VtValue object if the usd_version
    //! passed in matches the Usd version this library was built with,
    //! otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to a VtValue* and
    //! possibly double-check its validity since a Value can be created
    //! without a valid binding. However, if Value::isValid() returns
    //! true then the underlying binding will also be valid.
    Value::Handle*        getVtValue(int usd_version);
    const Value::Handle*  getVtValue(int usd_version) const;

    //------------------------------------------------------
    // Get the value:
    // Note that Types which cannot return a const reference have dedicated 'get<Type>' accessor methods:

    //! Returns a const reference to the contained data. 
    template <typename T>
    const T&        get() const;

    //! Dedicated version returning a Token.
    usg::Token      getToken() const;

    //! Dedicated version returning a Path.
    usg::Path       getPath() const;

    //! Dedicated version returning an AssetPath.
    usg::AssetPath  getAssetPath() const;

    //! Dedicated version returning a PathExpr.
    usg::PathExpr  getPathExpr() const;

    //! Fills in the passed-in data object if this Value is valid, otherwise copies the provided default.
    template <typename T>
    void     get(T&       val,
                 const T& def_val) const;

    //! Fills in the passed-in array object if this Value is valid, otherwise copies the provided default array.
    template <typename T>
    void     get(std::vector<T>&       values,
                 const std::vector<T>& def_values) const;

    //! Specialized version for easier string handling (retrieves a Type::String)
    void     get(std::string& val,
                 const char*  def_val) const;

    //------------------------------------------------------

    //! Returns whether the value is empty.
    bool               isEmpty() const;

    //! Return this value's Type enumeration.
    Type               getType() const;
    //! Return this value's Type name.
    const std::string& getTypeName() const;

    //! Return this value's non-aliased Type enumeration.
    //! ie if this value's Type is Color3f or Color then the base type is Float3.
    Type               getBaseType() const;

    //! Return this Type's base DataType enumeration.
    //! ie if this value's Type is Color3f or Color then the base data type is FLOAT_DATA.
    DataType           getBaseDataType() const;

    //! Returns true if this is an array-typed value.
    bool               isArray()      const;
    //! Returns the size of the array if an array type value, otherwise 0.
    size_t             getArraySize() const;
    //! Return a hash of the value using VtHashValue
    size_t             getHash() const;

    //! Returns true if this value can be cast to the same type as the `other` value.
    bool               canCastToTypeOf(const usg::Value& other);

    //! Attempts to cast to the same type as the `other` value.
    void               castToTypeOf(const usg::Value& other);

    //! Prints a summary of the attribute to a stream
    friend USG_API std::ostream& operator << (std::ostream& o, const Value& b);

    //! Tests two values for equality
    friend USG_API bool operator == (const Value& lhs, const Value& rhs);
    friend USG_API bool operator != (const Value& lhs, const Value& rhs);

  public:
    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------
    class Impl; //!< Wrapper abstraction for implementation type
    Value(const Value::Impl&);
    Value& operator = (const Value::Impl&);
    ~Value(); // required for unique_ptr

    //! Takes ownership of 'b' - returns true if resulting Value is valid.
    bool  setFromImpl(Value::Impl&& b);

    //!
    Value::Impl*       impl()       { return _pImpl.get(); }
    const Value::Impl* impl() const { return _pImpl.get(); }


  protected:
    friend class Attribute; // Allow direct access
    friend class Relationship; // Allow direct access
    std::unique_ptr<Value::Impl, void(*)(Value::Impl*)> _pImpl;
  };

  template <> USG_API Value::Value(const std::string& data);


  //! Calls through to << operator on the contained data
  USG_API  std::ostream& operator << (std::ostream& o, const Value&);
  USG_API bool operator == (const Value& lhs, const Value& rhs);
  USG_API bool operator != (const Value& lhs, const Value& rhs);


} // namespace usg

#endif
