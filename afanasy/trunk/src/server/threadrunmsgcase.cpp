#include "threadrun.h"

#include <stdio.h>
#include <stdlib.h>

#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/msgclasses/mctaskspos.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"

#include "msgaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void ThreadRun::msgCase( MsgAf *msg)
{
switch ( msg->type())
{
   case af::Msg::TTalkExit:
   {
      af::MCGeneral mcgeneral( msg);
      talks->action( mcgeneral, msg->type(), NULL, monitors);
      break;
   }
   case af::Msg::TTalkDeregister:
   {
      if( talks->setZombie( msg->int32())) monitors->addEvent( af::Msg::TMonitorTalksDel, msg->int32());
      break;
   }
   case af::Msg::TMonitorExit:
   {
      af::MCGeneral mcgeneral( msg);
      monitors->action( mcgeneral, msg->type(), NULL, NULL);
      break;
   }
   case af::Msg::TMonitorDeregister:
   {
      if( monitors->setZombie( msg->int32())) monitors->addEvent( af::Msg::TMonitorMonitorsDel, msg->int32());
      break;
   }
   case af::Msg::TMonitorMessage:
   {
      af::MCGeneral mcgeneral( msg);
      monitors->sendMessage( mcgeneral);
      break;
   }
   case af::Msg::TMonitorSubscribe:
   case af::Msg::TMonitorUnsubscribe:
   case af::Msg::TMonitorUsersJobs:
   case af::Msg::TMonitorJobsIdsAdd:
   case af::Msg::TMonitorJobsIdsSet:
   case af::Msg::TMonitorJobsIdsDel:
   {
      af::MCGeneral ids( msg);
      monitors->setInterest( msg->type(), ids);
      break;
   }
   case af::Msg::TRenderAnnotate:
   case af::Msg::TRenderPriority:
   case af::Msg::TRenderCapacity:
   case af::Msg::TRenderSetService:
   case af::Msg::TRenderRestoreDefaults:
   case af::Msg::TRenderUser:
   case af::Msg::TRenderNIMBY:
   case af::Msg::TRenderNimby:
   case af::Msg::TRenderFree:
   case af::Msg::TRenderEject:
   case af::Msg::TRenderDelete:
   case af::Msg::TRenderRestart:
   case af::Msg::TRenderStart:
   case af::Msg::TRenderReboot:
   case af::Msg::TRenderShutdown:
   case af::Msg::TRenderExit:
   {
      af::MCGeneral mcgeneral( msg);
      renders->action( mcgeneral, msg->type(), jobs, monitors);
      break;
   }
   case af::Msg::TRenderDeregister:
   {
      RenderContainerIt rendersIt( renders);
      RenderAf* render = rendersIt.getRender( msg->int32());
      if( render != NULL) render->deregister( jobs, monitors);
      break;
   }
   case af::Msg::TUserAnnotate:
   case af::Msg::TUserHostsMask:
   case af::Msg::TUserHostsMaskExclude:
   case af::Msg::TUserMaxRunningTasks:
   case af::Msg::TUserPriority:
   case af::Msg::TUserErrorsAvoidHost:
   case af::Msg::TUserErrorRetries:
   case af::Msg::TUserErrorsTaskSameHost:
   case af::Msg::TUserErrorsForgiveTime:
   case af::Msg::TUserJobsLifeTime:
   {
      af::MCGeneral mcgeneral( msg);
      users->action( mcgeneral, msg->type(), NULL, monitors);
      break;
   }
   case af::Msg::TUserMoveJobsUp:
   case af::Msg::TUserMoveJobsDown:
   case af::Msg::TUserMoveJobsTop:
   case af::Msg::TUserMoveJobsBottom:
   {
      UserContainerIt usersIt( users);
      af::MCGeneral mcgeneral( msg);
      UserAf* user = usersIt.getUser( mcgeneral.getNumber());
      user->moveJobs( mcgeneral, msg->type());
      monitors->addUser( user);
      break;
   }
   case af::Msg::TUserDel:
   case af::Msg::TUserAdd:
   {
      af::MCGeneral mcgeneral( msg);
      // If existing users IDs provided, it's simple action to change users permanent property
      if( mcgeneral.getCount()) users->action( mcgeneral, msg->type(), NULL, monitors);
      // If user with specified name does not exisit, new user must be created and put in container
      else users->setPermanent( mcgeneral, (msg->type()==af::Msg::TUserAdd)?true:false, monitors);
      break;
   }
   case af::Msg::TJobAnnotate:
   case af::Msg::TJobHostsMask:
   case af::Msg::TJobHostsMaskExclude:
   case af::Msg::TJobDependMask:
   case af::Msg::TJobDependMaskGlobal:
   case af::Msg::TJobMaxRunningTasks:
   case af::Msg::TJobWaitTime:
   case af::Msg::TJobLifeTime:
   case af::Msg::TJobPriority:
   case af::Msg::TJobStart:
   case af::Msg::TJobStop:
   case af::Msg::TJobRestart:
   case af::Msg::TJobRestartErrors:
   case af::Msg::TJobResetErrorHosts:
   case af::Msg::TJobPause:
   case af::Msg::TJobRestartPause:
   case af::Msg::TJobDelete:
   case af::Msg::TJobNeedOS:
   case af::Msg::TJobNeedProperties:
   case af::Msg::TJobCmdPost:
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
   {
      af::MCGeneral mcgeneral( msg);
      jobs->action( mcgeneral, msg->type(), renders, monitors);
      break;
   }
   case af::Msg::TTasksSkip:
   {
      JobContainerIt jobsIt( jobs);
      af::MCTasksPos taskspos( msg);
      JobAf* job = jobsIt.getJob( taskspos.getJobId());
      if( job != NULL) job->skipTasks( taskspos, renders, monitors);
      break;
   }
   case af::Msg::TTasksRestart:
   {
      JobContainerIt jobsIt( jobs);
      af::MCTasksPos taskspos( msg);
      JobAf* job = jobsIt.getJob( taskspos.getJobId());
      if( job != NULL) job->restartTasks( taskspos, renders, monitors);
      break;
   }
   case af::Msg::TTaskListenOutput:
   {
      af::MCListenAddress mclass( msg);
      JobContainerIt jobsIt( jobs);
      JobAf* job = jobsIt.getJob( mclass.getJobId());
      if( mclass.fromRender() == false ) mclass.setIP( msg->getAddress());
mclass.stdOut();
      if( job ) job->listenOutput( mclass, renders);
      break;
   }
   case af::Msg::TTaskUpdatePercent:
   case af::Msg::TTaskUpdateState:
   {
      af::MCTaskUp taskup( msg);
      jobs->updateTaskState( taskup, renders, monitors);
      break;
   }
   case af::Msg::TConfirm:
   {
      printf("Thread run: af::Msg::TConfirm: %d\n", msg->int32());
      break;
   }
   default:
   {
      AFERROR("ThreadRun::msgCase: message with unknown type recieved.\n");
      msg->stdOut();
   }
}
delete msg;
}
