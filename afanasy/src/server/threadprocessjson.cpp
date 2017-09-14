#include "afcommon.h"
#include "jobcontainer.h"
#include "monitoraf.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#include "../libafanasy/farm.h"
#include "../libafanasy/msgclasses/mctask.h"
#include "../libafanasy/rapidjson/stringbuffer.h"
#include "../libafanasy/rapidjson/prettywriter.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::Msg * jsonSaveObject( rapidjson::Document & i_obj);

af::Msg * threadProcessJSON( ThreadArgs * i_args, af::Msg * i_msg)
{
	rapidjson::Document document;
	std::string error;
	char * data = af::jsonParseMsg( document, i_msg, &error);
	if( data == NULL )
	{
		AFCommon::QueueLogError( error);
		delete i_msg;
		return af::jsonMsgError(error);
	}

	af::Msg * o_msg_response = NULL;

	JSON & getObj = document["get"];
	if( getObj.IsObject())
	{
		std::string type, mode;
		bool binary = false;
		af::jr_string("type", type, getObj);
		af::jr_string("mode", mode, getObj);
		af::jr_bool("binary", binary, getObj);

		bool json = true;
		if( binary )
			json = false;
		bool full = false;
		if( mode == "full")
			full = true;

		std::vector<int32_t> ids;
		af::jr_int32vec("ids", ids, getObj);

		std::string mask;
		af::jr_string("mask", mask, getObj);

		if( type == "jobs" )
		{
			if( getObj.HasMember("uids"))
			{
				std::vector<int32_t> uids;
				af::jr_int32vec("uids", uids, getObj);
				if( uids.size())
				{
					AfContainerLock jLock( i_args->jobs,  AfContainerLock::READLOCK);
					AfContainerLock uLock( i_args->users, AfContainerLock::READLOCK);
					o_msg_response = i_args->users->generateJobsList( uids, type, json);
				}
			}
			if( getObj.HasMember("users"))
			{
				std::vector<std::string> users;
				af::jr_stringvec("users", users, getObj);
				if( users.size())
				{
					AfContainerLock jLock( i_args->jobs,  AfContainerLock::READLOCK);
					AfContainerLock uLock( i_args->users, AfContainerLock::READLOCK);
					o_msg_response = i_args->users->generateJobsList( users, type, json);
				}
			}
			else if( mode == "output")
			{
				std::vector<int32_t> block_ids;
				std::vector<int32_t> task_ids;
				int number = 0;
				int mon_id = 0;

				af::jr_int32vec("block_ids", block_ids, getObj);
				af::jr_int32vec("task_ids", task_ids, getObj);
				af::jr_int("number", number, getObj);

				// This request can be from afcmd, for example
				bool has_monitor = af::jr_int("mon_id", mon_id, getObj);

				if(( ids.size() == 1 ) && ( block_ids.size() == 1 ) && ( task_ids.size() == 1 ))
				{
					af::MCTask mctask( ids[0], block_ids[0], task_ids[0], number);
					std::string error;

					// Get output from job, it can return a request message for render or a filename
					{
						AfContainerLock jlock( i_args->jobs,    AfContainerLock::READLOCK);

						JobContainerIt it( i_args->jobs);
						JobAf * job = it.getJob( ids[0]);
						if( job == NULL )
							error = "Invalid job ID";
						else
							job->v_getTaskOutput( mctask, error);
					}

					if( mctask.hasOutput()) // Reading output from file
					{
						int readsize = -1;
						char * data = af::fileRead( mctask.getOutput(), &readsize, af::Msg::SizeDataMax, &error);
						if( data )
						{
							mctask.updateOutput( std::string( data, readsize));
							o_msg_response = mctask.generateMessage( binary);

							delete [] data;
						}
					}
					else if( mctask.m_render_id && has_monitor ) // Retrieving output from render
					{
						{
							AfContainerLock rLock( i_args->renders, AfContainerLock::WRITELOCK);

							RenderContainerIt rendersIt( i_args->renders);
							RenderAf * render = rendersIt.getRender( mctask.m_render_id);
							if( render )
							{
								render->addTaskOutput( af::MCTaskPos( ids[0], block_ids[0], task_ids[0]));
							}
							else
							{
								error = std::string("Render not founded with ID = ")
									+ af::itos( mctask.m_render_id);
								mctask.m_render_id = 0;
							}
						}

						// If render founded, set monitor to wait output:
						if( mctask.m_render_id )
						{					
							AfContainerLock mLock( i_args->monitors, AfContainerLock::WRITELOCK);

							MonitorContainerIt it( i_args->monitors);
							MonitorAf * monitor = it.getMonitor( mon_id);
							if( monitor )
							{
								monitor->waitOutput( mctask);
								std::string info = "Retrieving running task output from render...";
								mctask.setOutput( info);
								o_msg_response = mctask.generateMessage( binary);
							}
							else
								error = std::string("MonitorContainer::waitOutput: No monitor with ID = ")
									+ af::itos( mon_id);
						}
					}
					else if( mctask.m_render_id && ( false == has_monitor ))
					{
						error = std::string("Can't get ouput of a running task.");
					}
	
					if( o_msg_response == NULL )
					{
						if( error.empty())
							error = "Failed to get task ouput...\nSee server logs for details.";

						mctask.setOutput( std::string("ERROR: ") + error);

						o_msg_response = mctask.generateMessage( binary);
					}
				}
			}
			else
			{
				AfContainerLock lock( i_args->jobs, AfContainerLock::READLOCK);
				JobAf * job = NULL;
				if( ids.size() == 1 )
				{
					JobContainerIt it( i_args->jobs);
					job = it.getJob( ids[0]);
					if( job == NULL )
						o_msg_response = af::jsonMsgError( "Invalid ID");
				}

				if( job )
				{
					std::vector<int32_t> block_ids;
					af::jr_int32vec("block_ids", block_ids, getObj);
					if( block_ids.size() && ( block_ids[0] != -1 ))
					{
						std::vector<int32_t> task_ids;
						af::jr_int32vec("task_ids", task_ids, getObj);
						if( task_ids.size() && ( task_ids[0] != -1))
							o_msg_response = job->writeTask( block_ids[0], task_ids[0], mode, binary);
						else
						{
							std::vector<std::string> modes;
							af::jr_stringvec("mode", modes, getObj);
							o_msg_response = job->writeBlocks( block_ids, modes, binary);
						}
					}
					else if( mode.size())
					{
						if( mode == "thumbnail" )
							o_msg_response = job->writeThumbnail( binary);
						else if( mode == "progress" )
							o_msg_response = job->writeProgress( json);
						else if( mode == "error_hosts" )
							o_msg_response = job->writeErrorHosts( binary);
						else if( mode == "log" )
							o_msg_response = job->writeLog( binary);
					}
				}

				if( o_msg_response == NULL )
				{
					std::vector<int64_t> serials;
					if( af::jr_int64vec("serials", serials, getObj))
					{
						ids = i_args->jobs->getIdsBySerials( serials);
					}

					o_msg_response = i_args->jobs->generateList(
						full ? af::Msg::TJob : af::Msg::TJobsList, type, ids, mask, json);
				}
			}
		}
		else if( type == "renders")
		{
			AfContainerLock lock( i_args->renders, AfContainerLock::READLOCK);
			if( mode.size())
			{
				RenderAf * render = NULL;
				if( ids.size() == 1 )
				{
					RenderContainerIt it( i_args->renders);
					render = it.getRender( ids[0]);
					if( render == NULL )
						o_msg_response = af::jsonMsgError( "Invalid ID");
				}
				if( render )
				{
					if( full )
						o_msg_response = render->writeFullInfo( binary);
					else if( mode == "log" )
						o_msg_response = render->writeLog( binary);
					else if( mode == "tasks_log" )
						o_msg_response = render->writeTasksLog( binary);
				}
			}
			if( o_msg_response == NULL )
			{
				if( mode == "resources" )
					o_msg_response = i_args->renders->generateList( af::Msg::TRendersResources, type, ids, mask, json);
				else
					o_msg_response = i_args->renders->generateList( af::Msg::TRendersList, type, ids, mask, json);
			}
		}
		else if( type == "users")
		{
			AfContainerLock lock( i_args->users, AfContainerLock::READLOCK);
			if( mode.size())
			{
				UserAf * user = NULL;
				if( ids.size() == 1 )
				{
					UserContainerIt it( i_args->users);
					user = it.getUser( ids[0]);
					if( user == NULL )
						o_msg_response = af::jsonMsgError( "Invalid ID");
				}
				if( user )
				{
					if( mode == "jobs_order" )
						o_msg_response = user->writeJobdsOrder( binary);
					else if( mode == "log" )
						o_msg_response = user->writeLog( binary);
				}
			}
			if( o_msg_response == NULL )
				o_msg_response = i_args->users->generateList( af::Msg::TUsersList, type, ids, mask, json);
		}
		else if( type == "monitors")
		{
			AfContainerLock lock( i_args->monitors, AfContainerLock::READLOCK);
			if( mode.size())
			{
				MonitorAf * monitor = NULL;
				if( ids.size() == 1 )
				{
					MonitorContainerIt it( i_args->monitors);
					monitor = it.getMonitor( ids[0]);
					if( NULL == monitor )
						o_msg_response = af::jsonMsg("{\"monitor\":{\"id\":0}}");
				}
				if( monitor )
				{
					if( mode == "events")
						o_msg_response = monitor->getEventsJSON();
					else if( mode == "log")
						o_msg_response = monitor->writeLog( binary);
				}
			}
			if( NULL == o_msg_response )
				o_msg_response = i_args->monitors->generateList( af::Msg::TMonitorsList, type, ids, mask, json);
		}
		else if( type == "files")
		{
			std::string path;
			std::ostringstream files;
			af::jr_string("path", path, getObj);
			std::vector<std::string> list = af::getFilesListSafe( path);
			files << "{\"path\":\"" << path << "\",\n";
			files << "\"files\":[";
			for( int i = 0; i < list.size(); i++)
			{
				if( i )
					files << ',';
				files << '"' << list[i] << '"';
			}
			files << "]}";
			o_msg_response = af::jsonMsg( files);
		}
		else if( type == "config" )
		{
			o_msg_response = af::jsonMsg( af::Environment::getConfigData());
		}
		else if( type == "farm" )
		{
			o_msg_response = af::jsonMsg( af::farm()->getText());
		}
		else if( type == "services_limits" )
		{
			o_msg_response = af::jsonMsg( af::farm()->jsonWriteLimits() );
		}
		else
		{
			o_msg_response = af::jsonMsgError(std::string("Invalid get type = '") + type + "'");
		}
	}
	else if( document.HasMember("action"))
	{
		// This message for Run thread:
		delete [] data;
		return NULL;
	}
	else if( document.HasMember("job"))
	{
		if( af::Environment::isDemoMode() )
		{
			std::string errlog = "Job registration is not allowed: Server demo mode.";
			AFCommon::QueueLogError( errlog);
			o_msg_response = af::jsonMsgError( errlog);
		}
		else
		{
			// No containers locks needed here.
			// Job registration is a complex procedure.
			// It locks and unlocks needed containers itself.
			o_msg_response = i_args->jobs->registerJob( document["job"], i_args->users, i_args->monitors);
		}
	}
	else if( document.HasMember("monitor"))
	{
		bool binary = false;
		af::jr_bool("binary", binary, document["monitor"]);

		AfContainerLock mlock( i_args->monitors, AfContainerLock::WRITELOCK);
		AfContainerLock ulock( i_args->users,    AfContainerLock::READLOCK);
		MonitorAf * newMonitor = new MonitorAf( document["monitor"], i_args->users);
		newMonitor->setAddressIP( i_msg->getAddress());
		o_msg_response = i_args->monitors->addMonitor( newMonitor, binary);
	}
	else if( document.HasMember("user"))
	{
		AfContainerLock ulock( i_args->users, AfContainerLock::WRITELOCK);
		o_msg_response = i_args->users->addUser( new UserAf( document["user"]), i_args->monitors);
	}
	else if( document.HasMember("reload_farm"))
	{
		AfContainerLock mLock( i_args->monitors, AfContainerLock::WRITELOCK);
		AfContainerLock rlock( i_args->renders,  AfContainerLock::WRITELOCK);

		std::string status;
		bool success = i_args->renders->farmLoad( status, i_args->monitors);
		o_msg_response = af::jsonMsgStatus( success,"reload_farm", status);
	}
	else if( document.HasMember("reload_config"))
	{
		AfContainerLock jlock( i_args->jobs,	AfContainerLock::WRITELOCK);
		AfContainerLock rlock( i_args->renders, AfContainerLock::WRITELOCK);
		AfContainerLock ulock( i_args->users,	AfContainerLock::WRITELOCK);
		printf("\n	========= RELOADING CONFIG =========\n\n");
		std::string message;
		if( af::Environment::reload())
		{
			printf("\n	========= CONFIG RELOADED SUCCESSFULLY =========\n\n");
			o_msg_response = af::jsonMsgStatus( true, "reload_config",
				"Reloaded successfully.");
		}
		else
		{
			printf("\n	========= CONFIG RELOADING FAILED =========\n\n");
			o_msg_response = af::jsonMsgStatus( false, "reload_config",
				"Failed, see server logs fo details.");
		}
	}
	else if( document.HasMember("announce"))
	{
		std::string text;
		af::jr_string("announce", text, document);
		if( text.size())
		{
			i_args->monitors->announce( text);
			o_msg_response = af::jsonMsgStatus( true,"announce","OK");
		}
	}
	else if( document.HasMember("save"))
	{
		o_msg_response = jsonSaveObject( document);
	}

	delete [] data;

	return o_msg_response;
}

af::Msg * jsonSaveObject( rapidjson::Document & i_obj)
{
	JSON & jSave = i_obj["save"];
	if( false == jSave.IsObject())
		return af::jsonMsgError("\"save\" is not an object.");

	JSON & jPath = jSave["path"];
	if( false == jPath.IsString())
		return af::jsonMsgError("\"path\" is not an string.");

	JSON & jObj = jSave["object"];
	if( false == jObj.IsObject())
		return af::jsonMsgError("\"object\" is not an object.");

	std::string path((char*)jPath.GetString());
	while(( path[0] == '/' ) || ( path[0] == '/' ) || ( path[0] == '.'))
		path = path.substr(1);
	path = af::Environment::getCGRULocation() + '/' + path + ".json";

	std::string info;
	info = std::string("Created by afserver at ") + af::time2str();
	jObj.AddMember("__cgru__", info.c_str(), i_obj.GetAllocator());

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	writer.SetIndent('\t',1);
	jObj.Accept(writer);
	std::string text( buffer.GetString());

	std::ostringstream str;
	str << "{\"save\":\n";
	str << "\t\"path\":\"" << path << "\",\n";
	if( af::pathFileExists( path)) str << "\t\"overwrite\":true,\n";
	str << "\t\"size\":" << text.size() << "\n";
	str << "}}";

	if( false == AFCommon::writeFile( text, path))
		return af::jsonMsgError( std::string("Unable to write to \"") + path + "\", see server log for details.");

	return af::jsonMsg( str);
}

