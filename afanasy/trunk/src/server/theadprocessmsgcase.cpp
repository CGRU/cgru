#include "theadprocessmsg.h"

#include <QtCore/QFile>

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msgclasses/mcjobsweight.h"

#include "../libafnetwork/communications.h"

#include "msgaf.h"
#include "monitoraf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

//############################################################################################################
MsgAf* TheadReadMsg::msgCase( MsgAf *msg)
{
//msg->stdOut();
   MsgAf *msg_response = NULL;

   switch( msg->type())
   {
   case af::Msg::TNULL:
   case af::Msg::TDATA:
   case af::Msg::TTESTDATA:
   case af::Msg::TQString:
   case af::Msg::TQStringList:
   {
      msg->stdOutData();
      break;
   }
   case af::Msg::TStatRequest:
   {
      msg_response = new MsgAf;
      com::statwrite( msg_response);
      break;
   }
   case af::Msg::TConfirm:
   {
      printf("Thread process message: Msg::TConfirm: %d\n", msg->int32());
      msgQueue ->pushMsg( new MsgAf( af::Msg::TConfirm, 1));
      msg_response = new MsgAf( af::Msg::TConfirm, 1 - msg->int32());
      break;
   }
   case af::Msg::TConfigLoad:
   {
      AfContainerLock tlock( talks, AfContainer::WRITELOCK);   AfContainerLock rlock( renders, AfContainer::WRITELOCK);
      AfContainerLock jlock(  jobs, AfContainer::WRITELOCK);   AfContainerLock ulock(   users, AfContainer::WRITELOCK);
      QString message;
      if( af::Environment::reload()) message = "Reloaded successfully.";
      else                message = "Failed, see server logs fo details.";
      msg_response = new MsgAf();
      msg_response->setString( message);
      break;
   }
   case af::Msg::TFarmLoad:
   {
      AfContainerLock rlock( renders,  AfContainer::WRITELOCK);
      AfContainerLock mLock( monitors, AfContainer::WRITELOCK);

      printf("RELOADING FARM");
      QString message;
      if( af::loadFarm( true))
      {
         RenderContainerIt rendersIt( renders);
         for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
         {
            render->getFarmHost();
            monitors->addEvent( af::Msg::TMonitorRendersChanged, render->getId());
            render->stdOut();
         }
         message = "Reloaded successfully.";
      }
      else message = "Failed, see server logs fo details.";
      msg_response = new MsgAf();
      msg_response->setString( message);
      break;
   }

// ---------------------------------- Monitor ---------------------------------//
   case af::Msg::TMonitorRegister:
   {
      AfContainerLock lock( monitors, AfContainer::WRITELOCK);

      msg_response = monitors->addMonitor( new MonitorAf( msg, msg->getAddress()));
      break;
   }
   case af::Msg::TMonitorUpdateId:
   {
      AfContainerLock lock( monitors, AfContainer::READLOCK);

      if( monitors->updateId( msg->int32())) msg_response = new MsgAf( af::Msg::TMonitorId, msg->int32());
      else msg_response = new MsgAf( af::Msg::TMonitorId, 0);
      break;
   }
   case af::Msg::TMonitorsListRequest:
   {
      AfContainerLock lock( monitors, AfContainer::READLOCK);

      msg_response = monitors->generateList( af::Msg::TMonitorsList);
      break;
   }
   case af::Msg::TMonitorsListRequestIds:
   {
      AfContainerLock lock( monitors, AfContainer::READLOCK);

      af::MCGeneral ids( msg);
      msg_response = monitors->generateList( af::Msg::TMonitorsList, ids);
      break;
   }

// ---------------------------------- Talk ---------------------------------//
   case af::Msg::TTalkRegister:
   {
      AfContainerLock tlock( talks,    AfContainer::WRITELOCK);
      AfContainerLock mlock( monitors, AfContainer::WRITELOCK);

      msg_response = talks->addTalk( new TalkAf( msg, msg->getAddress()), monitors);
      break;
   }
   case af::Msg::TTalksListRequest:
   {
      AfContainerLock lock( talks, AfContainer::READLOCK);

      msg_response = talks->generateList( af::Msg::TTalksList);
      break;
   }
   case af::Msg::TTalksListRequestIds:
   {
      AfContainerLock lock( talks, AfContainer::READLOCK);

      af::MCGeneral ids( msg);
      msg_response = talks->generateList( af::Msg::TTalksList, ids);
      break;
   }
   case af::Msg::TTalkUpdateId:
   {
      AfContainerLock lock( talks, AfContainer::READLOCK);

      if( talks->updateId( msg->int32())) msg_response = talks->generateList( af::Msg::TTalksList);
      else msg_response = new MsgAf( af::Msg::TTalkId, 0);
      break;
   }
   case af::Msg::TTalkDistributeData:
   {
      AfContainerLock lock( talks, AfContainer::READLOCK);

      talks->distributeData( msg);
      break;
   }

// ---------------------------------- Render -------------------------------//
   case af::Msg::TRenderRegister:
   {
//printf("case af::Msg::TRenderRegister:\n");
      AfContainerLock rLock( renders,  AfContainer::WRITELOCK);
      AfContainerLock mLock( monitors, AfContainer::WRITELOCK);

      msg_response = renders->addRender( new RenderAf( msg, msg->getAddress()), monitors);
      break;
   }
   case af::Msg::TRenderUpdate:
   {
//printf("case af::Msg::TRenderUpdate:\n");
      AfContainerLock lock( renders, AfContainer::READLOCK);

      af::Render render_up( msg);
//printf("Msg::TRenderUpdate: %s - %s\n", render_up.getName().toUtf8().data(), time2Qstr( time(NULL)).toUtf8().data());
      RenderContainerIt rendersIt( renders);
      RenderAf* render = rendersIt.getRender( render_up.getId());
      int id = 0;
      if((render != NULL) && ( render->update( &render_up))) id = render->getId();
      msg_response = new MsgAf( af::Msg::TRenderId, id);
      break;
   }
   case af::Msg::TRendersListRequest:
   {
      AfContainerLock lock( renders, AfContainer::READLOCK);

      msg_response = renders->generateList( af::Msg::TRendersList);
      break;
   }
   case af::Msg::TRendersListRequestIds:
   {
      AfContainerLock lock( renders, AfContainer::READLOCK);

      af::MCGeneral ids( msg);
      msg_response = renders->generateList( af::Msg::TRendersList, ids);
      break;
   }
   case af::Msg::TRendersUpdateRequestIds:
   {
      AfContainerLock lock( renders, AfContainer::READLOCK);

      af::MCGeneral ids( msg);
      msg_response = renders->generateList( af::Msg::TRendersListUpdates, ids);
      break;
   }
   case af::Msg::TRenderLogRequestId:
   {
      AfContainerLock lock( renders,  AfContainer::READLOCK);

      RenderContainerIt rendersIt( renders);
      RenderAf* render = rendersIt.getRender( msg->int32());
      if( render == NULL ) break;
      QStringList *list = render->getLog();
      if( list == NULL ) break;
      msg_response = new MsgAf();
      msg_response->setStringList( *list);
      break;
   }
   case af::Msg::TRenderServicesRequestId:
   {
      AfContainerLock lock( renders,  AfContainer::READLOCK);

      RenderContainerIt rendersIt( renders);
      RenderAf* render = rendersIt.getRender( msg->int32());
      if( render == NULL ) break;
      msg_response = new MsgAf();
      render->getServices( msg_response);
      break;
   }

// ---------------------------------- Users -------------------------------//
   case af::Msg::TUserIdRequest:
   {
      AfContainerLock lock( users, AfContainer::READLOCK);

      af::MsgClassUserHost usr( msg);
      QString name = usr.getUserName();
      int id = 0;
      UserContainerIt usersIt( users);
      for( af::User *user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
         if( user->getName() == name) id = user->getId();
      msg_response = new MsgAf( af::Msg::TUserId, id);
      break;
   }
   case af::Msg::TUsersListRequest:
   {
      AfContainerLock lock( users, AfContainer::READLOCK);

      msg_response = users->generateList( af::Msg::TUsersList);
      break;
   }
   case af::Msg::TUsersListRequestIds:
   {
      AfContainerLock lock( users, AfContainer::READLOCK);

      af::MCGeneral ids( msg);
      msg_response = users->generateList( af::Msg::TUsersList, ids);
      break;
   }
   case af::Msg::TUserLogRequestId:
   {
      AfContainerLock lock( users,  AfContainer::READLOCK);

      UserContainerIt usersIt( users);
      UserAf* user = usersIt.getUser( msg->int32());
      if( user == NULL ) break;
      QStringList *list = user->getLog();
      if( list == NULL ) break;
      msg_response = new MsgAf();
      msg_response->setStringList( *user->getLog());
      break;
   }
   case af::Msg::TUserJobsOrderRequestId:
   {
      AfContainerLock lock( users,  AfContainer::READLOCK);

      UserContainerIt usersIt( users);
      UserAf* user = usersIt.getUser( msg->int32());
      if( user == NULL ) break;
      af::MCGeneral ids;
      user->generateJobsIds( ids);
      msg_response = new MsgAf( af::Msg::TUserJobsOrder, &ids);
      break;
   }

// ------------------------------------- Job -------------------------------//
   case af::Msg::TJobRegister:
   {
      jobs->job_register( new JobAf( msg), users, monitors);
      break;
   }
   case af::Msg::TJobRequestId:
   {
      AfContainerLock lock( jobs,  AfContainer::READLOCK);

      JobContainerIt jobsIt( jobs);
      JobAf* job = jobsIt.getJob( msg->int32());
      if( job == NULL )
      {
         msg_response = new MsgAf( af::Msg::TJobRequestId, 0);
         break;
      }
      msg_response = new MsgAf( af::Msg::TJob, job);
      break;
   }
   case af::Msg::TJobLogRequestId:
   {
      AfContainerLock lock( jobs,  AfContainer::READLOCK);

      JobContainerIt jobsIt( jobs);
      JobAf* job = jobsIt.getJob( msg->int32());
      if( job == NULL ) break;
      QStringList *list = job->getLog();
      if( list == NULL ) break;
      msg_response = new MsgAf();
      msg_response->setStringList( *job->getLog());
      break;
   }
   case af::Msg::TJobErrorHostsRequestId:
   {
      AfContainerLock lock( jobs,  AfContainer::READLOCK);

      JobContainerIt jobsIt( jobs);
      JobAf* job = jobsIt.getJob( msg->int32());
      if( job == NULL ) break;
      QStringList avoidHostsList;
      msg_response = new MsgAf();
      if( job->getErrorHostsList( avoidHostsList))
         msg_response->setStringList( avoidHostsList);
      else
      {
         QString message("No error hosts.");
         msg_response->setString( message);
      }
      break;
   }
   case af::Msg::TJobProgressRequestId:
   {
      AfContainerLock lock( jobs,  AfContainer::READLOCK);

      JobContainerIt jobsIt( jobs);
      JobAf* job = jobsIt.getJob( msg->int32());
      if( job == NULL )
      {
         msg_response = new MsgAf( af::Msg::TJobProgressRequestId, 0);
         break;
      }
      msg_response = new MsgAf;
      job->writeProgress( *msg_response);
      break;
   }
   case af::Msg::TJobsListRequest:
   {
      AfContainerLock lock( jobs, AfContainer::READLOCK);

      msg_response = jobs->generateList( af::Msg::TJobsList);
      break;
   }
   case af::Msg::TJobsListRequestIds:
   {
      AfContainerLock lock( jobs, AfContainer::READLOCK);

      af::MCGeneral ids( msg);
      msg_response = jobs->generateList( af::Msg::TJobsList, ids);
      break;
   }
   case af::Msg::TJobsListRequestUserId:
   {
      AfContainerLock jLock( jobs,  AfContainer::READLOCK);
      AfContainerLock uLock( users, AfContainer::READLOCK);

      msg_response = users->generateJobsList( msg->int32());
      if( msg_response == NULL ) msg_response = new MsgAf( af::Msg::TUserId, 0);
      break;
   }
   case af::Msg::TJobsListRequestUsersIds:
   {
      AfContainerLock jLock( jobs,  AfContainer::READLOCK);
      AfContainerLock uLock( users, AfContainer::READLOCK);

      af::MCGeneral ids( msg);
      msg_response = users->generateJobsList( ids);
      break;
   }
   case af::Msg::TTaskRequest:
   {
      AfContainerLock lock( jobs,  AfContainer::READLOCK);

      af::MCTaskPos mctaskpos( msg);
      JobContainerIt jobsIt( jobs);
      JobAf* job = jobsIt.getJob( mctaskpos.getJobId());
      if( job == NULL )
      {
         msg_response = new MsgAf();
         QString str = QString("Msg::TTaskRequest: No job with id=%d").arg(mctaskpos.getJobId());
         msg_response->setString( str);
      }
      af::TaskExec * task = job->generateTask( mctaskpos.getNumBlock(), mctaskpos.getNumTask());
      if( task )
      {
         msg_response = new MsgAf( af::Msg::TTask, task);
         delete task;
      }
      else
      {
         msg_response = new MsgAf();
         QString str = QString("Msg::TTaskRequest: No such task[%d][%d][%d]")
                       .arg(mctaskpos.getJobId()).arg( mctaskpos.getNumBlock()).arg( mctaskpos.getNumTask());
         msg_response->setString( str);
      }
      break;
   }
   case af::Msg::TTaskLogRequest:
   {
      AfContainerLock lock( jobs,  AfContainer::READLOCK);

      af::MCTaskPos mctaskpos( msg);
      JobContainerIt jobsIt( jobs);
      JobAf* job = jobsIt.getJob( mctaskpos.getJobId());
      if( job == NULL )
      {
         msg_response = new MsgAf();
         QString str = QString("Msg::TTaskLogRequest: No job with id=%d").arg(mctaskpos.getJobId());
         msg_response->setString( str);
      }
      QStringList *list = job->getTaskLog( mctaskpos.getNumBlock(), mctaskpos.getNumTask());
      if( list == NULL ) break;
      msg_response = new MsgAf();
      msg_response->setStringList( *job->getTaskLog( mctaskpos.getNumBlock(), mctaskpos.getNumTask()));
      break;
   }
   case af::Msg::TTaskErrorHostsRequest:
   {
      AfContainerLock lock( jobs,  AfContainer::READLOCK);

      af::MCTaskPos mctaskpos( msg);
      JobContainerIt jobsIt( jobs);
      JobAf* job = jobsIt.getJob( mctaskpos.getJobId());
      if( job == NULL ) break;
      QStringList list;
      msg_response = new MsgAf();
      if( job->getErrorHostsList( list, mctaskpos.getNumBlock(), mctaskpos.getNumTask()))
         msg_response->setStringList( list);
      else
      {
         QString message("No error hosts.");
         msg_response->setString( message);
      }
      break;
   }
   case af::Msg::TTaskOutputRequest:
   {
      MsgAf * msg_request_render = NULL;
      QString filename;
      af::MCTaskPos mctaskpos( msg);
      msg_response = new MsgAf();
//printf("TheadReadMsg::msgCase: case af::Msg::TJobTaskOutputRequest: job=%d, block=%d, task=%d, number=%d\n", mctaskpos.getJobId(), mctaskpos.getNumBlock(), mctaskpos.getNumTask(), mctaskpos.getNumber());
      {
         AfContainerLock jLock( jobs,    AfContainer::READLOCK);
         AfContainerLock rLock( renders, AfContainer::READLOCK);

         JobContainerIt jobsIt( jobs);
         JobAf* job = jobsIt.getJob( mctaskpos.getJobId());
         if( job != NULL )
         {
            // Trying to set message to request output from running remote host.
            if( job->getTaskStdOut( mctaskpos, msg_response, filename, renders) == false )
            {
               // If false, message contains error text to send back to client.
               break;
            }
         }
         else
         {
            QString str("job is NULL");
            msg_response->setString( str);
            break;
         }
      }
      if( msg_response->isNull() )
      {
      //
      //    Retrieving output from file
      //
         QFile outFile( filename);
         if( outFile.exists() == false )
         {
            QString str(QString("No output file '%1'.").arg(filename));
            msg_response->setString( str);
         }
         else if( outFile.open( QIODevice::ReadOnly ) == false )
         {
            QString str(QString("Can't open output file '%1'.").arg(filename));
            msg_response->setString( str);
         }
         else
         {
            QByteArray output = outFile.readAll();
            int output_length = output.size();
            if( output_length == 0)
            {
               QString str(QString("Output file is empty '%1'.").arg(filename));
               msg_response->setString( str);
            }
            else
               msg_response->setData( output_length, output.data());
         }
      }
      else
      {
      //
      //    Retrieving output from render
      //
         msg_request_render = msg_response;
         msg_response = new MsgAf();
         if( msg_request_render->request( msg_response) == false )
         {
            delete msg_response;
            msg_response = new MsgAf();
            QString str("Retrieving output from render failed. See server logs for details.");
            msg_response->setString( str);
         }
         delete msg_request_render;
      }
      break;
   }
   case af::Msg::TJobsWeightRequest:
   {
      AfContainerLock jLock( jobs,    AfContainer::READLOCK);

      af::MCJobsWeight jobsWeight;
      jobs->getWeight( jobsWeight);
      msg_response = new MsgAf( af::Msg::TJobsWeight, &jobsWeight);
      break;
   }
   case af::Msg::TTaskUpdateState:
   {
      af::MCTaskUp taskup( msg);
      af::MCTaskPos taskpos( taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumber());
      msg_response = new MsgAf( af::Msg::TRenderCloseTask, &taskpos);
   }
   case af::Msg::TTaskUpdatePercent:
   case af::Msg::TTaskListenOutput:
   case af::Msg::TTasksSkip:
   case af::Msg::TBlockErrorsAvoidHost:
   case af::Msg::TBlockErrorRetries:
   case af::Msg::TBlockErrorsSameHost:
   case af::Msg::TBlockTasksMaxRunTime:
   case af::Msg::TBlockResetErrorHosts:
   case af::Msg::TBlockDependMask:
   case af::Msg::TBlockTasksDependMask:
   case af::Msg::TBlockHostsMask:
   case af::Msg::TBlockHostsMaskExclude:
   case af::Msg::TBlockMaxHosts:
   case af::Msg::TBlockCommand:
   case af::Msg::TBlockWorkingDir:
   case af::Msg::TBlockFiles:
   case af::Msg::TBlockCmdPost:
   case af::Msg::TBlockTasksType:
   case af::Msg::TBlockParserType:
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
   case af::Msg::TJobMaxHosts:
   case af::Msg::TJobWaitTime:
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
   case af::Msg::TRenderPriority:
   case af::Msg::TRenderCapacity:
   case af::Msg::TRenderSetService:
   case af::Msg::TRenderRestoreDefaults:
   case af::Msg::TRenderNIMBY:
   case af::Msg::TRenderNimby:
   case af::Msg::TRenderUser:
   case af::Msg::TRenderFree:
   case af::Msg::TRenderEject:
   case af::Msg::TRenderExit:
   case af::Msg::TRenderDelete:
   case af::Msg::TRenderRestart:
   case af::Msg::TRenderStart:
   case af::Msg::TRenderReboot:
   case af::Msg::TRenderShutdown:
   case af::Msg::TRenderDeregister:
   case af::Msg::TUserAnnotate:
   case af::Msg::TUserAdd:
   case af::Msg::TUserDel:
   case af::Msg::TUserHostsMask:
   case af::Msg::TUserHostsMaskExclude:
   case af::Msg::TUserMaxHosts:
   case af::Msg::TUserPriority:
   case af::Msg::TUserErrorsAvoidHost:
   case af::Msg::TUserErrorRetries:
   case af::Msg::TUserErrorsTaskSameHost:
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
   {
//printf("TheadReadMsg::msgCase: pushing message to run thread:\n"); msg->stdOut();
      msgQueue->pushMsg( msg);
      return msg_response;
   }

// -------------------------------------------------------------------------//
   case af::Msg::TVersionMismatch:
   {
      msg->stdOut();
      msg_response = new MsgAf( af::Msg::TVersionMismatch, 1);
      break;
   }
   case af::Msg::TInvalid:
   {
      msg->stdOut();
      AFERROR("TheadReadMsg::msgCase: Invalid message recieved.\n");
      break;
   }
   default:
   {
      msg->stdOut();
      AFERROR("TheadReadMsg::msgCase: Message with unknown type recieved.\n");
      break;
   }
   }
//if(msg->type()==Msg::TJobRegister)printf("Msg::TJobRegister:returning.\n");
   delete msg;
   return msg_response;
}
