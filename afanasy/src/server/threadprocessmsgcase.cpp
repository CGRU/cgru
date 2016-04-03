#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/farm.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgqueue.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msgclasses/mcjobsweight.h"

#include "afcommon.h"
#include "jobcontainer.h"
#include "monitoraf.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::Msg * threadProcessJSON( ThreadArgs * i_args, af::Msg * i_msg);

af::Msg* threadProcessMsgCase( ThreadArgs * i_args, af::Msg * i_msg)
{
	//i_msg->stdOut();
	af::Msg * o_msg_response = NULL;

	switch( i_msg->type())
	{
	case af::Msg::TVersionMismatch:
	{
		AFCommon::QueueLogError( i_msg->v_generateInfoString( false));
		o_msg_response = new af::Msg( af::Msg::TVersionMismatch, 1);
		break;
	}
	case af::Msg::TInvalid:
	{
		AFCommon::QueueLogError( std::string("Invalid message recieved: ") + i_msg->v_generateInfoString( false));
		break;
	}
	case af::Msg::TNULL:
	case af::Msg::TDATA:
	case af::Msg::TTESTDATA:
	case af::Msg::TStringList:
	{
		i_msg->stdOutData();
		break;
	}
	case af::Msg::THTTP:
	case af::Msg::TJSON:
	case af::Msg::TJSONBIN:
	{
		return threadProcessJSON( i_args, i_msg);
	}
	case af::Msg::TString:
	{
		std::string str = i_msg->getString();
		if( str.empty()) break;

		AFCommon::QueueLog( str);
//		AfContainerLock mLock( i_args->monitors, AfContainerLock::WRITELOCK);
//		i_args->monitors->sendMessage( str);
		break;
	}
	case af::Msg::TStatRequest:
	{
		o_msg_response = new af::Msg;
		af::statwrite( o_msg_response);
		break;
	}
	case af::Msg::TConfirm:
	{
		printf("Thread process message: Msg::TConfirm: %d\n", i_msg->int32());
		i_args->msgQueue->pushMsg( new af::Msg( af::Msg::TConfirm, 1));
		o_msg_response = new af::Msg( af::Msg::TConfirm, 1 - i_msg->int32());
		break;
	}

// ---------------------------------- Monitor ---------------------------------//
	case af::Msg::TMonitorUpdateId:
	{
	  AfContainerLock lock( i_args->monitors, AfContainerLock::READLOCK);
		MonitorContainerIt it( i_args->monitors);
		MonitorAf * node = it.getMonitor( i_msg->int32());

		if( node )
			o_msg_response = node->getEventsBin();
		else
			o_msg_response = new af::Msg( af::Msg::TMonitorId, 0);

	  break;
	}

// ---------------------------------- Render -------------------------------//
	case af::Msg::TRenderRegister:
	{
//printf("case af::Msg::TRenderRegister:\n");
		AfContainerLock mLock( i_args->monitors, AfContainerLock::WRITELOCK);
		AfContainerLock rLock( i_args->renders,  AfContainerLock::WRITELOCK);

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
	case af::Msg::TRendersResourcesRequestIds:
	{
	  AfContainerLock lock( i_args->renders, AfContainerLock::READLOCK);

	  af::MCGeneral ids( i_msg);
	  o_msg_response = i_args->renders->generateList( af::Msg::TRendersResources, ids);
	  break;
	}

// ---------------------------------- Users -------------------------------//
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
		ids.setId( user->getId());
		ids.setList( user->generateJobsIds());
	  o_msg_response = new af::Msg( af::Msg::TUserJobsOrder, &ids);
	  break;
	}

// ------------------------------------- Job -------------------------------//
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
	case af::Msg::TJobsWeightRequest:
	{
	  AfContainerLock jLock( i_args->jobs,	 AfContainerLock::READLOCK);

	  af::MCJobsWeight jobsWeight;
	  i_args->jobs->getWeight( jobsWeight);
	  o_msg_response = new af::Msg( af::Msg::TJobsWeight, &jobsWeight);
	  break;
	}
	// Cases for run cycle thread:
	case af::Msg::TTaskUpdateState:
	{
	  af::MCTaskUp taskup( i_msg);
	  af::MCTaskPos taskpos( taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumber());
	  o_msg_response = new af::Msg( af::Msg::TRenderCloseTask, &taskpos);
	}
	case af::Msg::TTaskUpdatePercent:
	case af::Msg::TRenderDeregister:
	case af::Msg::TMonitorDeregister:
	{
		// Push message for run cycle thread.
		i_args->msgQueue->pushMsg( i_msg);
		// Need to return here to not to delete input message (i_msg) later.
		return o_msg_response;
		//  ( o_msg_response is NULL in all cases except Msg::TTaskUpdateState,
		//	 in that case render should recieve an answer to close task
		//	 and finish sending any updates for the task )
	}
	// -------------------------------------------------------------------------//
	default:
	{
		AFCommon::QueueLogError( std::string("Unknown message recieved: ") + i_msg->v_generateInfoString( false));
		break;
	}
	}

	// Deleting input message as it not needed any more.
	delete i_msg;

	// Returning an answer
	return o_msg_response;
}
