#include "cmd_confirm.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdConfirm::CmdConfirm()
{
   setCmd("cfm");
   setInfo("Send confirm message.");
   setHelp("Watching server output you can see threads response speed. For debug purposes.");
   setMsgType( af::Msg::TConfirm);
   setRecieving();
}

CmdConfirm::~CmdConfirm(){}

bool CmdConfirm::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( af::Msg::TConfirm);
   return true;
}
