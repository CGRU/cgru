#ifndef AF_PATTERN_H
#define AF_PATTERN_H

#include <QtCore/QString>
#include <QtCore/QRegExp>

#include "host.h"

namespace af
{
class Pattern
{
public:
   Pattern( const QString & string);
   ~Pattern();

   bool isValid() const;

   void stdOut( bool full = false ) const;

   bool setMask( const QString & string);
   inline void setDescription( const QString & string) { description = string;}

   inline const QString & getName() const { return name;}
   inline const QString & getMask() const { return mask;}
   inline const QString & getDescription() const { return description;}

   inline void setHost( const Host & newhost)       {    host.copy( newhost ); }
   inline void getHost(       Host & newhost) const { newhost.copy( host    ); }

   inline bool match( const QString & hostname) const { return regexp.exactMatch( hostname); }

   Pattern * ptr_next;

private:
   QString name;
   QString mask;
   QString description;
   Host host;

private:
   QRegExp regexp;
};
}
#endif
