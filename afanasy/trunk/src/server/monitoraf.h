#pragma once

#include "../libafanasy/monitor.h"

class MonitorAf: public af::Monitor
{
public:

   MonitorAf( af::Msg * msg, const af::Address * addr = NULL);

   ~MonitorAf();

   void setZombie();

   void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

   bool setInterest( int type, const af::MCGeneral & ids);

   bool hasJobUid( int uid) const;

   bool hasJobEvent( int type, int uid) const;

   bool hasJobId( int id) const;

private:
   void setEvents( const af::MCGeneral & ids, bool value);
   void setJobsUsersIds( const af::MCGeneral & ids);
   void addJobIds( const af::MCGeneral & ids);
   void setJobIds( const af::MCGeneral & ids);
   void delJobIds( const af::MCGeneral & ids);
};
