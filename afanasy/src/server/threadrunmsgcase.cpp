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
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void threadRunJSON( ThreadArgs * i_args, const af::Msg * i_msg);

void threadRunCycleCase( ThreadArgs * i_args, af::Msg * i_msg)
{
switch ( i_msg->type())
{
	case af::Msg::THTTP:
	case af::Msg::TJSON:
	case af::Msg::TJSONBIN:
	{
		threadRunJSON( i_args, i_msg);
		break;
	}
	case af::Msg::TMonitorDeregister:
	{
		MonitorContainerIt it( i_args->monitors);
		MonitorAf* node = it.getMonitor( i_msg->int32());
		if( node ) node->deregister();
		break;
	}
/*	case af::Msg::TMonitorMessage:
	{
		af::MCGeneral mcgeneral( i_msg);
		i_args->monitors->sendMessage( mcgeneral);
		break;
	}*/
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
	case af::Msg::TRenderDeregister:
	{
		RenderContainerIt rendersIt( i_args->renders);
		RenderAf* render = rendersIt.getRender( i_msg->int32());
		if( render != NULL) render->deregister( i_args->jobs, i_args->monitors);
		break;
	}
	case af::Msg::TTaskListenOutput:
	{
		af::MCListenAddress mclass( i_msg);
		JobContainerIt jobsIt( i_args->jobs);
		JobAf* job = jobsIt.getJob( mclass.getJobId());
		if( mclass.fromRender() == false ) mclass.setIP( i_msg->getAddress());
		//mclass.v_stdOut();
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
		AFCommon::QueueLogError( std::string("Run: Unknown message recieved: ") + i_msg->v_generateInfoString( false));
		break;
	}
}
delete i_msg;
}
