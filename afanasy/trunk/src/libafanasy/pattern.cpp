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

bool Pattern::setMask( const std::string & string)
{
   mask = string;
   if( mask.size() == 0 ) return false;
   regexp.setPattern( QString::fromUtf8( mask.c_str()));
   if( regexp.isValid() == false)
   {
      AFERRAR("Pattern::setMask: \"%s\" set invalid mask \"%s\":\n", name.c_str(), mask.c_str());
      printf("%s\n", regexp.errorString().toUtf8().data());
      return false;
   }
   return true;
}

bool Pattern::isValid() const
{
   if( name.size() == 0 )
   {
      AFERROR("Pattern::isValid: name is empty.\n");
      return false;
   }
   if( mask.size() == 0 )
   {
      AFERRAR("Pattern::isValid: \"%s\" mask is empty.\n", name.c_str());
      return false;
   }
   if( regexp.isValid() == false)
   {
      AFERRAR("Pattern::isValid: \"%s\" mask \"%s\" is invalid:\n", name.c_str(), mask.c_str());
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
   if( full)
   {
      stream << "Pattern: \"" << name << "\" (" << description << "):";
      stream << " Mask =\"" << mask << "\"";
      if( remservices.size())
      {
         stream << "\n   Remove services:";
         for( std::list<std::string>::const_iterator it = remservices.begin(); it != remservices.end(); it++)
            stream << " \"" << *it << "\"";
      }
   }
   else
   {
      stream << "P:\"" << mask << "\"-\"" << name << "\": ";
   }
   host.generateInfoStream( stream, full);
}

void Pattern::stdOut( bool full) const
{
   std::ostringstream stream;
   generateInfoStream( stream, full);
   std::cout << stream.str() << std::endl;
}
