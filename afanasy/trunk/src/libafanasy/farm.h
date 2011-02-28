#pragma once

#include <map>

#include "pattern.h"

namespace af
{
class ServiceLimit
{
public:
   ServiceLimit( int MaxCount, int MaxHosts);

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   bool canRun( const std::string & hostname) const;
   bool increment( const std::string & hostname);
   bool releaseHost( const std::string & hostname);
   void getLimits( const ServiceLimit & other);

protected:
   int maxcount;
   int maxhosts;
   int counter;
   std::list< std::string> hostslist;
};

class Farm
{
public:
   Farm( const std::string & File, bool Verbose = false);
   ~Farm();

   inline bool isValid() const { return valid;}

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   bool addPattern( Pattern * p);

   void stdOut( bool full = false ) const;

   bool getHost( const QString & hostname, Host & host, QString & name, QString & description) const;

   /// Check if farm can run a service on a hostname:
   bool serviceLimitCheck( const std::string & service, const std::string & hostname) const;

   /// Add service limit, return false if limits were already reached and this adding is over limit.
   bool serviceLimitAdd( const std::string & service, const std::string & hostname);

   /// Release service limit.
   bool serviceLimitRelease( const std::string & service, const std::string & hostname);

   void servicesLimitsGet( const Farm & other);

   const std::string serviceLimitsInfoString( bool full = false) const; /// Generate services limits information.

private:
   int count;
   bool valid;

   std::string filename;

   Pattern * ptr_first;
   Pattern * ptr_last;

   /// Services limits description:
   std::map< std::string, ServiceLimit * > servicelimits;

private:
   void addServiceLimit( const std::string & name, int maxcount, int maxhosts);
};
}
