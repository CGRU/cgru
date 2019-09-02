#include "cmd_pool.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/regexp.h"
#include "../libafanasy/pool.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool Verbose;

CmdPoolList::CmdPoolList()
{
	setCmd("plist");
	setInfo("Get a list of pools.");
	setMsgType(af::Msg::TJSON);
}
CmdPoolList::~CmdPoolList(){}
bool CmdPoolList::v_processArguments(int argc, char** argv, af::Msg &msg)
{
	m_str << "{\"get\":{\"type\":\"pools\"}}";
	return true;
}
void CmdPoolList::v_msgOut(af::Msg& msg)
{
	af::MCAfNodes list(&msg);
	list.v_stdOut(Verbose);
}

CmdPoolLog::CmdPoolLog()
{
	setCmd("plog");
	setArgsCount(1);
	setInfo("Get pool log.");
	setUsage("plog id");
	setHelp("Get pool log by id.");
	setMsgType( af::Msg::TJSON);
}
CmdPoolLog::~CmdPoolLog(){}
bool CmdPoolLog::v_processArguments(int argc, char** argv, af::Msg &msg)
{
	int id = atoi(argv[0]);
	m_str << "{\"get\":{\"type\":\"pools\",\"mode\":\"log\",\"ids\":[" << id << "]}}";
	return true;
}

CmdPoolPriority::CmdPoolPriority()
{
	setCmd("ppri");
	setArgsCount(2);
	setInfo("Change pool priority.");
	setHelp("ppri [name] [priority]\nSet pool priority.");
	setMsgType(af::Msg::TJSON);
}
CmdPoolPriority::~CmdPoolPriority(){}
bool CmdPoolPriority::v_processArguments(int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	int priority = atoi(argv[1]);

	af::jsonActionParamsStart(m_str, "pools", name);
	m_str << "\n\"priority\":" << priority;
	af::jsonActionParamsFinish(m_str);

	return true;
}

CmdPoolPause::CmdPoolPause()
{
	setCmd("ppause");
	setArgsCount(1);
	setInfo("Set pool paused.");
	setHelp("ppause [name] Set pool paused state.");
	setMsgType(af::Msg::TJSON);
}
CmdPoolPause::~CmdPoolPause(){}
bool CmdPoolPause::v_processArguments(int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];

	af::jsonActionParamsStart(m_str,"pools", name);
	m_str << "\n\"paused\":true";
	af::jsonActionParamsFinish(m_str);

	return true;
}
CmdPoolUnpause::CmdPoolUnpause()
{
	setCmd("punpause");
	setArgsCount(1);
	setInfo("Set pool unpaused.");
	setHelp("punpause [name] Unset pool paused state.");
	setMsgType(af::Msg::TJSON);
}
CmdPoolUnpause::~CmdPoolUnpause(){}
bool CmdPoolUnpause::v_processArguments(int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];

	af::jsonActionParamsStart(m_str,"pools", name);
	m_str << "\n\"paused\":false";
	af::jsonActionParamsFinish(m_str);

	return true;
}

CmdPoolDelete::CmdPoolDelete()
{
	setCmd("pdel");
	setInfo("Delete pool.");
	setHelp("pdel [name] Delete pool with specified name (path).");
	setMsgType(af::Msg::TJSON);
}
CmdPoolDelete::~CmdPoolDelete(){}
bool CmdPoolDelete::v_processArguments(int argc, char** argv, af::Msg &msg)
{
	std::string name( af::Environment::getHostName());
	if (argc > 0) name = argv[0];
	af::jsonActionOperation(m_str,"pools","delete", name);
	return true;
}

CmdPoolServiceAdd::CmdPoolServiceAdd()
{
	setCmd("psrvadd");
	setInfo("Add pool service.");
	setHelp("psrvadd [name|mask] [service] Add pool service.");
	setArgsCount(2);
	setMsgType( af::Msg::TJSON);
}
CmdPoolServiceAdd::~CmdPoolServiceAdd(){}
bool CmdPoolServiceAdd::v_processArguments(int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if (af::RegExp::Validate(mask) == false)
		return false;

	af::jsonActionOperationStart(m_str,"pools","service", mask);
	m_str << ",\n\"name\":\"" << argv[1] << "\"";
	m_str << ",\n\"add\":true";
	af::jsonActionOperationFinish(m_str);

	return true;
}

CmdPoolServiceDel::CmdPoolServiceDel()
{
	setCmd("psrvdel");
	setInfo("Delete pool service.");
	setHelp("psrvdel [name|mask] [service] Delete pool service.");
	setArgsCount(2);
	setMsgType(af::Msg::TJSON);
}
CmdPoolServiceDel::~CmdPoolServiceDel(){}
bool CmdPoolServiceDel::v_processArguments(int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if (af::RegExp::Validate(mask) == false)
		return false;

	af::jsonActionOperationStart(m_str,"pools","service", mask);
	m_str << ",\n\"name\":\"" << argv[1] << "\"";
	m_str << ",\n\"delete\":true";
	af::jsonActionOperationFinish(m_str);

	return true;
}
