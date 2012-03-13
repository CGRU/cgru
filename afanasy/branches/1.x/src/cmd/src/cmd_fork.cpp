#include "cmd_fork.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

CmdFork::CmdFork()
{
   setCmd("fork");
   setArgsCount(1);
   setInfo("Fork process.");
   setHelp("fork [count] Fork command process. For debug purposes only !");
}

CmdFork::~CmdFork(){}

bool CmdFork::processArguments( int argc, char** argv, af::Msg &msg)
{
   bool ok;
   int count = (QString::fromUtf8(argv[0])).toInt( &ok);
   if( ok == false ) return false;
   for( int i = 0; i < count; i++) fork();
   return true;
}
