#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/farm.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgqueue.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msgclasses/mcjobsweight.h"

#include "../libafnetwork/communications.h"

#include "afcommon.h"
#include "jobcontainer.h"
#include "monitoraf.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "talkcontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::Msg* threadProcessMsgCase( ThreadArgs * i_args, af::Msg * i_msg)
{
//i_msg->stdOut();
   af::Msg * o_msg_response = NULL;

   switch( i_msg->type())
   {
   case af::Msg::TNULL:
   case af::Msg::TDATA:
   case af::Msg::TTESTDATA:
   case af::Msg::TStringList:
   {
      i_msg->stdOutData();
      break;
   }
   case af::Msg::TString:
   {
      std::string str = i_msg->getString();
      if( str.empty()) break;

      AFCommon::QueueLog( str);
      AfContainerLock mLock( i_args->monitors, AfContainerLock::WRITELOCK);
      i_args->monitors->sendMessage( str);
      break;
   }
   case af::Msg::TStatRequest:
   {
      o_msg_response = new af::Msg;
      com::statwrite( o_msg_response);
      break;
   }
   case af::Msg::TConfirm:
   {
      printf("Thread process message: Msg::TConfirm: %d\n", i_msg->int32());
      i_args->msgQueue->pushMsg( new af::Msg( af::Msg::TConfirm, 1));
      o_msg_response = new af::Msg( af::Msg::TConfirm, 1 - i_msg->int32());
      break;
   }
   case af::Msg::TConfigLoad:
   {
      AfContainerLock tlock( i_args->talks,   AfContainerLock::WRITELOCK);
      AfContainerLock rlock( i_args->renders, AfContainerLock::WRITELOCK);
      AfContainerLock jlock( i_args->jobs,    AfContainerLock::WRITELOCK);
      AfContainerLock ulock( i_args->users,   AfContainerLock::WRITELOCK);
      std::string message;
      if( af::Environment::reload())
      {
         message = "Reloaded successfully.";
      }
      else
      {
         message = "Failed, see server logs fo details.";
      }
      o_msg_response = new af::Msg();
      o_msg_response->setString( message);
      break;
   }
   case af::Msg::TFarmLoad:
   {
      AfContainerLock rlock( i_args->renders,  AfContainerLock::WRITELOCK);
      AfContainerLock mLock( i_args->monitors, AfContainerLock::WRITELOCK);

      printf("\n   ========= RELOADING FARM =========\n\n");
      std::string message;
      if( af::loadFarm( true))
      {
         RenderContainerIt rendersIt( i_args->renders);
         for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
         {
            render->getFarmHost();
            i_args->monitors->addEvent( af::Msg::TMonitorRendersChanged, render->getId());
         }
         message = "Reloaded successfully.";
         printf("\n   ========= FARM RELOADED SUCCESSFULLY =========\n\n");
      }
      else
      {
         message = "Failed, see server logs fo details. Check farm with \"afcmd fcheck\" at first.";
         printf("\n   ========= FARM RELOADING FAILED =========\n\n");
      }
      o_msg_response = new af::Msg();
      o_msg_response->setString( message);
      break;
   }

// ---------------------------------- Monitor ---------------------------------//
   case af::Msg::TMonitorRegister:
   {
      AfContainerLock lock( i_args->monitors, AfContainerLock::WRITELOCK);

      MonitorAf * newMonitor = new MonitorAf( i_msg);
      newMonitor->setAddressIP( i_msg->getAddress());
      o_msg_response = i_args->monitors->addMonitor( newMonitor);
      break;
   }
   case af::Msg::TMonitorUpdateId:
   {
      AfContainerLock lock( i_args->monitors, AfContainerLock::READLOCK);

      if( i_args->monitors->updateId( i_msg->int32()))
      {
         o_msg_response = new af::Msg( af::Msg::TMonitorId, i_msg->int32());
      }
      else
      {
         o_msg_response = new af::Msg( af::Msg::TMonitorId, 0);
      }
      break;
   }
   case af::Msg::TMonitorsListRequest:
   {
      AfContainerLock lock( i_args->monitors, AfContainerLock::READLOCK);

      o_msg_response = i_args->monitors->generateList( af::Msg::TMonitorsList);
      break;
   }
   case af::Msg::TMonitorsListRequestIds:
   {
      AfContainerLock lock( i_args->monitors, AfContainerLock::READLOCK);

      af::MCGeneral ids( i_msg);
      o_msg_response = i_args->monitors->generateList( af::Msg::TMonitorsList, ids);
      break;
   }

// ---------------------------------- Talk ---------------------------------//
   case af::Msg::TTalkRegister:
   {
      AfContainerLock tlock( i_args->talks,    AfContainerLock::WRITELOCK);
      AfContainerLock mlock( i_args->monitors, AfContainerLock::WRITELOCK);

      TalkAf * newTalk = new TalkAf( i_msg);
      newTalk->setAddressIP( i_msg->getAddress());
      o_msg_response = i_args->talks->addTalk( newTalk, i_args->monitors);
      break;
   }
   case af::Msg::TTalksListRequest:
   {
      AfContainerLock lock( i_args->talks, AfContainerLock::READLOCK);

      o_msg_response = i_args->talks->generateList( af::Msg::TTalksList);
      break;
   }
   case af::Msg::TTalksListRequestIds:
   {
      AfContainerLock lock( i_args->talks, AfContainerLock::READLOCK);

      af::MCGeneral ids( i_msg);
      o_msg_response = i_args->talks->generateList( af::Msg::TTalksList, ids);
      break;
   }
   case af::Msg::TTalkUpdateId:
   {
      AfContainerLock lock( i_args->talks, AfContainerLock::READLOCK);

      if( i_args->talks->updateId( i_msg->int32()))
      {
         o_msg_response = i_args->talks->generateList( af::Msg::TTalksList);
      }
      else
      {
         o_msg_response = new af::Msg( af::Msg::TTalkId, 0);
      }
      break;
   }
   case af::Msg::TTalkDistributeData:
   {
      AfContainerLock lock( i_args->talks, AfContainerLock::READLOCK);

      i_args->talks->distributeData( i_msg);
      break;
   }

// ---------------------------------- Render -------------------------------//
   case af::Msg::TRenderRegister:
   {
//printf("case af::Msg::TRenderRegister:\n");
      AfContainerLock rLock( i_args->renders,  AfContainerLock::WRITELOCK);
      AfContainerLock mLock( i_args->monitors, AfContainerLock::WRITELOCK);

      RenderAf * newRender = new RenderAf( i_msg);
      newRender->setAddressIP( i_msg->getAddress());
      o_msg_response = i_args->renders->addRender( newRender, i_args->monitors);
      break;
   }
   case af::Msg::TRenderUpdate:
   {
//printf("case af::Msg::TRenderUpdate:\n");
      AfContainerLock lock( i_args->renders, AfContainerLock::READLOCK);

      af::Render render_up( i_msg);
//printf("Msg::TRenderUpdate: %s - %s\n", render_up.getName().toUtf8().data(), time2Qstr( time(NULL)).toUtf8().data());
      RenderContainerIt rendersIt( i_args->renders);
      RenderAf* render = rendersIt.getRender( render_up.getId());

      int id = 0;
      // If there is not such render, a zero id will be send.
      // It is a signal for client to register again (may be server was restarted).
      if((render != NULL) && ( render->update( &render_up)))
      {
         id = render->getId();
      }
      o_msg_response = new af::Msg( af::Msg::TRenderId, id);
      break;
   }
   case af::Msg::TRendersListRequest:
   {
      AfContainerLock lock( i_args->renders, AfContainerLock::READLOCK);

      o_msg_response = i_args->renders->generateList( af::Msg::TRendersList);
      break;
   }
   case af::Msg::TRendersListRequestIds:
   {
      AfContainerLock lock( i_args->renders, AfContainerLock::READLOCK);

      af::MCGeneral ids( i_msg);
      o_msg_response = i_args->renders->generateList( af::Msg::TRendersList, ids);
      break;
   }
   case af::Msg::TRendersUpdateRequestIds:
   {
      AfContainerLock lock( i_args->renders, AfContainerLock::READLOCK);

      af::MCGeneral ids( i_msg);
      o_msg_response = i_args->renders->generateList( af::Msg::TRendersListUpdates, ids);
      break;
   }
   case af::Msg::TRenderLogRequestId:
   {
      AfContainerLock lock( i_args->renders,  AfContainerLock::READLOCK);

      RenderContainerIt rendersIt( i_args->renders);
      RenderAf* render = rendersIt.getRender( i_msg->int32());
      if( render == NULL )
      { // FIXME: Better to return some message in any case.
         break;
      }
      o_msg_response = new af::Msg;
      o_msg_response->setStringList( render->getLog());
      break;
   }
   case af::Msg::TRenderTasksLogRequestId:
   {
      AfContainerLock lock( i_args->renders,  AfContainerLock::READLOCK);

      RenderContainerIt rendersIt( i_args->renders);
      RenderAf* render = rendersIt.getRender( i_msg->int32());
      if( render == NULL )
      { // FIXME: Better to return some message in any case.
         break;
      }
      o_msg_response = new af::Msg;
      if( render->getTasksLog().empty())
      {
         o_msg_response->setString("No tasks execution log.");
      }
      else
      {
         o_msg_response->setStringList( render->getTasksLog());
      }
      break;
   }
   case af::Msg::TRenderInfoRequestId:
   {
      AfContainerLock lock( i_args->renders,  AfContainerLock::READLOCK);

      RenderContainerIt rendersIt( i_args->renders);
      RenderAf* render = rendersIt.getRender( i_msg->int32());
      if( render == NULL )
      { // FIXME: Better to return some message in any case.
         break;
      }
      o_msg_response = new af::Msg();

      std::string str = render->generateInfoString( true);
      str += "\n";
      str += render->getServicesString();
      std::string servicelimits = af::farm()->serviceLimitsInfoString( true);
      if( servicelimits.size())
      {
         str += "\n";
         str += servicelimits;
      }

      o_msg_response->setString( str);
      break;
   }

// ---------------------------------- Users -------------------------------//
   case af::Msg::TUserIdRequest:
   {
      AfContainerLock lock( i_args->users, AfContainerLock::READLOCK);

      af::MsgClassUserHost usr( i_msg);
      std::string name = usr.getUserName();
      int id = 0;
      UserContainerIt usersIt( i_args->users);
      for( af::User *user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
      {
         if( user->getName() == name)
         {
            id = user->getId();
         }
      }
      o_msg_response = new af::Msg( af::Msg::TUserId, id);
      break;
   }
   case af::Msg::TUsersListRequest:
   {
      AfContainerLock lock( i_args->users, AfContainerLock::READLOCK);

      o_msg_response = i_args->users->generateList( af::Msg::TUsersList);
      break;
   }
   case af::Msg::TUsersListRequestIds:
   {
      AfContainerLock lock( i_args->users, AfContainerLock::READLOCK);

      af::MCGeneral ids( i_msg);
      o_msg_response = i_args->users->generateList( af::Msg::TUsersList, ids);
      break;
   }
   case af::Msg::TUserLogRequestId:
   {
      AfContainerLock lock( i_args->users,  AfContainerLock::READLOCK);

      UserContainerIt usersIt( i_args->users);
      UserAf* user = usersIt.getUser( i_msg->int32());
      if( user == NULL )
      { // FIXME: Better to return some message in any case.
         break;
      }
      o_msg_response = new af::Msg();
      o_msg_response->setStringList( user->getLog());
      break;
   }
   case af::Msg::TUserJobsOrderRequestId:
   {
      AfContainerLock lock( i_args->users,  AfContainerLock::READLOCK);

      UserContainerIt usersIt( i_args->users);
      UserAf* user = usersIt.getUser( i_msg->int32());
      if( user == NULL )
      { // FIXME: Better to return some message in any case.
         break;
      }
      af::MCGeneral ids;
      user->generateJobsIds( ids);
      o_msg_response = new af::Msg( af::Msg::TUserJobsOrder, &ids);
      break;
   }

// ------------------------------------- Job -------------------------------//
   case af::Msg::TJobRegister:
   {
      // No containers locks needed here.
      // Job registration is a complex procedure.
      // It locks and unlocks needed containers itself.
      i_args->jobs->job_register( new JobAf( i_msg), i_args->users, i_args->monitors);
      break;
   }
   case af::Msg::TJobRequestId:
   {
      AfContainerLock lock( i_args->jobs,  AfContainerLock::READLOCK);

      JobContainerIt jobsIt( i_args->jobs);
      JobAf* job = jobsIt.getJob( i_msg->int32());
      if( job == NULL )
      {
         o_msg_response = new af::Msg( af::Msg::TJobRequestId, 0);
         break;
      }
      o_msg_response = new af::Msg( af::Msg::TJob, job);
      break;
   }
   case af::Msg::TJobLogRequestId:
   {
      AfContainerLock lock( i_args->jobs,  AfContainerLock::READLOCK);

      JobContainerIt jobsIt( i_args->jobs);
      JobAf* job = jobsIt.getJob( i_msg->int32());
      if( job == NULL )
      { // FIXME: Better to return some message in any case.
         break;
      }
      o_msg_response = new af::Msg();
      o_msg_response->setStringList( job->getLog());
      break;
   }
   case af::Msg::TJobErrorHostsRequestId:
   {
      AfContainerLock lock( i_args->jobs,  AfContainerLock::READLOCK);

      JobContainerIt jobsIt( i_args->jobs);
      JobAf* job = jobsIt.getJob( i_msg->int32());
      if( job == NULL )
      { // FIXME: Better to return some message in any case.
         break;
      }
      o_msg_response = new af::Msg();
      o_msg_response->setString( job->getErrorHostsListString());
      break;
   }
   case af::Msg::TJobProgressRequestId:
   {
      AfContainerLock lock( i_args->jobs,  AfContainerLock::READLOCK);

      JobContainerIt jobsIt( i_args->jobs);
      JobAf* job = jobsIt.getJob( i_msg->int32());
      if( job == NULL )
      {
         // FIXME: Send back the same message on error - is it good?
         o_msg_response = new af::Msg( af::Msg::TJobProgressRequestId, 0);
         break;
      }
      o_msg_response = new af::Msg;
      job->writeProgress( *o_msg_response);
      break;
   }
   case af::Msg::TJobsListRequest:
   {
      AfContainerLock lock( i_args->jobs, AfContainerLock::READLOCK);

      o_msg_response = i_args->jobs->generateList( af::Msg::TJobsList);
      break;
   }
   case af::Msg::TJobsListRequestIds:
   {
      AfContainerLock lock( i_args->jobs, AfContainerLock::READLOCK);

      af::MCGeneral ids( i_msg);
      o_msg_response = i_args->jobs->generateList( af::Msg::TJobsList, ids);
      break;
   }
   case af::Msg::TJobsListRequestUserId:
   {
      AfContainerLock jLock( i_args->jobs,  AfContainerLock::READLOCK);
      AfContainerLock uLock( i_args->users, AfContainerLock::READLOCK);

      o_msg_response = i_args->users->generateJobsList( i_msg->int32());
      if( o_msg_response == NULL )
      {
         o_msg_response = new af::Msg( af::Msg::TUserId, 0);
      }
      break;
   }
   case af::Msg::TJobsListRequestUsersIds:
   {
      AfContainerLock jLock( i_args->jobs,  AfContainerLock::READLOCK);
      AfContainerLock uLock( i_args->users, AfContainerLock::READLOCK);

      af::MCGeneral ids( i_msg);
      o_msg_response = i_args->users->generateJobsList( ids);
      break;
   }
   case af::Msg::TTaskRequest:
   {
      AfContainerLock lock( i_args->jobs,  AfContainerLock::READLOCK);

      af::MCTaskPos mctaskpos( i_msg);
      JobContainerIt jobsIt( i_args->jobs);
      JobAf* job = jobsIt.getJob( mctaskpos.getJobId());
      if( job == NULL )
      {
         o_msg_response = new af::Msg();
         std::ostringstream stream;
         stream << "Msg::TTaskRequest: No job with id=" << mctaskpos.getJobId();
         o_msg_response->setString( stream.str());
         break;
      }
      af::TaskExec * task = job->generateTask( mctaskpos.getNumBlock(), mctaskpos.getNumTask());
      if( task )
      {
         o_msg_response = new af::Msg( af::Msg::TTask, task);
         delete task;
      }
      else
      {
         o_msg_response = new af::Msg();
         std::ostringstream stream;
         stream << "Msg::TTaskRequest: No such task[" << mctaskpos.getJobId() << "][" << mctaskpos.getNumBlock() << "][" << mctaskpos.getNumTask() << "]";
         o_msg_response->setString( stream.str());
      }
      break;
   }
   case af::Msg::TTaskLogRequest:
   {
      AfContainerLock lock( i_args->jobs,  AfContainerLock::READLOCK);

      af::MCTaskPos mctaskpos( i_msg);
      JobContainerIt jobsIt( i_args->jobs);
      JobAf* job = jobsIt.getJob( mctaskpos.getJobId());
      if( job == NULL )
      {
         o_msg_response = new af::Msg();
         std::ostringstream stream;
         stream << "Msg::TTaskLogRequest: No job with id=" << mctaskpos.getJobId();
         o_msg_response->setString( stream.str());
         break;
      }
      const std::list<std::string> * list = &(job->getTaskLog( mctaskpos.getNumBlock(), mctaskpos.getNumTask()));
      if( list == NULL )
      {
         // FIXME: Better to return some message in any case.
         break;
      }
      o_msg_response = new af::Msg();
      if( list->size() == 0)
      {
         std::list<std::string> list;
         list.push_back("Task log is empty.");
         o_msg_response->setStringList( list);
      }
      else
      {
         o_msg_response->setStringList( *list);
      }
      break;
   }
   case af::Msg::TTaskErrorHostsRequest:
   {
      AfContainerLock lock( i_args->jobs,  AfContainerLock::READLOCK);

      af::MCTaskPos mctaskpos( i_msg);
      JobContainerIt jobsIt( i_args->jobs);
      JobAf* job = jobsIt.getJob( mctaskpos.getJobId());
      if( job == NULL )
      {
         // FIXME: Better to return some message in any case.
         break;
      }
      o_msg_response = new af::Msg();
      o_msg_response->setString( job->getErrorHostsListString( mctaskpos.getNumBlock(), mctaskpos.getNumTask()));
      break;
   }
   case af::Msg::TTaskOutputRequest:
   {
      af::Msg * msg_request_render = NULL;
      std::string filename;
      af::MCTaskPos mctaskpos( i_msg);
      o_msg_response = new af::Msg();
//printf("ThreadReadMsg::msgCase: case af::Msg::TJobTaskOutputRequest: job=%d, block=%d, task=%d, number=%d\n", mctaskpos.getJobId(), mctaskpos.getNumBlock(), mctaskpos.getNumTask(), mctaskpos.getNumber());
      {
         AfContainerLock jLock( i_args->jobs,    AfContainerLock::READLOCK);
         AfContainerLock rLock( i_args->renders, AfContainerLock::READLOCK);

         JobContainerIt jobsIt( i_args->jobs);
         JobAf* job = jobsIt.getJob( mctaskpos.getJobId());
         if( job != NULL )
         {
            // Trying to set message to request output from running remote host.
            if( job->getTaskStdOut( mctaskpos, o_msg_response, filename, i_args->renders) == false )
            {
               // If false, message contains error text to send back to client.
               break;
            }
         }
         else
         {
            o_msg_response->setString("Job is NULL.");
            break;
         }
      }
      if( o_msg_response->isNull() )
      {
      //
      //    Retrieving output from file
      //
         std::string err;
         int readsize = -1;
         char * data = af::fileRead( filename, readsize, af::Msg::SizeDataMax, &err);
         if( data )
         {
            o_msg_response->setData( readsize, data);
            delete [] data;
         }
         if( err.size())
         {
            err = std::string("Getting task output: ") + err;
            AFCommon::QueueLogError( err);
            if( o_msg_response->isNull())
            {
               if( af::pathFileExists( filename))
               {
                  err = std::string("ERROR: ") + err;
                  o_msg_response->setString( err);
               }
               else
               {
                  o_msg_response->setString("No output exists.");
               }
            }
         }
      }
      else
      {
      //
      //    Retrieving output from render
      //
         msg_request_render = o_msg_response;
         o_msg_response = new af::Msg();
         if( af::msgRequest( msg_request_render, o_msg_response) == false )
         {
            delete o_msg_response;
            o_msg_response = new af::Msg();
            o_msg_response->setString("Retrieving output from render failed. See server logs for details.");
         }
         delete msg_request_render;
      }
      break;
   }
   case af::Msg::TJobsWeightRequest:
   {
      AfContainerLock jLock( i_args->jobs,    AfContainerLock::READLOCK);

      af::MCJobsWeight jobsWeight;
      i_args->jobs->getWeight( jobsWeight);
      o_msg_response = new af::Msg( af::Msg::TJobsWeight, &jobsWeight);
      break;
   }
   case af::Msg::TTaskUpdateState:
   {
      af::MCTaskUp taskup( i_msg);
      af::MCTaskPos taskpos( taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumber());
      o_msg_response = new af::Msg( af::Msg::TRenderCloseTask, &taskpos);
   }
   // Cases for run cycle thread:
   case af::Msg::TTaskUpdatePercent:
   case af::Msg::TTaskListenOutput:
   case af::Msg::TTasksSkip:
   case af::Msg::TBlockErrorsAvoidHost:
   case af::Msg::TBlockErrorRetries:
   case af::Msg::TBlockErrorsSameHost:
   case af::Msg::TBlockErrorsForgiveTime:
   case af::Msg::TBlockTasksMaxRunTime:
   case af::Msg::TBlockResetErrorHosts:
   case af::Msg::TBlockDependMask:
   case af::Msg::TBlockTasksDependMask:
   case af::Msg::TBlockHostsMask:
   case af::Msg::TBlockHostsMaskExclude:
   case af::Msg::TBlockMaxRunningTasks:
   case af::Msg::TBlockMaxRunTasksPerHost:
   case af::Msg::TBlockCommand:
   case af::Msg::TBlockWorkingDir:
   case af::Msg::TBlockFiles:
   case af::Msg::TBlockCmdPost:
   case af::Msg::TBlockService:
   case af::Msg::TBlockParser:
   case af::Msg::TBlockCapacity:
   case af::Msg::TBlockCapacityCoeffMin:
   case af::Msg::TBlockCapacityCoeffMax:
   case af::Msg::TBlockMultiHostMin:
   case af::Msg::TBlockMultiHostMax:
   case af::Msg::TBlockMultiHostWaitMax:
   case af::Msg::TBlockMultiHostWaitSrv:
   case af::Msg::TBlockNeedMemory:
   case af::Msg::TBlockNeedPower:
   case af::Msg::TBlockNeedHDD:
   case af::Msg::TBlockNeedProperties:
   case af::Msg::TTasksRestart:
   case af::Msg::TJobAnnotate:
   case af::Msg::TJobHostsMask:
   case af::Msg::TJobHostsMaskExclude:
   case af::Msg::TJobDependMask:
   case af::Msg::TJobDependMaskGlobal:
   case af::Msg::TJobMaxRunningTasks:
   case af::Msg::TJobMaxRunTasksPerHost:
   case af::Msg::TJobWaitTime:
   case af::Msg::TJobLifeTime:
   case af::Msg::TJobPriority:
   case af::Msg::TJobNeedOS:
   case af::Msg::TJobNeedProperties:
   case af::Msg::TJobCmdPost:
   case af::Msg::TJobStart:
   case af::Msg::TJobStop:
   case af::Msg::TJobRestart:
   case af::Msg::TJobRestartErrors:
   case af::Msg::TJobResetErrorHosts:
   case af::Msg::TJobPause:
   case af::Msg::TJobRestartPause:
   case af::Msg::TJobDelete:
   case af::Msg::TRenderAnnotate:
   case af::Msg::TRenderSetPriority:
   case af::Msg::TRenderSetCapacity:
   case af::Msg::TRenderSetMaxTasks:
   case af::Msg::TRenderSetService:
   case af::Msg::TRenderRestoreDefaults:
   case af::Msg::TRenderSetNIMBY:
   case af::Msg::TRenderSetNimby:
   case af::Msg::TRenderSetUser:
   case af::Msg::TRenderSetFree:
   case af::Msg::TRenderEjectTasks:
   case af::Msg::TRenderEjectNotMyTasks:
   case af::Msg::TRenderDelete:
   case af::Msg::TRenderRestart:
   case af::Msg::TRenderReboot:
   case af::Msg::TRenderShutdown:
   case af::Msg::TRenderDeregister:
   case af::Msg::TRenderExit:
   case af::Msg::TRenderWOLSleep:
   case af::Msg::TRenderWOLWake:
   case af::Msg::TUserAnnotate:
   case af::Msg::TUserAdd:
   case af::Msg::TUserDel:
   case af::Msg::TUserHostsMask:
   case af::Msg::TUserHostsMaskExclude:
   case af::Msg::TUserMaxRunningTasks:
   case af::Msg::TUserPriority:
   case af::Msg::TUserJobsLifeTime:
   case af::Msg::TUserJobsSolveMethod:
   case af::Msg::TUserErrorsAvoidHost:
   case af::Msg::TUserErrorRetries:
   case af::Msg::TUserErrorsTaskSameHost:
   case af::Msg::TUserErrorsForgiveTime:
   case af::Msg::TUserMoveJobsUp:
   case af::Msg::TUserMoveJobsDown:
   case af::Msg::TUserMoveJobsTop:
   case af::Msg::TUserMoveJobsBottom:
   case af::Msg::TTalkDeregister:
   case af::Msg::TMonitorSubscribe:
   case af::Msg::TMonitorUnsubscribe:
   case af::Msg::TMonitorDeregister:
   case af::Msg::TMonitorUsersJobs:
   case af::Msg::TMonitorJobsIdsAdd:
   case af::Msg::TMonitorJobsIdsSet:
   case af::Msg::TMonitorJobsIdsDel:
   case af::Msg::TMonitorMessage:
   case af::Msg::TMonitorExit:
   case af::Msg::TTalkExit:
   {
//printf("ThreadReadMsg::msgCase: pushing message to run thread:\n"); i_msg->stdOut();
      // Push message for run cycle thread.
      i_args->msgQueue->pushMsg( i_msg);
      // Need to return here to not to delete input message (i_msg) later.
      return o_msg_response;
      //  ( o_msg_response is NULL in all cases except Msg::TTaskUpdateState,
      //    in that case render should recieve an answer to close task
      //    and finish sending any updates for the task )
   }
// -------------------------------------------------------------------------//
   case af::Msg::TVersionMismatch:
   {
      AFCommon::QueueLogError( i_msg->generateInfoString( false));
      o_msg_response = new af::Msg( af::Msg::TVersionMismatch, 1);
      break;
   }
   case af::Msg::TInvalid:
   {
      AFCommon::QueueLogError( std::string("Invalid message recieved: ") + i_msg->generateInfoString( false));
      break;
   }
   default:
   {
      AFCommon::QueueLogError( std::string("Unknown message recieved: ") + i_msg->generateInfoString( false));
      break;
   }
   }
//if(i_msg->type()==Msg::TJobRegister)printf("Msg::TJobRegister:returning.\n");

   // Deleting input message as it not needed any more.
   delete i_msg;

   // Returning an answer
   return o_msg_response;
}
