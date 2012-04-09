#include "afcommon.h"
#include "jobcontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::Msg * threadProcessJSON( ThreadArgs * i_args, af::Msg * i_msg)
{
	int datalen = i_msg->dataLen();
	char * data = new char[datalen+1];
	memcpy( data, i_msg->data(), datalen);
	data[datalen] = '\0';

	rapidjson::Document document;
	if (document.ParseInsitu<0>(data).HasParseError())
	{
		std::string error = "JSON: Parsing failed at character " + af::itos( int( document.GetErrorOffset()));
		error += ":\n";
		error += document.GetParseError();
		AFCommon::QueueLogError( error);
		delete [] data;
		return NULL;
	}

	if( false == document.IsObject())
	{
		std::string error = "JSON: Can't find root object.";
		AFCommon::QueueLogError( error);
		delete [] data;
		return NULL;
	}
	
    af::Msg * o_msg_response = NULL;

	if( document.HasMember("job"))
	{
		// No containers locks needed here.
		// Job registration is a complex procedure.
		// It locks and unlocks needed containers itself.
		i_args->jobs->job_register( new JobAf( document["job"]), i_args->users, i_args->monitors);
	}

	JSON & getObj = document["get"];
	if( getObj.IsObject())
	{
		std::string type;
		af::Af::jr_string("type", type, getObj);

		std::vector<int32_t> ids;
		af::Af::jr_int32vec("ids", ids, getObj);

		std::string mask;
		af::Af::jr_string("mask", mask, getObj);

		if( type == "jobs" )
		{
			std::vector<int32_t> uids;
			af::Af::jr_int32vec("uids", uids, getObj);
			if( uids.size())
			{
				AfContainerLock jLock( i_args->jobs,  AfContainerLock::READLOCK);
				AfContainerLock uLock( i_args->users, AfContainerLock::READLOCK);
				o_msg_response = i_args->users->generateJobsList( uids, true);
			}
			else
			{
				AfContainerLock lock( i_args->jobs, AfContainerLock::READLOCK);
				o_msg_response = i_args->jobs->generateList( af::Msg::TJobsList, ids, mask, true);
			}
		}
	}

	delete [] data;

	return o_msg_response;
}
