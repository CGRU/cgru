#include "cmd_task.h"

#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msgclasses/mctaskspos.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdTaskLog::CmdTaskLog()
{
	setCmd("tlog");
	setArgsCount(4);
	setInfo("Get task log.");
	setHelp("tout [jobid] [block] [task] [start] Get task log.");
	setMsgType( af::Msg::TTaskLogRequest);
	setRecieving();
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
	af::MCTaskPos mctaskpos( job, block, task, number);
	msg.set( getMsgType(), &mctaskpos);
	return true;
}

CmdTaskOutput::CmdTaskOutput()
{
	setCmd("tout");
	setArgsCount(4);
	setInfo("Get task output.");
	setHelp("tout [jobid] [block] [task] [start] Get task output.");
	setMsgType( af::Msg::TTaskOutputRequest);
	setRecieving();
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
	af::MCTaskPos mctaskpos( job, block, task, number);
	msg.set( getMsgType(), &mctaskpos);
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
