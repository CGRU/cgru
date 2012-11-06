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
	void jsonWrite( std::ostringstream & o_str) const; /// Generate information.

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

   inline bool isValid() const { return m_valid;}

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   bool addPattern( Pattern * p);

   void stdOut( bool full = false ) const;

   bool getHost( const std::string & hostname, Host & host, std::string & name, std::string & description) const;

   /// Check if farm can run a service on a hostname:
   bool serviceLimitCheck( const std::string & service, const std::string & hostname) const;

   /// Add service limit, return false if limits were already reached and this adding is over limit.
   bool serviceLimitAdd( const std::string & service, const std::string & hostname);

   /// Release service limit.
   bool serviceLimitRelease( const std::string & service, const std::string & hostname);

   void servicesLimitsGetUsage( const Farm & other);

   const std::string serviceLimitsInfoString( bool full = false) const; /// Generate services limits information.

	void jsonWriteLimits( std::ostringstream & o_str) const; /// Generate services limits information.

private:
	int m_count;
	bool m_valid;

	std::string m_filename;

	Pattern * m_ptr_first;
	Pattern * m_ptr_last;

	/// Services limits description:
	std::map< std::string, ServiceLimit * > m_servicelimits;

private:
	bool getFarm( const JSON & i_obj);
	void addServiceLimit( const std::string & name, int maxcount, int maxhosts);
};
}
