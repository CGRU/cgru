/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	poolsrv.h - Server side renders pool class.
*/
#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/pool.h"

#include "afnodefarm.h"

class Action;
class PoolsContainer;
class RenderAf;

/// Server side of Afanasy pool.
class PoolSrv : public af::Pool, public AfNodeFarm
{
public:
	/// Create a new pool on a new render.
	PoolSrv(PoolSrv * i_parent, const std::string & i_path);

	/// Construct from store.
	PoolSrv(const std::string & i_store_dir);

	~PoolSrv();

	bool initialize();

	bool addPool(PoolSrv * i_pool);

	inline const PoolSrv * getParent() const {return m_parent;}

	void v_refresh(time_t i_current_time, AfContainer * i_container, MonitorContainer * i_monitoring);

	virtual void v_action(Action & i_action);

	inline void appendPoolLog(const std::string & i_info, bool i_store = true){appendLog(af::Log("pool", getName(),i_info), i_store);}

	virtual int v_calcWeight() const;

	bool hasRender(const RenderAf * i_render) const;
	void addRender(RenderAf * i_render);
	void removeRender(RenderAf * i_render);

	bool assignRender(RenderAf * i_render);

	void taskAcuire (const af::TaskExec * i_taskexec, const std::list<std::string> & i_new_tickets, MonitorContainer * i_monitoring);
	void taskRelease(const af::TaskExec * i_taskexec, const std::list<std::string> & i_exp_tickets, MonitorContainer * i_monitoring);

	inline int findMaxTasksHost() const
		{ if (m_max_tasks_host < 0 && m_parent) return m_parent->findMaxTasksHost(); else return m_max_tasks_host;}
	inline int findCapacityHost() const
		{ if (m_capacity_host  < 0 && m_parent) return m_parent->findCapacityHost(); else return m_capacity_host; }

	inline const std::string & findPropertiesHost() const
		{if (m_properties_host.empty() && m_parent) return m_parent->findPropertiesHost(); else return m_properties_host;}
	inline int findPowerHost() const
		{if (m_power_host < 0 && m_parent) return m_parent->findPowerHost(); else return m_power_host;}

	inline bool newNimby()  const
		{ if (!m_new_nimby  && m_parent) return m_parent->newNimby();  else return m_new_nimby; }
	inline bool newPaused() const
		{ if (!m_new_paused && m_parent) return m_parent->newPaused(); else return m_new_paused;}

	inline int getSickErrorsCount() const
		{if (m_sick_errors_count < 0 && m_parent) return m_parent->getSickErrorsCount(); else return m_sick_errors_count;}

	inline int calcPriority() const {if (m_parent) return (m_priority + m_parent->calcPriority())/2; else return m_priority;}

	inline bool isReady() const {
		if ((m_priority == 0) || isPaused()) return false;
		if (m_parent) return m_parent->isReady();
		return true;
	}

	bool hasPoolTicket(const std::string & i_name, const int32_t & i_count, const bool i_ticket_running) const;


	// Farm config
	inline int32_t getHeartBeatSec() const
		{if (m_heartbeat_sec <= 0 && m_parent) return m_parent->getHeartBeatSec(); else return m_heartbeat_sec;}

	inline int32_t getResourcesUpdatePeriod() const
		{if (m_resources_update_period <= 0 && m_parent) return m_parent->getResourcesUpdatePeriod(); else return m_resources_update_period;}

	inline int32_t getZombieTime() const
		{if (m_zombie_time <= 0 && m_parent) return m_parent->getZombieTime(); else return m_zombie_time;}

	inline int32_t getExitNoTaskTime() const
		{if (m_exit_no_task_time <= 0 && m_parent) return m_parent->getExitNoTaskTime(); else return m_exit_no_task_time;}

	inline int32_t getNoTaskEventTime() const
		{if (m_no_task_event_time <= 0 && m_parent) return m_parent->getNoTaskEventTime(); else return m_no_task_event_time;}

	inline int32_t getOverloadEventTime() const
		{if (m_overload_event_time <= 0 && m_parent) return m_parent->getOverloadEventTime(); else return m_overload_event_time;}


	// Idle & Busy
	inline int get_idle_wolsleep_time() const
		{ if (m_idle_wolsleep_time < 0 && m_parent) return m_parent->get_idle_wolsleep_time(); else return m_idle_wolsleep_time;}
	inline int get_idle_free_time()     const
		{ if (m_idle_free_time     < 0 && m_parent) return m_parent->get_idle_free_time();     else return m_idle_free_time;    }
	inline int get_busy_nimby_time()    const
		{ if (m_busy_nimby_time    < 0 && m_parent) return m_parent->get_busy_nimby_time();    else return m_busy_nimby_time;   }
	inline int get_idle_cpu()           const
		{ if (m_idle_cpu           < 0 && m_parent) return m_parent->get_idle_cpu();           else return m_idle_cpu;          }
	inline int get_busy_cpu()           const
		{ if (m_busy_cpu           < 0 && m_parent) return m_parent->get_busy_cpu();           else return m_busy_cpu;          }
	inline int get_idle_mem()           const
		{ if (m_idle_mem           < 0 && m_parent) return m_parent->get_idle_mem();           else return m_idle_mem;          }
	inline int get_busy_mem()           const
		{ if (m_busy_mem           < 0 && m_parent) return m_parent->get_busy_mem();           else return m_busy_mem;          }
	inline int get_idle_swp()           const
		{ if (m_idle_swp           < 0 && m_parent) return m_parent->get_idle_swp();           else return m_idle_swp;          }
	inline int get_busy_swp()           const
		{ if (m_busy_swp           < 0 && m_parent) return m_parent->get_busy_swp();           else return m_busy_swp;          }
	inline int get_idle_hddgb()         const
		{ if (m_idle_hddgb         < 0 && m_parent) return m_parent->get_idle_hddgb();         else return m_idle_hddgb;        }
	inline int get_busy_hddgb()         const
		{ if (m_busy_hddgb         < 0 && m_parent) return m_parent->get_busy_hddgb();         else return m_busy_hddgb;        }
	inline int get_idle_hddio()         const
		{ if (m_idle_hddio         < 0 && m_parent) return m_parent->get_idle_hddio();         else return m_idle_hddio;        }
	inline int get_busy_hddio()         const
		{ if (m_busy_hddio         < 0 && m_parent) return m_parent->get_busy_hddio();         else return m_busy_hddio;        }
	inline int get_idle_netmbs()        const
		{ if (m_idle_netmbs        < 0 && m_parent) return m_parent->get_idle_netmbs();        else return m_idle_netmbs;       }
	inline int get_busy_netmbs()        const
		{ if (m_busy_netmbs        < 0 && m_parent) return m_parent->get_busy_netmbs();        else return m_busy_netmbs;       }

public:
	inline static void setPoolsContainer(PoolsContainer * i_pools ) { ms_pools = i_pools;}

private:
	bool hasPool(const std::string & i_name) const;
	bool hasPool(const PoolSrv * i_pool) const;
	void removePool(PoolSrv * i_pool);

	void actionAddPool(Action & i_action);
	void actionDelete(Action & i_action);
	void actionService(Action & i_action);
	void actionHealSick(Action & i_action);
	void actionLaunchCmd(Action & i_action, const std::string & i_cmd, bool i_exit);
	void actionEjectTasks(Action & i_action);
	void actionExitRenders(Action & i_action);
	void actionDeleteRenders(Action & i_action);

	void dispatchFarmConfig();

private:
	std::list<PoolSrv*> m_pools_list;
	std::list<RenderAf*> m_renders_list;

private:
	static PoolsContainer * ms_pools;
};
