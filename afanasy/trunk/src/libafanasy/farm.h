#pragma once

#include "pattern.h"

namespace af
{
class Farm
{
public:
   Farm( const QString & File, bool Verbose = false);
   ~Farm();

   inline bool isValid() const { return valid;}

   bool addPattern( Pattern * p);

   void stdOut( bool full = false ) const;

   bool getHost( const QString & hostname, Host & host) const;
   bool getHost( const QString & hostname, Host & host, QString & name) const;
   bool getHost( const QString & hostname, Host & host, QString & name, QString & description) const;

private:
   int count;
   QString filename;
   Pattern * ptr_first;
   Pattern * ptr_last;
   bool valid;
};
}
