#include "pool.h"

#include "environment.h"
#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "logger.h"

using namespace af;

const std::string Pool::FilterName(const std::string & i_name)
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

Pool::Pool(Msg * msg)
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
	m_pattern.setCaseInsensitive();

	m_time_creation = 0;
	m_time_offline = 0;
	m_time_empty = 0;

	m_pools_num = 0;
	m_pools_total = 0;
	m_renders_num = 0;
	m_renders_total = 0;

	m_new_nimby = false;
	m_new_paused = false;

	m_run_tasks = 0;
	m_max_tasks = -1;
	m_max_tasks_per_host = -1;

	m_run_capacity = 0;
	m_max_capacity = -1;
	m_max_capacity_per_host = -1;

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

Pool::~Pool()
{
}

void Pool::v_jsonWrite(std::ostringstream & o_str, int i_type) const // Thread-safe
{
	o_str << "{";

	Node::v_jsonWrite(o_str, i_type);

	if (notRoot() && m_pattern.notEmpty())
		o_str << ",\n\"pattern\":\""   << af::strEscape(m_pattern.getPattern()) << "\"";

	o_str << ",\n\"parent\":\""       << m_parent_path << "\"";
	o_str << ",\n\"time_creation\":"  << m_time_creation;
	o_str << ",\n\"pools_num\":"      << m_pools_num;
	o_str << ",\n\"pools_total\":"    << m_pools_total;
	o_str << ",\n\"renders_num\":"    << m_renders_num;
	o_str << ",\n\"renders_total\":"  << m_renders_total;
/*
	o_str << ",\n\"st\":" << m_state;
	o_str << ",\n";
	jw_state(m_state, o_str);
*/
	if (m_run_tasks >= 0)
		o_str << ",\n\"run_tasks\":" << m_run_tasks;
	if (m_max_tasks >= 0)
		o_str << ",\n\"max_tasks\":" << m_max_tasks;
	if (m_max_tasks_per_host >= 0)
		o_str << ",\n\"max_tasks_per_host\":" << m_max_tasks_per_host;

	if (m_run_capacity >= 0)
		o_str << ",\n\"run_capacity\":" << m_run_capacity;
	if (m_max_capacity >= 0)
		o_str << ",\n\"max_capacity\":" << m_max_capacity;
	if (m_max_capacity_per_host >= 0)
		o_str << ",\n\"max_capacity_per_host\":" << m_max_capacity_per_host;

	o_str << ",\n\"task_start_finish_time\":" << m_task_start_finish_time;


	if (m_new_nimby)
		o_str << ",\n\"new_nimby\": true";
	if (m_new_paused)
		o_str << ",\n\"new_paused\": true";

	Farm::jsonWrite(o_str, i_type);

	if (m_idle_wolsleep_time >= 0) o_str << ",\n\"idle_wolsleep_time\":" << m_idle_wolsleep_time;
	if (m_idle_free_time     >= 0) o_str << ",\n\"idle_free_time\":"     << m_idle_free_time;
	if (m_busy_nimby_time    >= 0) o_str << ",\n\"busy_nimby_time\":"    << m_busy_nimby_time;
	if (m_idle_cpu           >= 0) o_str << ",\n\"idle_cpu\":"           << m_idle_cpu;
	if (m_busy_cpu           >= 0) o_str << ",\n\"busy_cpu\":"           << m_busy_cpu;
	if (m_idle_mem           >= 0) o_str << ",\n\"idle_mem\":"           << m_idle_mem;
	if (m_busy_mem           >= 0) o_str << ",\n\"busy_mem\":"           << m_busy_mem;
	if (m_idle_swp           >= 0) o_str << ",\n\"idle_swp\":"           << m_idle_swp;
	if (m_busy_swp           >= 0) o_str << ",\n\"busy_swp\":"           << m_busy_swp;
	if (m_idle_hddgb         >= 0) o_str << ",\n\"idle_hddgb\":"         << m_idle_hddgb;
	if (m_busy_hddgb         >= 0) o_str << ",\n\"busy_hddgb\":"         << m_busy_hddgb;
	if (m_idle_hddio         >= 0) o_str << ",\n\"idle_hddio\":"         << m_idle_hddio;
	if (m_busy_hddio         >= 0) o_str << ",\n\"busy_hddio\":"         << m_busy_hddio;
	if (m_idle_netmbs        >= 0) o_str << ",\n\"idle_netmbs\":"        << m_idle_netmbs;
	if (m_busy_netmbs        >= 0) o_str << ",\n\"busy_netmbs\":"        << m_busy_netmbs;

	o_str << "\n}";
}

bool Pool::jsonRead(const JSON &i_object, std::string * io_changes)
{
	if (false == i_object.IsObject())
	{
		AFERROR("Pool::jsonRead: Not a JSON object.")
		return false;
	}

	if (notRoot())
		jr_regexp("pattern", m_pattern, i_object, io_changes);

	jr_bool  ("new_nimby",             m_new_nimby,             i_object, io_changes);
	jr_bool  ("new_paused",            m_new_paused,            i_object, io_changes);

	jr_int32 ("max_tasks",             m_max_tasks,             i_object, io_changes);
	jr_int32 ("max_tasks_per_host",    m_max_tasks_per_host,    i_object, io_changes);
	jr_int32 ("max_capacity",          m_max_capacity,          i_object, io_changes);
	jr_int32 ("max_capacity_per_host", m_max_capacity_per_host, i_object, io_changes);

	jr_int32 ("idle_wolsleep_time",    m_idle_wolsleep_time,    i_object, io_changes);
	jr_int32 ("idle_free_time",        m_idle_free_time,        i_object, io_changes);
	jr_int32 ("busy_nimby_time",       m_busy_nimby_time,        i_object, io_changes);
	jr_int32 ("idle_cpu",              m_idle_cpu,              i_object, io_changes);
	jr_int32 ("busy_cpu",              m_busy_cpu,              i_object, io_changes);
	jr_int32 ("idle_mem",              m_idle_mem,              i_object, io_changes);
	jr_int32 ("busy_mem",              m_busy_mem,              i_object, io_changes);
	jr_int32 ("idle_swp",              m_idle_swp,              i_object, io_changes);
	jr_int32 ("busy_swp",              m_busy_swp,              i_object, io_changes);
	jr_int32 ("idle_hddgb",            m_idle_hddgb,            i_object, io_changes);
	jr_int32 ("busy_hddgb",            m_busy_hddgb,            i_object, io_changes);
	jr_int32 ("idle_hddio",            m_idle_hddio,            i_object, io_changes);
	jr_int32 ("busy_hddio",            m_busy_hddio,            i_object, io_changes);
	jr_int32 ("idle_netmbs",           m_idle_netmbs,           i_object, io_changes);
	jr_int32 ("busy_netmbs",           m_busy_netmbs,           i_object, io_changes);

	bool paused;
	if (jr_bool("paused", paused, i_object, io_changes))
		setPaused(paused);

	// There can`t be infinite max tasks per host on farm (on root pool).
	if (isRoot() && (m_max_tasks_per_host < 0))
		m_max_tasks_per_host = 0;

	// Paramers below are not editable and read only on creation
	// (but they can be changes by other actions, like disable service)
	// When use edit parameters, log provided to store changes
	if (io_changes)
		return true;

	Node::jsonRead(i_object);

	jr_int64("st", m_state, i_object);

	Farm::jsonRead(i_object);

	return true;
}

void Pool::v_readwrite(Msg * msg)
{
	Node::v_readwrite(msg);

	if (notRoot())
		rw_RegExp (m_pattern, msg);

	// NEW_VERSION

	rw_int64_t(m_state,                  msg);
	rw_int64_t(m_flags,                  msg);
	rw_String (m_annotation,             msg);
	rw_String (m_parent_path,            msg);
	rw_int64_t(m_time_creation,          msg);

	rw_int32_t(m_run_tasks,              msg);
	rw_int32_t(m_max_tasks,              msg);
	rw_int32_t(m_max_tasks_per_host,     msg);

	rw_int32_t(m_run_capacity,           msg);
	rw_int32_t(m_max_capacity,           msg);
	rw_int32_t(m_max_capacity_per_host,  msg);

	rw_int64_t(m_task_start_finish_time, msg);
}

int Pool::v_calcWeight() const
{
	int weight = Node::v_calcWeight() + Farm::calcWeight();
	weight += sizeof(Pool) - sizeof(Node);
	return weight;
}

void Pool::v_generateInfoStream(std::ostringstream & stream, bool full) const
{
	if (full)
	{
		stream << "Pool " << m_name << " (id=" << m_id << "):";

		stream << "\n Status:";
		if (isHidden()) stream << " Hidden";
		if (isPaused()) stream << " Paused";

		stream << "\n Priority = " << int(m_priority);

		//m_host.v_generateInfoStream(stream ,full);
	}
	else
	{
		stream << "P['" << m_name << "':" << m_id << "]";
		if (isPaused()) stream << " P";
	}
}
