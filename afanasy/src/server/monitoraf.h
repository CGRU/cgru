#pragma once

#include "../libafanasy/monitor.h"
#include "../libafanasy/taskprogress.h"

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

	inline bool collectingEvents() { return m_event_nodeids != NULL;}

	void addEvents( int i_type, const std::list<int32_t> i_ids);

	af::Msg * getEvents();

	void addTaskProgress( int i_j, int i_b, int i_t, const af::TaskProgress * i_tp);

	void addBlock( int i_j, int i_b, int i_mode);

private:
   void setEvents( const std::vector<int32_t> & i_ids, bool value);
   void setJobsUsersIds( const std::vector<int32_t> & i_ids);
   void addJobIds( const std::vector<int32_t> & i_ids);
   void setJobIds( const std::vector<int32_t> & i_ids);
   void delJobIds( const std::vector<int32_t> & i_ids);

private:
	std::list<int32_t> * m_event_nodeids;

	struct MTaskProgresses {
		int job_id;
		std::vector<int> blocks;
		std::vector<int> tasks;
		std::vector<af::TaskProgress> tp;
	};
	std::vector<MTaskProgresses> m_tp;

	struct MBlocksIds {
		int job_id;
		int block_num;
		int mode;
	};
	std::vector<MBlocksIds> m_bids;

private:
	static MonitorContainer * m_monitors;
};
