#include "pattern.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Pattern::Pattern( const std::string & patternName):
   ptr_next( NULL),
   name( patternName),
   description("")
{
}

Pattern::~Pattern()
{
}

bool Pattern::isValid() const
{
   if( name.size() == 0 )
   {
      AFERROR("Pattern::isValid: name is empty.")
      return false;
   }
   if( regexp.empty())
   {
      AFERRAR("Pattern::isValid: \"%s\" mask is empty.", name.c_str())
      return false;
   }
   return true;
}

void Pattern::getHost( Host & newhost) const
{
   newhost.merge( host);
   if( remservices.size()) newhost.remServices( remservices);
}

void Pattern::generateInfoStream( std::ostringstream & stream, bool full) const
{
   host.generateInfoStream( stream, full);
   if( full)
   {
      stream << std::endl;
      stream << "Pattern: \"" << name << "\" (" << description << "):";
      stream << " Mask =\"" << regexp.getPattern() << "\"";
      if( remservices.size())
      {
         stream << "\n   Remove services:";
         for( std::list<std::string>::const_iterator it = remservices.begin(); it != remservices.end(); it++)
            stream << " \"" << *it << "\"";
      }
   }
   else
   {
      stream << "P:\"" << regexp.getPattern() << "\"-\"" << name << "\": ";
   }
}

void Pattern::stdOut( bool full) const
{
   std::ostringstream stream;
   generateInfoStream( stream, full);
   std::cout << stream.str() << std::endl;
}
