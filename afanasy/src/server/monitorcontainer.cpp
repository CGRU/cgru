#include "monitorcontainer.h"

#include "../include/afanasy.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/job.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mctasksprogress.h"
#include "../libafanasy/msg.h"

#include "afcommon.h"
#include "useraf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

MonitorContainer::MonitorContainer():
	ClientContainer( "Monitors", AFMONITOR::MAXCOUNT),
	m_events( NULL),
	m_jobEvents( NULL),
	m_jobEventsUids( NULL)
{
	MonitorAf::setMonitorContainer( this);
//	MonitorAf::setMsgQueue( ms_msg_queue);

	m_events = new std::list<int32_t>[ af::Monitor::EVT_COUNT];
	m_jobEvents	  = new std::list<int32_t>[ af::Monitor::EVT_JOBS_COUNT];
	m_jobEventsUids = new std::list<int32_t>[ af::Monitor::EVT_JOBS_COUNT];
AFINFA("MonitorContainer::MonitorContainer: Events Count = %d, Job Events = %d\n", af::Monitor::EVT_COUNT, af::Monitor::EVT_JOBS_COUNT);
}

MonitorContainer::~MonitorContainer()
{
AFINFO("MonitorContainer::~MonitorContainer:\n");
	if( m_events        != NULL ) delete [] m_events;
	if( m_jobEvents     != NULL ) delete [] m_jobEvents;
	if( m_jobEventsUids != NULL ) delete [] m_jobEventsUids;
}

af::Msg * MonitorContainer::addMonitor( MonitorAf * i_monitor, bool i_binary)
{
	if( add( i_monitor))
	{
		i_monitor->setRegisterTime();
		AFCommon::QueueLog("Monitor registered: " + i_monitor->v_generateInfoString( false));
		addEvent( af::Monitor::EVT_monitors_add, i_monitor->getId());
	}
	else
	{
		delete i_monitor;
	}

	if( i_binary )
		return new af::Msg( af::Msg::TMonitor, i_monitor);

	std::ostringstream str;
	str << "{\"monitor\":";
	i_monitor->v_jsonWrite( str, 0);
	str << "}";
	return af::jsonMsg( str);
}

bool MonitorContainer::setInterest( int i_type, af::MCGeneral & i_ids)
{
	int monitorId = i_ids.getId();
	if( monitorId == 0 )
	{
		AFCommon::QueueLogError("MonitorContainer::action: Zero monitor ID.");
		return false;
	}
	MonitorContainerIt mIt( this);
	MonitorAf * monitor = mIt.getMonitor( monitorId);
	if( monitor == NULL )
	{
		AFCommon::QueueLogError("MonitorContainer::action: No monitor with id = " + af::itos( monitorId));
		return false;
	}

	if( monitor->setInterest( i_type, i_ids) == false ) return false;

	addEvent( af::Monitor::EVT_monitors_change, monitorId);
	return true;
}

void MonitorContainer::addEvent( int i_type, int i_nodeId)
{
	if(( i_type < 0 ) || ( i_type >= af::Monitor::EVT_COUNT))
	{
		AFCommon::QueueLogError( std::string("MonitorContainer::addEvent: Invalid event type number: ") + af::itos(i_type));
		return;
	}

	af::addUniqueToList( m_events[i_type], i_nodeId);
}

void MonitorContainer::addJobEvent( int i_type, int i_jid, int i_uid)
{
	if(( i_type < 0 ) || ( i_type >= af::Monitor::EVT_JOBS_COUNT ))
	{
		AFCommon::QueueLogError( std::string("MonitorContainer::addJobEvent: Invalid job event type = ") + af::itos(i_type));
		return;
	}

	if( af::addUniqueToList( m_jobEvents[i_type], i_jid))
		m_jobEventsUids[i_type].push_back( i_uid);
}

void MonitorContainer::dispatch()
{
	//
	// Common Events:
	//
	for( int e = 0; e < af::Monitor::EVT_COUNT; e++)
	{
		if( m_events[e].size() < 1) continue;

//      int eventType = af::Msg::TMonitorCommonEvents_BEGIN+1 + e;
//      int eventType = e + af::Monitor::EventsShift;
//printf("MonitorContainer::dispatch: [%s]\n", af::Msg::TNAMES[eventType]);
//		af::Msg * msg = NULL;
		MonitorContainerIt monitorsIt( this);
		for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
		{
			if( monitor->hasEvent( e))
			{
//				if( monitor->collectingEvents())
				{
					monitor->addEvents( e, m_events[e]);
				}
/*				else
				{
					if( msg == NULL )
					{
						af::MCGeneral mcIds;
						mcIds.setList( m_events[e]);
						msg = new af::Msg( eventType, &mcIds );
					}
					msg->addAddress( monitor);
				}*/
			}
		}
/*		if( msg )
		{
			 ms_msg_queue->pushMsg( msg);
		}*/
	}

	//
	// Job Events, which depends on uid:
	//
	for( int e = 0; e < af::Monitor::EVT_JOBS_COUNT; e++)
	{
		if( m_jobEvents[e].size() < 1) continue;
//      int eventType = af::Msg::TMonitorJobEvents_BEGIN+1 + e;
		if( m_jobEvents[e].size() != m_jobEventsUids[e].size())
		{
			AFCommon::QueueLogError( std::string("MonitorContainer::dispatch: \
				jobEvents and jobEventsUids has different sizes: ")
				+ af::itos(m_jobEvents[e].size()) + " != " + af::itos(m_jobEventsUids[e].size()));
			continue;
		}

		MonitorContainerIt monitorsIt( this);
		for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
		{
			std::list<int32_t> ids;
			bool found = false;

			std::list<int32_t>::const_iterator jIt = m_jobEvents[e].begin();
			std::list<int32_t>::const_iterator jIt_end = m_jobEvents[e].end();
			std::list<int32_t>::const_iterator uIt = m_jobEventsUids[e].begin();

			while( jIt != jIt_end)
			{
				if( monitor->hasJobEvent( e, *uIt))
				{
					if( !found) found = true;
						ids.push_back( *jIt);
				}
				jIt++;
				uIt++;
			}
			if( ! found ) continue;

//			if( monitor->collectingEvents())
			{
				monitor->addEvents( e, ids);
			}
/*			else
			{
				af::MCGeneral mcIds;
				mcIds.setList( ids);
				af::Msg * msg = new af::Msg( eventType, &mcIds );
				msg->addAddress( monitor);
				ms_msg_queue->pushMsg( msg);
			}*/
		}
	}

	//
	// Tasks progress events:
	//
	std::list<af::MCTasksProgress*>::const_iterator tIt = m_tasks.begin();
	while( tIt != m_tasks.end())
	{
//		af::Msg * msg = NULL;
		MonitorContainerIt monitorsIt( this);
		for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
		{
			if( monitor->hasJobId( (*tIt)->getJobId()))
			{
//			if( monitor->collectingEvents())
			{
				const std::list<af::TaskProgress*> * progresses  = (*tIt)->getTasksRun();
				std::list<int32_t>::const_iterator blocksIt = (*tIt)->getBlocks()->begin();
				std::list<int32_t>::const_iterator tasksIt = (*tIt)->getTasks()->begin();
				std::list<af::TaskProgress*>::const_iterator progressIt = progresses->begin();
				while( progressIt != progresses->end())
				{
					monitor->addTaskProgress( (*tIt)->getJobId(), *blocksIt, *tasksIt, *progressIt);
					blocksIt++; tasksIt++; progressIt++;
				}
			}
/*			else
			{
	            if( msg == NULL)
	 	        {
		  	       msg = new af::Msg( af::Msg::TTasksRun, *tIt);
					}
	            msg->addAddress( monitor);
			}*/
			}
		}
/*		if( msg )
		{
			ms_msg_queue->pushMsg( msg);
		}*/
		tIt++;
	}

	//
	// Blocks changed:
	//
	{
	MonitorContainerIt monitorsIt( this);
	for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
	{
//		af::MCAfNodes mcblocks;
//		int type = 0;
		std::list<af::BlockData*>::iterator bIt = m_blocks.begin();
		std::list<int32_t>::iterator tIt = m_blocks_types.begin();
		for( ; bIt != m_blocks.end(); bIt++, tIt++)
		{
			if( monitor->hasJobId( (*bIt)->getJobId()))
			{
//				if( monitor->collectingEvents())
				{
					monitor->addBlock( (*bIt)->getJobId(), (*bIt)->getBlockNum(), *tIt);
				}
/*				else
				{
		 			mcblocks.addNode( (*bIt));
					if( type < *tIt) type = *tIt;
				}*/
			}
		}
/*		if( mcblocks.getCount() < 1) continue;

		af::Msg * msg = new af::Msg( type, &mcblocks);
		msg->setAddress( monitor);
		ms_msg_queue->pushMsg( msg);*/
	}
	}

	//
	// Users jobs order:
	//
	{
	std::list<UserAf*>::iterator uIt = m_usersJobOrderChanged.begin();
	while( uIt != m_usersJobOrderChanged.end())
	{
//		af::Msg * msg = NULL;
		std::vector<int32_t> jids = (*uIt)->generateJobsIds();
//		af::MCGeneral mcIds;
//		mcIds.setId( (*uIt)->getId());
//		mcIds.setList( ids);

		MonitorContainerIt monitorsIt( this);
		for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
			if( monitor->sameUid((*uIt)->getId()))
			{
//				if( monitor->collectingEvents() )
				{
					monitor->setUserJobsOrder( jids);
				}
/*				else
				{
					if( msg == NULL )
						msg = new af::Msg( af::Msg::TUserJobsOrder, &mcIds);
					msg->addAddress( monitor);
				}*/
			}
/*
		if( msg )
			ms_msg_queue->pushMsg( msg);
*/
		uIt++;
	}
	}

	//
	// Delete all events:
	//
	clearEvents();
}

void MonitorContainer::clearEvents()
{
	for( int e = 0; e < af::Monitor::EVT_COUNT; e++) m_events[e].clear();

	for( int e = 0; e < af::Monitor::EVT_JOBS_COUNT; e++)
	{
		m_jobEvents[e].clear();
		m_jobEventsUids[e].clear();
	}

	std::list<af::MCTasksProgress*>::iterator tIt = m_tasks.begin();
	while( tIt != m_tasks.end())
	{
		delete *tIt;
		tIt++;
	}

	m_tasks.clear();

	m_blocks.clear();
	m_blocks_types.clear();

	m_usersJobOrderChanged.clear();
}
/*
void MonitorContainer::sendMessage( const af::MCGeneral & mcgeneral)
{
//printf("MonitorContainer::sendMessage:\n"); mcgeneral.stdOut( true);
	int idscount = mcgeneral.getCount();
	if( idscount < 1 ) return;

	af::Msg * msg = new af::Msg;
	bool found = false;
	MonitorContainerIt mIt( this);
	for( int i = 0; i < idscount; i++)
	{
		MonitorAf * monitor = mIt.getMonitor( mcgeneral.getId(i));
		if( monitor == NULL) continue;
		msg->addAddress( monitor);
		if( !found) found = true;
	}
	if( found)
	{
		msg->setString( mcgeneral.getUserName() + ": " + mcgeneral.getString() );
		ms_msg_queue->pushMsg( msg);
	}
	else delete msg;
}

void MonitorContainer::sendMessage( const std::string & str)
{
	af::Msg * msg = new af::Msg;
	msg->setString( str);
	MonitorContainerIt mIt( this);

	for( MonitorAf * monitor = mIt.monitor(); monitor != NULL; mIt.next(), monitor = mIt.monitor())
		msg->addAddress( monitor);
	if( msg->addressesCount() )
		ms_msg_queue->pushMsg( msg);
	else delete msg;
}
*/
void MonitorContainer::addTask( int i_jobid, int i_block, int i_task, af::TaskProgress * i_tp)
{
	af::MCTasksProgress * t = NULL;

	std::list<af::MCTasksProgress*>::const_iterator tIt = m_tasks.begin();
	while( tIt != m_tasks.end())
	{
		if( (*tIt)->getJobId() == i_jobid)
		{
			t = (*tIt);
			break;
		}
		tIt++;
	}

	if( t == NULL )
	{
		t = new af::MCTasksProgress( i_jobid);
		m_tasks.push_back( t);
	}

	t->add( i_block, i_task, i_tp);
}

void MonitorContainer::addBlock( int i_type, af::BlockData * i_block)
{
	std::list<af::BlockData*>::const_iterator bIt = m_blocks.begin();
	std::list<int32_t>::iterator tIt = m_blocks_types.begin();
	for( ; bIt != m_blocks.end(); bIt++, tIt++)
	{
		if( i_block == *bIt)
		{
			if( i_type < *tIt ) *tIt = i_type;
			return;
		}
	}
	m_blocks.push_back( i_block);
	m_blocks_types.push_back( i_type);
}

void MonitorContainer::addUser( UserAf * i_user)
{
	std::list<UserAf*>::const_iterator uIt = m_usersJobOrderChanged.begin();
	while( uIt != m_usersJobOrderChanged.end())
		if( *(uIt++) == i_user)
			return;
	m_usersJobOrderChanged.push_back( i_user);
}

//##############################################################################
MonitorContainerIt::MonitorContainerIt( MonitorContainer* container, bool skipZombies):
	AfContainerIt( (AfContainer*)container, skipZombies)
{
}

MonitorContainerIt::~MonitorContainerIt()
{
}
