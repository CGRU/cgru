#pragma once

#include <regex.h>

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
   inline const std::string & getPattern() const { return pattern;}

   bool setPattern( const std::string & str, std::string * strError = NULL);

   inline void setCaseSensitive()   { cflags = compile_flags; }
   inline void setCaseInsensitive() { cflags = compile_flags | REG_ICASE; }
   inline void setMatch()   { contain = false; }
   inline void setContain() { contain = true;  }
   inline void setInclude() { exclude = false; }
   inline void setExclude() { exclude = true;  }

   bool match( const std::string & str);

   int weigh() const;

private:

   int cflags;
   bool exclude;
   bool contain;
   regex_t regexp;
   std::string pattern;

   static const int compile_flags;
};
}
