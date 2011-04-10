#include "regexp.h"

#ifdef WINNT
#include <regex>
#endif

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
   if( false == pattern.empty()) regfree( &regexp);
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
   if( pattern.empty()) return true;
   else return false == exclude;
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

bool RegExp::Validate( const std::string & str, std::string * errOutput)
{
   if( str.empty()) return true;

#ifdef WINNT

   bool valid = true;
   std::string errStr;
   try 
   {
      std::tr1::regex rx( str);
   }
   catch( const std::tr1::regex_error& rerr)
   {
      errStr = rerr.what();
      valid = false;
   }
   catch (...) 
   {
      errStr = "Unknown exception.";
      valid = false;
   }
   if( false == valid )
   {
      if( errOutput ) *errOutput = errStr;
      else
         AFERRAR("%x\n", errStr.c_str());
   }
   return valid;

#else

   regex_t check_re;
   int retval = regcomp( &check_re, str.c_str(), compile_flags);

   if( retval != 0 )
   {
      static const int buflen = 0xff;
      char buffer[ buflen];
      regerror( retval, &check_re, buffer, buflen);
      if( errOutput )
         *errOutput = buffer;
      else
         AFERRAR("REGEXP: %s\n", buffer);
   }

   regfree( &check_re);

   return retval == 0;

#endif
}
