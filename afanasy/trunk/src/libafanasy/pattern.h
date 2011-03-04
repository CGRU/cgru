#pragma once

#include <QtCore/QString>
#include <QtCore/QRegExp>

#include "host.h"

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

   bool setMask( const std::string & string);
   inline void setDescription( const std::string & string) { description = string;}

   inline const std::string & getName() const { return name;}
   inline const std::string & getMask() const { return mask;}
   inline const std::string & getDescription() const { return description;}

   inline void setHost( const Host & newhost) { host.copy( newhost );}
   inline void remServices( const std::list<std::string> & names) { remservices = names;}
   void getHost( Host & newhost) const;

   inline bool match( const std::string & hostname) const { return regexp.exactMatch( QString::fromUtf8( hostname.c_str())); }

   Pattern * ptr_next;

private:
   std::string name;
   std::string mask;
   std::string description;
   Host host;

private:
   QRegExp regexp;
   std::list<std::string> remservices;
};
}
