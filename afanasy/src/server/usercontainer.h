#pragma once

#include "../libafanasy/msgclasses/mcgeneral.h"

#include "afcontainer.h"
#include "afcontainerit.h"
#include "aflist.h"
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

    /// Add user, called on start with user created from batadase, or on "afcmd uadd".
    int    addUser( UserAf * user);

    /// Set user permanent, such users do not deleted when have no jobs, they stored in database.
    void setPermanent( const af::MCGeneral & usr, bool permanent, MonitorContainer * monitoring);

    /// Generate MCJobs message for user with \c id , return NULL if no such \c id exists.
    af::Msg* generateJobsList( int id);

    /// Generate MCJobs message for users with provided ids.
	af::Msg* generateJobsList( const std::vector<int32_t> & ids,
			const std::string & i_type_name, bool json = false);

    /// Generate task for \c render , return \c true on success.
    bool solve( RenderAf *render, MonitorContainer * monitoring);

    static UserAf * getUser( const std::string & i_name );

private:
    AfList m_userslist; ///< Users list.

    static UserContainer * ms_users;
};

//########################## Iterator ##############################

/// Users iterator.
class UserContainerIt: public AfContainerIt
{
public:
    UserContainerIt( UserContainer* userslist, bool skipZombies = true);
    ~UserContainerIt();

	inline UserAf * user() { return (UserAf*)(getNode()); }
	inline UserAf * getUser( int id) { return (UserAf*)(get( id)); }

private:
};
