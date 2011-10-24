#include "pattern.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Pattern::Pattern( const std::string & patternName):
   ptr_next( NULL),
   name( patternName),
   clear_services( false),
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
   if( clear_services) newhost.clearServices();
   else if( remservices.size()) newhost.remServices( remservices);
   newhost.merge( host);
}

void Pattern::generateInfoStream( std::ostringstream & stream, bool full) const
{
   host.generateInfoStream( stream, full);
   stream << std::endl;
   host.generateServicesStream( stream);
   if( full)
   {
      stream << std::endl;
      stream << "Pattern: \"" << name << "\" (" << description << "):";
      stream << " Mask =\"" << regexp.getPattern() << "\"";
      if( clear_services ) stream << "\n   Clear services.\n";
      else if( remservices.size())
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
