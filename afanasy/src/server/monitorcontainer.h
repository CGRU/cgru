#pragma once

#include "afcontainer.h"
#include "afcontainerit.h"

#include "../libafanasy/monitorevents.h"
#include "../libafanasy/taskprogress.h"

#include "monitoraf.h"

class MonitorAf;
class RenderContainer;
class UserAf;

/// Monitors container.
class MonitorContainer : public AfContainer
{
public:
   MonitorContainer();
   ~MonitorContainer();

	/// Add new Monitor to container.
	af::Msg * addMonitor( MonitorAf * i_monitor, bool i_binary);


	// Send message to all monitors:
	inline void announce( const std::string & i_str) { m_announcement = i_str;}

   void addEvent( int i_type, int i_nodeId);

   void addJobEvent( int i_type, int i_jid, int i_uid);

   void addTask( int i_jobid, int i_block, int i_task, af::TaskProgress * i_tp);

   void addBlock( int i_type, af::BlockData * i_block);

   void addUser( UserAf * i_user);

	void addListened( const af::MCTask & i_mctask);

	void outputsReceived( const std::vector<af::MCTaskPos> & i_outspos, const std::vector<std::string> & i_outputs);

   void dispatch( RenderContainer * i_renders);

private:

	std::list<int32_t> * m_events;

	std::list<int32_t> * m_jobEvents;
	std::list<int32_t> * m_jobEventsUids;

	std::list<af::BlockData*> m_blocks;
	std::list<int32_t> m_blocks_types;

	std::list<af::MCTasksProgress*> m_tasks;

	std::list<UserAf*> m_usersJobOrderChanged;

	std::vector<af::MCTask> m_listens;

	std::string m_announcement;

   void clearEvents();
};

/// Monitors iterator.
class MonitorContainerIt : public AfContainerIt
{
public:
   MonitorContainerIt( MonitorContainer* m_container, bool skipZombies = true);
   ~MonitorContainerIt();

	inline MonitorAf * monitor() { return (MonitorAf*)(getNode()); }
	inline MonitorAf * getMonitor( int id) { return (MonitorAf*)(get( id)); }

private:
};
