#include "action.h"
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

void threadRunJSON( ThreadArgs * i_args, af::Msg * i_msg)
{
//write(1,i_msg->data(),i_msg->dataLen());write(1,"\n",1);
	Action action( i_msg, i_args);
	if( action.isInvalid())
		return;

	if(( action.permissions & Action::PReadOnly ) && ( action.type != "monitors"))
	{
		AFCommon::QueueLogError( std::string("Action has \"readonly\" permissions only. Type = \"" + action.type + "\""));
		return;
	}

	if( action.type == "monitors")
		i_args->monitors->action( action);
	else if( action.type == "jobs")
		i_args->jobs->action( action);
	else if( action.type == "renders")
		i_args->renders->action( action);
	else if( action.type == "users")
		i_args->users->action( action);
	else if( action.type == "talks")
		i_args->talks->action( action);
	else
		AFCommon::QueueLogError(std::string("JSON action has unknown type - \"") + action.type + "\"");
}
