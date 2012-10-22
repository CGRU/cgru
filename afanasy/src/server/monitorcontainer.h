#pragma once

#include "clientcontainer.h"

#include "../libafanasy/taskprogress.h"

#include "monitoraf.h"

class MonitorAf;
class UserAf;

/// Monitors container.
class MonitorContainer : public ClientContainer
{
public:
   MonitorContainer();
   ~MonitorContainer();

	/// Add new Monitor to container.
	af::Msg * addMonitor( MonitorAf * i_monitor, bool i_json = false);

   void sendMessage( const af::MCGeneral & mcgeneral);

   void sendMessage( const std::string & str);

   bool setInterest( int type, af::MCGeneral & ids);

   void addEvent( int type, int id);

   void addJobEvent( int type, int id, int uid);

   void addTask( int jobid, int block, int task, af::TaskProgress * tp);

   void addBlock( int type, af::BlockData * block);

   void addUser( UserAf * user);

   void dispatch();

private:

   std::list<int32_t> * events;

   std::list<int32_t> * jobEvents;
   std::list<int32_t> * jobEventsUids;
   int jobEventsCount;

   std::list<af::BlockData*> blocks;
   std::list<int32_t> blocks_types;

   std::list<af::MCTasksProgress*> tasks;

	std::list<UserAf*> usersJobOrderChanged;

   void clearEvents();
};

/// Monitors interator.
class MonitorContainerIt : public AfContainerIt
{
public:
   MonitorContainerIt( MonitorContainer* container, bool skipZombies = true);
   ~MonitorContainerIt();

	inline MonitorAf * monitor() { return (MonitorAf*)(getNode()); }
	inline MonitorAf * getMonitor( int id) { return (MonitorAf*)(get( id)); }

private:
};
