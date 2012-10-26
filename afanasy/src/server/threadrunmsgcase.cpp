#include <stdio.h>
#include <stdlib.h>

#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"
#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/msgclasses/mctaskspos.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgqueue.h"

#include "afcommon.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "talkcontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void threadRunJSON( ThreadArgs * i_args, af::Msg * i_msg);

void threadRunCycleCase( ThreadArgs * i_args, af::Msg * i_msg)
{
switch ( i_msg->type())
{
	case af::Msg::THTTP:
	case af::Msg::TJSON:
	{
		threadRunJSON( i_args, i_msg);
		break;
	}
    case af::Msg::TTalkExit:
    {
        af::MCGeneral mcgeneral( i_msg);
        i_args->talks->action( mcgeneral, i_msg->type(), NULL, i_args->monitors);
        break;
    }
    case af::Msg::TTalkDeregister:
    {
        if( i_args->talks->setZombie( i_msg->int32())) i_args->monitors->addEvent( af::Msg::TMonitorTalksDel, i_msg->int32());
        break;
    }
    case af::Msg::TMonitorExit:
    {
        af::MCGeneral mcgeneral( i_msg);
        i_args->monitors->action( mcgeneral, i_msg->type(), NULL, NULL);
        break;
    }
    case af::Msg::TMonitorDeregister:
    {
        if( i_args->monitors->setZombie( i_msg->int32())) i_args->monitors->addEvent( af::Msg::TMonitorMonitorsDel, i_msg->int32());
        break;
    }
    case af::Msg::TMonitorMessage:
    {
        af::MCGeneral mcgeneral( i_msg);
        i_args->monitors->sendMessage( mcgeneral);
        break;
    }
    case af::Msg::TMonitorSubscribe:
    case af::Msg::TMonitorUnsubscribe:
    case af::Msg::TMonitorUsersJobs:
    case af::Msg::TMonitorJobsIdsAdd:
    case af::Msg::TMonitorJobsIdsSet:
    case af::Msg::TMonitorJobsIdsDel:
    {
        af::MCGeneral ids( i_msg);
        i_args->monitors->setInterest( i_msg->type(), ids);
        break;
    }
    case af::Msg::TRenderAnnotate:
    case af::Msg::TRenderSetPriority:
    case af::Msg::TRenderSetCapacity:
    case af::Msg::TRenderSetMaxTasks:
    case af::Msg::TRenderSetService:
    case af::Msg::TRenderRestoreDefaults:
    case af::Msg::TRenderSetUser:
    case af::Msg::TRenderSetNIMBY:
    case af::Msg::TRenderSetNimby:
    case af::Msg::TRenderSetFree:
    case af::Msg::TRenderEjectTasks:
    case af::Msg::TRenderEjectNotMyTasks:
    case af::Msg::TRenderDelete:
    case af::Msg::TRenderRestart:
    case af::Msg::TRenderReboot:
    case af::Msg::TRenderShutdown:
    case af::Msg::TRenderExit:
    case af::Msg::TRenderWOLSleep:
    case af::Msg::TRenderWOLWake:
	case af::Msg::TRenderHideShow:
    {
        af::MCGeneral mcgeneral( i_msg);
        i_args->renders->action( mcgeneral, i_msg->type(), i_args->jobs, i_args->monitors);
        break;
    }
    case af::Msg::TRenderDeregister:
    {
        RenderContainerIt rendersIt( i_args->renders);
        RenderAf* render = rendersIt.getRender( i_msg->int32());
        if( render != NULL) render->deregister( i_args->jobs, i_args->monitors);
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
    case af::Msg::TUserJobsSolveMethod:
    {
        af::MCGeneral mcgeneral( i_msg);
        i_args->users->action( mcgeneral, i_msg->type(), NULL, i_args->monitors);
        break;
    }
    case af::Msg::TUserMoveJobsUp:
    case af::Msg::TUserMoveJobsDown:
    case af::Msg::TUserMoveJobsTop:
    case af::Msg::TUserMoveJobsBottom:
    {
        UserContainerIt usersIt( i_args->users);
        af::MCGeneral mcgeneral( i_msg);
        UserAf* user = usersIt.getUser( mcgeneral.getNumber());
        user->moveJobs( mcgeneral, i_msg->type());
        i_args->monitors->addUser( user);
        break;
    }
    case af::Msg::TUserDel:
    case af::Msg::TUserAdd:
    {
        af::MCGeneral mcgeneral( i_msg);
        // If existing users IDs provided, it's simple action to change users permanent property
        if( mcgeneral.getCount()) i_args->users->action( mcgeneral, i_msg->type(), NULL, i_args->monitors);
        // If user with specified name does not exisit, new user must be created and put in container
        else i_args->users->setPermanent( mcgeneral, (i_msg->type()==af::Msg::TUserAdd)?true:false, i_args->monitors);
        break;
    }
    case af::Msg::TJobAnnotate:
    case af::Msg::TJobSetUser:
    case af::Msg::TJobHostsMask:
    case af::Msg::TJobHostsMaskExclude:
    case af::Msg::TJobDependMask:
    case af::Msg::TJobDependMaskGlobal:
    case af::Msg::TJobMaxRunningTasks:
    case af::Msg::TJobMaxRunTasksPerHost:
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
	case af::Msg::TJobHideShow:
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
    case af::Msg::TBlockNonSequential:
    {
        af::MCGeneral mcgeneral( i_msg);
        i_args->jobs->action( mcgeneral, i_msg->type(), i_args->renders, i_args->monitors);
        break;
    }
    case af::Msg::TTasksSkip:
    {
        JobContainerIt jobsIt( i_args->jobs);
        af::MCTasksPos taskspos( i_msg);
        JobAf* job = jobsIt.getJob( taskspos.getJobId());
        if( job != NULL) job->skipTasks( taskspos, i_args->renders, i_args->monitors);
        break;
    }
    case af::Msg::TTasksRestart:
    {
        JobContainerIt jobsIt( i_args->jobs);
        af::MCTasksPos taskspos( i_msg);
        JobAf* job = jobsIt.getJob( taskspos.getJobId());
        if( job != NULL) job->restartTasks( taskspos, i_args->renders, i_args->monitors);
        break;
    }
    case af::Msg::TTaskListenOutput:
    {
        af::MCListenAddress mclass( i_msg);
        JobContainerIt jobsIt( i_args->jobs);
        JobAf* job = jobsIt.getJob( mclass.getJobId());
        if( mclass.fromRender() == false ) mclass.setIP( i_msg->getAddress());
mclass.stdOut();
        if( job ) job->listenOutput( mclass, i_args->renders);
        break;
    }
    case af::Msg::TTaskUpdatePercent:
    case af::Msg::TTaskUpdateState:
    {
        af::MCTaskUp taskup( i_msg);
        i_args->jobs->updateTaskState( taskup, i_args->renders, i_args->monitors);
        break;
    }
    case af::Msg::TConfirm:
    {
        AFCommon::QueueLog( std::string("af::Msg::TConfirm: ") + af::itos( i_msg->int32()));
        break;
    }
    default:
    {
        AFCommon::QueueLogError( std::string("Run: Unknown message recieved: ") + i_msg->generateInfoString( false));
        break;
    }
}
delete i_msg;
}
