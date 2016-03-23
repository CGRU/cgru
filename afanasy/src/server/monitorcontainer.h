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

   void sendMessage( const af::MCGeneral & i_mcgeneral);

   void sendMessage( const std::string & i_str);

   bool setInterest( int i_type, af::MCGeneral & i_ids);

   void addEvent( int i_type, int i_nodeId);

   void addJobEvent( int i_type, int i_jid, int i_uid);

   void addTask( int i_jobid, int i_block, int i_task, af::TaskProgress * i_tp);

   void addBlock( int i_type, af::BlockData * i_block);

   void addUser( UserAf * i_user);

   void dispatch();

private:

	std::list<int32_t> * m_events;

	std::list<int32_t> * m_jobEvents;
	std::list<int32_t> * m_jobEventsUids;

	std::list<af::BlockData*> m_blocks;
	std::list<int32_t> m_blocks_types;

	std::list<af::MCTasksProgress*> m_tasks;

	std::list<UserAf*> m_usersJobOrderChanged;

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
