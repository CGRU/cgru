#include "afcommon.h"
#include "jobcontainer.h"
#include "rendercontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void threadRunJSON( ThreadArgs * i_args, af::Msg * i_msg)
{
	rapidjson::Document document;
	std::string error;
	char * data = af::jsonParseMsg( document, i_msg, &error);
	if( data == NULL )
	{
		AFCommon::QueueLogError( error);
		return;
	}

	JSON & action = document["action"];
	if( false == action.IsObject())
	{
		AFCommon::QueueLogError("JSON action is not an object.");
		delete [] data;
		return;
	}

	if( action.HasMember("jobs"))
		i_args->jobs->action( action, i_args->renders, i_args->monitors);
	else if( action.HasMember("renders"))
		i_args->renders->action( action, i_args->jobs, i_args->monitors);
	else if( action.HasMember("users"))
		i_args->users->action( action, NULL, i_args->monitors);

	delete [] data;
}
