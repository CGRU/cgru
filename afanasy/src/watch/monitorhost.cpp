#include "monitorhost.h"

#include "../libafanasy/msgclasses/mcgeneral.h"

#include "watch.h"

#define AFOUTPUT
//#undef AFOUTPUT
#include "../include/macrooutput.h"

MonitorHost::MonitorHost()
{
	m_id = 0;
	m_events_counts = new int[EVT_COUNT];
	for( int e = 0; e < EVT_COUNT; e++) m_events_counts[e] = 0;
}

MonitorHost::~MonitorHost()
{
	delete m_events_counts;
}

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

void MonitorHost::connectionLost()
{
	for( int e = 0; e < EVT_COUNT; e++)
	{
		m_events_counts[e] = 0;
		m_events[e] = false;
	}
}

void MonitorHost::connectionEstablished(){}

void MonitorHost::setJobId( int type, int jId)
{
AFINFA("MonitorHost::setJobId: type=[%s], id=%d", af::Msg::TNAMES[type], jId);
	af::MCGeneral ids;
	ids.setId( Watch::getId());
#ifdef AFOUTPUT
{
int counts = m_jobsIds_counts.size();
std::list<int32_t>::iterator jIt = m_jobsIds.begin();
std::list<int32_t>::iterator cIt = m_jobsIds_counts.begin();
if( counts ) for( int j = 0; j < counts; j++, jIt++, cIt++)
	printf("jobsIds[%d] = %d, jobsIds_counts[%d] = %d\n", j, *jIt, j, *cIt);
else printf("No ids.\n");
}
#endif
	std::list<int32_t>::iterator jIt = m_jobsIds.begin();
	std::list<int32_t>::iterator cIt = m_jobsIds_counts.begin();
	int counts = int( m_jobsIds_counts.size());
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
					m_jobsIds.erase( jIt);
					m_jobsIds_counts.erase( cIt);
				}
			}
			break;
		}
	}
	if((found == false) && (type == af::Msg::TMonitorJobsIdsAdd))
	{
		ids.addId( jId);
		m_jobsIds.push_back( jId);
		m_jobsIds_counts.push_back( 1);
	}
#ifdef AFOUTPUT
{
int counts = m_jobsIds_counts.size();
std::list<int32_t>::iterator jIt = m_jobsIds.begin();
std::list<int32_t>::iterator cIt = m_jobsIds_counts.begin();
if( counts ) for( int j = 0; j < counts; j++, jIt++, cIt++)
	printf("R: jobsIds[%d] = %d, jobsIds_counts[%d] = %d\n", j, *jIt, j, *cIt);
else printf("R: No ids.\n");
}
#endif
	if( ids.getCount() && Watch::isConnected()) Watch::sendMsg( new af::Msg( type, &ids));
}

void MonitorHost::setUid( int uid)
{
AFINFO("MonitorHost::setUid:");
	m_jobsUsersIds.clear();
	m_jobsUsersIds.push_back( uid);
	af::MCGeneral ids;
	ids.setId( Watch::getId());
	ids.addId( uid);
	if( Watch::isConnected()) Watch::sendMsg( new af::Msg( af::Msg::TMonitorUsersJobs, &ids));
}
