#pragma once

#ifndef WINNT
#include <regex.h>
#else
// Provide empty not used values:
static const int REG_EXTENDED = 0;
static const int REG_ICASE = 0;
#endif

#include "name_af.h"

namespace af
{
/// POSIX regural expressions class.
class RegExp
{
public:

/// Constructor.
   RegExp();

   ~RegExp();

   inline const bool empty() const { return pattern.empty(); }
   inline const bool notEmpty() const { return false == pattern.empty(); }
   inline const std::string & getPattern() const { return pattern;}

   static bool Validate( const std::string & str, std::string * strError = NULL);

   bool setPattern( const std::string & str, std::string * strError = NULL);

   inline void setCaseSensitive()   { cflags = compile_flags; }
   inline void setCaseInsensitive() { cflags = compile_flags | REG_ICASE; }

   inline void setMatch()   { contain = false; }
   inline void setContain() { contain = true;  }
   inline void setInclude() { exclude = false; }
   inline void setExclude() { exclude = true;  }

   bool match( const std::string & str) const;

   int weigh() const;

private:

   int cflags;
   bool exclude;
   bool contain;
   std::string pattern;
#ifndef WINNT
   regex_t regexp;
#endif

   static const int compile_flags;
};
}
