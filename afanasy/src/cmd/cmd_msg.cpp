#include "cmd_msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdMsg::CmdMsg()
{
   setCmd("msg");
   setArgsCount(1);
   setInfo("Send message to all GUI monitors.");
   setHelp("msg [message]");
   setMsgType( af::Msg::TString);
}

CmdMsg::~CmdMsg(){}

bool CmdMsg::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.setString( argv[0]);
   return true;
}
