// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_BASE_STRINGUTILS_H
#define FDK_BASE_STRINGUTILS_H

#include "fdk/api.h"
#include "fdk/base/StringTypes.h"
#include "fdk/base/Hash.h"

#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace fdk
{


//--------------------------------------------------------------------------------
// Misc string functions
//--------------------------------------------------------------------------------


/*! Find all instances of string 'find' in 'str' and replace
    them in place with string 'replace'.
*/
int
stringReplaceAll(std::string& str,
                 const char*  find,
                 const char*  replace_with);

/*! Trim off characters from left side of string.
    Defaults to whitespace characters.
*/
std::string
stringTrimLeft(const std::string& str,
               const char*        trim = " \t\r\n");

/*! Trim off characters from right side of string.
    Defaults to whitespace characters.
*/
std::string
stringTrimRight(const std::string& str,
                const char*        trim = " \t\r\n");

/*! Trim off characters from both sides of string.
    Defaults to whitespace characters.
*/
std::string
stringTrim(const std::string& str,
           const char*        trim = " \t\r\n");

/*! Split a string based on a delimiter list.
*/
void
stringSplit(const std::string&        src,
            const char*               delimiters,
            std::vector<std::string>& results);

//! Alias for common alternative method name
inline void
splitString(const std::string&        src,
            const char*               delimiters,
            std::vector<std::string>& results) { stringSplit(src, delimiters, results); }

//! Create a Hash from a list of strings.
fdk::Hash
buildStringListHash(const fdk::StringList& strings);


//--------------------------------------------------------------------------------
// Pattern matching
//--------------------------------------------------------------------------------


//! Does the string start with 'pattern'?
bool
stringStartsWith(const std::string& str,
                 const char*        pattern);
bool
stringStartsWith(const std::string& str,
                 const std::string& pattern);

//! Does the string end with 'pattern'?
bool
stringEndsWith(const std::string& str,
               const char*        pattern);
bool
stringEndsWith(const std::string& str,
               const std::string& pattern);


/*! Match 'pattern' string in 'text' string using glob-like rules.
    Supports '*' and '?' wildcards.
*/
bool
globMatch(const char* pattern,
          const char* text);
bool
globMatch(const std::string& pattern,
          const std::string& text);


//! Substitute ${VAR} tokens in a string. The \a mappings function will be called to expand tokens.
FDK_API std::string substitute(const std::string& s, const std::function<std::string(const std::string&)>& mappings);

//! Substitute ${VAR} tokens in a string. The \a mappings map will be used to expand tokens.
FDK_API std::string substitute(const std::string& s, const std::map<std::string, std::string>& mappings);




/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*                   Inline Function Implementations                   */
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/


inline bool stringStartsWith(const std::string& a, const char* b) { return (a.find(b) == 0); }
inline bool stringStartsWith(const std::string& a, const std::string& b) { return (a.find(b) == 0); }
//!
inline bool stringEndsWith(const std::string& a, const char* b) { return (a.rfind(b) == (a.size() - strlen(b))); }
inline bool stringEndsWith(const std::string& a, const std::string& b) { return (a.rfind(b) == (a.size() - b.size())); }


inline int
stringReplaceAll(std::string& str,
                 const char*  find,
                 const char*  replace_with)
{
  if (!find || !find[0] || !replace_with) {
    return 0;
  }

  size_t find_len = strlen(find);
  size_t repl_len = strlen(replace_with);
  if (repl_len > 0) {
    ++repl_len;
  }

  int count = 0;
  size_t i = 0;
  while (1)
  {
    const size_t a = str.find(find, i);
    if (a == std::string::npos) { return count; }
    str.replace(a, find_len, replace_with);
    i = a + repl_len;
    ++count;
  }
}

inline std::string
stringTrimLeft(const std::string& str, const char* trim)
{
  if (!str.empty()) {
    const size_t a = str.find_first_not_of(trim);
    if (a != std::string::npos) { return str.substr(a, std::string::npos); }
  }
  return std::string();
}

inline std::string
stringTrimRight(const std::string& str, const char* trim)
{
  if (!str.empty()) {
    const size_t a = str.find_last_not_of(trim);
    if (a != std::string::npos) { return str.substr(0, a+1); }
  }
  return std::string();
}

inline std::string
stringTrim(const std::string& str, const char* trim)
{
  const size_t a = str.find_first_not_of(trim);
  if (a == std::string::npos) { return std::string(); }
  const size_t b = str.find_last_not_of(trim);
  if (b == std::string::npos) { return std::string(); } // shouldn't happen!
  return str.substr(a, (b-a+1));
}

inline void
stringSplit(const std::string&        src,
            const char*               delimiters,
            std::vector<std::string>& results)
{
  results.reserve(3);
  const size_t src_len = src.size();

  size_t i = 0;
  while (1) {
    const size_t a = src.find_first_of(delimiters, i);
    if (a == std::string::npos) {
      if (i < src_len) {
        results.push_back(src.substr(i, std::string::npos)); // add last piece
      }
      return;
    }
    if (i < a) {
      results.push_back(src.substr(i, (a-i)));
    }
    i = a+1;
  }
}

inline fdk::Hash buildStringListHash(const fdk::StringList& strings)
{
  fdk::Hash hash;
  for (const std::string& s : strings) {
    hash.append(s);
  }
  return hash;
}

inline bool
globMatch(const char* pattern,
          const char* text)
{
  if (pattern == nullptr || text == nullptr) {
    return false;
  }

  const char *pp, *tp;
  bool restart_pattern, have_asterisk = false;
  // Loop until we don't have to evaluate pattern string anymore:
  while (1) {
    restart_pattern = false;
    for (pp=pattern, tp=text; *tp != 0; ++pp, ++tp) {
      if (*pp == '*') { // match all
        have_asterisk = true;
        pattern = pp;
        text    = tp;
        if (*++pattern == 0) {
          return true;
        }
        restart_pattern = true;
        break;
      }
      else if (*pp == '?') { // match single
        if (*tp == '.') {
          if (!have_asterisk) {
            return false;
          }
          ++text;
          restart_pattern = true;
          break;
        }
      }
      else {
        if (*tp != *pp) {
          if (!have_asterisk) {
            return false;
          }
          ++text;
          restart_pattern = true;
          break;
        }
      }
    }
    if (!restart_pattern) {
      // We're done:
      if (*pp == '*') {
        ++pp;
      }
      return (*pp == 0);
    }
  }
}

inline bool
globMatch(const std::string& pattern,
          const std::string& text)
{
  return globMatch(pattern.c_str(), text.c_str());
}



} // namespace fdk


#endif
