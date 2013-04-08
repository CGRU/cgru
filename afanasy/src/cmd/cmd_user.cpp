#include "cmd_user.h"

#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/regexp.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool Verbose;

CmdUserList::CmdUserList()
{
	setCmd("ulist");
	setInfo("List of permanent users.");
	setMsgType( af::Msg::TUsersListRequest);
	setMsgOutType( af::Msg::TUsersList);
	setRecieving();
}
CmdUserList::~CmdUserList(){}
bool CmdUserList::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	msg.set( getMsgType());
	return true;
}
void CmdUserList::v_msgOut( af::Msg& msg)
{
	af::MCAfNodes list( &msg);
	list.v_stdOut( Verbose);
}

CmdUserJobsList::CmdUserJobsList()
{
	setCmd("ujobs");
	setArgsCount(1);
	setInfo("List of user jobs.");
	setHelp("ujobs [id] Request a list of jobs of user with given id.");
	setMsgType( af::Msg::TJobsListRequestUserId);
	setMsgOutType( af::Msg::TJobsList);
	setRecieving();
}
CmdUserJobsList::~CmdUserJobsList(){}
bool CmdUserJobsList::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   int number = atoi(argv[0]);
   msg.set( getMsgType(), number);
   return true;
}
void CmdUserJobsList::v_msgOut( af::Msg& msg)
{
	af::MCAfNodes list( &msg);
	list.v_stdOut( Verbose);
}

CmdUserAdd::CmdUserAdd()
{
	setCmd("uadd");
	setArgsCount(1);
	setInfo("Add permanent user.");
	setHelp("uadd [name] Add a permanent user with spcecified name.");
	setMsgType( af::Msg::TUserAdd);
}
CmdUserAdd::~CmdUserAdd(){}
bool CmdUserAdd::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];

	af::MCGeneral mcgeneral( name, 1);
	msg.set( getMsgType(), &mcgeneral);

	return true;
}

CmdUserDelete::CmdUserDelete()
{
	setCmd("udel");
	setArgsCount(1);
	setInfo("Delete permanent user.");
	setHelp("udel [name] Delete a permanent user with spcecified name.");
	setMsgType( af::Msg::TJSON);
}
CmdUserDelete::~CmdUserDelete(){}
bool CmdUserDelete::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];

	af::jsonActionParamsStart( m_str, "users", name);
	m_str << "\n\"permanent\":false";
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdUserPriority::CmdUserPriority()
{
	setCmd("upri");
	setArgsCount(2);
	setInfo("Change render priority.");
	setHelp("upri [name] [priority] Set user priority.");
	setMsgType( af::Msg::TJSON);
}
CmdUserPriority::~CmdUserPriority(){}
bool CmdUserPriority::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	int number = atoi(argv[1]);

	af::jsonActionParamsStart( m_str, "users", name);
	m_str << "\n\"priority\":" << number;
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdUserRunningTasksMaximum::CmdUserRunningTasksMaximum()
{
	setCmd("utmax");
	setArgsCount(2);
	setInfo("Set user maximum running tasks number.");
	setHelp("utmax [name] [number] Change user maximum running tasks number.");
	setMsgType( af::Msg::TJSON);
}
CmdUserRunningTasksMaximum::~CmdUserRunningTasksMaximum(){}
bool CmdUserRunningTasksMaximum::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	int number = atoi(argv[1]);

	af::jsonActionParamsStart( m_str, "users", name);
	m_str << "\n\"max_running_tasks\":" << number;
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdUserHostsMask::CmdUserHostsMask()
{
	setCmd("uhmask");
	setArgsCount(2);
	setInfo("Set user hosts max.");
	setHelp("uhmask [name] [string] Change user hosts mask.");
	setMsgType( af::Msg::TJSON);
}
CmdUserHostsMask::~CmdUserHostsMask(){}
bool CmdUserHostsMask::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	std::string mask = argv[1];
	if( af::RegExp::Validate( mask) == false ) return false;

	af::jsonActionParamsStart( m_str, "users", name);
	m_str << "\n\"hosts_mask\":\"" << mask << "\"";
	af::jsonActionParamsFinish( m_str);

	return true;
}
