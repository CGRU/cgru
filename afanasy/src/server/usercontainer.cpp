#include "usercontainer.h"

#include <stdio.h>
#include <string.h>

#include "../include/afanasy.h"
#include "../libafanasy/msgqueue.h"

#include "../libafsql/dbconnection.h"

#include "afcommon.h"
#include "renderaf.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

UserContainer * UserContainer::ms_users = NULL;

UserContainer::UserContainer():
   AfContainer( "Users", AFUSER::MAXCOUNT)
{
    ms_users = this;
}

UserContainer::~UserContainer()
{
AFINFO("UserContainer::~UserContainer:\n");
}

UserAf* UserContainer::addUser( const std::string & username, const std::string & hostname, MonitorContainer * monitoring)
{
   {
      UserContainerIt usersIt( this);
      for(UserAf *user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
      {
         if( username == user->getName())
         {
            if( user->getHostName() != hostname)
            {
               user->setHostName( hostname);
               if( monitoring) monitoring->addEvent( af::Msg::TMonitorUsersChanged, user->getId());
            }
            return user;
         }
      }
   }

   UserAf *user = new UserAf( username, hostname);
   if( addUser(user) == 0)
   {
      AFERROR("UserContainer::addUser: Can't add user to container.\n");
      delete user;
      return NULL;
   }

   if( monitoring) monitoring->addEvent( af::Msg::TMonitorUsersAdd, user->getId());

   AFCommon::QueueLog("User registered: " + user->v_generateInfoString( false));
   return user;
}

UserAf * UserContainer::getUser( const std::string & i_name )
{
    UserContainerIt usersIt( ms_users);
    for(UserAf * user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
    {
        if( user->getName() == i_name )
            return user;
    }
    return NULL;
}

int UserContainer::addUser( UserAf * user)
{
	// Add node to container:
	int newId = add(user);

	// On success add user to solving list:
	if( newId > 0 )
		m_userslist.add( user);

	return newId;
}

void UserContainer::setPermanent( const af::MCGeneral & usr, bool permanent, MonitorContainer * monitoring)
{
   std::string username( usr.getName());
   std::string hostname( usr.getHostName());

   //
   // Try to find user with provided name in container to edit permanent property
   //
   UserContainerIt usersIt( this);
   for(UserAf *user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
   {
      if( username == user->getName())
      {
         bool changed = false;
         if( user->getHostName() != hostname )
         {
            user->setHostName( hostname);
            changed = true;
         }

         // set permanent property if it differ
         if( user->isPermanent() != permanent)
         {
            changed = true;
            user->setPermanent( permanent);
            if( permanent) AFCommon::QueueDBAddItem( user);
            else AFCommon::QueueDBDelItem( user);
         }

         if( changed && monitoring)
         {
            monitoring->addEvent( af::Msg::TMonitorUsersChanged, user->getId());
         }

         // return if user exists in container
         return;
      }
   }
   //
   //    User with provided name does not exist container
   //
   if( permanent == false)
   {
      // Error: user to unset permanent does not exist
      AFERRAR("UserContainer::setPermanent: No user \"%s\" founded.\n", username.c_str());
      return;
   }
   //
   //    Create a new permanent user and put him to database
   //
   UserAf *user = new UserAf( username, hostname);
   if( addUser( user) == 0)
   {
      delete user;
      AFERRAR("UserContainer::setPermanent: Can't add user \"%s\" node to container.", username.c_str());
      return;
   }
   user->setPermanent( true);
   AFCommon::QueueDBAddItem( user);
   if( monitoring) monitoring->addEvent( af::Msg::TMonitorUsersAdd, user->getId());
   return;
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
