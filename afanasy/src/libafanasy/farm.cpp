#include "farm.h"

//#include <string.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

ServiceLimit::ServiceLimit( int MaxCount, int MaxHosts):
	maxcount( MaxCount),
	maxhosts( MaxHosts),
	counter( 0)
{
}


bool ServiceLimit::canRun( const std::string & hostname) const
{
	if( maxcount != -1 ) if( counter >= maxcount ) return false; // Check maximum m_count
	if( maxhosts != -1 ) // Check maximum hosts
	{
		// If host already exists service can run on it:
		for( std::list< std::string>::const_iterator it = hostslist.begin(); it != hostslist.end(); it++)
			if( *it == hostname ) return true;

		// Check whether we can add one more host:
		if( hostslist.size() >= maxhosts ) return false;
	}
	return true;
}

void ServiceLimit::generateInfoStream( std::ostringstream & stream, bool full) const
{
	if( full)
		stream << "Count = " << counter << "/" <<  maxcount << "; Hosts = " << hostslist.size() << "/" << maxhosts;
	else
		stream << "c" << counter << "/" <<  maxcount << " h" << hostslist.size() << "/" << maxhosts;
}
void ServiceLimit::jsonWrite( std::ostringstream & o_str) const
{
	o_str << "{";
	o_str << "\"m_count\":" << counter;
	o_str << ",\"max_count\":" << maxcount;
	o_str << ",\"hosts\":[";
	for( std::list<std::string>::const_iterator it = hostslist.begin(); it != hostslist.end(); it++)
	{
		if( it != hostslist.begin()) o_str << ",";
		o_str << "\"" << *it << "\"";
	}
	o_str << "],\"max_hosts\":" << maxhosts;
	o_str << "}";
}

bool ServiceLimit::increment( const std::string & hostname)
{
	bool retval = true;

	if( counter >= maxcount ) retval = false; // Check counter
	counter++;										  // Increase counter

	if( hostslist.size() >= maxhosts ) retval = false; // Check hosts list

	bool hostexists = false;
	for( std::list< std::string>::const_iterator it = hostslist.begin(); it != hostslist.end(); it++)
		if( *it == hostname )
		{
			hostexists = true;
			break;
		}
	if( false == hostexists ) hostslist.push_back( hostname); // Appent hostslist with hostname if it does not exist

	return retval;
}

bool ServiceLimit::releaseHost( const std::string & hostname)
{
	bool retval = true;

	if( counter <= 0 ) retval = false;
	else counter--;

	bool hostexists = false;
	std::list< std::string>::iterator it = hostslist.begin();
	for( ; it != hostslist.end(); it++)
		if( *it == hostname )
		{
			hostexists = true;
			hostslist.erase( it);
			break;
		}
	if( false == hostexists ) retval = false;

	return retval;
}

void ServiceLimit::getLimits( const ServiceLimit & other)
{
	if( other.counter >= 0 ) counter = other.counter;
	hostslist = other.hostslist;
}

//############################################## Farm ########################################

Farm::Farm( const std::string & File, bool Verbose ):
	m_count( 0),
	m_filename( File),
	m_ptr_first( NULL),
	m_ptr_last( NULL),
	m_valid( false)
{
	if( false == pathFileExists( m_filename))
	{
		return;
	}

	int filesize = -1;
	char * buffer = fileRead( m_filename, filesize);
	if( buffer == NULL )
	{
		printf("Farm: File \"%s\" reading error.\n", m_filename.c_str());
		return;
	}

	rapidjson::Document document;
	char * data = af::jsonParseData( document, buffer, filesize);
	if( data == NULL )
	{
		delete buffer;
		return;
	}

	const JSON & obj = document["farm"];
	if( false == obj.IsObject())
	{
		AFERRAR("Farm: Can't find document root \"farm\": object:\n%s\n", m_filename.c_str())
	}
	else
	{
		m_valid = getFarm( obj);
	}

	delete [] buffer;
	delete [] data;
}

bool Farm::getFarm( const JSON & i_obj)
{
	const JSON & patterns = i_obj["patterns"];
	if( false == patterns.IsArray())
	{
		AFERROR("Farm: \"patterns\" should be an array.\n");
		return false;
	}
	if( patterns.Size() < 1 )
	{
		AFERROR("Farm: \"patterns\" array has zero size.\n");
		return false;
	}

	for( int i = 0; i < patterns.Size(); i++)
	{
		if( false == patterns[i].IsObject() )
		{
			AFERRAR("Farm: Pattern[%d] is not an object.", i)
			return false;
		}

		std::string name, description, mask;
		std::vector<std::string> remservices;
		bool clear_services = false;

		jr_string("name", name, patterns[i]);
		jr_string("mask", mask, patterns[i]);

		if( name.empty())
		{
			AFERRAR("Pattern[%d] has no name.", i)
			return false;
		}
		if( mask.empty())
		{
			AFERRAR("Pattern '%s' has an empty hosts name mask.", name.c_str())
			return false;
		}

		Host host;

		jr_string("description", description, patterns[i]);
		jr_string("properties", host.m_properties, patterns[i]);
		jr_string("os", host.m_os, patterns[i]);
		jr_int32("capacity", host.m_capacity, patterns[i]);
		jr_int32("power", host.m_power, patterns[i]);
		jr_int32("maxtasks", host.m_max_tasks, patterns[i]);
		jr_int32("wol_idlesleep_time", host.m_wol_idlesleep_time, patterns[i]);
		jr_int32("nimby_idlefree_time", host.m_nimby_idlefree_time, patterns[i]);
		jr_int32("nimby_busyfree_time", host.m_nimby_busyfree_time, patterns[i]);
		jr_int32("idle_cpu", host.m_idle_cpu, patterns[i]);
		jr_int32("busy_cpu", host.m_busy_cpu, patterns[i]);
		if( jr_stringvec("remservices", remservices, patterns[i]))
			if( remservices.size() == 0 )
				clear_services = true;

		const JSON & services = patterns[i]["services"];
		if( services.IsArray())
		{
			for( int j = 0; j < services.Size(); j++)
			{
				std::string service_name;
				int service_count = 0;

				jr_string("name", service_name, services[j]);
				if( name.empty())
				{
					AFERRAR("Farm: Pattern['%s'] service[%d] has no name.", name.c_str(), j)
					return false;
				}

				jr_int("count", service_count, services[j]);

				host.setService( service_name, service_count);
			}
		}

		Pattern * pat = new Pattern( name);
		pat->setMask( mask);
		pat->setDescription( description);
		if( clear_services )
		{
			pat->clearServices();
		}
		else
		{
			pat->remServices( remservices);
		}
		pat->setHost( host);
		if( addPattern( pat) == false)
		{
			delete pat;
			return false;
		}
	}

	const JSON & limits = i_obj["limits"];
	if( false == limits.IsArray())
		return true;

	for( int i = 0; i < limits.Size(); i++)
	{
		if( false == limits[i].IsObject())
		{
			AFERRAR("Farm: limit[%d] is not an object.", i)
			return false;
		}

		std::string service;
		jr_string("service", service, limits[i]);
		if( service.empty())
		{
			AFERRAR("Farm: limit[%d] has no service name.", i)
			return false;
		}
		
		int maxcount, maxhosts = -1;
		jr_int("maxcount", maxcount, limits[i]);
		jr_int("maxhosts", maxhosts, limits[i]);

		if(( maxcount < 0 ) && ( maxhosts < 0 ))
		{
			AFERRAR("Service \"%s\" has invalid limits.", service.c_str())
			return false;
		}

		addServiceLimit( service, maxcount, maxhosts);
	}

	return true;
}

Farm::~Farm()
{
	while( m_ptr_first != NULL)
	{
		m_ptr_last = m_ptr_first;
		m_ptr_first = m_ptr_first->ptr_next;
		delete m_ptr_last;
	}
	for( std::map<std::string, ServiceLimit*>::const_iterator it = m_servicelimits.begin(); it != m_servicelimits.end(); it++)
		delete (*it).second;
}

void Farm::addServiceLimit( const std::string & name, int maxcount, int maxhosts)
{
	if( m_servicelimits.find( name) != m_servicelimits.end())
	{
		AFERRAR("Farm::addService: Service \"%s\" already exists.", name.c_str())
		return;
	}
	if( maxcount < -1 )
	{
		AFERRAR("Farm::addService: Service \"%s\" maxcount value is invalid \"%d\". Setting as \"-1\"", name.c_str(), maxcount)
		maxcount = -1;
	}
	if( maxhosts < -1 )
	{
		AFERRAR("Farm::addService: Service \"%s\" maxhosts value is invalid \"%d\". Setting as \"-1\"", name.c_str(), maxhosts)
		maxhosts = -1;
	}
	if(( maxcount == -1 ) && ( maxhosts == -1 ))
	{
		AFERRAR("Farm::addService: Service \"%s\" has and maxcount and maxhosts negative values.", name.c_str())
		return;
	}
	m_servicelimits[name] = new ServiceLimit( maxcount, maxhosts);
}

bool Farm::addPattern( Pattern * pattern)
{
	if( pattern->isValid() == false)
	{
		AFERRAR("Farm::addPattern: invalid pattern \"%s\"", pattern->getName().c_str())
		return false;
	}
	if( m_ptr_first == NULL)
	{
		m_ptr_first = pattern;
	}
	else
	{
		m_ptr_last->ptr_next = pattern;
	}
	m_ptr_last = pattern;
	m_count++;
	return true;
}

void Farm::generateInfoStream( std::ostringstream & stream, bool full) const
{
	stream << "Farm filename = \"" << m_filename << "\":";
	Pattern * pattern = m_ptr_first;
	while( pattern != NULL)
	{
		stream << std::endl;
		pattern->generateInfoStream( stream, full);
		pattern = pattern->ptr_next;
	}

	if( m_servicelimits.empty()) return;

	if( full ) stream << "\n\nServices Limits:";
	else stream << " limits:";
	for( std::map<std::string, ServiceLimit*>::const_iterator it = m_servicelimits.begin(); it != m_servicelimits.end(); it++)
	{
		stream << std::endl;
		if( full ) stream << "	";
		stream << (*it).first << ": ";
		(*it).second->generateInfoStream( stream, full);
	}
}

void Farm::stdOut( bool full) const
{
	std::ostringstream stream;
	generateInfoStream( stream, full);
	std::cout << stream.str() << std::endl;
}

bool Farm::getHost( const std::string & hostname, Host & host, std::string & name, std::string & description) const
{
	Pattern * ptr = NULL;
	for( Pattern * p = m_ptr_first; p != NULL; p = p->ptr_next)
	{
		if( p->match( hostname)) ptr = p;
		if( ptr == NULL) continue;
		ptr->getHost( host);
	}
	if( ptr == NULL ) return false;
	name = ptr->getName();
	description = ptr->getDescription();
	return true;
}

bool Farm::serviceLimitCheck( const std::string & service, const std::string & hostname) const
{
	// Find a service:
	std::map< std::string, ServiceLimit * >::const_iterator it = m_servicelimits.find( service);

	// If there is no limits description, it can be run in anyway:
	if( it == m_servicelimits.end()) return true;

	return (*it).second->canRun( hostname);
}

bool Farm::serviceLimitAdd( const std::string & service, const std::string & hostname)
{
	// Find a service:
	std::map< std::string, ServiceLimit * >::const_iterator it = m_servicelimits.find( service);

	// If there is no limits description, we do not add it:
	if( it == m_servicelimits.end()) return true;

	return (*it).second->increment( hostname);
}

bool Farm::serviceLimitRelease( const std::string & service, const std::string & hostname)
{
	// Find a service:
	std::map< std::string, ServiceLimit * >::const_iterator it = m_servicelimits.find( service);

	// If there is no limits description, we do not add it:
	if( it == m_servicelimits.end()) return true;

	return (*it).second->releaseHost( hostname);
}

void Farm::servicesLimitsGetUsage( const Farm & other)
{
	for( std::map<std::string, ServiceLimit*>::iterator it = m_servicelimits.begin(); it != m_servicelimits.end(); it++)
	{
		for( std::map<std::string, ServiceLimit*>::const_iterator oit = other.m_servicelimits.begin(); oit != other.m_servicelimits.end(); oit++)
		{
			if((*it).first == (*oit).first)
				(*it).second->getLimits(*((*oit).second));
		}
	}
}

const std::string Farm::serviceLimitsInfoString( bool full) const
{
	if( m_servicelimits.size() < 1 ) return std::string();

	std::ostringstream stream;

	if( full ) stream << "Services Limits:";
	else stream << " limits:";
	for( std::map<std::string, ServiceLimit*>::const_iterator it = m_servicelimits.begin(); it != m_servicelimits.end(); it++)
	{
		stream << std::endl;
		if( full ) stream << "	";
		stream << (*it).first << ": ";
		(*it).second->generateInfoStream( stream, full);
	}

	return stream.str();
}
void Farm::jsonWriteLimits( std::ostringstream & o_str) const
{
	o_str << "\"services_limits\":{";

	for( std::map<std::string, ServiceLimit*>::const_iterator it = m_servicelimits.begin(); it != m_servicelimits.end(); it++)
	{
		if( it != m_servicelimits.begin()) o_str << ",";
		o_str << "\"" << (*it).first << "\":";
		(*it).second->jsonWrite( o_str);
	}

	o_str << "}";
}

