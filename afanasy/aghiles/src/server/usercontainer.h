#pragma once

#include "../libafanasy/msgclasses/mcgeneral.h"

#include "afcontainer.h"
#include "afcontainerit.h"
#include "useraf.h"

class MsgAf;
class RenderAf;

/// Users container.
class UserContainer: public AfContainer
{
public:
   UserContainer();
   ~UserContainer();

/// Add user, called when job registering, if user with this name exists it's hostname set to \c hostname only.
   UserAf* addUser( const std::string & username, const std::string & hostname, MonitorContainer * monitoring);

/// Add user, called on start with user created from batadase.
   void    addUser( UserAf * user);

/// Set user permanent, such users do not deleted when have no jobs, they stored in database.
   void setPermanent( const af::MCGeneral & usr, bool permanent, MonitorContainer * monitoring);

/// Generate MCJobs message for user with \c id , return NULL if no such \c id exists.
   MsgAf* generateJobsList( int id);

/// Generate MCJobs message for users with provided ids.
   MsgAf* generateJobsList( const af::MCGeneral & ids);

/// Generate task for \c render , return \c true on success.
   bool genTask( RenderAf *render, MonitorContainer * monitoring);

private:
   int currentUserId;       ///< \c id of last user generated task.
   int currentPriority;     ///< Priority of last user generated task.
};

//########################## Iterator ##############################

/// Users iterator.
class UserContainerIt: public AfContainerIt
{
public:
   UserContainerIt( UserContainer* userslist, bool skipZombies = true);
   ~UserContainerIt();

   inline UserAf* user() { return (UserAf*)node; }
   inline UserAf* getUser( int id) { return (UserAf*)(get( id)); }

private:
};
