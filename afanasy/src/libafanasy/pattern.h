#pragma once

#include "host.h"
#include "regexp.h"

namespace af
{
class Pattern
{
public:
   Pattern( const std::string & patternName);
   ~Pattern();

   bool isValid() const;

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.
   void stdOut( bool full = false ) const;

   inline bool setMask( const std::string & string) { return regexp.setPattern( string);}
   inline void setDescription( const std::string & string) { description = string;}

   inline const std::string & getName() const { return name;}
   inline const std::string & getMask() const { return regexp.getPattern();}
   inline const std::string & getDescription() const { return description;}

   inline void setHost( const Host & newhost) { host.copy( newhost );}
	inline void remServices( const std::vector<std::string> & names) { remservices = names;}
   inline void clearServices() { clear_services = true; }
   void getHost( Host & newhost) const;

   inline bool match( const std::string & hostname) const { return regexp.match( hostname); }

   Pattern * ptr_next;

private:
   std::string name;
   std::string description;
   Host host;

private:
   RegExp regexp;
   std::vector<std::string> remservices;
   bool clear_services;
};
}
