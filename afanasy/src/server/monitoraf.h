#pragma once

#include "../libafanasy/monitor.h"

class MonitorAf: public af::Monitor
{
public:

   MonitorAf( af::Msg * msg);

   ~MonitorAf();

   void setZombie();

   void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

   bool setInterest( int type, const af::MCGeneral & ids);

   bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

   virtual void v_action( const JSON & i_action, const std::string & i_author, std::string & io_changes,
						   AfContainer * i_container, MonitorContainer * i_monitoring);

   bool hasJobUid( int uid) const;

   bool hasJobEvent( int type, int uid) const;

   bool hasJobId( int m_id) const;

private:
   void setEvents( const af::MCGeneral & ids, bool value);
   void setJobsUsersIds( const af::MCGeneral & ids);
   void addJobIds( const af::MCGeneral & ids);
   void setJobIds( const af::MCGeneral & ids);
   void delJobIds( const af::MCGeneral & ids);
};
