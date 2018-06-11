#include "monitorcontainer.h"

#include "../include/afanasy.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/job.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msgclasses/mctasksprogress.h"
#include "../libafanasy/msg.h"

#include "afcommon.h"
#include "renderaf.h"
#include "rendercontainer.h"
#include "useraf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

MonitorContainer::MonitorContainer():
	AfContainer( "Monitors", AFMONITOR::MAXCOUNT),
	m_events( NULL),
	m_jobEvents( NULL),
	m_jobEventsUids( NULL)
{
	MonitorAf::setMonitorContainer( this);

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

void MonitorContainer::outputsReceived( const std::vector<af::MCTaskPos> & i_outspos, const std::vector<std::string> & i_outputs)
{
	if( i_outspos.size() != i_outputs.size())
	{
		AFERRAR("MonitorContainer::outputsReceived: i_outspos.size(%d) != i_outputs.size(%d)",
			int( i_outspos.size()), int( i_outputs.size()))
		return;
	}

	bool founded = false;
	MonitorContainerIt monitorsIt( this);
	for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
		for( int i = 0; i < i_outspos.size(); i++)
			if( monitor->isWaintingOutput( i_outspos[i]))
			{
				monitor->addOutput( i_outspos[i], i_outputs[i]);
				founded = true;
			}

	if( false == founded )
		AFERROR("MonitorContainer::outputsReceived: No one waiting such outputs.")
}

void MonitorContainer::addListened( const af::MCTask & i_mctask)
{
	for( int i = 0; i < m_listens.size(); i++)
	{
		if( m_listens[i].isSameTask( i_mctask))
		{
			m_listens[i].appendListened( i_mctask.getListened());
			return;
		}
	}

	m_listens.push_back( i_mctask);
}

void MonitorContainer::dispatch( RenderContainer * i_renders)
{
	//
	// Common Events:
	//
	for( int e = 0; e < af::Monitor::EVT_COUNT; e++)
	{
		if( m_events[e].size() < 1) continue;

		MonitorContainerIt monitorsIt( this);
		for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
			if( monitor->hasEvent( e))
				monitor->addEvents( e, m_events[e]);
	}


	//
	// Job Events, which depends on uid:
	//
	for( int e = 0; e < af::Monitor::EVT_JOBS_COUNT; e++)
	{
		if( m_jobEvents[e].size() < 1) continue;
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

			monitor->addEvents( e, ids);
		}
	}


	//
	// Tasks progress events:
	//
	std::list<af::MCTasksProgress*>::const_iterator tIt = m_tasks.begin();
	while( tIt != m_tasks.end())
	{
		MonitorContainerIt monitorsIt( this);
		for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
		{
			if( monitor->hasJobId( (*tIt)->getJobId()))
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
		}
		tIt++;
	}

	//
	// Blocks changed:
	//
	{
	MonitorContainerIt monitorsIt( this);
	for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
	{
		std::list<af::BlockData*>::iterator bIt = m_blocks.begin();
		std::list<int32_t>::iterator tIt = m_blocks_types.begin();
		for( ; bIt != m_blocks.end(); bIt++, tIt++)
			if( monitor->hasJobId( (*bIt)->getJobId()))
				monitor->addBlock( (*bIt)->getJobId(), (*bIt)->getBlockNum(), *tIt);
	}
	}

	//
	// Users jobs order:
	//
	{
	std::list<UserAf*>::iterator uIt = m_usersJobOrderChanged.begin();
	while( uIt != m_usersJobOrderChanged.end())
	{
		std::vector<int32_t> jids = (*uIt)->generateJobsIds();

		MonitorContainerIt monitorsIt( this);
		for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
			if( monitor->sameUid((*uIt)->getId()))
				monitor->setUserJobsOrder( jids);
		uIt++;
	}
	}

	//
	// Listening output:
	//
	for( int i = 0; i < m_listens.size(); i++)
	{
		bool founded = false;

		MonitorContainerIt monitorsIt( this);
		for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
		{
			if( monitor->isListening( m_listens[i]))
			{
				monitor->addListened( m_listens[i]);
				founded = true;
			}
		}

		if( false == founded )
		{
			AFCommon::QueueLog( std::string("Removing not listening task: ") +
					m_listens[i].m_task_name);

			RenderContainerIt rIt( i_renders);
			RenderAf * render = rIt.getRender( m_listens[i].m_render_id);
			if( render )
				render->listenTask( m_listens[i].getPos(), false);
		}
	}

	//
	// Annoncements:
	//
	if( m_announcement.size())
	{
		MonitorContainerIt monitorsIt( this);
		for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
			monitor->sendMessage( m_announcement);
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

	m_listens.clear();

	m_announcement.clear();
}

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
			// Greater type number has more information,
			// that include all info of a smaller type.
			if( i_type > *tIt ) *tIt = i_type;
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
	AF_DEBUG << "m_usersJobOrderChanged.push_back( i_user)";
}

//##############################################################################
MonitorContainerIt::MonitorContainerIt( MonitorContainer* container, bool skipZombies):
	AfContainerIt( (AfContainer*)container, skipZombies)
{
}

MonitorContainerIt::~MonitorContainerIt()
{
}
