#pragma once

#include "../libafanasy/monitor.h"
#include "../libafanasy/monitorevents.h"
#include "../libafanasy/taskprogress.h"

#include "afnodesrv.h"

class Action;
class UserContainer;

class MonitorAf: public af::Monitor, public AfNodeSrv
{
public:

	MonitorAf( af::Msg * msg, UserContainer * i_users);

	MonitorAf( const JSON & i_obj, UserContainer * i_users);

	~MonitorAf();

	void v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

	virtual void v_action( Action & i_action);

	bool sameUid( int i_uid) const { return i_uid == m_uid; }

	bool hasJobEvent( int type, int uid) const;

	bool hasJobId( int m_id) const;

	static void setMonitorContainer( MonitorContainer * i_monitors) { m_monitors = i_monitors;}

	void addEvents( int i_type, const std::list<int32_t> i_ids);

	af::Msg * getEventsBin();
	af::Msg * getEventsJSON();

	void addTaskProgress( int i_j, int i_b, int i_t, const af::TaskProgress * i_tp);

	void addBlock( int i_j, int i_b, int i_mode);

	inline void setUserJobsOrder( std::vector<int32_t> i_jids) { m_e.m_jobs_order_ids = i_jids;}

	void deregister();

	void waitOutput( const af::MCTaskPos & i_tp);
	bool isWaintingOutput( const af::MCTaskPos & i_tp);
	void addOutput( const af::MCTaskPos & i_tp, const std::string & i_output);

	bool isListening( const af::MonitorEvents::MListen & i_listen) const;
	inline void addListened( const af::MonitorEvents::MListen & i_listen) { m_e.addListened( i_listen); }

	inline void announce( const std::string & i_str) { m_e.m_announcement = i_str; }

private:
	void setEvents( const std::vector<int32_t> & i_ids, bool value);
	void addJobIds( const std::vector<int32_t> & i_ids);
	void delJobIds( const std::vector<int32_t> & i_ids);

	bool setListening( int i_j, int i_b, int i_t, bool i_subscribe);

private:
	af::MonitorEvents m_e;

	std::list<int> m_lis_j;
	std::list<int> m_lis_b;
	std::list<int> m_lis_t;

	std::vector<af::MCTaskPos> m_wait_output;

	DlMutex m_mutex;

private:
	static MonitorContainer * m_monitors;
};
