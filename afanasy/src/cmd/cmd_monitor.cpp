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
	setMsgType( af::Msg::TJSON);
}
CmdMonitorList::~CmdMonitorList(){}
bool CmdMonitorList::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	m_str << "{\"get\":{\"type\":\"monitors\"}}";
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
	setMsgType( af::Msg::TJSON);
}
CmdMonitorLog::~CmdMonitorLog(){}
bool CmdMonitorLog::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	int id = atoi(argv[0]);
	m_str << "{\"get\":{\"type\":\"monitors\",\"mode\":\"log\",\"ids\":[" << id << "]}}";
	return true;
}

CmdMonitorMsg::CmdMonitorMsg()
{
   setCmd("mmsg");
   setArgsCount(1);
   setInfo("Send message to all GUI monitors.");
   setHelp("mmsg [message]");
	setMsgType( af::Msg::TJSON);
}

CmdMonitorMsg::~CmdMonitorMsg(){}

bool CmdMonitorMsg::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	af::jsonActionOperationStart( m_str,"monitors","message",".*");
	m_str << ",\n\"text\":\"" << af::strEscape( argv[0]) << "\"";
	af::jsonActionOperationFinish( m_str);

	return true;
}
