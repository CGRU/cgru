#include "monitoraf.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "afcommon.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

MonitorContainer * MonitorAf::m_monitors = NULL;

MonitorAf::MonitorAf( af::Msg * msg):
	af::Monitor( msg),
	AfNodeSrv( this),
	m_event_nodeids( NULL)
{
}

MonitorAf::MonitorAf( const JSON & obj):
	AfNodeSrv( this),
	af::Monitor( obj)
{
	m_event_nodeids = new std::list<int32_t>[af::Monitor::EventsCount];
}

MonitorAf::~MonitorAf()
{
   if( m_event_nodeids != NULL ) delete [] m_event_nodeids;
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

	AfNodeSrv::setZombie();
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
			std::vector<int32_t> eids;
			std::vector<int32_t> uids;
			if( opstatus == "subscribe")
				subscribe = true;

			if( opclass == "jobs")
			{
				eids.push_back( af::Msg::TMonitorJobsAdd);
				eids.push_back( af::Msg::TMonitorJobsChanged);
				eids.push_back( af::Msg::TMonitorJobsDel);
				af::jr_int32vec("uids", uids, operation);
				if( uids.size())
				{
					// Set ids of user(s) whos jobs we are interested in
					// Zero id means super user - interested in all jobs
					setJobsUsersIds( uids);
					appendLog("User ids set to: '" + af::vectToStr( uids) + "'");
				}
			}
			else if( opclass == "tasks")
			{
				std::vector<int32_t> jids;
				af::jr_int32vec("ids", jids, operation);
				if( subscribe )
					setJobIds( jids);
				else
					delJobIds( jids);
			}
			else if( opclass == "renders")
			{
				eids.push_back( af::Msg::TMonitorRendersAdd);
				eids.push_back( af::Msg::TMonitorRendersChanged);
				eids.push_back( af::Msg::TMonitorRendersDel);
			}
			else if( opclass == "users")
			{
				eids.push_back( af::Msg::TMonitorUsersAdd);
				eids.push_back( af::Msg::TMonitorUsersChanged);
				eids.push_back( af::Msg::TMonitorUsersDel);
			}
			else
			{
				appendLog("Unknown operation \"" + optype + "\" class \"" + opclass + "\" status \"" + opstatus + "\" by " + i_author);
				return;
			}
			if( eids.size())
			{
				// Subscribe or unsubscibe common events:
				setEvents( eids, subscribe);
			}
			m_monitors->addEvent( af::Msg::TMonitorMonitorsChanged, getId());
			appendLog("Operation \"" + optype + "\" class \"" + opclass + "\" status \"" + opstatus + "\" by " + i_author);
			m_time_activity = time( NULL);
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

void MonitorAf::addEvents( int i_type, const std::list<int32_t> i_ids)
{
	if( m_event_nodeids == NULL)
	{
		AFERRAR("Monitor '%s' does not collecting events.", m_name.c_str())
		return;
	}

	if(( i_type >= af::Monitor::EventsCount ) || ( i_type < 0 ))
	{
		AFERRAR("MonitorAf::addEvents: Event %d is invalid.", i_type)
		return;
	}

	std::list<int32_t>::const_iterator it = i_ids.begin();
	while( it != i_ids.end())
	{
		af::addUniqueToList( m_event_nodeids[i_type], *it);
		it++;
	}

//printf("MonitorAf::addEvents: i_ids.size()=%lu\n", i_ids.size());
}

void MonitorAf::addTaskProgress( int i_j, int i_b, int i_t, const af::TaskProgress * i_tp)
{
//std::ostringstream str;af::jw_state( i_tp->state, str);printf("MonitorAf::addTaskProgress():j=%d b=%d t=%d s='%s'\n", i_j, i_b, i_t, str.str().c_str());

	for( int j = 0; j < m_tp.size(); j++)
	{
		if( m_tp[j].job_id != i_j ) continue;

		for( int t = 0; t < m_tp[j].tp.size(); t++)
		{
			if(( m_tp[j].blocks[t] == i_t ) && ( m_tp[j].tasks[t] == i_b ))
			{
				m_tp[j].tp[t] = *i_tp;
//printf("MonitorAf::addTaskProgress(): Task progress updated.\n");
				return;
			}
		}

		m_tp[j].blocks.push_back( i_b);
		m_tp[j].tasks.push_back(  i_t);
		m_tp[j].tp.push_back( *i_tp);

//printf("MonitorAf::addTaskProgress(): Task progress of the same job pushed.\n");
		return;
	}

	const int last = m_tp.size();
	m_tp.push_back( MTaskProgresses());
	m_tp[last].job_id = i_j;
	m_tp[last].blocks.push_back( i_b);
	m_tp[last].tasks.push_back( i_t);
	m_tp[last].tp.push_back( *i_tp);
//printf("MonitorAf::addTaskProgress(): New job task progress pushed.\n");
}

void MonitorAf::addBlock( int i_j, int i_b, int i_mode)
{
	for( int i = 0; i < m_bids.size(); i++)
	{
		if(( m_bids[i].job_id == i_j ) && ( m_bids[i].block_num == i_b ))
		{
			if( m_bids[i].mode < i_mode )
				m_bids[i].mode = i_mode;
			return;
		}
	}

	int i = m_bids.size();
	m_bids.push_back( MBlocksIds());
	m_bids[i].job_id = i_j;
	m_bids[i].block_num = i_b;
	m_bids[i].mode = i_mode;
}

af::Msg * MonitorAf::getEvents()
{
	updateTime();

	af::Msg * msg = new af::Msg();
	if( m_event_nodeids == NULL)
	{
		AFERRAR("Monitor '%s' does not collecting events.", m_name.c_str())
		return msg;
	}

	std::ostringstream stream;
	stream << "{\"events\":";
	bool hasevents = false;

	// Nodes events:
	for( int e = 0; e < af::Monitor::EventsCount; e++)
	{
		if( m_event_nodeids[e].size() == 0 )
			continue;

		if( hasevents )
			stream << ",";
		else
			stream << "{";
		stream << "\n\"" << af::Monitor::EventsNames[e] << "\":";
		stream << "[";

		std::list<int32_t>::const_iterator it = m_event_nodeids[e].begin();
		while( it != m_event_nodeids[e].end())
		{
			if( it != m_event_nodeids[e].begin())
				stream << ",";
			stream << *it;
			it++;
		}

		stream << "]";
		hasevents = true;
		m_event_nodeids[e].clear();
	}

	// Tasks progress:
	if( m_tp.size())
	{
		if( hasevents ) stream << ",";
		stream << "\n{\"tasks_progress\":[";
		for( int j = 0; j < m_tp.size(); j++)
		{
			if( j > 0 ) stream << ",";
			stream << "{\"job_id\":" << m_tp[j].job_id;
			stream << ",\"blocks\":[";
			for( int t = 0; t < m_tp[j].blocks.size(); t++)
			{
				if( t > 0 ) stream << ",";
				stream << m_tp[j].blocks[t];
			}
			stream << "],\"tasks\":[";
			for( int t = 0; t < m_tp[j].tasks.size(); t++)
			{
				if( t > 0 ) stream << ",";
				stream << m_tp[j].tasks[t];
			}
			stream << "],\"progress\":[";
			for( int t = 0; t < m_tp[j].tp.size(); t++)
			{
				if( t > 0 ) stream << ",";
				m_tp[j].tp[t].jsonWrite( stream);
			}	
			stream << "]}";
		}
		stream << "]";
		hasevents = true;
		m_tp.clear();
	}

	// Blocks ids:
	if( m_bids.size())
	{
		if( hasevents ) stream << ",";
		stream << "\n\"block_ids\":{";

		stream << "\"job_id\":[";
		for( int i = 0; i < m_bids.size(); i++)
		{
			if( i > 0 ) stream << ",";
			stream << m_bids[i].job_id;
		}
		stream << "],";

		stream << "\"block_num\":[";
		for( int i = 0; i < m_bids.size(); i++)
		{
			if( i > 0 ) stream << ",";
			stream << m_bids[i].block_num;
		}
		stream << "],";

		stream << "\"mode\":[";
		for( int i = 0; i < m_bids.size(); i++)
		{
			if( i > 0 ) stream << ",";
			stream << '"' << af::BlockData::DataModeFromMsgType( m_bids[i].mode) << '"';
		}
		stream << "]}";

		hasevents = true;
		m_bids.clear();
	}

	if( false == hasevents )
		stream << "\nnull";
	else
		stream << "\n}";
	stream << "\n}";

//if( hasevents ) printf("MonitorAf::getEvents():\n%s\n", stream.str().c_str());

	msg->setData( stream.str().size(), stream.str().c_str(), af::Msg::TJSON);

	return msg;
}

