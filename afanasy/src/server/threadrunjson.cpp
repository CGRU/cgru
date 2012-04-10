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

	JSON & jaction = document["action"];
	if( false == jaction.IsObject())
	{
		AFCommon::QueueLogError("JSON action is not an object.");
		delete [] data;
		return;
	}

	delete [] data;
}
