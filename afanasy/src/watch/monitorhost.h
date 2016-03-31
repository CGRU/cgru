#pragma once

#include "../libafanasy/monitor.h"
#include "../libafanasy/msg.h"

#include <QtCore/QList>

class MonitorHost: public af::Monitor
{
public:
	MonitorHost();
	~MonitorHost();

	static af::Msg * genRegisterMsg();

	static void subscribe( const std::string & i_class, bool i_subscribe);

	static const af::Address & getClientAddress() { return m_->getAddress();}

	static inline void addJobId( int jId ) { setJobId( af::Msg::TMonitorJobsIdsAdd, jId);}
	static inline void delJobId( int jId ) { setJobId( af::Msg::TMonitorJobsIdsDel, jId);}

	static void setUid( int i_uid);
	static int getUid() { return ms_uid ;}

	static void connectionLost();
	static void connectionEstablished( int i_id, int i_uid);

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
