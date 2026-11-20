// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_BASE_TOKEN_H
#define USG_BASE_TOKEN_H

#include "usg/api.h"
#include "usg/base/Array.h"

#include <ostream>
#include <string>
#include <vector>
#include <set>

namespace fdk {
  class Hash;
}

namespace usg {
  //! A  wrapper for TfToken
  class USG_API Token
  {
  public:
    class Handle; //!< Abstraction handle for casting to underlying implementation type

    Token();
    Token(const char* text);
    Token(const std::string& text);
    Token(const Token::Handle& handle);
    ~Token();

    // movable:
    Token(Token&& rhs) noexcept;
    Token& operator=(Token&& rhs) noexcept;

    // and copyable
    Token(const Token& rhs);
    Token& operator=(const Token& rhs);

    const char* text() const;
    const std::string& asString() const;

    bool empty() const;

    bool operator==(const Token& rhs) const;
    bool operator!=(const Token& rhs) const { return !(*this == rhs); }

    bool operator<(const Token& rhs) const;
    bool operator>(const Token& rhs) const { return (rhs < *this); }
    bool operator<=(const Token& rhs) const { return !(*this > rhs); }
    bool operator>=(const Token& rhs) const { return !(*this < rhs); }

    //! Get the underlying TfToken
    Token::Handle&       getToken(int usd_version);
    const Token::Handle& getToken(int usd_version) const;

    //! Get the hash of the underlying TfToken
    uint64_t hash() const;


    friend std::ostream& operator << (std::ostream& o, const Token& t) { return o << t.asString(); }

  public:
    //! Returns the index of the underlying handle. For internal use only!
    int handleIndex() const { return _handle; }
    //! Assign from a pre-known handle index. For internal use only!
    explicit Token(int handleIndex) : _handle(handleIndex) {}

  public:
    //! Lazily contruct a Token when requested. Primarily used for the static Token structs.
    class Lazy {
    public:
      Lazy(const char* text) : _text(text) {}

      const Token& token() const
      {
        if (!_token) {
          const_cast<Token::Lazy*>(this)->_token = new Token(_text);
        }
        return *_token;
      }

      operator const Token&() const { return token(); }
      operator       Token&() { return const_cast<Token&>(token()); }

      //! Convenience to avoid writing 'MyStaticTokens.MyToken.token().asString()'.
      const std::string& asString() const { return token().asString(); }

      Token* _token;
      const char* _text;
    };

  private:
    int _handle;
  };

  using TokenArray = usg::Array<Token>;
  using TokenSet   = std::set<Token>;

}


// Allows Token use in std unordered_map, etc:
namespace std
{
  template <>
  struct hash<usg::Token>
  {
    std::size_t operator()(const usg::Token& token) const noexcept { return static_cast<size_t>(token.hash()); }
  };
}

#endif
