#include "cmd_passwd.h"

#include "../libafanasy/common/md5.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdPasswd::CmdPasswd()
{
	setCmd("md5");
	setArgsCount(1);
	setInfo("Calculate MD5.");
	setHelp("md5 [string] Calculate and print MD5.");
}

CmdPasswd::~CmdPasswd(){}

bool CmdPasswd::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string str( argv[0]);
	unsigned char key[32];

	MD5_CTX context;
	MD5Init   ( &context);
	MD5Update ( &context, (unsigned char *)str.c_str(), str.size());
	MD5Final  ( key, &context);

	std::string md5;
	char buffer[3];
	buffer[2] = '\0';
	for( int i = 0; i < 16; i++)
	{
		sprintf( buffer, "%02x", key[i]);
		md5 += buffer;
	}

	printf("%s\n", md5.c_str());

	return true;
}
