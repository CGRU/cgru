#pragma once

#include <map>

#include "farmpattern.h"

namespace af
{
class ServiceLimit
{
public:
	ServiceLimit( int i_max_count, int i_max_hosts);

	void generateInfoStream( std::ostringstream & o_stream, bool i_full = false) const; /// Generate information.
	void jsonWrite( std::ostringstream & o_str) const; /// Generate information.

	bool canRun(      const std::string & i_hostname) const;
	void increment(   const std::string & i_hostname);
	void releaseHost( const std::string & i_hostname);

	void getLimits( const ServiceLimit & i_other);

private:
	int m_max_count;
	int m_max_hosts;

	int m_counter;
	std::list<std::string> m_hosts_names;
	std::list<int> m_hosts_counts;
};

class Farm
{
public:
	Farm( const std::string & File, bool Verbose = false);
	~Farm();

	inline bool isValid() const { return m_valid;}

	void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

	bool addPattern( FarmPattern * p);

	void stdOut( bool full = false ) const;

	bool getHost( const std::string & hostname, Host & host, std::string & name, std::string & description, bool i_verbose = false) const;

	/// Check if farm can run a service on a hostname:
	bool serviceLimitCheck( const std::string & service, const std::string & hostname) const;

	/// Add service limit, return false if limits were already reached and this adding is over limit.
	void serviceLimitAdd( const std::string & service, const std::string & hostname);

	/// Release service limit.
	void serviceLimitRelease( const std::string & service, const std::string & hostname);

	void servicesLimitsGetUsage( const Farm & other);

	const std::string serviceLimitsInfoString( bool full = false) const; /// Generate services limits information.

	void jsonWriteLimits( std::ostringstream & o_str) const; /// Generate services limits information.
	const std::string jsonWriteLimits( ) const; /// Generate services limits information.

	inline const std::string & getText() const { return m_text; }

private:
	bool m_valid;

	std::string m_filename;
	std::string m_text;

	std::vector<FarmPattern *> m_patterns;

	/// Services limits description:
	std::map< std::string, ServiceLimit * > m_servicelimits;

private:
	bool getFarm( const JSON & i_obj);
	void addServiceLimit( const std::string & name, int maxcount, int maxhosts);
};
}
