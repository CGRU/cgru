#include "cmd_passwd.h"

#include "../libafanasy/passwd/passwd.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdPasswd::CmdPasswd()
{
   setCmd("passwd");
   setArgsCount(1);
   setInfo("Calculate a password.");
   setHelp("passwd [string] Calculate and print a password from string.");
}

CmdPasswd::~CmdPasswd(){}

bool CmdPasswd::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   int times = 1;
   if( argc > 1)
   {
      bool ok;
      int num = af::stoi( argv[1], &ok);
      if( ok ) times = num;
   }
   af::Passwd passwd;
   unsigned char digest[af::Passwd::digest_len];

   for( int t = 0; t < times; t++) passwd.calculate( digest, argv[0]);

   af::Passwd::PrintKey( digest);

   return true;
}
