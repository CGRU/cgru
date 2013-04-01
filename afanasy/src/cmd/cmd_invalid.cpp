#include "cmd_invalid.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdInvalid::CmdInvalid()
{
   setCmd("inv");
   setInfo("Try invalid message.");
   setHelp("Send invalid type of message. For debug purposes.");
   setMsgType( af::Msg::TInvalid);
}

CmdInvalid::~CmdInvalid(){}

bool CmdInvalid::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( af::Msg::TInvalid);
   return true;
}
