#include "monitorhost.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "watch.h"

#define AFOUTPUT
//#undef AFOUTPUT
#include "../include/macrooutput.h"

MonitorHost * MonitorHost::m_ = NULL;

int MonitorHost::ms_uid = -1;

std::list<int32_t> MonitorHost::m_jobsIds_counts;

MonitorHost::MonitorHost()
{
	m_ = this;
//	m_id = 0;
//	m_events_counts = new int[EVT_COUNT];
//	for( int e = 0; e < EVT_COUNT; e++) m_events_counts[e] = 0;
}

MonitorHost::~MonitorHost()
{
//	delete m_events_counts;
}

af::Msg * MonitorHost::genRegisterMsg()
{
//{"monitor":{"user_name":"timurhai","host_name":"pc","engine":"firefox"}}

	std::ostringstream str;

	str << "{\"monitor\":{";
	str << "\"binary\":true";
	str << ",\"user_name\":\"" << af::Environment::getUserName() << "\"";
	str << ",\"host_name\":\"" << af::Environment::getHostName() << "\"";
	str << ",\"engine\":\"" << af::Environment::getVersionCGRU() << "\"";
	str << "}}";

	af::Msg * msg = af::jsonMsg( str);

	msg->setReceiving( true);

	return msg;
}

/*
void MonitorHost::setEvents( int type, const QList<int> & eIds)
{
#ifdef AFOUTPUT
if( type == af::Msg::TMonitorSubscribe ) printf("Adding events:"); else printf("Removing events:");
for( int e = 0; e < eIds.size(); e++) printf(" %s", EVT_NAMES[eIds[e]]);
printf("\n");
#endif

	int count = eIds.size();
	if( count < 1 ) return;
	af::MCGeneral ids;
	ids.setId( Watch::getId());
	for( int e = 0; e < count; e++)
	{
		if( type == af::Msg::TMonitorSubscribe )
		{
			if( m_events_counts[eIds[e]] == 0 )
			{
				m_events[eIds[e]] = true;
				ids.addId( eIds[e]);
			}
			m_events_counts[eIds[e]]++;
		}
		else
		{
			if( m_events_counts[eIds[e]] >  0 ) m_events_counts[eIds[e]]--;
			if( m_events_counts[eIds[e]] == 0 )
			{
				m_events[eIds[e]] = false;
				ids.addId( eIds[e]);
			}
		}
	}

#ifdef AFOUTPUT
if( type == af::Msg::TMonitorSubscribe ) printf("Subscribing:"); else printf("Unsubscribing:");
for( int e = 0; e < ids.getCount(); e++) printf(" %s", EVT_NAMES[ids.getId(e)]);
printf("\n");
#endif

	if( ids.getCount() && Watch::isConnected()) Watch::sendMsg( new af::Msg( type, &ids));
}
*/
void MonitorHost::connectionLost()
{
	m_->m_id  =  0;
	m_->m_uid =  0;
	ms_uid    = -1;
//	for( int e = 0; e < EVT_COUNT; e++)
//	{
//		m_events_counts[e] = 0;
//		m_events[e] = false;
//	}
}

void MonitorHost::connectionEstablished( int i_id, int i_uid)
{
	m_->m_id = i_id;
	m_->m_uid = i_uid;
	ms_uid = i_uid;
}

void MonitorHost::subscribe( const std::string & i_class, bool i_subscribe)
{
	std::vector<int> ids;
	ids.push_back( m_->getId());

	std::ostringstream str;
	af::jsonActionOperationStart( str,"monitors","watch", std::string(), ids);
	str << ",\"class\":\"" << i_class << "\"";
	str << ",\"status\":\"" << ( i_subscribe ? "subscribe":"unsubscribe") << "\"";
	af::jsonActionOperationFinish( str);

	Watch::sendMsg( af::jsonMsg( str));
}

void MonitorHost::setJobId( int type, int jId)
{
AFINFA("MonitorHost::setJobId: type=[%s], id=%d", af::Msg::TNAMES[type], jId);
	af::MCGeneral ids;
	ids.setId( m_->getId());
#ifdef AFOUTPUT
{
int counts = m_->m_jobsIds_counts.size();
std::list<int32_t>::iterator jIt = m_->m_jobsIds.begin();
std::list<int32_t>::iterator cIt = m_->m_jobsIds_counts.begin();
if( counts ) for( int j = 0; j < counts; j++, jIt++, cIt++)
	printf("jobsIds[%d] = %d, jobsIds_counts[%d] = %d\n", j, *jIt, j, *cIt);
else printf("No ids.\n");
}
#endif
	std::list<int32_t>::iterator jIt = m_->m_jobsIds.begin();
	std::list<int32_t>::iterator cIt = m_->m_jobsIds_counts.begin();
	int counts = int( m_->m_jobsIds_counts.size());
	bool found = false;
	for( int j = 0; j < counts; j++, jIt++, cIt++)
	{
		if( jId == *jIt)
		{
			found = true;
			if( type == af::Msg::TMonitorJobsIdsAdd)
			{
				if( *cIt == 0) ids.addId( jId);
				(*cIt)++;
			}
			else
			{
				if( *cIt > 0 ) (*cIt)--;
				if( *cIt == 0 )
				{
					ids.addId( jId);
					m_->m_jobsIds.erase( jIt);
					m_->m_jobsIds_counts.erase( cIt);
				}
			}
			break;
		}
	}
	if((found == false) && (type == af::Msg::TMonitorJobsIdsAdd))
	{
		ids.addId( jId);
		m_->m_jobsIds.push_back( jId);
		m_->m_jobsIds_counts.push_back( 1);
	}
#ifdef AFOUTPUT
{
int counts = m_->m_jobsIds_counts.size();
std::list<int32_t>::iterator jIt = m_->m_jobsIds.begin();
std::list<int32_t>::iterator cIt = m_->m_jobsIds_counts.begin();
if( counts ) for( int j = 0; j < counts; j++, jIt++, cIt++)
	printf("R: jobsIds[%d] = %d, jobsIds_counts[%d] = %d\n", j, *jIt, j, *cIt);
else printf("R: No ids.\n");
}
#endif
	if( ids.getCount() && Watch::isConnected()) Watch::sendMsg( new af::Msg( type, &ids));
}

void MonitorHost::setUid( int i_uid)
{
AFINFO("MonitorHost::setUid:");

	// If it is first time, we store it;
	if( ms_uid < 0 )
		ms_uid = i_uid;

	// Negative value means restore original:
	if( i_uid < 0 )
		i_uid = m_->m_uid;

	std::vector<int> ids;
	ids.push_back( m_->getId());

	std::ostringstream str;
	af::jsonActionOperationStart( str,"monitors","watch","", ids);
	str << ",\"class\":\"perm\"";
	str << ",\"uid\":" << i_uid;
	af::jsonActionOperationFinish( str);

	Watch::sendMsg( af::jsonMsg( str));
}

