#include "cmd_monitor.h"

#include "../libafanasy/msgclasses/mcafnodes.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

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
bool CmdMonitorList::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( af::Msg::TMonitorsListRequest);
   return true;
}
void CmdMonitorList::v_msgOut( af::Msg& msg)
{
   af::MCAfNodes list( &msg);
   list.v_stdOut( Verbose);
}

CmdMonitorLog::CmdMonitorLog()
{
	setCmd("mlog");
	setArgsCount(1);
	setInfo("Get Monitor log by id.");
	setHelp("mlog [id] Get monitor log with given id.");
	setMsgType( af::Msg::TMonitorLogRequestId);
	setRecieving();
}
CmdMonitorLog::~CmdMonitorLog(){}
bool CmdMonitorLog::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	int number = atoi(argv[0]);
	msg.set( getMsgType(), number);
	return true;
}

