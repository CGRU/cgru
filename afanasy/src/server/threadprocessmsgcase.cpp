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
#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/renderupdate.h"

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
#include "../libafanasy/logger.h"

af::Msg * threadProcessJSON( ThreadArgs * i_args, af::Msg * i_msg);

af::Msg* threadProcessMsgCase( ThreadArgs * i_args, af::Msg * i_msg)
{
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
		o_msg_response = af::jsonMsgInfo("error","TInvalid message received.");
		break;
	}
	case af::Msg::TNULL:
	case af::Msg::TDATA:
	case af::Msg::TTESTDATA:
	case af::Msg::TStringList:
	{
		i_msg->stdOutData();
		o_msg_response = af::jsonMsgInfo("log","Received data verbosed.");
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
		o_msg_response = af::jsonMsgInfo("log","Log appended.");

		if( str.empty()) break;
		AFCommon::QueueLog( str);

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
		AF_LOG << "Thread process message: Msg::TConfirm: " << i_msg->int32();
		return NULL;
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
		AfContainerLock jLock( i_args->jobs,     AfContainerLock::WRITELOCK);
		AfContainerLock mLock( i_args->monitors, AfContainerLock::WRITELOCK);
		AfContainerLock rLock( i_args->renders,  AfContainerLock::WRITELOCK);

		RenderAf * newRender = new RenderAf( i_msg);
		newRender->setAddressIP( i_msg->getAddress());
		o_msg_response = i_args->renders->addRender( newRender, i_args->jobs, i_args->monitors);
		break;
	}
	case af::Msg::TRenderUpdate:
	{
		af::RenderUpdate * rup = new af::RenderUpdate( i_msg);
		bool render_found = false;

		{
			AfContainerLock rlock( i_args->renders, AfContainerLock::WRITELOCK);

			RenderContainerIt rendersIt( i_args->renders);
			RenderAf * render = rendersIt.getRender( rup->getId());

			if( NULL == render)
			{
				// If there is not such online render, a zero id will be send.
				// It is a signal for client to register again (may be server was restarted).
				o_msg_response = new af::Msg( af::Msg::TRenderId, 0);
			}
			else if ( render->isOffline())
			{
				// The server has not received any registration message from
				// this render, but it is nevertheless running. The conclusion
				// to reach is that the render was still running while the
				// server have been restarted (maybe because it crashed).
				// We ask the render to send a list of the tasks it is in charge
				// of to reconnect it. It should then send a TRenderReconnect.
				o_msg_response = new af::Msg( af::Msg::TRenderId, 0);
			}
			else
			{
				o_msg_response = render->update( *rup);
				render_found = true;
			}
		}

		if( render_found)
		{
			// Task outputs received:
			if( rup->m_outputs.size())
			{
				AfContainerLock mlock( i_args->monitors, AfContainerLock::WRITELOCK);

				i_args->monitors->outputsReceived( rup->m_outspos, rup->m_outputs);
			}

			// To update tasks (if any) we push message to run thread:
			if( rup->m_taskups.size())
			{
				i_args->rupQueue->pushUp( rup);
				return o_msg_response;
			}
		}

		delete rup;

		break;
	}
	case af::Msg::TRendersResourcesRequestIds:
	{
	  AfContainerLock lock( i_args->renders, AfContainerLock::READLOCK);

	  af::MCGeneral ids( i_msg);
	  o_msg_response = i_args->renders->generateList( af::Msg::TRendersResources, ids);
	  break;
	}

// ------------------------------------- Job -------------------------------//
	case af::Msg::TJobsWeightRequest:
	{
	  AfContainerLock jLock( i_args->jobs,	 AfContainerLock::READLOCK);

	  af::MCJobsWeight jobsWeight;
	  i_args->jobs->getWeight( jobsWeight);
	  o_msg_response = new af::Msg( af::Msg::TJobsWeight, &jobsWeight);
	  break;
	}


	// Cases for run cycle thread:
	case af::Msg::TRenderDeregister:
	case af::Msg::TMonitorDeregister:
	{
		// This messages for  run cycle thread.
		return NULL;
	}


	// -------------------------------------------------------------------------//
	default:
	{
		AFCommon::QueueLogError( std::string("Unknown message recieved: ") + i_msg->v_generateInfoString( false));
		break;
	}
	}

	// Returning an answer
	return o_msg_response;
}
