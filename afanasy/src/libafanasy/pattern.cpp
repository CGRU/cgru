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
	if( full)
	{
		stream << "\nPattern: \"" << name << "\" (" << description << "):";
		stream << "\n    Mask =\"" << regexp.getPattern() << "\"";
		if( clear_services ) stream << "\n    Clear services.";
		else if( remservices.size())
		{
			stream << "\n    Remove services:";
			for( int i = 0; i < remservices.size(); i++)
				stream << " \"" << remservices[i] << "\"";
		}
		stream << std::endl;
	}

	host.v_generateInfoStream( stream, full);
	stream << std::endl;
	host.generateServicesStream( stream);

	if( false == full )
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
