#pragma once

#include "name_af.h"

#include "affarm.h"
#include "afnode.h"
#include "regexp.h"

namespace af
{
/// Afanasy render slave.
class Pool : public Node, public Farm
{
  public:
	Pool(const std::string &i_path);

	// To construct from store:
	Pool(int i_id = 0);

	Pool(Msg *msg);

	virtual ~Pool();

	inline bool isRoot() const { return m_name == "/"; }
	inline bool notRoot() const { return m_name != "/"; }

	void v_generateInfoStream(std::ostringstream &stream, bool full = false) const;

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	virtual void v_jsonWrite(std::ostringstream &o_str, int type) const;

	bool jsonRead(const JSON &i_object, std::string *io_changes = NULL);

	inline bool isBusy() const { return (m_state & SBusy); }
	inline void setBusy(bool set) { m_state = set ? m_state | SBusy : m_state & (~SBusy); }

	inline bool isPaused() const { return (m_state & SPaused); }
	inline void setPaused(bool set) { m_state = set ? m_state | SPaused : m_state & (~SPaused); }

	inline const std::string &getParentPath() const { return m_parent_path; }

	inline const std::string &getPatternStr() const { return m_pattern.getPattern(); }

	inline long long getTimeCreation() const { return m_time_creation; }

	inline bool isNewRenderNimby() const { return m_new_nimby; }
	inline bool isNewRenderPaused() const { return m_new_paused; }

	inline int32_t getPoolsTotal() const { return m_pools_total; }
	inline int32_t getRendersTotal() const { return m_renders_total; }
	inline int32_t getRendersBusy() const { return m_renders_busy; }
	inline int32_t getRendersReady() const { return m_renders_ready; }
	inline int32_t getRendersOnline() const { return m_renders_online; }
	inline int32_t getRendersOffline() const { return m_renders_offline; }
	inline int32_t getRendersNimby() const { return m_renders_nimby; }
	inline int32_t getRendersPaused() const { return m_renders_paused; }
	inline int32_t getRendersSick() const { return m_renders_sick; }

	inline int getRunTasks() const { return m_run_tasks; }
	inline int getRunCapacity() const { return m_run_capacity; }

	inline int64_t getTaskStartFinishTime() const { return m_task_start_finish_time; }

	inline int getSickErrorsCount() const { return m_sick_errors_count; }

  public:
	enum State
	{
		SBusy = 1ULL << 0,
		SPaused = 1ULL << 1,
	};

	int32_t m_heartbeat_sec;
	int32_t m_resources_update_period;
	int32_t m_zombie_time;
	int32_t m_exit_no_task_time;
	int32_t m_no_task_event_time;
	int32_t m_overload_event_time;

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
	static const std::string FilterName(const std::string &i_name);

  protected:
	RegExp m_pattern;

	std::string m_parent_path;

	int64_t m_time_creation;

	bool m_new_nimby;
	bool m_new_paused;

	int32_t m_sick_errors_count;

	int32_t m_pools_total;
	int32_t m_renders_total;
	int32_t m_renders_busy;
	int32_t m_renders_ready;
	int32_t m_renders_online;
	int32_t m_renders_offline;
	int32_t m_renders_nimby;
	int32_t m_renders_paused;
	int32_t m_renders_sick;

	int32_t m_run_tasks;
	int32_t m_run_capacity;

	int64_t m_task_start_finish_time; ///< Task start or finish time.

  private:
	void initDefaultValues();

  protected:
	void v_readwrite(Msg *msg); ///< Read or write Pool in message.
};
} // namespace af
