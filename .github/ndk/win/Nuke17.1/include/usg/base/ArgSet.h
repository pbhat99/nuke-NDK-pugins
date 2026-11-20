// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_BASE_ARGSET_H
#define USG_BASE_ARGSET_H

#include "usg/base/Token.h"
#include "usg/base/Value.h"

#include "fdk/base/Hash.h"
#include "fdk/base/StringTypes.h"

#include "fdk/math/Vec2.h"
#include "fdk/math/Vec3.h"
#include "fdk/math/Vec4.h"
#include "fdk/math/Mat4.h"
#include "fdk/math/Box2.h"
#include "fdk/math/Box3.h"


namespace usg {

  using TokenValueMapSorted = std::map<Token, Value>;
  using TokenValueMap       = std::unordered_map<Token, Value>;
  using TokenValuePtrMap    = std::unordered_map<Token, const Value*>;

  //! Convenience wrapper class around a TokenValueMap providing
  //! argument get/set access methods.
  class USG_API ArgSet
  {
  public:
    //! Default ctor is an empty set
    ArgSet();
    //! Copy ctors.
    ArgSet(const ArgSet&);
    ArgSet(const TokenValueMap&);


  public:
    //! Raw read access to private TokenValueMap (std::unordered_map<Token, Value>)
    const TokenValueMap& args() const { return _args; }

    //! Replaces the contents with another ArgSet or TokenValueMap.
    ArgSet& operator= (const ArgSet&);
    ArgSet& operator= (const TokenValueMap&);

    //! Number of args in set.
    size_t size()  const { return _args.size(); }
    size_t nArgs() const { return _args.size(); }

    //! Erase all args.
    void clear() { _args.clear(); }

    //! Iterator support
    typedef TokenValueMap::iterator       iterator;
    typedef TokenValueMap::const_iterator const_iterator;

    iterator       begin()       { return _args.begin(); }
    const_iterator begin() const { return _args.begin(); }
    iterator       end()         { return _args.end(); }
    const_iterator end()   const { return _args.end(); }

    //! Convenience function to transmogrify to an alphabetically-sorted map.
    void getAsSorted(TokenValueMapSorted& sorted_args) const;


  public:
    //! Does the set have a matching arg?
    bool has(const Token& key) const;
    bool has(const char*  key) const;

    template<class T>
    T get(const Token& key, const T& dflt_val) const;
    template<class T>
    T get(const char* key, const T& dflt_val) const;

    template<class T>
    void set(const Token& key, const T& value);
    template<class T>
    void set(const char* key, const T& value);

    //! Removes the arg from the ArgSet.
    void  remove(const Token& key);
    void  remove(const char* key);


    //! Print all args
    void print(std::ostream&) const;
    //void print(const char*   prefix,
    //           std::ostream&,
    //           bool          show_expanded=false) const;
    friend std::ostream& operator << (std::ostream& o, const ArgSet& b) { b.print(o); return o; }


  public:
    //-------------------------------------------------------------------------
    // Typed read access.
    //-------------------------------------------------------------------------
    std::string getString(const Token& key, const std::string& dflt_val = std::string()) const;
    std::string getString(const char*  key, const std::string& dflt_val = std::string()) const;
    //
    int32_t     getInt(const Token& key, int32_t dflt_val = 0) const;
    int32_t     getInt(const char*  key, int32_t dflt_val = 0) const;
    double      getDouble(const Token& key, double dflt_val = 0.0) const;
    double      getDouble(const char*  key, double dflt_val = 0.0) const;
    bool        getBool(const Token& key, bool dflt_val = false) const;
    bool        getBool(const char*  key, bool dflt_val = false) const;
    fdk::Hash   getHash(const Token& key, uint64_t dflt_val = fdk::Hash::DefaultValue) const;
    fdk::Hash   getHash(const char*  key, uint64_t dflt_val = fdk::Hash::DefaultValue) const;
    //
    fdk::Vec2d  getVec2d(const Token& key, const fdk::Vec2d& dflt_val = fdk::Vec2d(0.0)) const;
    fdk::Vec2d  getVec2d(const char*  key, const fdk::Vec2d& dflt_val = fdk::Vec2d(0.0)) const;
    //
    fdk::Vec3d  getVec3d(const Token& key, const fdk::Vec3d& dflt_val = fdk::Vec3d(0.0)) const;
    fdk::Vec3d  getVec3d(const char*  key, const fdk::Vec3d& dflt_val = fdk::Vec3d(0.0)) const;
    //
    fdk::Vec4d  getVec4d(const Token& key, const fdk::Vec4d& dflt_val = fdk::Vec4d(0.0)) const;
    fdk::Vec4d  getVec4d(const char*  key, const fdk::Vec4d& dflt_val = fdk::Vec4d(0.0)) const;
    //
    fdk::Mat4d  getMat4d(const Token& key, const fdk::Mat4d& dflt_val = fdk::Mat4d::getIdentity()) const;
    fdk::Mat4d  getMat4d(const char*  key, const fdk::Mat4d& dflt_val = fdk::Mat4d::getIdentity()) const;
    //
    fdk::Box2d  getBox2d(const Token& key, const fdk::Box2d& dflt_val = fdk::Box2d()) const;
    fdk::Box2d  getBox2d(const char*  key, const fdk::Box2d& dflt_val = fdk::Box2d()) const;
    //
    fdk::Box3d  getBox3d(const Token& key, const fdk::Box3d& dflt_val = fdk::Box3d()) const;
    fdk::Box3d  getBox3d(const char*  key, const fdk::Box3d& dflt_val = fdk::Box3d()) const;


  public:
    //-------------------------------------------------------------------------
    // Typed write access.
    //-------------------------------------------------------------------------
    void  setString(const Token& key, const std::string& value);
    void  setString(const Token& key, const char*        value);
    void  setString(const Token& key, const Token&       value);
    void  setString(const char*  key, const std::string& value);
    void  setString(const char*  key, const char*        value);
    void  setString(const char*  key, const Token&       value);
    void  appendString(const Token& key, char separator, const std::string& value);
    void  appendString(const Token& key, char separator, const char*        value);
    void  appendString(const Token& key, char separator, const Token&       value);
    void  appendString(const char*  key, char separator, const std::string& value);
    void  appendString(const char*  key, char separator, const char*        value);
    void  appendString(const char*  key, char separator, const Token&       value);
    //
    void  setInt(const Token& key, int32_t value);
    void  setDouble(const Token& key, double value);
    void  setBool(const Token& key, bool value);
    void  setHash(const Token& key, const fdk::Hash& value);
    //
    void  setVec2d(const Token& key, const fdk::Vec2d& value);
    void  setVec3d(const Token& key, const fdk::Vec3d& value);
    void  setVec4d(const Token& key, const fdk::Vec4d& value);
    //
    void  setMat4d(const Token& key, const fdk::Mat4d& value);
    //
    void  setBox2d(const Token& key, const fdk::Box2d& value);
    void  setBox3d(const Token& key, const fdk::Box3d& value);


  private:
    TokenValueMap _args;

  }; // ArgSet



/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*                   Inline Function Implementations                   */
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/

inline ArgSet::ArgSet() { _args.clear(); }
inline ArgSet::ArgSet(const ArgSet& b) : _args(b._args) {}
inline ArgSet::ArgSet(const TokenValueMap& b) : _args(b) {}
inline ArgSet& ArgSet::operator= (const ArgSet& b) { _args = b._args; return *this; }
inline ArgSet& ArgSet::operator= (const TokenValueMap& b) { _args = b; return *this; }

//-----------------------------------

inline bool ArgSet::has(const Token& key) const { return (_args.find(key) != _args.end()); }
inline bool ArgSet::has(const char*  key) const { return has(Token(key)); }

inline void ArgSet::remove(const Token& key)
{
  const TokenValueMap::const_iterator it = _args.find(key);
  if (it != _args.end()) {
    _args.erase(it);
  }
}
inline void ArgSet::remove(const char* key) { remove(Token(key)); }

inline std::string ArgSet::getString(const char* key, const std::string& dflt_val) const { return getString(Token(key), dflt_val); }

//-----------------------------------

template<class T>
inline T ArgSet::get(const char* key, const T& dflt_val) const { return get<T>(Token(key), dflt_val); }

template<class T>
inline void ArgSet::set(const char* key, const T& value) { return set<T>(Token(key), value); }

inline int32_t   ArgSet::getInt(const char* key, int32_t dflt_val) const { return getInt(Token(key), dflt_val); }
inline double    ArgSet::getDouble(const char* key, double dflt_val) const { return getDouble(Token(key), dflt_val); }
inline bool      ArgSet::getBool(const char* key, bool dflt_val) const { return getBool(Token(key), dflt_val); }
inline fdk::Hash ArgSet::getHash(const char* key, uint64_t dflt_val) const { return getHash(Token(key), dflt_val); }

inline fdk::Vec2d ArgSet::getVec2d(const char* key, const fdk::Vec2d& dflt) const { return getVec2d(Token(key), dflt); }
inline fdk::Vec3d ArgSet::getVec3d(const char* key, const fdk::Vec3d& dflt) const { return getVec3d(Token(key), dflt); }
inline fdk::Vec4d ArgSet::getVec4d(const char* key, const fdk::Vec4d& dflt) const { return getVec4d(Token(key), dflt); }
inline fdk::Mat4d ArgSet::getMat4d(const char* key, const fdk::Mat4d& dflt) const { return getMat4d(Token(key), dflt); }
inline fdk::Box2d ArgSet::getBox2d(const char* key, const fdk::Box2d& dflt) const { return getBox2d(Token(key), dflt); }
inline fdk::Box3d ArgSet::getBox3d(const char* key, const fdk::Box3d& dflt) const { return getBox3d(Token(key), dflt); }

inline void ArgSet::setString(const Token& key, const std::string& value) { _args[key] = Value(value); }
inline void ArgSet::setString(const Token& key, const char*        value) { _args[key] = Value(std::string(value)); }
inline void ArgSet::setString(const Token& key, const Token&       value) { _args[key] = Value(value.asString()); }
inline void ArgSet::setString(const char*  key, const std::string& value) { _args[Token(key)] = Value(value); }
inline void ArgSet::setString(const char*  key, const char*        value) { _args[Token(key)] = Value(std::string(value)); }
inline void ArgSet::setString(const char*  key, const Token&       value) { _args[Token(key)] = Value(value.asString()); }

inline void ArgSet::appendString(const Token& key, char sep, const char*        value) { appendString(key, sep, std::string(value)); }
inline void ArgSet::appendString(const Token& key, char sep, const Token&       value) { appendString(key, sep, value.asString()); }
inline void ArgSet::appendString(const char*  key, char sep, const std::string& value) { appendString(Token(key), sep, value); }
inline void ArgSet::appendString(const char*  key, char sep, const char*        value) { appendString(Token(key), sep, std::string(value)); }
inline void ArgSet::appendString(const char*  key, char sep, const Token&       value) { appendString(Token(key), sep, value.asString()); }

}

#endif
