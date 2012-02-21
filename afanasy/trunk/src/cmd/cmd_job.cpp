#include "cmd_job.h"

#include <iostream>
#include <stdio.h>

#include "../libafanasy/job.h"
#include "../libafanasy/jobprogress.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/msgclasses/mcjobsweight.h"

#include "../libafapi/apijob.h"
#include "../libafapi/apiblock.h"
#include "../libafapi/apitask.h"

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
   std::string name = argv[0];
   int number = atoi(argv[1]);
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
   std::string name = argv[0];
   int number = atoi(argv[1]);
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
   std::string name = argv[0];
   std::string mask = argv[1];
   if( af::RegExp::Validate( mask) == false ) return false;
   af::MCGeneral mcgeneral( name, mask);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdJobsSetUser::CmdJobsSetUser()
{
   setCmd("juser");
   setArgsCount(2);
   setInfo("Change job owner.");
   setHelp("juser [name] [user] Change job owner.");
   setMsgType( af::Msg::TJobSetUser);
}
CmdJobsSetUser::~CmdJobsSetUser(){}
bool CmdJobsSetUser::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name = argv[0];
   std::string user = argv[1];
   af::MCGeneral mcgeneral( name, user);
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
   int number = atoi(argv[0]);
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
//   setMsgOutType( af::Msg::TData);
   setRecieving();
}
CmdJobLog::~CmdJobLog(){}
bool CmdJobLog::processArguments( int argc, char** argv, af::Msg &msg)
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
bool CmdJobProgress::processArguments( int argc, char** argv, af::Msg &msg)
{
   int number = atoi(argv[0]);
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
   std::string mask = argv[0];
   if( af::RegExp::Validate( mask) == false ) return false;
   af::MCGeneral mcgeneral( mask, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdJobsPause::CmdJobsPause()
{
   setCmd("jpause");
   setArgsCount(1);
   setInfo("Pause jobs.");
   setHelp("jpause [mask] Pause jobs which names match the mask.");
   setMsgType( af::Msg::TJobPause);
}
CmdJobsPause::~CmdJobsPause(){}
bool CmdJobsPause::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string mask = argv[0];
   if( af::RegExp::Validate( mask) == false ) return false;
   af::MCGeneral mcgeneral( mask, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdJobsStart::CmdJobsStart()
{
   setCmd("jstart");
   setArgsCount(1);
   setInfo("Start jobs.");
   setHelp("jstart [mask] Start jobs which names match the mask.");
   setMsgType( af::Msg::TJobStart);
}
CmdJobsStart::~CmdJobsStart(){}
bool CmdJobsStart::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string mask = argv[0];
   if( af::RegExp::Validate( mask) == false ) return false;
   af::MCGeneral mcgeneral( mask, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdJobsStop::CmdJobsStop()
{
   setCmd("jstop");
   setArgsCount(1);
   setInfo("Stop jobs.");
   setHelp("jstop [mask] Stop jobs which names match the mask.");
   setMsgType( af::Msg::TJobStop);
}
CmdJobsStop::~CmdJobsStop(){}
bool CmdJobsStop::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string mask = argv[0];
   if( af::RegExp::Validate( mask) == false ) return false;
   af::MCGeneral mcgeneral( mask, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdJobsRestart::CmdJobsRestart()
{
   setCmd("jrestart");
   setArgsCount(1);
   setInfo("Restart jobs.");
   setHelp("jrestart [mask] Restart jobs which names match the mask.");
   setMsgType( af::Msg::TJobRestart);
}
CmdJobsRestart::~CmdJobsRestart(){}
bool CmdJobsRestart::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string mask = argv[0];
   if( af::RegExp::Validate( mask) == false ) return false;
   af::MCGeneral mcgeneral( mask, 0);
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
   int count = 30;
   if( argc > 0 ) count = atoi( argv[0]);
   int numblocks = 3;
   int numtasks = 300;

   afapi::Job ** jobs = new afapi::Job * [count];
   af::JobProgress ** progresses = new af::JobProgress * [count];

   std::cout << "Creating " << count << " job(s)..." << std::endl;
   for( int i = 0; i < count; i++)
   {
      jobs[i] = new afapi::Job();
      for( int b = 0; b < numblocks; b++)
      {
         afapi::Block * block = new afapi::Block();
         block->setName("block name");
         block->setCommand("block command %1");
         block->setTasksName("block tasks %1");
         for( int t = 0; t < numtasks; t++)
         {
            afapi::Task * task = new afapi::Task();
            task->setName("task name");
            task->setCommand("task command");
            block->appendTask( task);
         }
         block->fillTasksArrayFromList();
         jobs[i]->appendBlock( block);
      }
      jobs[i]->fillBlocksDataPointersFromList();
      progresses[i] = new af::JobProgress( jobs[i]);
   }

   for( int i = 0; i < count; i++)
   {
      jobs[i]->stdOut();
   }

   std::cout << "Deleteing " << count << " job(s)..." << std::endl;
   for( int i = 0; i < count; i++)
   {
      delete jobs[i];
      delete progresses[i];
   }

   delete [] jobs;
   delete [] progresses;

   return true;
}
