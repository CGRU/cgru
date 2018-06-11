#include <stdio.h>
#include <stdlib.h>

#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgqueue.h"
#include "../libafanasy/renderupdate.h"

#include "afcommon.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::Msg * threadRunJSON( ThreadArgs * i_args, const af::Msg * i_msg);

af::Msg * threadRunCycleCase( ThreadArgs * i_args, af::Msg * i_msg)
{
	switch ( i_msg->type())
	{
	case af::Msg::THTTP:
	case af::Msg::TJSON:
	case af::Msg::TJSONBIN:
	{
		return threadRunJSON( i_args, i_msg);
	}
	case af::Msg::TMonitorDeregister:
	{
		MonitorContainerIt it( i_args->monitors);
		MonitorAf* node = it.getMonitor( i_msg->int32());
		if( node )
		{
			node->deregister();
			return af::jsonMsgInfo("log", "Deregistered.");
		}
		else
			return af::jsonMsgError("No monitor with provided ID found.");
	}
	case af::Msg::TRenderDeregister:
	{
		RenderContainerIt rendersIt( i_args->renders);
		RenderAf* render = rendersIt.getRender( i_msg->int32());
		if( render )
		{
			render->deregister( i_args->jobs, i_args->monitors);
			return af::jsonMsgInfo("log", "Deregistered.");
		}
		else
			return af::jsonMsgError("No render with provided ID found.");
	}
	case af::Msg::TConfirm:
	{
		std::string confirm = "af::Msg::TConfirm: ";
		confirm += af::itos( i_msg->int32());
		AFCommon::QueueLog( confirm);
		return af::jsonMsgInfo("log", confirm);
	}
	default:
	{
		std::string errlog = "Unknown message was received by run thread: ";
		errlog += i_msg->v_generateInfoString( false);
		AFCommon::QueueLogError( errlog);
		return af::jsonMsgError( errlog);
	}
	}

	return af::jsonMsgInfo("log","Message was processed by Run thread with no details.");
}

