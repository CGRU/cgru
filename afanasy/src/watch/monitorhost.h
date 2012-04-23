#pragma once

#include "../libafanasy/monitor.h"
#include "../libafanasy/msg.h"

#include <QtCore/QList>

class MonitorHost: public af::Monitor
{
public:
   MonitorHost();
   ~MonitorHost();

   inline void setId( int new_id) { m_id = new_id;}

   inline void   subscribe( const QList<int> & eIds ) { setEvents( af::Msg::TMonitorSubscribe,   eIds);}
   inline void unsubscribe( const QList<int> & eIds ) { setEvents( af::Msg::TMonitorUnsubscribe, eIds);}

   inline void addJobId( int jId ) { setJobId( af::Msg::TMonitorJobsIdsAdd, jId);}
   inline void delJobId( int jId ) { setJobId( af::Msg::TMonitorJobsIdsDel, jId);}

   void setUid( int uid);

   void connectionLost();
   void connectionEstablished();

private:
   int * events_counts;
   std::list<int32_t> jobsUsersIds_counts;
   std::list<int32_t> jobsIds_counts;

private:
   void setEvents( int type, const QList<int> & eIds);
   void setJobId(  int type, int m_id);
};
