#include "cmd_job.h"

#include "../libafanasy/job.h"
#include "../libafanasy/jobprogress.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/msgclasses/mcjobsweight.h"

#include "../libafapi/apijob.h"

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
bool CmdJobsList::processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( getMsgType());
   return true;
}
void CmdJobsList::msgOut( af::Msg& msg)
{
   af::MCAfNodes list( &msg);
   list.stdOut();
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
bool CmdJobsWeight::processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( getMsgType());
   return true;
}
void CmdJobsWeight::msgOut( af::Msg& msg)
{
   af::MCJobsWeight jobsWeight( &msg);
   jobsWeight.stdOut( true);
}

CmdJobPriority::CmdJobPriority()
{
   setCmd("jpri");
   setArgsCount(2);
   setInfo("Change job priority.");
   setHelp("jpri [name] [priority] Set job priority.");
   setMsgType( af::Msg::TJobPriority);
}
CmdJobPriority::~CmdJobPriority(){}
bool CmdJobPriority::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   bool ok; int number = (QString::fromUtf8(argv[1])).toInt( &ok);
   if( ok == false ) return false;
   af::MCGeneral mcgeneral( name, number);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdJobRunningTasksMaximum::CmdJobRunningTasksMaximum()
{
   setCmd("jtmax");
   setArgsCount(2);
   setInfo("Set job running tasks maximum.");
   setHelp("jtmax [name] [number] Change job running tasks maximum.");
   setMsgType( af::Msg::TJobMaxRunningTasks);
}
CmdJobRunningTasksMaximum::~CmdJobRunningTasksMaximum(){}
bool CmdJobRunningTasksMaximum::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   bool ok; int number = (QString::fromUtf8(argv[1])).toInt( &ok);
   if( ok == false ) return false;
   af::MCGeneral mcgeneral( name, number);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdJobHostsMask::CmdJobHostsMask()
{
   setCmd("jhmask");
   setArgsCount(2);
   setInfo("Set job hosts max.");
   setHelp("jhmask [name] [string] Change job hosts mask.");
   setMsgType( af::Msg::TJobHostsMask);
}
CmdJobHostsMask::~CmdJobHostsMask(){}
bool CmdJobHostsMask::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   QString mask = QString::fromUtf8(argv[1]);
   QRegExp rx( mask, Qt::CaseInsensitive);
   if( rx.isValid() == false )
   {
      printf("QRegExp: %s\n", rx.errorString().toUtf8().data());
      return false;
   }
   af::MCGeneral mcgeneral( name, mask);
   msg.set( getMsgType(), &mcgeneral);
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
bool CmdJobId::processArguments( int argc, char** argv, af::Msg &msg)
{
   bool ok; int number = (QString::fromUtf8(argv[0])).toInt( &ok);
   if( ok == false ) return false;
   msg.set( getMsgType(), number);
   return true;
}
void CmdJobId::msgOut( af::Msg& msg)
{
   af::Job job( &msg);
   job.stdOut( true);
}

CmdJobLog::CmdJobLog()
{
   setCmd("jlog");
   setArgsCount(1);
   setInfo("Get job log.");
   setHelp("jlog [id] Get job log with given id.");
   setMsgType( af::Msg::TJobLogRequestId);
   setRecieving();
}
CmdJobLog::~CmdJobLog(){}
bool CmdJobLog::processArguments( int argc, char** argv, af::Msg &msg)
{
   bool ok; int number = (QString::fromUtf8(argv[0])).toInt( &ok);
   if( ok == false ) return false;
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
bool CmdJobProgress::processArguments( int argc, char** argv, af::Msg &msg)
{
   bool ok; int number = (QString::fromUtf8(argv[0])).toInt( &ok);
   if( ok == false ) return false;
   msg.set( getMsgType(), number);
   return true;
}
void CmdJobProgress::msgOut( af::Msg& msg)
{
   af::JobProgress jobprogress( &msg);
   jobprogress.stdOut();
}

CmdJobsDelete::CmdJobsDelete()
{
   setCmd("jdel");
   setArgsCount(1);
   setInfo("Delete jobs.");
   setHelp("jdel [mask] Delete jobs which names match the mask.");
   setMsgType( af::Msg::TJobDelete);
}
CmdJobsDelete::~CmdJobsDelete(){}
bool CmdJobsDelete::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name( QString::fromUtf8(argv[0]));
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdJob::CmdJob()
{
   setCmd("job");
   setInfo("Create a job.");
   setHelp("Job creation test. No arguments. For debug purposes only. This job will not be send.");
}
CmdJob::~CmdJob(){}
bool CmdJob::processArguments( int argc, char** argv, af::Msg &msg)
{
   afapi::Job job;
   job.output(true);
   return true;
}
