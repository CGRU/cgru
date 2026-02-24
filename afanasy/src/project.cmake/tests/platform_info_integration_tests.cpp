#include "../../libafanasy/platform_info.h"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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
#if defined(LINUX) || defined(MACOSX) || defined(WINNT)
const bool g_has_supported_platform_details = true;
#else
const bool g_has_supported_platform_details = false;
#endif

bool expect(bool i_condition, const std::string & i_message)
{
	if( i_condition)
		return true;
	std::cerr << "FAILED: " << i_message << std::endl;
	return false;
}

bool hasToken(const std::vector<std::string> & i_tokens, const std::string & i_token)
{
	for( std::size_t i = 0; i < i_tokens.size(); i++)
		if( i_tokens[i] == i_token)
			return true;
	return false;
}

std::size_t countToken(const std::vector<std::string> & i_tokens, const std::string & i_token)
{
	std::size_t count = 0;
	for( std::size_t i = 0; i < i_tokens.size(); i++)
		if( i_tokens[i] == i_token)
			count++;
	return count;
}

#ifdef LINUX
bool getLinuxExpected(std::string & o_name, std::string & o_version)
{
	o_name.clear();
	o_version.clear();

	std::ifstream os_release("/etc/os-release");
	if( false == os_release.is_open())
		return false;

	std::string id;
	std::string name;
	std::string version;
	if( false == af::platformParseOsRelease(os_release, id, name, version))
		return false;

	if( false == id.empty())
		o_name = af::platformNormalizeToken(id);
	else
		o_name = af::platformNormalizeToken(name);
	o_version = af::platformNormalizeToken(version);

	return ( false == o_name.empty()) || ( false == o_version.empty());
}
#endif

#ifdef MACOSX
bool getMacExpected(std::string & o_name, std::string & o_version)
{
	o_name = "macos";
	o_version.clear();

	char value[256];
	size_t size = sizeof(value);
	if( sysctlbyname("kern.osproductversion", value, &size, NULL, 0) == 0 )
	{
		o_version = af::platformNormalizeToken(std::string(value, size > 0 ? size - 1 : size));
		return true;
	}

	struct utsname uname_result;
	if( uname(&uname_result) == 0 )
	{
		o_version = af::platformNormalizeToken(uname_result.release);
		return true;
	}

	return false;
}
#endif

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

bool getWindowsExpected(std::string & o_name, std::string & o_version)
{
	o_name.clear();
	o_version.clear();

	unsigned long major = 0;
	unsigned long minor = 0;
	unsigned long build = 0;
	if( false == getWindowsVersion(major, minor, build))
		return false;

	o_name = "windows";
	if( major == 10 )
	{
		o_version = ( build >= 22000 ) ? "11" : "10";
		return true;
	}

	std::ostringstream version_stream;
	version_stream << major << "." << minor << "." << build;
	o_version = af::platformNormalizeToken(version_stream.str());
	return true;
}
#endif

bool testDetectDetailsIntegration()
{
	std::string detected_name;
	std::string detected_version;
	bool detected = af::platformDetectDetails(detected_name, detected_version);

	if( false == g_has_supported_platform_details)
		return true;

	std::string expected_name;
	std::string expected_version;
	bool expected = false;

#ifdef LINUX
	expected = getLinuxExpected(expected_name, expected_version);
#endif
#ifdef MACOSX
	expected = getMacExpected(expected_name, expected_version);
#endif
#ifdef WINNT
	expected = getWindowsExpected(expected_name, expected_version);
#endif

	if( false == expect(expected, "platform-specific API/file query should succeed"))
		return false;
	if( false == expect(detected, "platformDetectDetails should succeed"))
		return false;
	if( false == expect(detected_name == expected_name, "detected name should match host query"))
		return false;
	if( false == expect(detected_version == expected_version, "detected version should match host query"))
		return false;

	return true;
}

bool testTokenVectorIntegration()
{
	const std::vector<std::string> tokens = af::platformGetTokens();
	if( false == expect(false == tokens.empty(), "token vector should not be empty"))
		return false;

#ifdef WINNT
	if( false == expect(tokens[0] == "windows", "first token should be windows"))
		return false;
#else
	if( false == expect(tokens[0] == "unix", "first token should be unix"))
		return false;
#endif

#ifdef LINUX
	if( false == expect(hasToken(tokens, "linux"), "linux token should be present"))
		return false;
#endif
#ifdef MACOSX
	if( false == expect(hasToken(tokens, "macosx"), "macosx token should be present"))
		return false;
#endif

#if defined(__aarch64__) || defined(__arm64__)
	if( false == expect(hasToken(tokens, "aarch64"), "aarch64 token should be present"))
		return false;
#elif defined(__arm__)
	if( false == expect(hasToken(tokens, "arm32"), "arm32 token should be present"))
		return false;
#else
	if( sizeof(void*) == 4 )
	{
		if( false == expect(hasToken(tokens, "32"), "32 token should be present"))
			return false;
	}
	if( sizeof(void*) == 8 )
	{
		if( false == expect(hasToken(tokens, "64"), "64 token should be present"))
			return false;
	}
#endif

	std::string name;
	std::string version;
	bool details_detected = af::platformDetectDetails(name, version);
	if( g_has_supported_platform_details )
	{
		if( false == expect(details_detected, "platformDetectDetails should succeed on supported platform"))
			return false;
	}
	if( false == details_detected)
		return true;
	if( false == name.empty() )
	{
		if( false == expect(hasToken(tokens, name), "detail name token should be present"))
			return false;
	}
	if( false == version.empty() )
	{
		if( false == expect(hasToken(tokens, version), "detail version token should be present"))
			return false;
	}

#ifdef WINNT
	if( false == expect(countToken(tokens, "windows") == 1, "windows token should not be duplicated"))
		return false;
#endif

	return true;
}
}

int main()
{
	if( false == testDetectDetailsIntegration())
		return 1;
	if( false == testTokenVectorIntegration())
		return 1;

	std::cout << "test_platform_info_integration: ok" << std::endl;
	return 0;
}
