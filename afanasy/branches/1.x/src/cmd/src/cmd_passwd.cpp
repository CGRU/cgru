#include "cmd_passwd.h"

#include <passwd/passwd.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

CmdPasswd::CmdPasswd()
{
   setCmd("passwd");
   setArgsCount(1);
   setInfo("Calculate a password.");
   setHelp("passwd [string] Calculate and print a password from string.");
}

CmdPasswd::~CmdPasswd(){}

bool CmdPasswd::processArguments( int argc, char** argv, af::Msg &msg)
{
   int times = 1;
   if( argc > 1)
   {
      bool ok;
      int num = QString( argv[1]).toInt( &ok);
      if( ok ) times = num;
   }
   af::Passwd passwd;
   unsigned char digest[af::Passwd::digest_len];

   for( int t = 0; t < times; t++) passwd.calculate( digest, argv[0]);

   af::Passwd::PrintKey( digest);

   return true;
}
