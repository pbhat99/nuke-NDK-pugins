// Attribute.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Attribute_H
#define DDImage_Attribute_H

#include "DDImage/DDImage_API.h"
#include "DDImage/Channel.h"
#include "DDImage/Vector2.h"
#include "DDImage/Vector3.h"
#include "DDImage/Vector4.h"
#include "DDImage/Matrix3.h"
#include "DDImage/Matrix4.h"
#include "DDImage/Allocators.h"
#include "DDImage/RefCountedObject.h"

#include <vector>

namespace DD
{
  namespace Image
  {

    /*! Attribute data type enumerations. */
    enum AttribType {
      INVALID_ATTRIB = -1, //!< Data type not set

      FLOAT_ATTRIB,       //!< 1 float
      VECTOR2_ATTRIB,     //!< Vector2(2 floats)
      VECTOR3_ATTRIB,     //!< Vector3(3 floats)
      VECTOR4_ATTRIB,     //!< Vector4(4 floats)
      NORMAL_ATTRIB,      //!< Normal vector - Vector3(3 floats)

      INT_ATTRIB,         //!< Int
      STRING_ATTRIB,      //!< Char*
      STD_STRING_ATTRIB,  //!< std::string. Variable size
      POINTER_ATTRIB,     //!< Void*
      
      MATRIX3_ATTRIB,     //!< Matrix3 (9 floats)
      MATRIX4_ATTRIB      //!< Matrix4 (16 floats)
    };


    //
    // Names for common attributes.
    //

    static const char* const kUVAttrName = "uv";
    static const char* const kNormalAttrName = "N";
    static const char* const kColorAttrName = "Cf";
    static const char* const kPWAttrName = "PW";
    static const char* const kVelocityAttrName = "vel";
    static const char* const kSelectionAttrName = "soft_select";
    static const char* const kSizeAttrName = "size";
    static const char* const kNameAttrName = "name";
    static const char* const kTransformAttrName = "transform";


    //----------------------------------------------------------------------

    //! General Attribute class
    class DDImage_API Attribute : public RefCountedObject
    {
      friend class GeoInfo;
      friend class GeoOp;
    public:
      // Attributes cannot be changed after creation, so don't
      // allow any access to innards except the data:
      /*! Copy constructor. */
      Attribute(const Attribute &src) { *this = src; }

      /*! Basic constructor requires a name and type.  Size is defaulted to 0.
       */
      Attribute(const char* name, AttribType type, size_t size = 0);
      ~Attribute() override;

      bool valid() const { return type_ != INVALID_ATTRIB; }
      bool invalid() const { return type_ == INVALID_ATTRIB; }

      /*! Assignment operator.  Empty then copy all data from 'b' into this.
       */
      Attribute& operator = (const Attribute&);

      /*! Destructively copy a single item from 'b_start' to 'dest'.
       */
      void copy(int dest, const Attribute&, int b_start);

      /*! Destructively copy a range of items from 'b' to a range beginning at 'dest'.
       */
      void copy(int dest, const Attribute&, int b_start, int b_end);

      /*! Appends data elements from a same type Attribute onto the end
          of this one.  Fails if both types are not the same.
       */
      bool append(const Attribute&, int b_start);

      /*! Appends a range of elements from a same type Attribute onto the end
          of this one.  Fails if both types are not the same.
       */
      bool append(const Attribute&, int b_start, int b_end);

      const char* name() const { return name_; }
      AttribType type() const { return type_; }

      /*! Number of data elements in list.
       */
      unsigned size() const;

      /* Returns the memory reserve of the list.
       */
      unsigned capacity() const;

      /*! Add 'n' elements to end of list. */
      void add(unsigned n = 1);

      /*! Reserve memory in list for 'n' elements.  If you already know the
          maximum size of the attribute, this can save a lot of overhead
          if you call this before adding single elements.
       */
      void reserve(unsigned size);

      /*! Expand or contract element list size to 'n' elements.
       */
      void resize(size_t size);

      /*! Clear the attribute data, leave the data list structure intact.
       */
      void clear();

      /*! Return a void pointer to data element 'n'.
       */
      void* array(unsigned n = 0) const;

      /*! Size of a single data element in bytes.
       */
      size_t bytes() const;

      /*! Number of elements in data type.
       */
      unsigned data_elements() const;

      /*! Number of floats for this data attribute type.
       */
      unsigned floats() const;

      float&        flt(const size_t n = 0)     const { mFnAssert(type_ == FLOAT_ATTRIB); mFnAssert(n < float_list->size()); return (  *float_list)[n]; }
      Vector2&      vector2(const size_t n = 0) const { mFnAssert(type_ == VECTOR2_ATTRIB); mFnAssert(n < vector2_list->size()); return (*vector2_list)[n]; }
      Vector3&      vector3(const size_t n = 0) const { mFnAssert(type_ == VECTOR3_ATTRIB); mFnAssert(n < vector3_list->size()); return (*vector3_list)[n]; }
      Vector4&      vector4(const size_t n = 0) const { mFnAssert(type_ == VECTOR4_ATTRIB); mFnAssert(n < vector4_list->size()); return (*vector4_list)[n]; }
      Vector3&      normal(const size_t n = 0)  const { mFnAssert(type_ == NORMAL_ATTRIB); mFnAssert(n < vector3_list->size()); return (*vector3_list)[n]; }
      Matrix3&      matrix3(const size_t n = 0) const { mFnAssert(type_ == MATRIX3_ATTRIB); mFnAssert(n < matrix3_list->size()); return (*matrix3_list)[n]; }
      Matrix4&      matrix4(const size_t n = 0) const { mFnAssert(type_ == MATRIX4_ATTRIB); mFnAssert(n < matrix4_list->size()); return (*matrix4_list)[n]; }
      int&          integer(const size_t n = 0) const { mFnAssert(type_ == INT_ATTRIB); mFnAssert(n < int_list->size()); return (*int_list)[n]; }
      char*&        string(const size_t n = 0)  const { mFnAssert(type_ == STRING_ATTRIB); mFnAssert(n < string_list->size()); return ( *string_list)[n]; }
      std::string&  stdstring(const size_t n = 0)  const { mFnAssert(type_ == STD_STRING_ATTRIB); mFnAssert(n < std_string_list->size()); return ( *std_string_list)[n]; }
      void*&        pointer(const size_t n = 0) const { mFnAssert(type_ == POINTER_ATTRIB); mFnAssert(n < pointer_list->size()); return (*pointer_list)[n]; }

      /*! Print type information about this attribute.
       */
      void print_info() const;

      /*! Print a nicely formatted value for element 'n'.
       */
      void print_value(const size_t n) const;

      static const char* attribute_types[];
      static const char* type_string(int i) { return attribute_types[i]; }

      //protected:
    public:
      const char* name_; //!< Name of attribute
      AttribType type_;     //!< Data type

      // Union of list type pointers(only one is allowed per attribute).
      // As they all share the same memory location, all accesses of these
      // need to be done using the type_ variable:
      union {
        std::vector< float,         STL3DAllocator<float> >  * float_list;
        std::vector< Vector2,       STL3DAllocator<Vector2> >* vector2_list;
        std::vector< Vector3,       STL3DAllocator<Vector3> >* vector3_list;
        std::vector< Vector4,       STL3DAllocator<Vector4> >* vector4_list;
        std::vector< Matrix3,       STL3DAllocator<Matrix3> >* matrix3_list;
        std::vector< Matrix4,       STL3DAllocator<Matrix4> >* matrix4_list;
        std::vector< int,           STL3DAllocator<int> >    * int_list;
        std::vector< char*,         STL3DAllocator<char*> >  * string_list;
        std::vector< std::string,   STL3DAllocator<std::string> >  * std_string_list;
        std::vector< void*,         STL3DAllocator<void*> >  * pointer_list;
        void* data;
      };

    private:
      /*! Empty constructor is disabled. */
      Attribute() {}

      /*! Clear and delete any attribute data and it's allocation.
       */
      void destroy();
    };

    typedef RefCountedPtr<Attribute> AttributePtr;

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
