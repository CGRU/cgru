#pragma once

#include "affarm.h"
#include "client.h"
#include "hostres.h"
#include "name_af.h"

namespace af
{
/// Afanasy renderer client.
class Render : public Client, public Farm
{
  public:
	Render(Client::Flags i_flags = Client::DoNotGetAnyValues);

	/// Read Render data from message.
	Render(Msg *msg);

	virtual ~Render();

	void v_generateInfoStream(std::ostringstream &stream, bool full = false) const; /// Generate information.

	inline bool isOnline() const { return (m_state & SOnline); } ///< Whether Render is online.
	inline bool isBusy() const { return (m_state & SBusy); }	 ///< Whether Render is busy.
	inline bool isNIMBY() const { return (m_state & SNIMBY); }	 ///< Whether Render is NIMBY.
	inline bool isNimby() const { return (m_state & Snimby); }	 ///< Whether Render is nimby.
	inline bool isPaused() const { return (m_state & SPaused); } ///< Whether Render is paused.
	inline bool isFree() const
	{
		return false == (isNIMBY() || isNimby() || isPaused());
	} ///< Whether Render is free.
	inline bool isOffline() const { return false == (m_state & SOnline); } ///< Whether Render is offline.
	inline bool isDirty() const { return m_state & SDirty; }			   ///< Whether Render is dirty.

	inline bool isSick() const { return m_state & SSick; }
	inline bool isNotSick() const { return false == isSick(); }
	inline void setSick() { m_state |= SSick; }
	inline void unsetSick() { m_state &= ~SSick; }

	inline bool isWOLFalling() const { return m_state & SWOLFalling; }
	inline bool isWOLSleeping() const { return m_state & SWOLSleeping; }
	inline bool isWOLWaking() const { return m_state & SWOLWaking; }
	inline long long getWOLTime() const { return m_wol_operation_time; }
	inline long long getIdleTime() const { return m_idle_time; }
	inline long long getBusyTime() const { return m_busy_time; }
	inline void setWOLFalling(bool value)
	{
		if (value)
			m_state = m_state | SWOLFalling;
		else
			m_state = m_state & (~SWOLFalling);
	}
	inline void setWOLSleeping(bool value)
	{
		if (value)
			m_state = m_state | SWOLSleeping;
		else
			m_state = m_state & (~SWOLSleeping);
	}
	inline void setWOLWaking(bool value)
	{
		if (value)
			m_state = m_state | SWOLWaking;
		else
			m_state = m_state & (~SWOLWaking);
	}

	inline const std::string &getPool() const { return m_pool; }
	inline int getCapacityUsed() const { return m_capacity_used; }

	inline const HostRes &getHostRes() const { return m_hres; }

	/// Set free (unset nimby and NIMBY).
	inline void setFree()
	{
		m_state = m_state & (~Snimby);
		m_state = m_state & (~SNIMBY);
		m_busy_time = time(NULL);
	}

	/// Set Nimby
	inline void setNIMBY()
	{
		m_state = m_state | SNIMBY;
		m_state = m_state & (~Snimby);
		m_idle_time = time(NULL);
	}
	inline void setNimby()
	{
		m_state = m_state | Snimby;
		m_state = m_state & (~SNIMBY);
		m_idle_time = time(NULL);
	}
	inline void setPaused(bool set) { m_state = set ? m_state | SPaused : m_state & (~SPaused); }
	// if not to set idle time and to current, idle host with 'nimby_idlefree_time' will be set to free
	// immediately

	inline void setOnline()
	{
		m_state = m_state | SOnline;
		m_wol_operation_time = time(NULL);
	}
	inline void setOffline()
	{
		m_state = m_state & (~SOnline);
		m_wol_operation_time = time(NULL);
	}

	inline void setPriority(int value) { m_priority = value; } ///< Set priority.

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	inline long long getTasksStartFinishTime() const
	{
		return m_task_start_finish_time;
	} ///< Get tasks start or finish time.
	/// Take ownership of the task execs of the render
	std::list<af::TaskExec *> takeTasks();
	inline int getTasksNumber() const { return int(m_tasks.size()); }

	virtual void v_jsonWrite(std::ostringstream &o_str, int type) const;

	bool jsonRead(const JSON &i_object, std::string *io_changes = NULL);

	// Needed for GUI only:
	std::vector<int32_t> m_tasks_percents;

  public:
	enum State
	{
		SOnline = 1ULL << 0,
		Snimby = 1ULL << 1,
		SNIMBY = 1ULL << 2,
		SBusy = 1ULL << 3,
		SDirty = 1ULL << 4,
		SWOLFalling = 1ULL << 5,
		SWOLSleeping = 1ULL << 6,
		SWOLWaking = 1ULL << 7,
		SPaused =
			1ULL << 8,	   ///< Paused mode is a kind of "super nimby" mode that cannot be left automatically
		SSick = 1ULL << 9, ///< When render produces errors only
	};

  protected:
	inline void setBusy(bool Busy)
	{
		if (Busy)
			m_state = m_state | SBusy;
		else
			m_state = m_state & (~SBusy);
	}
	void checkDirty();

  protected:
	std::string m_pool;

	int32_t m_capacity_used;

	HostRes m_hres;

	std::list<TaskExec *> m_tasks;

	int64_t m_task_start_finish_time; ///< Task start or finish time.
	int64_t m_wol_operation_time;	  ///< Last WOL operation time (to sleep or to wake).
	int64_t m_idle_time;			  ///< Time when render became idle, no tasks and cpu < idle_cpu
	int64_t m_busy_time;			  ///< Time when render cpu became busy with no tasks and cpu > busy_cpu

  private:
	void construct();

  protected:
	void v_readwrite(Msg *msg); ///< Read or write Render in message.
};
} // namespace af
