#include "cmd_help.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

CmdHelp::CmdHelp()
{
   setCmd("h");
   setInfo("Set help mode.");
}

CmdHelp::~CmdHelp(){}

bool CmdHelp::processArguments( int argc, char** argv, af::Msg &msg)
{
   Help = true;
   return true;
}
