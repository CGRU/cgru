#include "cmd_task.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdTaskLog::CmdTaskLog()
{
	setCmd("tlog");
	setArgsCount(4);
	setInfo("Get task log.");
	setHelp("tout [jobid] [block] [task] [start] Get task log.");
	setMsgType( af::Msg::TJSON);
}
CmdTaskLog::~CmdTaskLog(){}
bool CmdTaskLog::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	bool ok; int job; int block; int task; int number;
	job    = af::stoi(argv[0], &ok);
	if( ok == false ) return false;
	block  = af::stoi(argv[1], &ok);
	if( ok == false ) return false;
	task   = af::stoi(argv[2], &ok);
	if( ok == false ) return false;
	number = af::stoi(argv[3], &ok);
	if( ok == false ) return false;

//{"get":{"type":"jobs","mode":"files","ids":[2],"block_ids":[0],"task_ids":[3],"binary":true}}
	m_str << "{\"get\":{\"type\":\"jobs\"";
	m_str << ",\"mode\":\"log\"";
	m_str << ",\"ids\":[" << job << "]";
	m_str << ",\"block_ids\":[" << block << "]";
	m_str << ",\"task_ids\":[" << task << "]";
	m_str << ",\"number\":" << number;
	m_str << ",\"binary\":true";
	m_str << "}}";

	return true;
}

CmdTaskOutput::CmdTaskOutput()
{
	setCmd("tout");
	setArgsCount(4);
	setInfo("Get task output.");
	setHelp("tout [jobid] [block] [task] [start] Get task output.");
	setMsgType( af::Msg::TJSON);
}
CmdTaskOutput::~CmdTaskOutput(){}
bool CmdTaskOutput::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	bool ok; int job; int block; int task; int number;
	job    = af::stoi(argv[0], &ok);
	if( ok == false ) return false;
	block  = af::stoi(argv[1], &ok);
	if( ok == false ) return false;
	task   = af::stoi(argv[2], &ok);
	if( ok == false ) return false;
	number = af::stoi(argv[3], &ok);
	if( ok == false ) return false;

	m_str << "{\"get\":{\"type\":\"jobs\"";
	m_str << ",\"mode\":\"output\"";
	m_str << ",\"ids\":[" << job << "]";
	m_str << ",\"block_ids\":[" << block << "]";
	m_str << ",\"task_ids\":[" << task << "]";
	m_str << ",\"number\":" << number;
	m_str << ",\"binary\":true";
	m_str << "}}";

	return true;
}

CmdTaskRestart::CmdTaskRestart()
{
	setCmd("trestart");
	setArgsCount(3);
	setInfo("Restart task.");
	setHelp("trestart [jobid] [block] [task].");
	setMsgType( af::Msg::TJSON);
}
CmdTaskRestart::~CmdTaskRestart(){}
bool CmdTaskRestart::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	bool ok; int job; int block; int task;
	job    = af::stoi(argv[0], &ok);
	if( ok == false ) return false;
	block  = af::stoi(argv[1], &ok);
	if( ok == false ) return false;
	task   = af::stoi(argv[2], &ok);
	if( ok == false ) return false;

	af::jsonActionStart( m_str, "jobs", "", std::vector<int>( 1, job));
	m_str << ",\n\"block_ids\":[" << block << ']';
	m_str << ",\n\"operation\":{";
	m_str << "\n\"type\":\"restart\"";
	m_str << ",\n\"task_ids\":[" << task << "]}";
	af::jsonActionFinish( m_str);
	return true;
}
