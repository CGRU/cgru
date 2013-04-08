#include "monitoraf.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "action.h"
#include "afcommon.h"
#include "monitorcontainer.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

MonitorContainer * MonitorAf::m_monitors = NULL;

MonitorAf::MonitorAf( af::Msg * msg):
	af::Monitor( msg),
	AfNodeSrv( this)
{
}

MonitorAf::MonitorAf( const JSON & i_obj, UserContainer * i_users):
	AfNodeSrv( this),
	af::Monitor( i_obj)
{
	m_event_nodeids.resize( af::Monitor::EventsCount);

	UserContainerIt usersIt( i_users);
	for( af::User *user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
	{
		if( user->getName() == m_user_name)
		{
			m_uid = user->getId();
		}
	}
}

MonitorAf::~MonitorAf()
{
}

void MonitorAf::v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
   if( getTimeUpdate() < (currentTime - af::Environment::getMonitorZombieTime()))
   {
      if( monitoring) monitoring->addEvent( af::Msg::TMonitorMonitorsDel, m_id);
      v_setZombie();
   }
}

void MonitorAf::v_setZombie()
{
   AFCommon::QueueLog("Deleting monitor: " + v_generateInfoString( false));

	AfNodeSrv::v_setZombie();
}

void MonitorAf::v_action( Action & i_action)
{
	const JSON & operation = (*i_action.data)["operation"];
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
		if( optype == "deregister")
		{
			v_setZombie();
			i_action.monitors->addEvent( af::Msg::TMonitorMonitorsDel, getId());
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
					addJobIds( jids);
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
				appendLog("Unknown operation \"" + optype + "\" class \"" + opclass + "\" status \"" + opstatus + "\" by " + i_action.author);
				return;
			}
			if( eids.size())
			{
				// Subscribe or unsubscibe common events:
				setEvents( eids, subscribe);
			}
			m_monitors->addEvent( af::Msg::TMonitorMonitorsChanged, getId());
			appendLog("Operation \"" + optype + "\" class \"" + opclass + "\" status \"" + opstatus + "\" by " + i_action.author);
			m_time_activity = time( NULL);
			return;
		}
		else
		{
			appendLog("Unknown operation \"" + optype + "\" by " + i_action.author);
			return;
		}
		appendLog("Operation \"" + optype + "\" by " + i_action.author);
	}
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
//printf("MonitorAf::addJobIds:[%d]",getId());for(int i=0;i<i_ids.size();i++)printf(" %d",i_ids[i]);printf("\n");
	for( int i = 0; i < i_ids.size(); i++)
	{
		if( hasJobId( i_ids[i]) == false)
		{
			jobsIds.push_back( i_ids[i]);
		}
	}
}

void MonitorAf::setJobIds( const std::vector<int32_t> & i_ids)
{
//printf("MonitorAf::setJobIds:[%d]",getId());for(int i=0;i<i_ids.size();i++)printf(" %d",i_ids[i]);printf("\n");
	jobsIds.clear();
	for( int i = 0; i < i_ids.size(); i++) jobsIds.push_back( i_ids[i]);
}

void MonitorAf::delJobIds( const std::vector<int32_t> & i_ids)
{
//printf("MonitorAf::delJobIds:[%d]",getId());for(int i=0;i<i_ids.size();i++)printf(" %d",i_ids[i]);printf("\n");
	for( int i = 0; i < i_ids.size(); i++) jobsIds.remove( i_ids[i]);
}

void MonitorAf::addEvents( int i_type, const std::list<int32_t> i_ids)
{
	if( m_event_nodeids.size() == 0)
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
		af::addUniqueToVect( m_event_nodeids[i_type], *it);
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

void MonitorAf::addUserJobsOrder( int32_t i_uid, std::vector<int32_t> i_jids)
{
	for( int i = 0; i < m_jobs_order_uids.size(); i++)
		if( m_jobs_order_uids[i] == i_uid )
		{
			m_jobs_order_jids[i] = i_jids;
			return;
		}

	m_jobs_order_uids.push_back( i_uid);
	m_jobs_order_jids.push_back( i_jids);
}

af::Msg * MonitorAf::getEvents()
{
	updateTime();

	af::Msg * msg = new af::Msg();
	if( false == collectingEvents())
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

		for( int i = 0; i < m_event_nodeids[e].size(); i++)
		{
			if( i )
				stream << ",";
			stream << m_event_nodeids[e][i];
		}

		stream << "]";
		hasevents = true;
		m_event_nodeids[e].clear();
	}

	// Tasks progress:
	if( m_tp.size())
	{
		if( hasevents )
			stream << ",";
		else
			stream << "{";

		stream << "\n\"tasks_progress\":[";
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
		if( hasevents )
			stream << ",";
		else
			stream << "{";

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

	if( m_jobs_order_uids.size())
	{
		if( hasevents )
			stream << ",";
		else
			stream << "{";

		stream << "\"jobs_order\":{\"uids\":[";
		for( int i = 0; i < m_jobs_order_uids.size(); i++)
		{
			if( i > 0 ) stream << ",";
			stream << m_jobs_order_uids[i];
		}
		stream << "],\"jids\":[";
		for( int i = 0; i < m_jobs_order_jids.size(); i++)
		{
			if( i > 0 ) stream << ",";
			stream << "[";
			for( int j = 0; j < m_jobs_order_jids[i].size(); j++)
			{
				if( j > 0 ) stream << ",";
				stream << m_jobs_order_jids[i][j];
			}
			stream << "]";
		}
		stream << "]}";

		hasevents = true;

		m_jobs_order_uids.clear();
		m_jobs_order_jids.clear();
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

