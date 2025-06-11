#include "pool.h"

#include "environment.h"
#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "logger.h"

using namespace af;

const std::string Pool::FilterName(const std::string &i_name)
{
	std::string name;

	for (int i = 0; i < i_name.size(); i++)
	{
		char c = i_name.at(i);
		if ((c < '0') || (c > 'z'))
			c = '_';

		name.push_back(c);
	}

	return name;
}

Pool::Pool(const std::string &i_path)
{
	initDefaultValues();
	m_name = i_path;
}

Pool::Pool(Msg *msg)
{
	initDefaultValues();
	read(msg);
}

Pool::Pool(int i_id)
{
	initDefaultValues();
	m_id = i_id;
}

void Pool::initDefaultValues()
{
	m_pattern.setRegEx();
	m_pattern.setCaseIns();

	m_time_creation = 0;

	m_pools_total = 0;
	m_renders_total = 0;
	m_renders_busy = 0;
	m_renders_ready = 0;
	m_renders_online = 0;
	m_renders_offline = 0;
	m_renders_nimby = 0;
	m_renders_paused = 0;
	m_renders_sick = 0;

	m_new_nimby = false;
	m_new_paused = false;

	m_heartbeat_sec = -1;
	m_resources_update_period = -1;
	m_zombie_time = -1;
	m_exit_no_task_time = -1;
	m_no_task_event_time = -1;
	m_overload_event_time = -1;

	m_sick_errors_count = -1;

	m_run_tasks = 0;
	m_run_capacity = 0;

	m_task_start_finish_time = 0;

	m_idle_wolsleep_time = -1;
	m_idle_free_time = -1;
	m_busy_nimby_time = -1;
	m_idle_cpu = -1;
	m_busy_cpu = -1;
	m_idle_mem = -1;
	m_busy_mem = -1;
	m_idle_swp = -1;
	m_busy_swp = -1;
	m_idle_hddgb = -1;
	m_busy_hddgb = -1;
	m_idle_hddio = -1;
	m_busy_hddio = -1;
	m_idle_netmbs = -1;
	m_busy_netmbs = -1;
}

Pool::~Pool() {}

void Pool::v_jsonWrite(std::ostringstream &o_str, int i_type) const // Thread-safe
{
	o_str << "{";

	Node::v_jsonWrite(o_str, i_type);

	if (notRoot() && m_pattern.notEmpty())
		o_str << ",\n\"pattern\":\"" << af::strEscape(m_pattern.getPattern()) << "\"";

	o_str << ",\n\"parent\":\"" << m_parent_path << "\"";
	o_str << ",\n\"time_creation\":" << m_time_creation;

	o_str << ",\n\"pools_total\":" << m_pools_total;
	o_str << ",\n\"renders_total\":" << m_renders_total;
	o_str << ",\n\"renders_busy\":" << m_renders_busy;
	o_str << ",\n\"renders_ready\":" << m_renders_ready;
	o_str << ",\n\"renders_online\":" << m_renders_online;
	o_str << ",\n\"renders_offline\":" << m_renders_offline;
	o_str << ",\n\"renders_nimby\":" << m_renders_nimby;
	o_str << ",\n\"renders_paused\":" << m_renders_paused;
	o_str << ",\n\"renders_sick\":" << m_renders_sick;

	o_str << ",\n\"st\":" << m_state;
	o_str << ",\n";
	jw_statePool(m_state, o_str);

	if (m_run_tasks > 0)
		o_str << ",\n\"run_tasks\":" << m_run_tasks;
	if (m_run_capacity > 0)
		o_str << ",\n\"run_capacity\":" << m_run_capacity;
	if (m_task_start_finish_time > 0)
		o_str << ",\n\"task_start_finish_time\":" << m_task_start_finish_time;

	if (m_heartbeat_sec > 0)
		o_str << ",\n\"heartbeat_sec\":" << m_heartbeat_sec;
	if (m_resources_update_period > 0)
		o_str << ",\n\"resources_update_period\":" << m_resources_update_period;
	if (m_zombie_time > 0)
		o_str << ",\n\"zombie_time\":" << m_zombie_time;
	if (m_exit_no_task_time > 0)
		o_str << ",\n\"exit_no_task_time\":" << m_exit_no_task_time;
	if (m_no_task_event_time > 0)
		o_str << ",\n\"no_task_event_time\":" << m_no_task_event_time;
	if (m_overload_event_time > 0)
		o_str << ",\n\"overload_event_time\":" << m_overload_event_time;

	if (m_new_nimby)
		o_str << ",\n\"new_nimby\": true";
	if (m_new_paused)
		o_str << ",\n\"new_paused\": true";

	if (m_sick_errors_count >= 0)
		o_str << ",\n\"sick_errors_count\":" << m_sick_errors_count;

	Farm::jsonWrite(o_str, i_type);

	if (m_idle_wolsleep_time >= 0)
		o_str << ",\n\"idle_wolsleep_time\":" << m_idle_wolsleep_time;
	if (m_idle_free_time >= 0)
		o_str << ",\n\"idle_free_time\":" << m_idle_free_time;
	if (m_busy_nimby_time >= 0)
		o_str << ",\n\"busy_nimby_time\":" << m_busy_nimby_time;
	if (m_idle_cpu >= 0)
		o_str << ",\n\"idle_cpu\":" << m_idle_cpu;
	if (m_busy_cpu >= 0)
		o_str << ",\n\"busy_cpu\":" << m_busy_cpu;
	if (m_idle_mem >= 0)
		o_str << ",\n\"idle_mem\":" << m_idle_mem;
	if (m_busy_mem >= 0)
		o_str << ",\n\"busy_mem\":" << m_busy_mem;
	if (m_idle_swp >= 0)
		o_str << ",\n\"idle_swp\":" << m_idle_swp;
	if (m_busy_swp >= 0)
		o_str << ",\n\"busy_swp\":" << m_busy_swp;
	if (m_idle_hddgb >= 0)
		o_str << ",\n\"idle_hddgb\":" << m_idle_hddgb;
	if (m_busy_hddgb >= 0)
		o_str << ",\n\"busy_hddgb\":" << m_busy_hddgb;
	if (m_idle_hddio >= 0)
		o_str << ",\n\"idle_hddio\":" << m_idle_hddio;
	if (m_busy_hddio >= 0)
		o_str << ",\n\"busy_hddio\":" << m_busy_hddio;
	if (m_idle_netmbs >= 0)
		o_str << ",\n\"idle_netmbs\":" << m_idle_netmbs;
	if (m_busy_netmbs >= 0)
		o_str << ",\n\"busy_netmbs\":" << m_busy_netmbs;

	o_str << "\n}";
}

bool Pool::jsonRead(const JSON &i_object, std::string *io_changes)
{
	if (false == i_object.IsObject())
	{
		AFERROR("Pool::jsonRead: Not a JSON object.")
		return false;
	}

	if (notRoot())
		jr_regexp("pattern", m_pattern, i_object, io_changes);

	jr_int32("heartbeat_sec", m_heartbeat_sec, i_object, io_changes);
	jr_int32("resources_update_period", m_resources_update_period, i_object, io_changes);
	jr_int32("zombie_time", m_zombie_time, i_object, io_changes);
	jr_int32("exit_no_task_time", m_exit_no_task_time, i_object, io_changes);
	jr_int32("no_task_event_time", m_no_task_event_time, i_object, io_changes);
	jr_int32("overload_event_time", m_overload_event_time, i_object, io_changes);

	jr_bool("new_nimby", m_new_nimby, i_object, io_changes);
	jr_bool("new_paused", m_new_paused, i_object, io_changes);

	jr_int32("sick_errors_count", m_sick_errors_count, i_object, io_changes);

	jr_int32("idle_wolsleep_time", m_idle_wolsleep_time, i_object, io_changes);
	jr_int32("idle_free_time", m_idle_free_time, i_object, io_changes);
	jr_int32("busy_nimby_time", m_busy_nimby_time, i_object, io_changes);
	jr_int32("idle_cpu", m_idle_cpu, i_object, io_changes);
	jr_int32("busy_cpu", m_busy_cpu, i_object, io_changes);
	jr_int32("idle_mem", m_idle_mem, i_object, io_changes);
	jr_int32("busy_mem", m_busy_mem, i_object, io_changes);
	jr_int32("idle_swp", m_idle_swp, i_object, io_changes);
	jr_int32("busy_swp", m_busy_swp, i_object, io_changes);
	jr_int32("idle_hddgb", m_idle_hddgb, i_object, io_changes);
	jr_int32("busy_hddgb", m_busy_hddgb, i_object, io_changes);
	jr_int32("idle_hddio", m_idle_hddio, i_object, io_changes);
	jr_int32("busy_hddio", m_busy_hddio, i_object, io_changes);
	jr_int32("idle_netmbs", m_idle_netmbs, i_object, io_changes);
	jr_int32("busy_netmbs", m_busy_netmbs, i_object, io_changes);

	bool paused;
	if (jr_bool("paused", paused, i_object, io_changes))
		setPaused(paused);

	// There can`t be infinite max tasks per host on farm (on root pool).
	if (isRoot() && (m_max_tasks_host < 0))
		m_max_tasks_host = 1;

	Farm::jsonRead(i_object, io_changes);

	// Paramers below are not editable and read only on creation
	// (but they can be changes by other actions, like disable service)
	// When use edit parameters, log provided to store changes
	if (io_changes)
		return true;

	Node::jsonRead(i_object);

	jr_int64("st", m_state, i_object);

	jr_int64("time_creation", m_time_creation, i_object);

	return true;
}

void Pool::v_readwrite(Msg *msg)
{
	Node::v_readwrite(msg);
	Farm::readwrite(msg);

	if (notRoot())
		rw_RegExp(m_pattern, msg);

	rw_String(m_parent_path, msg);
	rw_int64_t(m_time_creation, msg);

	rw_int32_t(m_heartbeat_sec, msg);
	rw_int32_t(m_resources_update_period, msg);
	rw_int32_t(m_zombie_time, msg);
	rw_int32_t(m_exit_no_task_time, msg);
	rw_int32_t(m_no_task_event_time, msg);
	rw_int32_t(m_overload_event_time, msg);

	rw_bool(m_new_nimby, msg);
	rw_bool(m_new_paused, msg);

	rw_int32_t(m_sick_errors_count, msg);

	rw_int32_t(m_pools_total, msg);
	rw_int32_t(m_renders_total, msg);
	rw_int32_t(m_renders_busy, msg);
	rw_int32_t(m_renders_ready, msg);
	rw_int32_t(m_renders_online, msg);
	rw_int32_t(m_renders_offline, msg);
	rw_int32_t(m_renders_nimby, msg);
	rw_int32_t(m_renders_paused, msg);
	rw_int32_t(m_renders_sick, msg);

	rw_int32_t(m_run_tasks, msg);
	rw_int32_t(m_run_capacity, msg);

	rw_int64_t(m_task_start_finish_time, msg);

	rw_int32_t(m_idle_wolsleep_time, msg);
	rw_int32_t(m_idle_free_time, msg);
	rw_int32_t(m_busy_nimby_time, msg);

	rw_int32_t(m_idle_cpu, msg);
	rw_int32_t(m_busy_cpu, msg);
	rw_int32_t(m_idle_mem, msg);
	rw_int32_t(m_busy_mem, msg);
	rw_int32_t(m_idle_swp, msg);
	rw_int32_t(m_busy_swp, msg);
	rw_int32_t(m_idle_hddgb, msg);
	rw_int32_t(m_busy_hddgb, msg);
	rw_int32_t(m_idle_hddio, msg);
	rw_int32_t(m_busy_hddio, msg);
	rw_int32_t(m_idle_netmbs, msg);
	rw_int32_t(m_busy_netmbs, msg);
}

int Pool::v_calcWeight() const
{
	int weight = Node::v_calcWeight() + Farm::calcWeight();
	weight += sizeof(Pool) - sizeof(Node);
	return weight;
}

void Pool::v_generateInfoStream(std::ostringstream &stream, bool full) const
{
	if (full)
	{
		stream << "Pool " << m_name << " (id=" << m_id << "):";

		stream << "\n Status:";
		if (isHidden())
			stream << " Hidden";
		if (isPaused())
			stream << " Paused";

		stream << "\n Priority = " << int(m_priority);

		// m_host.v_generateInfoStream(stream ,full);
	}
	else
	{
		stream << "P['" << m_name << "':" << m_id << "]";
		if (isPaused())
			stream << " P";
	}
}
