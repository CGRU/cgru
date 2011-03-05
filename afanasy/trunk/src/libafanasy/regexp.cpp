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
   if( str.empty())
   {
      if( false == pattern.empty())
      {
         pattern.clear();
         regfree( &regexp);
      }
      return true;
   }

   regex_t check_re;
   int retval = regcomp( &check_re, str.c_str(), cflags);

   if( retval == 0 )
   {
      if( false == pattern.empty()) regfree( &regexp);
      pattern = str;
      regcomp( &regexp, pattern.c_str(), cflags);
      return true;
   }
   else
   {
      static const int buflen = 0xff;
      char buffer[ buflen];
      regerror( retval, &regexp, buffer, buflen);
      if( strError )
         *strError = buffer;
      else
         AFERRAR("REGEXP: %s\n", buffer);
      return false;
   }
}

bool RegExp::match( const std::string & str)
{
   if( pattern.empty()) return false == exclude;

   regmatch_t regmatch;

   int retval = regexec( &regexp, str.c_str(), 1, &regmatch, 0);
   if(( retval == 0 ) && ( false == contain ))
   {
      if( regmatch.rm_so != 0 ) retval = 1;
      if( regmatch.rm_eo < str.size() ) retval = 1;
   }
   return ((retval == 0) != exclude );
}

int RegExp::weigh() const
{
   return sizeof(RegExp) + af::weigh( pattern);
}
