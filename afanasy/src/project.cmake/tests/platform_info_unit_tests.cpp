#include "../../libafanasy/platform_info.h"

#include <iostream>
#include <sstream>
#include <string>

namespace
{
bool expect(bool i_condition, const std::string & i_message)
{
	if( i_condition)
		return true;
	std::cerr << "FAILED: " << i_message << std::endl;
	return false;
}

bool testParseLine()
{
	std::string key;
	std::string value;

	if( false == expect(af::platformParseOsReleaseLine("ID=ubuntu", key, value), "ID parse should succeed"))
		return false;
	if( false == expect(key == "ID", "ID key mismatch"))
		return false;
	if( false == expect(value == "ubuntu", "ID value mismatch"))
		return false;

	if( false == expect(af::platformParseOsReleaseLine("NAME=\"Rocky Linux\"", key, value), "NAME parse should succeed"))
		return false;
	if( false == expect(value == "Rocky Linux", "NAME value should be unquoted"))
		return false;

	if( false == expect(false == af::platformParseOsReleaseLine("# comment", key, value), "comment line should be ignored"))
		return false;
	if( false == expect(false == af::platformParseOsReleaseLine("NO_EQUALS", key, value), "invalid line should be ignored"))
		return false;

	return true;
}

bool testParseOsRelease()
{
	std::stringstream input;
	input << "NAME=\"AlmaLinux\"\n";
	input << "VERSION_ID=\"9.6\"\n";
	input << "ID=almalinux\n";

	std::string id;
	std::string name;
	std::string version;
	if( false == expect(af::platformParseOsRelease(input, id, name, version), "os-release parse should succeed"))
		return false;
	if( false == expect(id == "almalinux", "os-release ID mismatch"))
		return false;
	if( false == expect(name == "AlmaLinux", "os-release NAME mismatch"))
		return false;
	if( false == expect(version == "9.6", "os-release VERSION_ID mismatch"))
		return false;

	return true;
}

bool testNormalize()
{
	if( false == expect(af::platformNormalizeToken("Alma Linux") == "alma_linux", "normalize spaces"))
		return false;
	if( false == expect(af::platformNormalizeToken("  10.0.20348  ") == "10.0.20348", "normalize trim"))
		return false;
	if( false == expect(af::platformNormalizeToken("!@#") == "", "normalize symbols-only"))
		return false;
	if( false == expect(af::platformNormalizeToken("A__B") == "a_b", "normalize repeated underscore"))
		return false;

	return true;
}
}

int main()
{
	if( false == testParseLine())
		return 1;
	if( false == testParseOsRelease())
		return 1;
	if( false == testNormalize())
		return 1;

	std::cout << "test_platform_info_unit: ok" << std::endl;
	return 0;
}
