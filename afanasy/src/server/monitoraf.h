#pragma once

#include "../libafanasy/monitor.h"

class MonitorAf: public af::Monitor
{
public:

   MonitorAf( af::Msg * msg);

	MonitorAf( const JSON & obj);

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

	static void setMonitorContainer( MonitorContainer * i_monitors) { m_monitors = i_monitors;}

private:
   void setEvents( const std::vector<int32_t> & i_ids, bool value);
   void setJobsUsersIds( const std::vector<int32_t> & i_ids);
   void addJobIds( const std::vector<int32_t> & i_ids);
   void setJobIds( const std::vector<int32_t> & i_ids);
   void delJobIds( const std::vector<int32_t> & i_ids);

	static MonitorContainer * m_monitors;
};
