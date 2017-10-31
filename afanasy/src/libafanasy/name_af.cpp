#include "name_af.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../include/afanasy.h"
#include "../include/afjob.h"

#ifdef WINNT
#include <direct.h>
#include <io.h>
#include <winsock2.h>
#define getcwd _getcwd
#define open _open
#define read _read
#define snprintf _snprintf
#define sprintf sprintf_s
#define stat _stat
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
extern char **environ;
#endif

#include "blockdata.h"
#include "environment.h"
#include "farm.h"
#include "regexp.h"
#include "taskprogress.h"

af::Farm* ferma = NULL;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "logger.h"

#ifdef WINNT
bool LaunchProgramV(
	PROCESS_INFORMATION * o_pinfo,
	HANDLE * o_in,
	HANDLE * o_out,
	HANDLE * o_err,
	const char * i_commanline,
    const char * i_wdir,
	char * i_environ,
    DWORD i_flags,
	bool alwaysCreateWindow);

bool af::launchProgram( PROCESS_INFORMATION * o_pinfo,
                       const std::string & i_commandline, const std::string & i_wdir, char * i_environ,
                       HANDLE * o_in, HANDLE * o_out, HANDLE * o_err,
					   DWORD i_flags, bool alwaysCreateWindow)
{
    const char * wdir = NULL;
    if( i_wdir.size() > 0 )
        wdir = i_wdir.c_str();

	std::string shell_commandline = af::Environment::getCmdShell() + " ";
	shell_commandline += i_commandline;

	return LaunchProgramV( o_pinfo, o_in, o_out, o_err, shell_commandline.c_str(), wdir, i_environ, i_flags, alwaysCreateWindow);
}
void af::launchProgram( const std::string & i_commandline, const std::string & i_wdir)
{
    PROCESS_INFORMATION pinfo;
    af::launchProgram( &pinfo, i_commandline, i_wdir);
}
#else
int LaunchProgramV(
    FILE **o_in,
    FILE **o_out,
    FILE **o_err,
    const char * i_program,
    const char * i_args[],
    const char * wdir = NULL,
	char ** i_environ = NULL);

int af::launchProgram( const std::string & i_commandline, const std::string & i_wdir, char ** i_environ,
                       FILE ** o_in, FILE ** o_out, FILE ** o_err)
{
    const char * wdir = NULL;
    if( i_wdir.size() > 0 )
        wdir = i_wdir.c_str();

	std::vector<std::string> shellWithArgs = af::strSplit( af::Environment::getCmdShell());
	if( shellWithArgs.size() == 0 )
	{
		AFERROR("af::launchProgram: Shell is not defined.")
		return 0;
	}

	const char * shell = shellWithArgs.front().c_str();
	
	if( shellWithArgs.size() == 2)
	{
		// Shell has one argunet - most common case:
		// "bash -c" or "cmd.exe /c"
		const char * args[] = { shellWithArgs.back().c_str(), i_commandline.c_str(), NULL};
		return LaunchProgramV( o_in, o_out, o_err, shell, args, wdir, i_environ);
	}
	else if( shellWithArgs.size() == 1)
	{
		// Shell has no arguments:
		const char * args[] = { i_commandline.c_str(), NULL};
		return LaunchProgramV( o_in, o_out, o_err, shell, args, wdir, i_environ);
	}
	else
	{
		// Collect each shell argument pointer in a single array:
		const char ** args = new const char *[ shellWithArgs.size()+1];
		std::vector<std::string>::iterator it = shellWithArgs.begin();
		it++;
		int i = 0;
		while( i < shellWithArgs.size() - 1)
			args[i++] = (*(it++)).c_str();
		args[shellWithArgs.size()-1] = i_commandline.c_str();
		args[shellWithArgs.size()] = NULL;
		int result = LaunchProgramV( o_in, o_out, o_err, shell, args, wdir, i_environ);
		delete [] args;
		return result;
	}
}
#endif

#ifdef WINNT
char *  af::processEnviron( const std::map<std::string, std::string> & i_env_map)
#else
char ** af::processEnviron( const std::map<std::string, std::string> & i_env_map)
#endif
{
	if( i_env_map.empty())
		return NULL;

	std::vector<std::string> env_vec;
	int env_size = 0;
	#ifdef WINNT
	char * env_str = GetEnvironmentStrings();
	while( env_str[env_size] != '\0')
	{
		std::string str(env_str + env_size);
		env_size += str.size() + 1; ///< For "name=value" '\0' termination
		if( str.empty()) continue;
		env_vec.push_back( str);
	}
	FreeEnvironmentStrings( env_str);
	#else
	for (char ** e = environ; *e != 0; e++)
	{
		std::string str(*e);
		if (str.empty()) continue;
		env_vec.push_back(str);
		env_size += str.size() + 1; ///< For "name=value" '\0' termination
	}
	#endif

	for( std::map<std::string,std::string>::const_iterator it = i_env_map.begin(); it != i_env_map.end(); it++)
		if ((it->first).size() && (it->second).size())
		{
			std::string str = it->first + '=' + it->second;
			env_vec.push_back( str);
			env_size += str.size() + 1; ///< For "name=value" '\0' termination
		}

	env_size++; ///< For the last '\0' termination

	#ifdef WINNT
	char * o_environ = new char[env_size];
	int pos = 0;
	for (int i = 0; i < env_vec.size(); i++)
	{
		strncpy( o_environ + pos, env_vec[i].c_str(), env_vec[i].size());
		pos += env_vec[i].size();
		o_environ[pos] = '\0'; ///< "name=value" '\0' termination
		pos += 1;
	}
	o_environ[env_size-1] = '\0'; /// The last '\0' termination
	#else
	char ** o_environ = new char*[env_vec.size()+1];
	for( int i = 0; i < env_vec.size(); i++)
	{
		o_environ[i] = new char[env_vec[i].size()+1];
		memcpy( o_environ[i], env_vec[i].c_str(), env_vec[i].size());
		o_environ[i][env_vec[i].size()] = '\0'; ///< "name=value" '\0' termination
	}
	o_environ[env_vec.size()] = NULL; /// The last '\0' termination
	#endif

	return o_environ;
}

void af::outError( const char * errMsg, const char * baseMsg)
{
   if( baseMsg )
      AFERRAR("%s: %s", baseMsg, errMsg)
   else
      AFERRAR("%s", errMsg)
}

bool af::init( uint32_t flags)
{
   AFINFO("af::init:\n");
   if( flags & InitFarm)
   {
      AFINFO("af::init: trying to load farm.");

      if( loadFarm( flags & InitVerbose ? VerboseOn : VerboseOff) == false)
		  return false;
   }
   return true;
}

af::Farm * af::farm()
{
   return ferma;
}

bool af::loadFarm( VerboseMode i_verbose)
{
	std::string filename = af::Environment::getAfRoot() + "/farm.json";
	if( loadFarm( filename, i_verbose) == false)
	{
		filename = af::Environment::getAfRoot() + "/farm_example.json";
		if( loadFarm( filename, i_verbose) == false)
		{
			AF_ERR << "Can't load default farm settings file:\n" << filename;
			return false;
		}
	}
	return true;
}

bool af::loadFarm( const std::string & filename, VerboseMode i_verbose)
{
	af::Farm * new_farm = new Farm( filename);

	if( false == new_farm->isValid())
	{
		delete new_farm;
		return false;
	}

	if( ferma != NULL)
	{
		new_farm->servicesLimitsGetUsage( *ferma);
		delete ferma;
	}

	ferma = new_farm;

	if( i_verbose == VerboseOn)
		ferma->stdOut( true);

	return true;
}

void af::destroy()
{
   if( ferma != NULL) delete ferma;
}

void af::sleep_sec(  int i_seconds )
{
#ifdef WINNT
	Sleep( 1000 * i_seconds);
#else
	sleep( i_seconds);
#endif
}

void af::sleep_msec( int i_mseconds)
{
#ifdef WINNT
	Sleep(  i_mseconds);
#else
    usleep( 1000 * i_mseconds);
#endif
}

void af::printTime( time_t time_sec, const char * time_format)
{
   std::cout << time2str( time_sec, time_format);
}

const std::string af::sockAddrToStr( const struct sockaddr_storage * i_ss )
{
	std::ostringstream str;
	af::sockAddrToStr( str, i_ss);
	return str.str();
}
void af::sockAddrToStr( std::ostringstream & o_str, const struct sockaddr_storage * i_ss )
{
	static const int buffer_len = 256;
	char buffer[buffer_len];
	const char * addr_str = NULL;
	uint16_t port = 0;
	switch( i_ss->ss_family)
	{
	case AF_INET:
	{
		const struct sockaddr_in * sa = (const struct sockaddr_in*)(i_ss);
		port = sa->sin_port;
		addr_str = inet_ntoa( sa->sin_addr );
		break;
	}
	case AF_INET6:
	{
		const struct sockaddr_in6 * sa = (const struct sockaddr_in6*)(i_ss);
		port = sa->sin6_port;
		#ifdef WINNT
		addr_str = inet_ntop( AF_INET6, PVOID(&(sa->sin6_addr)), buffer, buffer_len);
		#else
		addr_str = inet_ntop( AF_INET6, &(sa->sin6_addr), buffer, buffer_len);
		#endif
		break;
	}
	default:
		o_str << "Unknown protocol";
	}
	if( addr_str ) o_str << addr_str << ":" << port;
}
void af::printAddress( const struct sockaddr_storage * i_ss )
{
	printf("Address = %s\n", af::sockAddrToStr( i_ss).c_str());
}

bool af::setRegExp( RegExp & regexp, const std::string & str, const std::string & name, std::string * errOutput)
{
   std::string errString;
   if( regexp.setPattern( str, &errString)) return true;

   if( errOutput ) *errOutput = std::string("REGEXP: '") + name + "': " + errString;
   else AFERRAR("REGEXP: '%s': %s", name.c_str(), errString.c_str())

   return false;
}

void af::rw_int32( int32_t& integer, char * data, bool write)
{
   int32_t bytes;
   if( write)
   {
      bytes = htonl( integer);
      memcpy( data, &bytes, 4);
   }
   else
   {
      memcpy( &bytes, data, 4);
      integer = ntohl( bytes);
   }
}

void af::rw_uint32( uint32_t& integer, char * data, bool write)
{
   uint32_t bytes;
   if( write)
   {
      bytes = htonl( integer);
      memcpy( data, &bytes, 4);
   }
   else
   {
      memcpy( &bytes, data, 4);
      integer = ntohl( bytes);
   }
}

bool af::addUniqueToList( std::list<int32_t> & o_list, int i_value)
{
	std::list<int32_t>::iterator it = o_list.begin();
	std::list<int32_t>::iterator end_it = o_list.end();
	while( it != end_it)
		if( (*it++) == i_value)
			return false;
	o_list.push_back( i_value);
	return true;
}

bool af::addUniqueToVect( std::vector<int> & o_vect, int i_value)
{
	for( int i = 0; i < o_vect.size(); i++)
		if( o_vect[i] == i_value)
			return false;
	o_vect.push_back( i_value);
	return true;
}

bool af::addUniqueToVect( std::vector<std::string> & o_vect, const std::string & i_str)
{
	for( int i = 0; i < o_vect.size(); i++)
		if( o_vect[i] == i_str)
			return false;
	o_vect.push_back( i_str);
	return true;
}

const std::string af::fillNumbers( const std::string & i_pattern, long long i_start, long long i_end)
{
	std::string str;
	int pos = 0;
	int nstart = -1;
	int part = 0;
	long long number = i_start;
	while( pos < i_pattern.size())
	{
	    if( i_pattern[pos] == ';')
	    {
	        // If we find ";" character reset number to beginning.
	        // Used for m_files
	        number = i_start;
	    }
		if( i_pattern[pos] == '@')
		{
			if(( nstart != -1) && (( pos - nstart ) > 1))
			{
				// we found the second "@" character in some pattern pair
				// store input string from the last pattern
				str += std::string( i_pattern.data()+part, nstart-part);

				// check for a negative number value
				bool negative;
				if( number < 0 )
				{
					number = -number;
					negative = true;
				}
				else
				{
					negative = false;
				}

				// convert integer to string
				std::string number_str = af::itos( number);
				int number_str_size = number_str.size();
				if( negative )
				{
					// increase size for padding zeors by 1 for "-" character
					// as it will be added after
					number_str_size ++;
				}
				if( number_str_size < ( pos - nstart - 1))
				{
					number_str = std::string( pos - nstart - 1 - number_str_size, '0') + number_str;
				}
				if( negative )
				{
					number_str = '-' + number_str;
				}
				str += number_str;

				// Change numbder from start to end and back (cycle)
				if( negative )
				{
					// return negative if was so
					number = -number;
				}
				if( number == i_start )
				{
					// if last replacement was start, next "should" be "end"
					number = i_end;
				}
				else
				{
					// if last replacement was "end", next should be "start"
					number = i_start;
				}

				// remember where we finished to replace last pattern
				part = pos + 1;

				// reset start position to search new pattern
				nstart = -1;
			}
			else
			{
				// start reading "@#*@" pattern
				// remember first "@" position
				nstart = pos;
			}
		}
		else if( nstart != -1) // "@" was started
		{
			if( i_pattern[pos] != '#') // but character is not "#"
			{
				// reset start position, considering that it is not a pattern
				nstart = -1;
			}
		}

		// go the next character
		pos++;
	}

	if( str.empty() )
	{
		// no patterns were found
		// returning and input string unchanged
		return i_pattern;
	}

	if(( part > 0 ) && ( part < i_pattern.size()))
	{
		// there were some pattern replacements
		// add unchanged string tail (all data from the last replacement)
		str += std::string( i_pattern.data()+part, i_pattern.size()-part);
	}

	return str;
}

const std::string af::replaceArgs( const std::string & pattern, const std::string & arg)
{
   if( pattern.empty()) return arg;

   std::string str( pattern);
   size_t pos = str.find("@#@");
   while( pos != std::string::npos )
   {
      str.replace( pos, 3, arg);
      pos = str.find("@#@");
   }

   return str;
}

int af::weigh( const std::string & str)
{
   return int( str.capacity());
}

int af::weigh( const std::list<std::string> & strlist)
{
   int w = 0;
   for( std::list<std::string>::const_iterator it = strlist.begin(); it != strlist.end(); it++) w += weigh( *it);
   return w;
}

int af::weigh( const std::vector<std::string> & i_list)
{
   int w = 0;
   for( int i = 0; i < i_list.size(); i++) w += weigh( i_list[i]);
   return w;
}

int af::weigh( const std::map<std::string, std::string> & i_map)
{
	int w = 0;
	for( std::map<std::string,std::string>::const_iterator it = i_map.begin(); it != i_map.end(); it++)
		w += weigh(it->first) + weigh(it->second);
	return w;
}

int af::weigh( const std::map<std::string, int32_t> & i_map)
{
	int w = 0;
	for( std::map<std::string,int32_t>::const_iterator it = i_map.begin(); it != i_map.end(); it++)
		w += weigh(it->first) + sizeof(it->second);
	return w;
}

const std::string af::getenv( const std::string & i_name) { return af::getenv( i_name.c_str()); }
const std::string af::getenv( const char * i_name)
{
	std::string envvar;
	char * ptr = ::getenv( i_name);
	if( ptr != NULL ) envvar = ptr;
	return envvar;
}

bool isDec( char c)
{
   if(( c >= '0' ) && ( c <= '9' )) return true;
   return false;
}
bool isHex( char c)
{
   if(( c >= '0' ) && ( c <= '9' )) return true;
   if(( c >= 'a' ) && ( c <= 'f' )) return true;
   if(( c >= 'A' ) && ( c <= 'F' )) return true;
   return false;
}
bool af::netIsIpAddr( const std::string & addr, bool verbose)
{
   bool isIPv4 = false;
   bool isIPv6 = false;
   for( int i = 0; i < addr.size(); i++)
   {
      if(( isIPv6 == false ) && ( addr[i] == '.' ))
      {
         isIPv4 = true;
         continue;
      }
      if(( isIPv4 == false ) && ( addr[i] == ':' ))
      {
         isIPv6 = true;
         continue;
      }
      if( isDec( addr[i])) continue;
      if( isIPv4 )
      {
         isIPv4 = false;
         break;
      }
      if( false == isHex( addr[i]))
      {
         isIPv6 = false;
         break;
      }
   }
   if( verbose)
   {
      if( isIPv4 ) printf("IPv4 address.\n");
      else if ( isIPv6 ) printf("IPv6 address.\n");
      else printf("Not an IP adress.\n");
   }
   if(( isIPv4 == false ) && ( isIPv6 == false )) return false;
   return true;
}
