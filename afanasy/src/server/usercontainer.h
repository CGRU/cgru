#pragma once

#include "../libafanasy/msgclasses/mcgeneral.h"

#include "afcontainer.h"
#include "afcontainerit.h"
#include "aflist.h"
#include "useraf.h"

class Action;
class MsgAf;
class RenderAf;

/// Users container.
class UserContainer: public AfContainer
{
public:
	UserContainer();
	~UserContainer();

	/// Add user, called when job registering, if user with this name exists it's hostname set to \c hostname only.
	UserAf * addUser( const std::string & i_username, const std::string & i_hostname, MonitorContainer * i_monitoring);

	/// Add user, called on start with user created from batadase
	int addUser( UserAf * i_user);

	/// Add user, used to create from JSON data
	af::Msg * addUser( UserAf * i_user, MonitorContainer * i_monitoring);

	/// Generate MCJobs message for user with \c id , return NULL if no such \c id exists.
	af::Msg * generateJobsList( int id);
	/// Generate MCJobs message for users with provided ids:
	af::Msg * generateJobsList( const std::vector<int32_t> & ids, const std::vector<std::string> & i_names,
		const std::string & i_type_name, bool i_json = false);
	/// Generate MCJobs message for users with provided ids:
	af::Msg * generateJobsList( const std::vector<int32_t> & ids, const std::string & i_type_name, bool i_json = false);
	/// Generate MCJobs message for users with provided names:
	af::Msg * generateJobsList( const std::vector<std::string> & i_names, const std::string & i_type_name, bool i_json = false);

	UserAf * getUser( const std::string & i_name );
	void updateTimeActivity( const std::string & i_name );
	void logAction( const Action & i_action, const std::string & i_node_name);
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
