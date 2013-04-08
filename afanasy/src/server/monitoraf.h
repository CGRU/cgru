#pragma once

#include "../libafanasy/monitor.h"
#include "../libafanasy/taskprogress.h"

#include "afnodesrv.h"

class Action;
class UserContainer;

class MonitorAf: public af::Monitor, public AfNodeSrv
{
public:

	MonitorAf( af::Msg * msg);

	MonitorAf( const JSON & i_obj, UserContainer * i_users);

	~MonitorAf();

	void v_setZombie();

	void v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

	bool setInterest( int type, const af::MCGeneral & ids);

	virtual void v_action( Action & i_action);

	bool hasJobUid( int uid) const;

	bool hasJobEvent( int type, int uid) const;

	bool hasJobId( int m_id) const;

	static void setMonitorContainer( MonitorContainer * i_monitors) { m_monitors = i_monitors;}

	inline bool collectingEvents() { return m_event_nodeids.size() != 0;}

	void addEvents( int i_type, const std::list<int32_t> i_ids);

	af::Msg * getEvents();

	void addTaskProgress( int i_j, int i_b, int i_t, const af::TaskProgress * i_tp);

	void addBlock( int i_j, int i_b, int i_mode);

	void addUserJobsOrder( int32_t i_uid, std::vector<int32_t> i_jids);

private:
	void setEvents( const std::vector<int32_t> & i_ids, bool value);
	void setJobsUsersIds( const std::vector<int32_t> & i_ids);
	void addJobIds( const std::vector<int32_t> & i_ids);
	void setJobIds( const std::vector<int32_t> & i_ids);
	void delJobIds( const std::vector<int32_t> & i_ids);

private:
	std::vector<std::vector<int> > m_event_nodeids;

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

	std::vector<int32_t> m_jobs_order_uids;
	std::vector<std::vector<int32_t> > m_jobs_order_jids;

private:
	static MonitorContainer * m_monitors;
};
