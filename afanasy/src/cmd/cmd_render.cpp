#include "cmd_render.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/regexp.h"
#include "../libafanasy/render.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool Verbose;

CmdRenderList::CmdRenderList()
{
	setCmd("rlist");
	setInfo("List of online Renders.");
	setMsgType( af::Msg::TRendersListRequest);
	setMsgOutType( af::Msg::TRendersList);
	setRecieving();
}
CmdRenderList::~CmdRenderList(){}
bool CmdRenderList::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	msg.set( getMsgType());
	return true;
}
void CmdRenderList::v_msgOut( af::Msg& msg)
{
	af::MCAfNodes list( &msg);
	list.v_stdOut( Verbose);
}

CmdRenderResoucesList::CmdRenderResoucesList()
{
	setCmd("rrlist");
	setArgsCount(1);
	setInfo("List of resources of Renders specified by mask.");
	setMsgType( af::Msg::TRendersResourcesRequestIds);
	setMsgOutType( af::Msg::TRendersResources);
	setRecieving();
}
CmdRenderResoucesList::~CmdRenderResoucesList(){}
bool CmdRenderResoucesList::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	af::MCGeneral mcgeneral( name, 0);
	msg.set( getMsgType(), &mcgeneral);
	return true;
}
void CmdRenderResoucesList::v_msgOut( af::Msg& msg)
{
	af::MCAfNodes list( &msg);
	printf("size=%zu\n", list.getCount());
	for( int i = 0; i < list.getCount(); i++)
	{
	   af::Render * render = (af::Render*)(list.getNode(i));
	   render->getHostRes().v_stdOut( false);
	}
}

CmdRenderPriority::CmdRenderPriority()
{
	setCmd("rpri");
	setArgsCount(2);
	setInfo("Change render priority.");
	setHelp("rpri [name] [priority] Set render priority.");
	setMsgType( af::Msg::TJSON);
}
CmdRenderPriority::~CmdRenderPriority(){}
bool CmdRenderPriority::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	int priority = atoi(argv[1]);

	af::jsonActionParamsStart( m_str, "renders", name);
	m_str << "\n\"priority\":" << priority;
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdRenderNimby::CmdRenderNimby()
{
	setCmd("rnimby");
	setArgsCount(1);
	setInfo("Set render nimby.");
	setHelp("rnimby [name] Set render to nimby state.");
	setMsgType( af::Msg::TJSON);
}
CmdRenderNimby::~CmdRenderNimby(){}
bool CmdRenderNimby::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];

	af::jsonActionParamsStart( m_str, "renders", name);
	m_str << "\n\"nimby\":true";
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdRenderNIMBY::CmdRenderNIMBY()
{
	setCmd("rNIMBY");
	setArgsCount(1);
	setInfo("Set render NIMBY.");
	setHelp("rNIMBY [name] Set render to NIMBY state.");
	setMsgType( af::Msg::TJSON);
}
CmdRenderNIMBY::~CmdRenderNIMBY(){}
bool CmdRenderNIMBY::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];

	af::jsonActionParamsStart( m_str, "renders", name);
	m_str << "\n\"NIMBY\":true";
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdRenderUser::CmdRenderUser()
{
	setCmd("ruser");
	setArgsCount(2);
	setInfo("Set render user.");
	setHelp("ruser [name] [user] Set render for specified user.");
	setMsgType( af::Msg::TJSON);
}
CmdRenderUser::~CmdRenderUser(){}
bool CmdRenderUser::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	std::string user = argv[1];

	af::jsonActionParamsStart( m_str, "renders", name);
	m_str << "\n\"user_name\":" << "\"" << user << "\"";
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdRenderFree::CmdRenderFree()
{
	setCmd("rfree");
	setArgsCount(1);
	setInfo("Set render free.");
	setHelp("rfree [name] Set render free - unset nimby and NIMBY state.");
	setMsgType( af::Msg::TJSON);
}
CmdRenderFree::~CmdRenderFree(){}
bool CmdRenderFree::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];

	af::jsonActionParamsStart( m_str, "renders", name);
	m_str << "\n\"nimby\":false";
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdRenderEjectTasks::CmdRenderEjectTasks()
{
	setCmd("reject");
	setArgsCount(1);
	setInfo("Eject tasks from render.");
	setHelp("reject [name] Eject tasks from specified render.");
	setMsgType( af::Msg::TJSON);
}
CmdRenderEjectTasks::~CmdRenderEjectTasks(){}
bool CmdRenderEjectTasks::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	af::jsonActionOperation( m_str, "renders", "eject_tasks", name);
	return true;
}

CmdRenderEjectNotMyTasks::CmdRenderEjectNotMyTasks()
{
	setCmd("rejnotmy");
	setArgsCount(1);
	setInfo("Eject not my tasks from render.");
	setHelp("rejnotmy [name] Eject not my tasks from specified render.");
	setMsgType( af::Msg::TJSON);
}
CmdRenderEjectNotMyTasks::~CmdRenderEjectNotMyTasks(){}
bool CmdRenderEjectNotMyTasks::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	af::jsonActionOperation( m_str, "renders", "eject_tasks_keep_my", name);
	return true;
}

CmdRenderExit::CmdRenderExit()
{
	setCmd("rexit");
	setInfo("Exit render.");
	setHelp("rexit [name] Exit render with specified name, in no name porivieded, local host name used.");
	setMsgType( af::Msg::TJSON);
}
CmdRenderExit::~CmdRenderExit(){}
bool CmdRenderExit::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name( af::Environment::getHostName());
	if( argc > 0) name = argv[0];
	af::jsonActionOperation( m_str, "renders", "exit", name);
	return true;
}

CmdRenderDelete::CmdRenderDelete()
{
	setCmd("rdel");
	setInfo("Delete render.");
	setHelp("rdel [name] Delete render with specified name, in no name porivieded, local host name used.");
	setMsgType( af::Msg::TJSON);
}
CmdRenderDelete::~CmdRenderDelete(){}
bool CmdRenderDelete::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name( af::Environment::getHostName());
	if( argc > 0) name = argv[0];
	af::jsonActionOperation( m_str, "renders", "delete", name);
	return true;
}

CmdRenderWOLSleep::CmdRenderWOLSleep()
{
	setCmd("rsleep");
	setInfo("Ask render(s) to sleep.");
	setHelp("rsleep [name] Ask render(s) to sleep.");
	setArgsCount(1);
	setMsgType( af::Msg::TJSON);
}
CmdRenderWOLSleep::~CmdRenderWOLSleep(){}
bool CmdRenderWOLSleep::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if( af::RegExp::Validate( mask) == false ) return false;
	af::jsonActionOperation( m_str, "renders", "wol_sleep", mask);
	return true;
}

CmdRenderWOLWake::CmdRenderWOLWake()
{
	setCmd("rwake");
	setInfo("Ask sleeping render(s) to wake up.");
	setHelp("rwake [name] Ask render(s) to wake up.");
	setArgsCount(1);
	setMsgType( af::Msg::TJSON);
}
CmdRenderWOLWake::~CmdRenderWOLWake(){}
bool CmdRenderWOLWake::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if( af::RegExp::Validate( mask) == false ) return false;
	af::jsonActionOperation( m_str, "renders", "wol_wake", mask);
	return true;
}

CmdRenderServiceOn::CmdRenderServiceOn()
{
	setCmd("rsrvon");
	setInfo("Enable disabled render service.");
	setHelp("rsrvon [name|mask] [service] Enable disabled render service.");
	setArgsCount(2);
	setMsgType( af::Msg::TJSON);
}
CmdRenderServiceOn::~CmdRenderServiceOn(){}
bool CmdRenderServiceOn::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if( af::RegExp::Validate( mask) == false ) return false;

	af::jsonActionOperationStart( m_str, "renders", "service", mask);
	m_str << ",\n\"name\":\"" << argv[1] << "\"";
	m_str << ",\n\"enable\":true";
	af::jsonActionOperationFinish( m_str);

	return true;
}

CmdRenderServiceOff::CmdRenderServiceOff()
{
	setCmd("rsrvoff");
	setInfo("Disable render service.");
	setHelp("rsrvoff [name|mask] [service] Disable render service.");
	setArgsCount(2);
	setMsgType( af::Msg::TJSON);
}
CmdRenderServiceOff::~CmdRenderServiceOff(){}
bool CmdRenderServiceOff::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if( af::RegExp::Validate( mask) == false ) return false;

	af::jsonActionOperationStart( m_str, "renders", "service", mask);
	m_str << ",\n\"name\":\"" << argv[1] << "\"";
	m_str << ",\n\"enable\":false";
	af::jsonActionOperationFinish( m_str);

	return true;
}
