// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_Hash_H
#define DD_Image_Hash_H

#include "DDImage_API.h"
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <vector>

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::Hash

       Generates and maintaines a hash sum of data. If the hashes of two
       pieces of data are equal it is \e extremely likely the two pieces of
       data are equal.

       To generate a hash, make one of these objects, then reset() it,
       then call append() with each piece of data you want to include.
       You can then compare it with other hashes, or extract the numerical
       value with value().

       The algorithim used is a 64-bit CRC checksum, using the
       CRC-64-ECMA-182 polynominal (lsb representation 0x42F0E1EBA9EA3693).
       Some people worry that this is not a cryptographcally sound checksum,
       but that is not needed when the data is not being produced by something
       that is actively trying to subvert the checksum. For random data it
       is equally strong.
     */
    class DDImage_API Hash
    {
      U64 _value;
      static unsigned counter;

      // Make this version of append private, and don't define it
      // If this isn't defined, then the compiler will auto-convert calls to 
      // append with a void* to bool, which is not what is probably intended
      void append(const void* data);

    public:
      static constexpr U64 kDefaultValue = ~0ULL;
      typedef U64 HashType;
      Hash() : _value(kDefaultValue) {}
      Hash(const Hash &h) : _value(h._value) {}
      Hash(U64 v) : _value(v) {}
      const Hash& operator=(const Hash& h) { _value = h._value;
                                             return *this; }


      /*! \fn void Hash::reset();
         Reset the hash to it's initial value. This is not zero.
       */
      void reset() { _value = kDefaultValue; }
      void reset(U64 v) { _value = v; }

      bool operator==(const Hash& h) const { return _value == h._value; }
      bool operator!=(const Hash& h) const { return _value != h._value; }
      bool operator<(const Hash& h) const { return _value < h._value; }

      U64  value() const { return _value; }
      U64  getHash() const { return _value; }
      bool isDefault() const { return (_value == kDefaultValue); }

      /*!
         Add the \a n bytes starting at \a pointer to the hash.

         Warning: this call is for appending blocks of data together.
         A zero-length block will not change the hash, and the hash
         is the same no matter how the same block is split into calls
         to this. This will cause the hash to fail if you actually
         care about this, for instance if you have several variable-sized
         arrays of data. Swapping a zero-length and non-zero-length one,
         or moving data from the end of one to the start of the next,
         will not change the hash. The solution is to append the length
         as well.
       */
      void append(const void* data, size_t length);

      /*!
         Add a null-terminated string or a null pointer to the hash.

         The zero-length string and the null pointer hash to different
         values, and those are different than not calling this.
       */
      void append(const char*);
      void append(const std::string& s) { append(s.c_str(), s.size() + 1); }

      /*!
         Add a bool value to the hash. This is different than any other
         possible append (as it only adds 1 bit to the hash rather than
         a multiple of 8) and thus it is also useful for marking where
         zero-length arrays are.
       */
      void append(bool);

      /*!
         Add an integer value to the hash. Same as using append(&value,
         sizeof(int)) on a LSB-first machine. This is intended for hashing
         enumeration values.
       */
      void append(int);

      /*!
         Add an unsigned value to the hash. Same as using append(&value,
         sizeof(unsigned)) on a LSB-first machine. This is intended for hashing
         enumeration or size values.
       */
      void append(unsigned);

      /*!
         Add a float value to the hash. Same as using append(&value,
         sizeof(float)) (this may change on MSB machines).
       */
      void append(float);

      /*!
         Add a double value to the hash. Same as using append(&value,
         sizeof(float)) (this may change on MSB machines).
       */
      void append(double);

      /*!
         Add another hash to this hash.
       */
      void append(const Hash&);

      /*!
         Add an intset to the hash.
       */
      void append(const std::set<int>& intset);

      /*!
         Add a std::map<int, int> to the hash.
       */
      void append(const std::map<int, int>& iimap);

      /*!
         Add a 64 bit value to this hash.
       */
      void append(U64);

      template<class T>
      void append(const std::vector<T>& vec)
      {
        unsigned size = (unsigned)vec.size();
        append(size);
        for (size_t i = 0; i < size; i++) {
          append(vec[i]);
        }
      }

      template<class T>
      Hash& operator << (const T& v) { append(v);
                                       return *this; }

      void newvalue() { _value = ++counter; }


    };

    template<class T>
    inline U64 getHashValue( const T s )
    {
      DD::Image::Hash h;
      h.append(s);
      return h.value();
    }


    inline std::ostream& operator << (std::ostream& o, const Hash& h)
    {
      return o << std::hex << h.value() << std::dec;
    }

  }
}


#endif
