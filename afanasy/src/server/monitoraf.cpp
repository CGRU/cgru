#include "monitoraf.h"

#include "../libafanasy/common/dlScopeLocker.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/monitorevents.h"
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
	UserAf * user = i_users->getUser( m_user_name);
	if( user )
	{
		m_uid = user->getId();
		user->updateTimeActivity();
	}
}

MonitorAf::~MonitorAf()
{
}

void MonitorAf::v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
	if( getTimeUpdate() < (currentTime - af::Environment::getMonitorZombieTime()))
	{
		if( monitoring) monitoring->addEvent( af::Monitor::EVT_monitors_del, m_id);
		{
			AFCommon::QueueLog("Monitor zombie: " + v_generateInfoString( false));
			setZombie();
		}
	}
}

void MonitorAf::deregister()
{
	AFCommon::QueueLog("Monitor deregister: " + v_generateInfoString( false));
	setZombie();
	m_monitors->addEvent( af::Monitor::EVT_monitors_del, getId());
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
			ms_msg_queue->pushMsg( msg);
			return;
		}
		if( optype == "deregister")
		{
			deregister();
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
				eids.push_back( af::Monitor::EVT_jobs_add);
				eids.push_back( af::Monitor::EVT_jobs_change);
				eids.push_back( af::Monitor::EVT_jobs_del);
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
				eids.push_back( af::Monitor::EVT_renders_add);
				eids.push_back( af::Monitor::EVT_renders_change);
				eids.push_back( af::Monitor::EVT_renders_del);
			}
			else if( opclass == "users")
			{
				eids.push_back( af::Monitor::EVT_users_add);
				eids.push_back( af::Monitor::EVT_users_change);
				eids.push_back( af::Monitor::EVT_users_del);
			}
			else if( opclass == "monitors")
			{
				eids.push_back( af::Monitor::EVT_monitors_add);
				eids.push_back( af::Monitor::EVT_monitors_change);
				eids.push_back( af::Monitor::EVT_monitors_del);
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
			m_monitors->addEvent( af::Monitor::EVT_monitors_change, getId());
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
	m_jobsUsersIds.clear();
	for( int i = 0; i < i_ids.size(); i++)
		m_jobsUsersIds.push_back( i_ids[i]);
}

void MonitorAf::setEvents( const std::vector<int32_t> & i_ids, bool value)
{
	for( int i = 0; i < i_ids.size(); i++)
	{
		int eventNum = i_ids[i];
		if(( eventNum >= 0) && ( eventNum < EVT_COUNT))
		{
			m_events[eventNum] = value;
		}
		else
		{
			AFERRAR("MonitorAf::addEvent: Invalid event number: %d\n", eventNum);
		}
	}
//printf("MonitorAf::setEvents:\n"); v_stdOut(true);
}

bool MonitorAf::hasJobUid( int uid) const
{
   for( std::list<int32_t>::const_iterator it = m_jobsUsersIds.begin(); it != m_jobsUsersIds.end(); it++)
      if( *it == uid) return true;
   return false;
}

bool MonitorAf::hasJobEvent( int type, int uid) const
{
   int count = m_jobsUsersIds.size();
   if( count < 1) return false;
   if( hasEvent( type) == false) return false;

   std::list<int32_t>::const_iterator it = m_jobsUsersIds.begin();
   if(( count == 1 ) && ( *it == 0 )) return true;

   return hasJobUid( uid);
}

bool MonitorAf::hasJobId( int id) const
{
	for( std::list<int32_t>::const_iterator it = m_jobsIds.begin(); it != m_jobsIds.end(); it++)
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
			m_jobsIds.push_back( i_ids[i]);
		}
	}
}

void MonitorAf::setJobIds( const std::vector<int32_t> & i_ids)
{
//printf("MonitorAf::setJobIds:[%d]",getId());for(int i=0;i<i_ids.size();i++)printf(" %d",i_ids[i]);printf("\n");
	m_jobsIds.clear();
	for( int i = 0; i < i_ids.size(); i++) m_jobsIds.push_back( i_ids[i]);
}

void MonitorAf::delJobIds( const std::vector<int32_t> & i_ids)
{
//printf("MonitorAf::delJobIds:[%d]",getId());for(int i=0;i<i_ids.size();i++)printf(" %d",i_ids[i]);printf("\n");
	for( int i = 0; i < i_ids.size(); i++) m_jobsIds.remove( i_ids[i]);
}

void MonitorAf::addEvents( int i_type, const std::list<int32_t> i_ids)
{
	if(( i_type >= af::Monitor::EVT_COUNT ) || ( i_type < 0 ))
	{
		AFERRAR("MonitorAf::addEvents: Event %d is invalid.", i_type)
		return;
	}

	std::list<int32_t>::const_iterator it = i_ids.begin();
	while( it != i_ids.end())
	{
		af::addUniqueToVect( m_e.m_events[i_type], *it);
		it++;
	}

//printf("MonitorAf::addEvents: i_ids.size()=%lu\n", i_ids.size());
}

void MonitorAf::addTaskProgress( int i_j, int i_b, int i_t, const af::TaskProgress * i_tp)
{
//std::ostringstream str;af::jw_state( i_tp->state, str);printf("MonitorAf::addTaskProgress():j=%d b=%d t=%d s='%s'\n", i_j, i_b, i_t, str.str().c_str());

	for( int j = 0; j < m_e.m_tp.size(); j++)
	{
		if( m_e.m_tp[j].job_id != i_j ) continue;

		for( int t = 0; t < m_e.m_tp[j].tp.size(); t++)
		{
			if(( m_e.m_tp[j].blocks[t] == i_t ) && ( m_e.m_tp[j].tasks[t] == i_b ))
			{
				m_e.m_tp[j].tp[t] = *i_tp;
//printf("MonitorAf::addTaskProgress(): Task progress updated.\n");
				return;
			}
		}

		m_e.m_tp[j].blocks.push_back( i_b);
		m_e.m_tp[j].tasks.push_back(  i_t);
		m_e.m_tp[j].tp.push_back( *i_tp);

//printf("MonitorAf::addTaskProgress(): Task progress of the same job pushed.\n");
		return;
	}

	const int last = m_e.m_tp.size();
	m_e.m_tp.push_back( af::MonitorEvents::MTaskProgresses());
	m_e.m_tp[last].job_id = i_j;
	m_e.m_tp[last].blocks.push_back( i_b);
	m_e.m_tp[last].tasks.push_back( i_t);
	m_e.m_tp[last].tp.push_back( *i_tp);
//printf("MonitorAf::addTaskProgress(): New job task progress pushed.\n");
}

void MonitorAf::addBlock( int i_j, int i_b, int i_mode)
{
	for( int i = 0; i < m_e.m_bids.size(); i++)
	{
		if(( m_e.m_bids[i].job_id == i_j ) && ( m_e.m_bids[i].block_num == i_b ))
		{
			if( m_e.m_bids[i].mode < i_mode )
				m_e.m_bids[i].mode = i_mode;
			return;
		}
	}

	int i = m_e.m_bids.size();
	m_e.m_bids.push_back( af::MonitorEvents::MBlocksIds());
	m_e.m_bids[i].job_id = i_j;
	m_e.m_bids[i].block_num = i_b;
	m_e.m_bids[i].mode = i_mode;
}

void MonitorAf::addUserJobsOrder( int32_t i_uid, std::vector<int32_t> i_jids)
{
	for( int i = 0; i < m_e.m_jobs_order_uids.size(); i++)
		if( m_e.m_jobs_order_uids[i] == i_uid )
		{
			m_e.m_jobs_order_jids[i] = i_jids;
			return;
		}

	m_e.m_jobs_order_uids.push_back( i_uid);
	m_e.m_jobs_order_jids.push_back( i_jids);
}

af::Msg * MonitorAf::getEventsBin()
{
	updateTime();

	DlScopeLocker mutex( &m_mutex);

	af::Msg * msg = NULL;

	if( m_e.isEmpty())
		msg = new af::Msg( af::Msg::TMonitorId, getId());
	else
		msg = new af::Msg( af::Msg::TMonitorEvents, &m_e);

	m_e.clear();

	return msg;
}

af::Msg * MonitorAf::getEventsJSON()
{
	updateTime();

	af::Msg * msg = new af::Msg();

	std::ostringstream stream;
	stream << "{\"events\":";
	bool hasevents = false;

	DlScopeLocker mutex( &m_mutex);

	// Nodes events:
	for( int e = 0; e < af::Monitor::EVT_COUNT; e++)
	{
		if( m_e.m_events[e].size() == 0 )
			continue;

		if( hasevents )
			stream << ",";
		else
			stream << "{";

		stream << "\n\"" << af::Monitor::EVT_NAMES[e] << "\":";
		stream << "[";

		for( int i = 0; i < m_e.m_events[e].size(); i++)
		{
			if( i )
				stream << ",";
			stream << m_e.m_events[e][i];
		}

		stream << "]";
		hasevents = true;
	}

	// Tasks progress:
	if( m_e.m_tp.size())
	{
		if( hasevents )
			stream << ",";
		else
			stream << "{";

		stream << "\n\"tasks_progress\":[";
		for( int j = 0; j < m_e.m_tp.size(); j++)
		{
			if( j > 0 ) stream << ",";
			stream << "{\"job_id\":" << m_e.m_tp[j].job_id;
			stream << ",\"blocks\":[";
			for( int t = 0; t < m_e.m_tp[j].blocks.size(); t++)
			{
				if( t > 0 ) stream << ",";
				stream << m_e.m_tp[j].blocks[t];
			}
			stream << "],\"tasks\":[";
			for( int t = 0; t < m_e.m_tp[j].tasks.size(); t++)
			{
				if( t > 0 ) stream << ",";
				stream << m_e.m_tp[j].tasks[t];
			}
			stream << "],\"progress\":[";
			for( int t = 0; t < m_e.m_tp[j].tp.size(); t++)
			{
				if( t > 0 ) stream << ",";
				m_e.m_tp[j].tp[t].jsonWrite( stream);
			}	
			stream << "]}";
		}
		stream << "]";
		hasevents = true;
	}

	// Blocks ids:
	if( m_e.m_bids.size())
	{
		if( hasevents )
			stream << ",";
		else
			stream << "{";

		stream << "\n\"block_ids\":{";

		stream << "\"job_id\":[";
		for( int i = 0; i < m_e.m_bids.size(); i++)
		{
			if( i > 0 ) stream << ",";
			stream << m_e.m_bids[i].job_id;
		}
		stream << "],";

		stream << "\"block_num\":[";
		for( int i = 0; i < m_e.m_bids.size(); i++)
		{
			if( i > 0 ) stream << ",";
			stream << m_e.m_bids[i].block_num;
		}
		stream << "],";

		stream << "\"mode\":[";
		for( int i = 0; i < m_e.m_bids.size(); i++)
		{
			if( i > 0 ) stream << ",";
			stream << '"' << af::BlockData::DataModeFromMsgType( m_e.m_bids[i].mode) << '"';
		}
		stream << "]}";

		hasevents = true;
	}

	if( m_e.m_jobs_order_uids.size())
	{
		if( hasevents )
			stream << ",";
		else
			stream << "{";

		stream << "\"jobs_order\":{\"uids\":[";
		for( int i = 0; i < m_e.m_jobs_order_uids.size(); i++)
		{
			if( i > 0 ) stream << ",";
			stream << m_e.m_jobs_order_uids[i];
		}
		stream << "],\"jids\":[";
		for( int i = 0; i < m_e.m_jobs_order_jids.size(); i++)
		{
			if( i > 0 ) stream << ",";
			stream << "[";
			for( int j = 0; j < m_e.m_jobs_order_jids[i].size(); j++)
			{
				if( j > 0 ) stream << ",";
				stream << m_e.m_jobs_order_jids[i][j];
			}
			stream << "]";
		}
		stream << "]}";

		hasevents = true;
	}


	if( false == hasevents )
		stream << "\nnull";
	else
		stream << "\n}";
	stream << "\n}";

//if( hasevents ) printf("MonitorAf::getEvents():\n%s\n", stream.str().c_str());

	msg->setData( stream.str().size(), stream.str().c_str(), af::Msg::TJSON);

	m_e.clear();

	return msg;
}

