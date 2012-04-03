#include "monitorhost.h"

#include "../libafanasy/msgclasses/mcgeneral.h"

#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

MonitorHost::MonitorHost()
{
   m_id = 0;
   events_counts = new int[EventsCount];
   for( int e = 0; e < EventsCount; e++) events_counts[e] = 0;
}

MonitorHost::~MonitorHost()
{
   delete events_counts;
}

void MonitorHost::setEvents( int type, const QList<int> & eIds)
{
   int count = eIds.size();
#ifdef AFOUTPUT
printf("MonitorHost::setEvents: type=[%s], count=%d:\n", af::Msg::TNAMES[type], count);
for( int e = 0; e < count; e++)
printf("   %c [%s]\n", type==af::Msg::TMonitorSubscribe ? '+':'-', af::Msg::TNAMES[eIds[e]]);
#endif
   if( count < 1 ) return;
   af::MCGeneral ids;
   ids.setId( Watch::getId());
   for( int e = 0; e < count; e++)
   {
      int eventtype = eIds[e];
      int eventnum  = eventtype - EventsShift;
      if( type == af::Msg::TMonitorSubscribe)
      {
         if( events_counts[eventnum] == 0)
         {
            events[eventnum] = true;
            ids.addId( eventtype);
         }
         events_counts[eventnum]++;
      }
      else
      {
         if( events_counts[eventnum] > 0) events_counts[eventnum]--;
         if( events_counts[eventnum] == 0)
         {
            events[eventnum] = false;
            ids.addId( eventtype);
         }
      }
   }
   if( ids.getCount() && Watch::isConnected()) Watch::sendMsg( new af::Msg( type, &ids));
}

void MonitorHost::connectionLost()
{
   for( int e = 0; e < EventsCount; e++)
   {
      events_counts[e] = 0;
      events[e] = false;
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
int counts = jobsIds_counts.size();
std::list<int32_t>::iterator jIt = jobsIds.begin();
std::list<int32_t>::iterator cIt = jobsIds_counts.begin();
if( counts ) for( int j = 0; j < counts; j++, jIt++, cIt++)
   printf("jobsIds[%d] = %d, jobsIds_counts[%d] = %d\n", j, *jIt, j, *cIt);
else printf("No ids.\n");
}
#endif
   std::list<int32_t>::iterator jIt = jobsIds.begin();
   std::list<int32_t>::iterator cIt = jobsIds_counts.begin();
   int counts = int( jobsIds_counts.size());
   bool founded = false;
   for( int j = 0; j < counts; j++, jIt++, cIt++)
   {
      if( jId == *jIt)
      {
         founded = true;
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
               jobsIds.erase( jIt);
               jobsIds_counts.erase( cIt);
            }
         }
         break;
      }
   }
   if((founded == false) && (type == af::Msg::TMonitorJobsIdsAdd))
   {
      ids.addId( jId);
      jobsIds.push_back( jId);
      jobsIds_counts.push_back( 1);
   }
#ifdef AFOUTPUT
{
int counts = jobsIds_counts.size();
std::list<int32_t>::iterator jIt = jobsIds.begin();
std::list<int32_t>::iterator cIt = jobsIds_counts.begin();
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
   jobsUsersIds.clear();
   jobsUsersIds.push_back( uid);
   af::MCGeneral ids;
   ids.setId( Watch::getId());
   ids.addId( uid);
   if( Watch::isConnected()) Watch::sendMsg( new af::Msg( af::Msg::TMonitorUsersJobs, &ids));
}
