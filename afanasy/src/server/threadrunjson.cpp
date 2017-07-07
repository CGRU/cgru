#include "action.h"
#include "afcommon.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::Msg * threadRunJSON( ThreadArgs * i_args, const af::Msg * i_msg)
{
	Action action( i_msg, i_args);
	if( action.isInvalid())
		return af::jsonMsgError("Invalid action.");

	if( action.type == "monitors")
		return i_args->monitors->action( action);

	if( action.type == "jobs")
		return i_args->jobs->action( action);

	if( action.type == "renders")
		return i_args->renders->action( action);

	if( action.type == "users")
		return i_args->users->action( action);

	return af::jsonMsgError(std::string("Action has unknown type: '") + action.type + "'");
}

