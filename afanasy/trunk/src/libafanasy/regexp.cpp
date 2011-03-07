#include "regexp.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

const int RegExp::compile_flags = REG_EXTENDED;

RegExp::RegExp():
      contain( false),
      exclude( false),
      cflags( RegExp::compile_flags)
{
}

RegExp::~RegExp()
{
}

bool RegExp::setPattern( const std::string & str, std::string * strError)
{
#ifdef WINNT
   if( RegExp::Validate( str, strError))
   {
      pattern = str;
      return true;
   }
   else
   {
      return false;
   }
#else

   if( str.empty())
   {
      if( false == pattern.empty())
      {
         pattern.clear();
         regfree( &regexp);
      }
      return true;
   }

   if( false == RegExp::Validate( str, strError )) return false;

   if( false == pattern.empty()) regfree( &regexp);
   pattern = str;
   regcomp( &regexp, pattern.c_str(), cflags);
   return true;
#endif
}

bool RegExp::match( const std::string & str) const
{
#ifdef WINNT
   return true;
#else

   if( pattern.empty()) return true;

   regmatch_t regmatch;

   int retval = regexec( &regexp, str.c_str(), 1, &regmatch, 0);
   if(( retval == 0 ) && ( false == contain ))
   {
      if( regmatch.rm_so != 0 ) retval = 1;
      if( regmatch.rm_eo < str.size() ) retval = 1;
   }
   return ((retval == 0) != exclude );

#endif
}

int RegExp::weigh() const
{
   return sizeof(RegExp) + af::weigh( pattern);
}

bool RegExp::Validate( const std::string & str, std::string * strError)
{
   if( str.empty()) return true;

#ifdef WINNT
   return true;
#else

   regex_t check_re;
   int retval = regcomp( &check_re, str.c_str(), compile_flags);

   if( retval == 0 )
   {
      return true;
   }
   else
   {
      static const int buflen = 0xff;
      char buffer[ buflen];
      regerror( retval, &check_re, buffer, buflen);
      if( strError )
         *strError = buffer;
      else
         AFERRAR("REGEXP: %s\n", buffer);
      return false;
   }

#endif
}
