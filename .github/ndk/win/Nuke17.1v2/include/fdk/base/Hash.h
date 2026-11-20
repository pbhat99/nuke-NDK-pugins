// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_BASE_HASH_H
#define FDK_BASE_HASH_H

#include "fdk/api.h"

#include <cstdint>
#include <iostream>
#include <string>


namespace fdk
{
  using HashType = uint64_t;

  /*! \class fdk::Hash

      Generates and maintains a hash sum of data. If the hashes of two
      pieces of data are equal it is \e extremely likely the two pieces of
      data are equal.

      To generate a hash, make one of these objects then call append() with
      each piece of data you want to include.
      You can then compare it with other hashes, or extract the numerical
      value with value().

      The algorithim used is a 64-bit CRC checksum, using the
      CRC-64-ECMA-182 polynominal (lsb representation 0x42F0E1EBA9EA3693).

      Some people worry that this is not a cryptographcally sound checksum,
      but that is not needed when the data is not being produced by something
      that is actively trying to subvert the checksum. For random data it
      is equally strong.
  */
  class FDK_API Hash
  {
  public:
    static constexpr const uint64_t  DefaultValue = ~0x0ULL; //!< Default value for hash (0xffffffffffffffff)


  public:
    Hash()              : _value(DefaultValue) {}
    Hash(const Hash& h) : _value(h._value) {}
    explicit Hash(const uint64_t& h) : _value(h) {}

    const Hash& operator = (const Hash& h) { _value = h._value; return *this; }

    //! Reset the hash to its initial value, which is not zero.
    void      reset()                  { _value = DefaultValue; }
    void      reset(const uint64_t& v) { _value = v; }

    bool      isDefault() const { return (_value == DefaultValue); }

    //! Changes the value to the next hash increment.
    void      newValue();

    bool operator == (const Hash& h) const { return (_value == h._value); }
    bool operator != (const Hash& h) const { return (_value != h._value); }
    bool operator  < (const Hash& h) const { return (_value  < h._value); }

    uint64_t  value()   const { return _value; }
    uint64_t  getHash() const { return _value; }

    //! Return a hex-formatted string version of the hash value.
    std::string  asString() const;

    //------------------------------------------------------

    /*! Add the \a n bytes starting at \a pointer to the hash.

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
    void  append(const void* data, size_t length);

    /*! Add a null-terminated string or a null pointer to the hash.
        The zero-length string and the null pointer hash to different
        values, and those are different than not calling this.
     */
    void  append(const char*);
    void  append(const std::string&);

    /*! Add a bool value to the hash. This is different than any other
        possible append (as it only adds 1 bit to the hash rather than
        a multiple of 8) and thus it is also useful for marking where
        zero-length arrays are.
    */
    void  append(bool);

    void  append(float);
    void  append(double);
    void  append(int32_t);
    void  append(uint32_t);
    void  append(uint64_t);

    //! Add another hash to this hash.
    void  append(const uint64_t&);
    void  append(const Hash&);
    //! If other hash is not at default value append it to this hash.
    void  appendIfNotDefault(const uint64_t&);
    void  appendIfNotDefault(const Hash&);

    //! If \a a is not default it's copied then \a b is appended if it's not
    //! default, otherwise if \a a is default then \a b is copied and \a a
    //! is ignored.
    static Hash  combineIfNotDefault(const Hash& a, const Hash& b);


  protected:
    uint64_t _value;


  private:
    /*! Make this version of append private, and don't define it.
        If this isn't defined then the compiler will auto-convert calls to 
        append with a void* to bool, which is likely not intended.
    */
    void  append(const void* data) = delete;
  };


inline std::ostream& operator << (std::ostream& o, const fdk::Hash& h)
{
  const char f = o.fill('0');
  const int w = o.width(16);
  o << std::hex << h.value() << std::dec;
  o.width(w);
  o.fill(f);
  return o;
}

inline void Hash::append(const std::string& s) { append(s.c_str(), s.size() + 1); }
inline void Hash::append(int32_t            v) { append(static_cast<uint32_t>(v)); }
inline void Hash::append(float              v) { append(&v, sizeof(float)); }
inline void Hash::append(double             v) { append(&v, sizeof(double)); }
inline Hash Hash::combineIfNotDefault(const Hash& a, const Hash& b)
{
  Hash h;
  if (!a.isDefault()) {
    h = a;
    h.appendIfNotDefault(b);
  }
  else {
    h = b;
  }
  return h;
}


} // namespace fdk

// Allows Hash use in std unordered_map, etc:
namespace std {
  template <>
  struct hash<fdk::Hash>
  {
    std::size_t operator()(const fdk::Hash& token) const noexcept { return static_cast<size_t>(token.value()); }
  };
}



#endif
