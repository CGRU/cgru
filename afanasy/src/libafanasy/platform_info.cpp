/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

#include "platform_info.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <sstream>

#ifdef MACOSX
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef WINNT
#include <windows.h>
#endif

namespace
{
void addUniqueToken(std::vector<std::string> & io_tokens, const std::string & i_token)
{
	if( i_token.empty())
		return;

	for( std::size_t i = 0; i < io_tokens.size(); i++)
		if( io_tokens[i] == i_token)
			return;

	io_tokens.push_back(i_token);
}

std::string trim(const std::string & i_str)
{
	size_t first = 0;
	while( first < i_str.size())
	{
		if( false == std::isspace(static_cast<unsigned char>(i_str[first])))
			break;
		first++;
	}
	if( first == i_str.size())
		return std::string();

	size_t last = i_str.size() - 1;
	while( last > first)
	{
		if( false == std::isspace(static_cast<unsigned char>(i_str[last])))
			break;
		last--;
	}

	return i_str.substr(first, last - first + 1);
}

std::string stripQuotes(const std::string & i_value)
{
	if( i_value.size() < 2)
		return i_value;

	char first = i_value[0];
	char last = i_value[i_value.size() - 1];
	if((( first == '"') && ( last == '"')) || (( first == '\'') && ( last == '\'')))
		return i_value.substr(1, i_value.size() - 2);

	return i_value;
}

#ifdef WINNT
bool getWindowsVersion(unsigned long & o_major, unsigned long & o_minor, unsigned long & o_build)
{
	typedef LONG(WINAPI *RtlGetVersionFn)(OSVERSIONINFOW*);
	RtlGetVersionFn rtlGetVersion = NULL;

	HMODULE module = GetModuleHandleA("ntdll.dll");
	if( module != NULL)
		rtlGetVersion = reinterpret_cast<RtlGetVersionFn>(GetProcAddress(module, "RtlGetVersion"));

	OSVERSIONINFOW info;
	ZeroMemory(&info, sizeof(info));
	info.dwOSVersionInfoSize = sizeof(info);

	if( rtlGetVersion && (rtlGetVersion(&info) == 0))
	{
		o_major = info.dwMajorVersion;
		o_minor = info.dwMinorVersion;
		o_build = info.dwBuildNumber;
		return true;
	}

	return false;
}

std::string windowsVersionToken(unsigned long i_major, unsigned long i_minor, unsigned long i_build)
{
	// Keep latest two desktop generations stable as major OS versions.
	if( i_major == 10 )
	{
		if( i_build >= 22000 )
			return "11";
		return "10";
	}

	std::ostringstream version_stream;
	version_stream << i_major << "." << i_minor << "." << i_build;
	return version_stream.str();
}
#endif
}

bool af::platformParseOsReleaseLine(const std::string & i_line, std::string & o_key, std::string & o_value)
{
	o_key.clear();
	o_value.clear();

	if( i_line.empty())
		return false;
	if( i_line[0] == '#')
		return false;

	size_t pos = i_line.find('=');
	if( pos == std::string::npos)
		return false;

	o_key = trim(i_line.substr(0, pos));
	o_value = trim(i_line.substr(pos + 1));
	o_value = stripQuotes(o_value);

	if( o_key.empty())
		return false;

	return true;
}

bool af::platformParseOsRelease(std::istream & io_stream, std::string & o_id, std::string & o_name, std::string & o_version)
{
	o_id.clear();
	o_name.clear();
	o_version.clear();

	std::string line;
	while( std::getline(io_stream, line))
	{
		std::string key;
		std::string value;
		if( false == platformParseOsReleaseLine(line, key, value))
			continue;

		if( key == "ID")
			o_id = value;
		else if( key == "NAME")
			o_name = value;
		else if( key == "VERSION_ID")
			o_version = value;
	}

	return ( false == o_id.empty()) || ( false == o_name.empty()) || ( false == o_version.empty());
}

std::string af::platformNormalizeToken(const std::string & i_token)
{
	std::string token = trim(i_token);
	if( token.empty())
		return token;

	std::string out;
	out.reserve(token.size());

	bool has_alnum = false;
	for( std::size_t i = 0; i < token.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(token[i]);
		if( std::isalnum(c))
		{
			has_alnum = true;
			out.push_back(static_cast<char>(std::tolower(c)));
		}
		else if(( c == '.') || ( c == '_') || ( c == '-') || std::isspace(c))
		{
			out.push_back(( std::isspace(c)) ? '_' : static_cast<char>(c));
		}
		else
			out.push_back('_');
	}

	while(( false == out.empty()) && ( out[0] == '_'))
		out.erase(out.begin());
	while(( false == out.empty()) && ( out[out.size() - 1] == '_'))
		out.erase(out.size() - 1, 1);

	std::string collapsed;
	collapsed.reserve(out.size());
	char prev = '\0';
	for( std::size_t i = 0; i < out.size(); i++)
	{
		char c = out[i];
		if(( c == '_') && ( prev == '_'))
			continue;
		collapsed.push_back(c);
		prev = c;
	}

	if( false == has_alnum)
		return std::string();

	return collapsed;
}

bool af::platformDetectDetails(std::string & o_name_token, std::string & o_version_token)
{
	o_name_token.clear();
	o_version_token.clear();

#ifdef LINUX
	std::ifstream os_release("/etc/os-release");
	if( os_release.is_open())
	{
		std::string id;
		std::string name;
		std::string version;
		if( platformParseOsRelease(os_release, id, name, version))
		{
			if( false == id.empty())
				o_name_token = platformNormalizeToken(id);
			else
				o_name_token = platformNormalizeToken(name);
			o_version_token = platformNormalizeToken(version);
		}
	}
#endif

#ifdef MACOSX
	o_name_token = "macos";

	char value[256];
	size_t size = sizeof(value);
	if( sysctlbyname("kern.osproductversion", value, &size, NULL, 0) == 0 )
		o_version_token = platformNormalizeToken(std::string(value, size > 0 ? size - 1 : size));
	else
	{
		struct utsname uname_result;
		if( uname(&uname_result) == 0 )
			o_version_token = platformNormalizeToken(uname_result.release);
	}
#endif

#ifdef WINNT
	unsigned long major = 0;
	unsigned long minor = 0;
	unsigned long build = 0;
	if( getWindowsVersion(major, minor, build))
	{
		o_name_token = "windows";
		o_version_token = platformNormalizeToken(windowsVersionToken(major, minor, build));
	}
#endif

	return ( false == o_name_token.empty()) || ( false == o_version_token.empty());
}

std::vector<std::string> af::platformGetTokens()
{
	std::vector<std::string> tokens;

#ifdef WINNT
	addUniqueToken(tokens, "windows");
#else
	addUniqueToken(tokens, "unix");
#endif

#ifdef MACOSX
	addUniqueToken(tokens, "macosx");
#endif

#ifdef LINUX
	addUniqueToken(tokens, "linux");
#endif

	switch( sizeof(void*))
	{
	case 4: addUniqueToken(tokens, "32"); break;
	case 8: addUniqueToken(tokens, "64"); break;
	}

	std::string name_token;
	std::string version_token;
	platformDetectDetails(name_token, version_token);

	addUniqueToken(tokens, name_token);
	addUniqueToken(tokens, version_token);

	return tokens;
}
