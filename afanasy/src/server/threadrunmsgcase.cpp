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
	af::Msg * o_msg_response = NULL;

	switch ( i_msg->type())
	{
	case af::Msg::THTTP:
	case af::Msg::TJSON:
	case af::Msg::TJSONBIN:
	{
		o_msg_response = threadRunJSON( i_args, i_msg);
		break;
	}
	case af::Msg::TMonitorDeregister:
	{
		MonitorContainerIt it( i_args->monitors);
		MonitorAf* node = it.getMonitor( i_msg->int32());
		if( node ) node->deregister();
		break;
	}
	case af::Msg::TRenderDeregister:
	{
		RenderContainerIt rendersIt( i_args->renders);
		RenderAf* render = rendersIt.getRender( i_msg->int32());
		if( render != NULL) render->deregister( i_args->jobs, i_args->monitors);
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

	if( o_msg_response == NULL )
		o_msg_response = af::jsonMsgInfo("log","Message was processed by Run thread with no answer.");

	return o_msg_response;
}

