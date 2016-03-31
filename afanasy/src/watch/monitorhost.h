#pragma once

#include "../libafanasy/monitor.h"
#include "../libafanasy/msg.h"

#include <QtCore/QList>

class MonitorHost: public af::Monitor
{
public:
	MonitorHost();
	~MonitorHost();

	static inline void setId( int new_id) { m_->m_id = new_id;}

//   inline void   subscribe( const QList<int> & eIds ) { setEvents( af::Msg::TMonitorSubscribe,   eIds);}
//   inline void unsubscribe( const QList<int> & eIds ) { setEvents( af::Msg::TMonitorUnsubscribe, eIds);}

	static inline void addJobId( int jId ) { setJobId( af::Msg::TMonitorJobsIdsAdd, jId);}
	static inline void delJobId( int jId ) { setJobId( af::Msg::TMonitorJobsIdsDel, jId);}

	static void setUid( int i_uid);

	static void connectionLost();
	static void connectionEstablished();

private:
	static MonitorHost * m_;

	static int ms_uid;

//   int * m_events_counts;
//   std::list<int32_t> m_jobsUsersIds_counts;
	static std::list<int32_t> m_jobsIds_counts;

private:
//   void setEvents( int type, const QList<int> & eIds);
	static void setJobId(  int type, int m_id);
};
