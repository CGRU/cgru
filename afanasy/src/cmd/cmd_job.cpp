#include "cmd_job.h"

#include <iostream>
#include <stdio.h>

#include "../libafanasy/job.h"
#include "../libafanasy/jobprogress.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/msgclasses/mcjobsweight.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool Verbose;

CmdJobsList::CmdJobsList()
{
	setCmd("jlist");
	setInfo("List of online jobs.");
	setMsgType( af::Msg::TJobsListRequest);
	setMsgOutType( af::Msg::TJobsList);
	setRecieving();
}
CmdJobsList::~CmdJobsList(){}
bool CmdJobsList::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	msg.set( getMsgType());
	return true;
}
void CmdJobsList::v_msgOut( af::Msg& msg)
{
	af::MCAfNodes list( &msg);
	list.v_stdOut();
}

CmdJobsWeight::CmdJobsWeight()
{
	setCmd("jweight");
	setInfo("Jobs memory weight.");
	setMsgType( af::Msg::TJobsWeightRequest);
	setMsgOutType( af::Msg::TJobsWeight);
	setRecieving();
}
CmdJobsWeight::~CmdJobsWeight(){}
bool CmdJobsWeight::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	msg.set( getMsgType());
	return true;
}
void CmdJobsWeight::v_msgOut( af::Msg& msg)
{
	af::MCJobsWeight jobsWeight( &msg);
	jobsWeight.v_stdOut( true);
}

CmdJobPriority::CmdJobPriority()
{
	setCmd("jpri");
	setArgsCount(2);
	setInfo("Change job priority.");
	setHelp("jpri [name] [priority] Set job priority.");
	setMsgType( af::Msg::TJSON);
}
CmdJobPriority::~CmdJobPriority(){}
bool CmdJobPriority::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	int number = atoi(argv[1]);

	af::jsonActionParamsStart( m_str, "jobs", name);
	m_str << "\n\"priority\":" << number;
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdJobRunningTasksMaximum::CmdJobRunningTasksMaximum()
{
	setCmd("jtmax");
	setArgsCount(2);
	setInfo("Set job running tasks maximum.");
	setHelp("jtmax [name] [number] Change job running tasks maximum.");
	setMsgType( af::Msg::TJSON);
}
CmdJobRunningTasksMaximum::~CmdJobRunningTasksMaximum(){}
bool CmdJobRunningTasksMaximum::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	int number = atoi(argv[1]);

	af::jsonActionParamsStart( m_str, "jobs", name);
	m_str << "\n\"max_running_tasks\":" << number;
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdJobHostsMask::CmdJobHostsMask()
{
	setCmd("jhmask");
	setArgsCount(2);
	setInfo("Set job hosts max.");
	setHelp("jhmask [name] [string] Change job hosts mask.");
	setMsgType( af::Msg::TJSON);
}
CmdJobHostsMask::~CmdJobHostsMask(){}
bool CmdJobHostsMask::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	std::string mask = argv[1];
	if( af::RegExp::Validate( mask) == false ) return false;

	af::jsonActionParamsStart( m_str, "jobs", name);
	m_str << "\n\"hosts_mask\":\"" << mask << '"';
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdJobsSetUser::CmdJobsSetUser()
{
	setCmd("juser");
	setArgsCount(2);
	setInfo("Change job owner.");
	setHelp("juser [name] [user] Change job owner.");
	setMsgType( af::Msg::TJSON);
}
CmdJobsSetUser::~CmdJobsSetUser(){}
bool CmdJobsSetUser::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string name = argv[0];
	std::string user = argv[1];

	af::jsonActionParamsStart( m_str, "jobs", name);
	m_str << "\n\"user_name\":\"" << user << '"';
	af::jsonActionParamsFinish( m_str);

	return true;
}

CmdJobId::CmdJobId()
{
	setCmd("jid");
	setArgsCount(1);
	setInfo("Get job.");
	setHelp("jid [id] Get job with given id.");
	setMsgType( af::Msg::TJobRequestId);
	setMsgOutType( af::Msg::TJob);
	setRecieving();
}
CmdJobId::~CmdJobId(){}
bool CmdJobId::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	int number = atoi(argv[0]);
	msg.set( getMsgType(), number);
	return true;
}
void CmdJobId::v_msgOut( af::Msg& msg)
{
	af::Job job( &msg);
	job.v_stdOut( true);
}

CmdJobLog::CmdJobLog()
{
	setCmd("jlog");
	setArgsCount(1);
	setInfo("Get job log.");
	setHelp("jlog [id] Get job log with given id.");
	setMsgType( af::Msg::TJobLogRequestId);
//   setMsgOutType( af::Msg::TData);
	setRecieving();
}
CmdJobLog::~CmdJobLog(){}
bool CmdJobLog::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	int number = atoi(argv[0]);
	msg.set( getMsgType(), number);
	return true;
}

CmdJobProgress::CmdJobProgress()
{
	setCmd("jprog");
	setArgsCount(1);
	setInfo("Get job progress.");
	setHelp("jprog [id] Get progress job with given id.");
	setMsgType( af::Msg::TJobProgressRequestId);
	setMsgOutType( af::Msg::TJobProgress);
	setRecieving();
}
CmdJobProgress::~CmdJobProgress(){}
bool CmdJobProgress::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	int number = atoi(argv[0]);
	msg.set( getMsgType(), number);
	return true;
}
void CmdJobProgress::v_msgOut( af::Msg& msg)
{
	af::JobProgress jobprogress( &msg);
	jobprogress.v_stdOut();
}

CmdJobsDelete::CmdJobsDelete()
{
	setCmd("jdel");
	setArgsCount(1);
	setInfo("Delete jobs.");
	setHelp("jdel [mask] Delete jobs which names match the mask.");
	setMsgType( af::Msg::TJSON);
}
CmdJobsDelete::~CmdJobsDelete(){}
bool CmdJobsDelete::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if( af::RegExp::Validate( mask) == false ) return false;

	af::jsonActionOperation( m_str, "jobs", "delete", mask);

	return true;
}

CmdJobsPause::CmdJobsPause()
{
	setCmd("jpause");
	setArgsCount(1);
	setInfo("Pause jobs.");
	setHelp("jpause [mask] Pause jobs which names match the mask.");
	setMsgType( af::Msg::TJSON);
}
CmdJobsPause::~CmdJobsPause(){}
bool CmdJobsPause::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if( af::RegExp::Validate( mask) == false ) return false;

	af::jsonActionOperation( m_str, "jobs", "pause", mask);

	return true;
}

CmdJobsStart::CmdJobsStart()
{
	setCmd("jstart");
	setArgsCount(1);
	setInfo("Start jobs.");
	setHelp("jstart [mask] Start jobs which names match the mask.");
	setMsgType( af::Msg::TJSON);
}
CmdJobsStart::~CmdJobsStart(){}
bool CmdJobsStart::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if( af::RegExp::Validate( mask) == false ) return false;

	af::jsonActionOperation( m_str, "jobs", "start", mask);

	return true;
}

CmdJobsStop::CmdJobsStop()
{
	setCmd("jstop");
	setArgsCount(1);
	setInfo("Stop jobs.");
	setHelp("jstop [mask] Stop jobs which names match the mask.");
	setMsgType( af::Msg::TJSON);
}
CmdJobsStop::~CmdJobsStop(){}
bool CmdJobsStop::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if( af::RegExp::Validate( mask) == false ) return false;

	af::jsonActionOperation( m_str, "jobs", "stop", mask);

	return true;
}

CmdJobsRestart::CmdJobsRestart()
{
	setCmd("jrestart");
	setArgsCount(1);
	setInfo("Restart jobs.");
	setHelp("jrestart [mask] Restart jobs which names match the mask.");
	setMsgType( af::Msg::TJSON);
}
CmdJobsRestart::~CmdJobsRestart(){}
bool CmdJobsRestart::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string mask = argv[0];
	if( af::RegExp::Validate( mask) == false ) return false;

	af::jsonActionOperation( m_str, "jobs", "restart", mask);

	return true;
}
