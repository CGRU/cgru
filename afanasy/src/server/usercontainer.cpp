#include "usercontainer.h"

#include <stdio.h>
#include <string.h>

#include "../include/afanasy.h"
#include "../libafanasy/msgqueue.h"

#include "../libafsql/dbconnection.h"

#include "action.h"
#include "afcommon.h"
#include "renderaf.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

UserContainer::UserContainer():
	AfContainer( "Users", AFUSER::MAXCOUNT)
{
	UserAf::setUserContainer( this);
}

UserContainer::~UserContainer()
{
AFINFO("UserContainer::~UserContainer:\n");
}

UserAf* UserContainer::addUser( const std::string & i_usernmae, const std::string & i_hostname, MonitorContainer * i_monitoring)
{
	{
		UserContainerIt usersIt( this);
		for(UserAf *user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
		{
			if( i_usernmae == user->getName())
			{
				if( user->getHostName() != i_hostname)
				{
					user->setHostName( i_hostname);
					if( i_monitoring) i_monitoring->addEvent( af::Msg::TMonitorUsersChanged, user->getId());
				}
				return user;
			}
		}
	}

	UserAf *user = new UserAf( i_usernmae, i_hostname);
	if( addUser(user) == 0)
	{
		AFERROR("UserContainer::addUser: Can't add user to container.\n");
		delete user;
		return NULL;
	}

	if( i_monitoring) i_monitoring->addEvent( af::Msg::TMonitorUsersAdd, user->getId());

	AFCommon::QueueLog("New job user registered: " + user->v_generateInfoString( false));
	return user;
}

UserAf * UserContainer::getUser( const std::string & i_name )
{
	UserContainerIt usersIt( this);
	for( UserAf * user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
	{
		if( user->getName() == i_name )
		return user;
	}
	return NULL;
}

void UserContainer::updateTimeActivity( const std::string & i_name)
{
	UserAf * user = getUser( i_name);
	if( user )
		user->updateTimeActivity();
}

void UserContainer::logAction( const Action & i_action, const std::string & i_node_name)
{
	UserAf * user = getUser( i_action.user_name);
	if( user )
		user->logAction( i_action, i_node_name);
}

int UserContainer::addUser( UserAf * i_user)
{
	// Add node to container:
	if( false == add( i_user))
		return 0;

	// Add user to solving list:
	m_userslist.add( i_user);

	// Initialize user:
	if( false == i_user->initialize())
		return 0;

	return i_user->getId();
}

af::Msg * UserContainer::addUser( UserAf * i_user, MonitorContainer * i_monitoring)
{
	UserContainerIt usersIt( this);
	for(UserAf *user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
	{
		if( user->getName() == i_user->getName())
		{
			AFERRAR("UserContainer::addUser: User \"%s\" already exists.", i_user->getName().c_str());
			delete i_user;
			std::ostringstream str;
			str << "{\"error\":\"exists\"";
			str << ",\n\"user\":\n";
			user->v_jsonWrite( str, /*type no matter*/ 0);
			str << "\n}";
			return af::jsonMsg( str);
		}
	}

	if( addUser( i_user) == 0)
	{
		AFERRAR("UserContainer::addUser: Can't add user \"%s\" node to container.", i_user->getName().c_str());
		delete i_user;
		return af::jsonMsgError("Unable to add node to container.");
	}

//AFCommon::QueueDBAddItem( i_user);
	i_user->store();
	if( i_monitoring) i_monitoring->addEvent( af::Msg::TMonitorUsersAdd, i_user->getId());

	AFCommon::QueueLog("User registered: " + i_user->v_generateInfoString( false));

	std::ostringstream str;
	str << "{\"user\":\n";
	i_user->v_jsonWrite( str, /*type no matter*/ 0);
	str << "\n}";
	return af::jsonMsg( str);
}

bool UserContainer::solve( RenderAf * i_render, MonitorContainer * i_monitoring)
{
//printf("\nUserContainer::genTask: render - %s\n", render->getName().c_str());
	return m_userslist.solve( af::Node::SolveByPriority, i_render, i_monitoring);
}

af::Msg* UserContainer::generateJobsList( int id)
{
	UserContainerIt usersIt( this);
	UserAf* user = usersIt.getUser( id);
	if( user == NULL) return NULL;

	MCAfNodes mcjobs;
	user->jobsinfo( mcjobs);

	return new af::Msg( af::Msg::TJobsList, &mcjobs);
}

af::Msg* UserContainer::generateJobsList( const std::vector<int32_t> & ids,
	const std::string & i_type_name, bool json)
{
	UserContainerIt usersIt( this);
	MCAfNodes mcjobs;
	std::ostringstream stream;
	bool has_jobs = false;

	if( json )
	{
		stream << "{\"" << i_type_name << "\":[\n";
	}

	for( int i = 0; i < ids.size(); i++)
	{
		UserAf* user = usersIt.getUser( ids[i]);
		if( user == NULL) continue;
		if( json )
		{
			if(( i != 0 ) && ( has_jobs ))
				stream << ",\n";
			has_jobs = user->getJobs( stream);
		}
		else
			user->jobsinfo( mcjobs);
	}

	if( json )
	{
		stream << "\n]}";
		return af::jsonMsg( stream);
	}

	af::Msg * msg = new af::Msg();
	msg->set( af::Msg::TJobsList, &mcjobs);
	return msg;
}

//############################################################################
//                               UserContainerIt
//############################################################################

UserContainerIt::UserContainerIt( UserContainer* userslist, bool skipZombies):
	AfContainerIt( (AfContainer*)userslist, skipZombies)
{
}

UserContainerIt::~UserContainerIt()
{
}
