#pragma once

#include "name_af.h"

#include "afnode.h"
#include "regexp.h"

namespace af
{
/// Afanasy render slave.
class Pool : public Node
{
public:

	Pool(const std::string &i_path);

	// To construct from store:
	Pool(int i_id = 0);

	Pool(Msg * msg);

	virtual ~Pool();

	inline const bool  isRoot() const { return m_name == "/"; }
	inline const bool notRoot() const { return m_name != "/"; }

	void v_generateInfoStream(std::ostringstream & stream, bool full = false) const;

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	virtual void v_jsonWrite(std::ostringstream & o_str, int type) const;

	bool jsonRead(const JSON & i_object, std::string * io_changes = NULL);

	inline bool isPaused() const { return (m_state & SPaused);}
	inline void setPaused(bool set) { m_state = set ? m_state | SPaused : m_state & (~SPaused); }

	inline int getRunTasks()        const { return m_run_tasks;         }
	inline int getMaxTasks()        const { return m_max_tasks;         }
	inline int getMaxTasksPerHost() const { return m_max_tasks_per_host;}

	inline int getRunCapacity()        const { return m_run_capacity;         }
	inline int getMaxCapacity()        const { return m_max_capacity;         }
	inline int getMaxCapacityPerHost() const { return m_max_capacity_per_host;}

public:
	enum State
	{
		SPaused  = 1ULL << 0,
		SBusy    = 1ULL << 1
	};

	int32_t m_idle_wolsleep_time;
	int32_t m_idle_free_time;
	int32_t m_busy_nimby_time;
	int32_t m_idle_cpu;
	int32_t m_busy_cpu;
	int32_t m_idle_mem;
	int32_t m_busy_mem;
	int32_t m_idle_swp;
	int32_t m_busy_swp;
	int32_t m_idle_hddgb;
	int32_t m_busy_hddgb;
	int32_t m_idle_hddio;
	int32_t m_busy_hddio;
	int32_t m_idle_netmbs;
	int32_t m_busy_netmbs;

public:
	static const std::string FilterName(const std::string & i_name);

protected:
	RegExp m_pattern;

	std::string m_parent_path;

	int64_t m_time_creation;

	bool m_new_nimby;
	bool m_new_paused;

	int32_t m_pools_num;
	int32_t m_pools_total;
	int32_t m_renders_num;
	int32_t m_renders_total;

	int64_t m_time_offline;
	int64_t m_time_empty;

	int32_t m_run_tasks;
	int32_t m_max_tasks;
	int32_t m_max_tasks_per_host;

	int32_t m_run_capacity;
	int32_t m_max_capacity;
	int32_t m_max_capacity_per_host;

	int64_t m_task_start_finish_time; ///< Task start or finish time.

	std::vector<std::string> m_services_disabled;

private:
	void initDefaultValues();

protected:
	void v_readwrite(Msg * msg); ///< Read or write Pool in message.
};
}
