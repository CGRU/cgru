#include "cmd_verbose.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

CmdVerbose::CmdVerbose()
{
   setCmd("v");
   setInfo("Set verbose mode.");
}

CmdVerbose::~CmdVerbose(){}

bool CmdVerbose::processArguments( int argc, char** argv, af::Msg &msg)
{
   Verbose = true;
   return true;
}
