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
   ClientContainer( "Monitors", AFTALK::MAXCOUNT),
   events( NULL),
   jobEvents( NULL),
   jobEventsUids( NULL),
   jobEventsCount( 0)
{
   events = new std::list<int32_t>[ af::Monitor::EventsCount];
   jobEventsCount = af::Msg::TMonitorJobEvents_END - af::Msg::TMonitorJobEvents_BEGIN - 1;
   jobEvents     = new std::list<int32_t>[ jobEventsCount];
   jobEventsUids = new std::list<int32_t>[ jobEventsCount];
//AFINFA("MonitorContainer::MonitorContainer: Events Count = %d, Job Events = %d\n", eventsCount, jobEventsCount);
AFINFA("MonitorContainer::MonitorContainer: Events Count = %d, Job Events = %d\n", af::Monitor::EventsCount, jobEventsCount);
}

MonitorContainer::~MonitorContainer()
{
AFINFO("MonitorContainer::~MonitorContainer:\n");
   if( events        != NULL ) delete [] events;
   if( jobEvents     != NULL ) delete [] jobEvents;
   if( jobEventsUids != NULL ) delete [] jobEventsUids;
}

af::Msg * MonitorContainer::addMonitor( MonitorAf * i_monitor, bool i_json)
{
	int id = 0;
	if( i_json )
	{
		// Considering that JSON monitors does not listening any port.
		// So we do not need check whether another client with the same address exists.
		id = add( i_monitor);
		if( id == 0 )
		{
			delete i_monitor;
		}
		else
		{
			i_monitor->setRegisterTime();
		}
	}
	else
	{
		// If client is listening a port, it has an address.
		// We should delete older client with the same address,
		// that is why monitor container pointer and event type is needed.
		id = addClient( i_monitor, true, this, af::Msg::TMonitorMonitorsDel);
	}

	if( id != 0 )
	{
		AFCommon::QueueLog("Monitor registered: " + i_monitor->v_generateInfoString( false));
		addEvent( af::Msg::TMonitorMonitorsAdd, id);
	}

	if( i_json == false )
		return new af::Msg( af::Msg::TMonitorId, id);

	std::ostringstream str;
	str << "{\"monitor\":{";
	i_monitor->v_jsonWrite( str, 0);
	str << "}}";
	return af::jsonMsg( str);
}

bool MonitorContainer::setInterest( int type, af::MCGeneral & ids)
{
   int monitorId = ids.getId();
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

   if( monitor->setInterest( type, ids) == false ) return false;

   addEvent( af::Msg::TMonitorMonitorsChanged, monitorId);
   return true;
}

void MonitorContainer::addEvent( int type, int id)
{
   if(( type <= af::Msg::TMonitorCommonEvents_BEGIN) || ( type >= af::Msg::TMonitorCommonEvents_END))
   {
      AFCommon::QueueLogError( std::string("MonitorContainer::addEvent: Invalid event type = ") + af::Msg::TNAMES[type]);
      return;
   }
//printf("MonitorContainer::addEvent: [%s] (%d<%d<%d)\n", af::Msg::TNAMES[type], af::Msg::TMonitorCommonEvents_BEGIN, type, af::Msg::TMonitorCommonEvents_END);
   int typeNum = type - af::Monitor::EventsShift;
   af::addUniqueToList( events[typeNum], id);
}

void MonitorContainer::addJobEvent( int type, int id, int uid)
{
   if(( type <= af::Msg::TMonitorJobEvents_BEGIN) || ( type >= af::Msg::TMonitorJobEvents_END))
   {
      AFCommon::QueueLogError( std::string("MonitorContainer::addJobEvent: Invalid job event type = ") + af::Msg::TNAMES[type]);
      return;
   }
//printf("MonitorContainer::addJobEvent: [%s] (%d<%d<%d)\n", af::Msg::TNAMES[type], af::Msg::TMonitorCommonEvents_BEGIN, type, af::Msg::TMonitorCommonEvents_END);
   int typeNum = type - af::Msg::TMonitorJobEvents_BEGIN -1;
   if( af::addUniqueToList( jobEvents[typeNum], id)) jobEventsUids[typeNum].push_back( uid);
}

void MonitorContainer::dispatch()
{
   //
   // Common Events:
   //
   for( int e = 0; e < af::Monitor::EventsCount; e++)
   {
      if( events[e].size() < 1) continue;

//      int eventType = af::Msg::TMonitorCommonEvents_BEGIN+1 + e;
      int eventType = e + af::Monitor::EventsShift;
//printf("MonitorContainer::dispatch: [%s]\n", af::Msg::TNAMES[eventType]);
      af::Msg * msg = NULL;
      MonitorContainerIt monitorsIt( this);
      for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
      {
			if( monitor->hasEvent( eventType))
			{
				if( monitor->collectingEvents())
				{
					monitor->addEvents( e, events[e]);
				}
				else
				{
					if( msg == NULL )
					{
						af::MCGeneral mcIds;
						mcIds.setList( events[e]);
						msg = new af::Msg( eventType, &mcIds );
					}
					msg->addAddress( monitor);
				}
			}
      }
      if( msg )
      {
          AFCommon::QueueMsgDispatch( msg);
      }
   }

   //
   // Job Events, which depend on interested users ids:
   //
   for( int e = 0; e < jobEventsCount; e++)
   {
      if( jobEvents[e].size() < 1) continue;
      int eventType = af::Msg::TMonitorJobEvents_BEGIN+1 + e;
      if( jobEvents[e].size() != jobEventsUids[e].size())
      {
         AFCommon::QueueLogError( std::string("MonitorContainer::dispatch: \
            jobEvents and jobEventsUids has different sizes fo [")
            + af::Msg::TNAMES[eventType] + "] ( " + af::itos(jobEvents[e].size()) + " != " + af::itos(jobEventsUids[e].size()) + " )");
         continue;
      }

      MonitorContainerIt monitorsIt( this);
      for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
      {
			std::list<int32_t> ids;
         bool founded = false;

         std::list<int32_t>::const_iterator jIt = jobEvents[e].begin();
         std::list<int32_t>::const_iterator jIt_end = jobEvents[e].end();
         std::list<int32_t>::const_iterator uIt = jobEventsUids[e].begin();

         while( jIt != jIt_end)
         {
            if( monitor->hasJobEvent( eventType, *uIt))
            {
               if( !founded) founded = true;
						ids.push_back( *jIt);
            }
            jIt++;
            uIt++;
         }
         if( ! founded ) continue;

			if( monitor->collectingEvents())
			{
				monitor->addEvents( eventType - af::Monitor::EventsShift, ids);
			}
			else
			{
				af::MCGeneral mcIds;
				mcIds.setList( ids);
				af::Msg * msg = new af::Msg( eventType, &mcIds );
				msg->addAddress( monitor);
				AFCommon::QueueMsgDispatch( msg);
			}
      }
   }

   //
   // Tasks progress events:
   //
   std::list<af::MCTasksProgress*>::const_iterator tIt = tasks.begin();
   while( tIt != tasks.end())
   {
      af::Msg * msg = NULL;
      MonitorContainerIt monitorsIt( this);
      for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
      {
         if( monitor->hasJobId( (*tIt)->getJobId()))
         {
			if( monitor->collectingEvents())
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
			else
			{
	            if( msg == NULL)
    	        {
        	       msg = new af::Msg( af::Msg::TTasksRun, *tIt);
            	}
	            msg->addAddress( monitor);
			}
         }
      }
      if( msg )
      {
          AFCommon::QueueMsgDispatch( msg);
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
      af::MCAfNodes mcblocks;
      int type = 0;
      std::list<af::BlockData*>::iterator bIt = blocks.begin();
      std::list<int32_t>::iterator tIt = blocks_types.begin();
      for( ; bIt != blocks.end(); bIt++, tIt++)
      {
         if( monitor->hasJobId( (*bIt)->getJobId()))
         {
			if( monitor->collectingEvents())
			{
				monitor->addBlock( (*bIt)->getJobId(), (*bIt)->getBlockNum(), *tIt);
			}
			else
			{
    	        mcblocks.addNode( (*bIt));
	            if( type < *tIt) type = *tIt;
			}
         }
      }
      if( mcblocks.getCount() < 1) continue;

      af::Msg * msg = new af::Msg( type, &mcblocks);
      msg->setAddress( monitor);
      AFCommon::QueueMsgDispatch( msg);
   }
   }

	//
	// Users jobs order:
	//
	{
/*	MonitorContainerIt monitorsIt( this);
	for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
	{
		std::list<UserAf*>::iterator uIt = usersJobOrderChanged.begin();
		while( uIt != usersJobOrderChanged.end())
		{
			if( monitor->hasJobUid((*uIt)->getId()))
			{
				af::MCGeneral ids;
				(*uIt)->generateJobsIds( ids);
				af::Msg * msg = new af::Msg( af::Msg::TUserJobsOrder, &ids);
				msg->setAddress( monitor);
				AFCommon::QueueMsgDispatch( msg);
			}
			uIt++;
		}
	}*/
	std::list<UserAf*>::iterator uIt = usersJobOrderChanged.begin();
	while( uIt != usersJobOrderChanged.end())
	{
		af::Msg * msg = NULL;
		std::vector<int32_t> ids = (*uIt)->generateJobsIds();
		af::MCGeneral mcIds;
		mcIds.setId( (*uIt)->getId());
		mcIds.setList( ids);

		MonitorContainerIt monitorsIt( this);
		for( MonitorAf * monitor = monitorsIt.monitor(); monitor != NULL; monitorsIt.next(), monitor = monitorsIt.monitor())
			if( monitor->hasJobUid((*uIt)->getId()))
			{
				if( monitor->collectingEvents() )
				{
					monitor->addUserJobsOrder((*uIt)->getId(), ids);
				}
				else
				{
					if( msg == NULL )
						msg = new af::Msg( af::Msg::TUserJobsOrder, &mcIds);
					msg->addAddress( monitor);
				}
			}

		if( msg )
			AFCommon::QueueMsgDispatch( msg);

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
   for( int e = 0; e < af::Monitor::EventsCount; e++) events[e].clear();

   for( int e = 0; e < jobEventsCount; e++)
   {
      jobEvents[e].clear();
      jobEventsUids[e].clear();
   }

   std::list<af::MCTasksProgress*>::iterator tIt = tasks.begin();
   while( tIt != tasks.end())
   {
      delete *tIt;
      tIt++;
   }

   tasks.clear();

   blocks.clear();
   blocks_types.clear();

   usersJobOrderChanged.clear();
}

void MonitorContainer::sendMessage( const af::MCGeneral & mcgeneral)
{
//printf("MonitorContainer::sendMessage:\n"); mcgeneral.stdOut( true);
   int idscount = mcgeneral.getCount();
   if( idscount < 1 ) return;

   af::Msg * msg = new af::Msg;
   bool founded = false;
   MonitorContainerIt mIt( this);
   for( int i = 0; i < idscount; i++)
   {
      MonitorAf * monitor = mIt.getMonitor( mcgeneral.getId(i));
      if( monitor == NULL) continue;
      msg->addAddress( monitor);
      if( !founded) founded = true;
   }
   if( founded)
   {
      msg->setString( mcgeneral.getUserName() + ": " + mcgeneral.getString() );
      AFCommon::QueueMsgDispatch( msg);
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
       AFCommon::QueueMsgDispatch( msg);
   else delete msg;
}

void MonitorContainer::addTask( int jobid, int block, int task, af::TaskProgress * tp)
{
   af::MCTasksProgress * t = NULL;

   std::list<af::MCTasksProgress*>::const_iterator tIt = tasks.begin();
   while( tIt != tasks.end())
   {
      if( (*tIt)->getJobId() == jobid)
      {
         t = (*tIt);
         break;
      }
      tIt++;
   }

   if( t == NULL )
   {
      t = new af::MCTasksProgress( jobid);
      tasks.push_back( t);
   }

   t->add( block, task, tp);
}

void MonitorContainer::addBlock( int type, af::BlockData * block)
{
   std::list<af::BlockData*>::const_iterator bIt = blocks.begin();
   std::list<int32_t>::iterator tIt = blocks_types.begin();
   for( ; bIt != blocks.end(); bIt++, tIt++)
   {
      if( block == *bIt)
      {
         if( type < *tIt ) *tIt = type;
         return;
      }
   }
   blocks.push_back( block);
   blocks_types.push_back( type);
}

void MonitorContainer::addUser( UserAf * user)
{
	std::list<UserAf*>::const_iterator uIt = usersJobOrderChanged.begin();
	while( uIt != usersJobOrderChanged.end())
		if( *(uIt++) == user)
			return;
	usersJobOrderChanged.push_back( user);
}

//##############################################################################
MonitorContainerIt::MonitorContainerIt( MonitorContainer* container, bool skipZombies):
   AfContainerIt( (AfContainer*)container, skipZombies)
{
}

MonitorContainerIt::~MonitorContainerIt()
{
}
