#include "cmd_monitor.h"

#include <msgclasses/mcafnodes.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

extern bool Verbose;

CmdMonitorList::CmdMonitorList()
{
   setCmd("mlist");
   setInfo("List of online Monitors.");
   setMsgType( af::Msg::TMonitorsListRequest);
   setMsgOutType( af::Msg::TMonitorsList);
   setRecieving();
}

CmdMonitorList::~CmdMonitorList(){}

bool CmdMonitorList::processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( af::Msg::TMonitorsListRequest);
   return true;
}

void CmdMonitorList::msgOut( af::Msg& msg)
{
   af::MCAfNodes list( &msg);
   list.stdOut( Verbose);
}
