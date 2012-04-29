#include "monitoraf.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "afcommon.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

MonitorContainer * MonitorAf::m_monitors = NULL;

MonitorAf::MonitorAf( af::Msg * msg):
   af::Monitor( msg)
{
}

MonitorAf::MonitorAf( const JSON & obj):
	af::Monitor( obj)
{
}

MonitorAf::~MonitorAf()
{
}

void MonitorAf::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
   if( getTimeUpdate() < (currentTime - af::Environment::getMonitorZombieTime()))
   {
      if( monitoring) monitoring->addEvent( af::Msg::TMonitorMonitorsDel, m_id);
      setZombie();
   }
}

void MonitorAf::setZombie()
{
   AFCommon::QueueLog("Deleting monitor: " + generateInfoString( false));

   Node::setZombie();
}

void MonitorAf::v_action( const JSON & i_action, const std::string & i_author, std::string & io_changes,
						AfContainer * i_container, MonitorContainer * i_monitoring)
{
	const JSON & operation = i_action["operation"];
	if( operation.IsObject())
	{
		std::string optype;
		af::jr_string("type", optype, operation);
		if( optype == "exit")
		{
			af::Msg* msg = new af::Msg( af::Msg::TClientExitRequest);
			msg->setAddress( this);
			AFCommon::QueueMsgDispatch( msg);
			return;
		}
		if( optype == "watch")
		{
			std::string opclass, opstatus;
			af::jr_string("class", opclass, operation);
			af::jr_string("status", opstatus, operation);
			bool subscribe = false;
			std::vector<int32_t> ids;
			if( opstatus == "subscribe")
				subscribe = true;
			if( opclass == "renders")
			{
				ids.push_back( af::Msg::TMonitorRendersAdd);
				ids.push_back( af::Msg::TMonitorRendersChanged);
				ids.push_back( af::Msg::TMonitorRendersDel);
			}
			else
			{
				appendLog("Unknown operation \"" + optype + "\" class \"" + opclass + "\" status \"" + opstatus + "\" by " + i_author);
				return;
			}
			if( ids.size())
			{
				setEvents( ids, subscribe);
				m_monitors->addEvent( af::Msg::TMonitorMonitorsChanged, getId());
				appendLog("Operation \"" + optype + "\" class \"" + opclass + "\" status \"" + opstatus + "\" by " + i_author);
			}
			return;
		}
		else
		{
			appendLog("Unknown operation \"" + optype + "\" by " + i_author);
			return;
		}
		appendLog("Operation \"" + optype + "\" by " + i_author);
	}
}

bool MonitorAf::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   switch( type)
   {
   case af::Msg::TMonitorExit:
   {
      af::Msg * msg = new af::Msg( af::Msg::TClientExitRequest);
      msg->setAddress( this);
      AFCommon::QueueMsgDispatch( msg);
      return true;
   }
   default:
   {
      return false;
   }
   }
   return true;
}

bool MonitorAf::setInterest( int type, const af::MCGeneral & ids)
{
   m_time_activity = time( NULL);
//printf("MonitorAf::setInterest: [%s]:\n", af::Msg::TNAMES[type]);
   switch(type)
   {
      case af::Msg::TMonitorSubscribe:
      {
         setEvents( ids.getList(), true);
         break;
      }
      case af::Msg::TMonitorUnsubscribe:
      {
         setEvents( ids.getList(), false);
         break;
      }
      case af::Msg::TMonitorUsersJobs:
      {
         setJobsUsersIds( ids.getList());
         break;
      }
      case af::Msg::TMonitorJobsIdsAdd:
      {
         addJobIds( ids.getList());
         break;
      }
      case af::Msg::TMonitorJobsIdsSet:
      {
         setJobIds( ids.getList());
         break;
      }
      case af::Msg::TMonitorJobsIdsDel:
      {
         delJobIds( ids.getList());
         break;
      }
      default:
      {
         AFERRAR("MonitorAf::action: Unknown action: %s\n", af::Msg::TNAMES[type]);
         return false;
      }
   }
   return true;
}

void MonitorAf::setJobsUsersIds( const std::vector<int32_t> & i_ids)
{
	jobsUsersIds.clear();
	for( int i = 0; i < i_ids.size(); i++)
		jobsUsersIds.push_back( i_ids[i]);
}

void MonitorAf::setEvents( const std::vector<int32_t> & i_ids, bool value)
{
	for( int i = 0; i < i_ids.size(); i++)
	{
		int eventNum = i_ids[i] - EventsShift;
		//printf("MonitorAf::setEvents: [%s] - %s\n", af::Msg::TNAMES[ids.getId(i)], value ? "ADD" : "DEL");
		if((eventNum >= 0) && (eventNum < EventsCount))
		{
			events[eventNum] = value;
		}
		else
		{
			AFERRAR("MonitorAf::addEvent: Invalid event: %d\n", i_ids[i]);
		}
	}
}

bool MonitorAf::hasJobUid( int uid) const
{
   for( std::list<int32_t>::const_iterator it = jobsUsersIds.begin(); it != jobsUsersIds.end(); it++)
      if( *it == uid) return true;
   return false;
}

bool MonitorAf::hasJobEvent( int type, int uid) const
{
   int count = jobsUsersIds.size();
   if( count < 1) return false;
   if( hasEvent( type) == false) return false;

   std::list<int32_t>::const_iterator it = jobsUsersIds.begin();
   if(( count == 1 ) && ( *it == 0 )) return true;

   return hasJobUid( uid);
}

bool MonitorAf::hasJobId( int id) const
{
	for( std::list<int32_t>::const_iterator it = jobsIds.begin(); it != jobsIds.end(); it++)
	{
		if( *it == id) return true;
	}
	return false;
}

void MonitorAf::addJobIds( const std::vector<int32_t> & i_ids)
{
	for( int i = 0; i < i_ids[i]; i++)
	{
		if( hasJobId( i_ids[i]) == false)
		{
			jobsIds.push_back( i_ids[i]);
		}
	}
}

void MonitorAf::setJobIds( const std::vector<int32_t> & i_ids)
{
	jobsIds.clear();
	for( int i = 0; i < i_ids.size(); i++) jobsIds.push_back( i_ids[i]);
}

void MonitorAf::delJobIds( const std::vector<int32_t> & i_ids)
{
	for( int i = 0; i < i_ids.size(); i++) jobsIds.remove( i_ids[i]);
}
