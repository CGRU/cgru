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

	m_max_run_tasks = -1;
	m_max_run_tasks_per_host = -1;
	m_task_start_finish_time = 0;
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
	o_str << ",\n\"task_start_finish_time\":" << m_task_start_finish_time;
	if (m_max_run_tasks >= 0)
		o_str << ",\n\"max_run_tasks\":" << m_max_run_tasks;
	if (m_max_run_tasks_per_host >= 0)
		o_str << ",\n\"max_run_tasks_per_host\":" << m_max_run_tasks_per_host;


	// We do not need to store host on hdd,
	// it will be taken from farm setup when online.
/*	if (i_type != 0)
	{
		o_str << ",\n";
		m_host.jsonWrite(o_str);
	}
*/

	if (m_services_disabled.size())
	{
		o_str << ",\n\"services_disabled\":[";
		for (int i = 0; i < m_services_disabled.size(); i++)
		{
			if (i) o_str << ",";
			o_str << '\"' << m_services_disabled[i] << '\"';
		}
		o_str << ']';
	}

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

	jr_int32 ("max_run_tasks", m_max_run_tasks, i_object, io_changes);
	jr_int32 ("max_run_tasks_per_hsot", m_max_run_tasks_per_host, i_object, io_changes);

	bool paused;
	if (jr_bool("paused", paused, i_object, io_changes))
		setPaused(paused);

	// Paramers below are not editable and read only on creation
	// (but they can be changes by other actions, like disable service)
	// When use edit parameters, log provided to store changes
	if (io_changes)
		return true;

	Node::jsonRead(i_object);

	jr_int64("st", m_state, i_object);

	jr_stringvec("services_disabled", m_services_disabled, i_object);

	return true;
}

void Pool::v_readwrite(Msg * msg)
{
	Node::v_readwrite(msg);

	if (notRoot())
		rw_RegExp (m_pattern, msg);

	rw_int64_t(m_state,                  msg);
	rw_int64_t(m_flags,                  msg);
	rw_String (m_annotation,             msg);
	rw_String (m_parent_path,            msg);
	rw_int64_t(m_time_creation,          msg);
	rw_int32_t(m_max_run_tasks,          msg);
	rw_int32_t(m_max_run_tasks_per_host, msg);
	rw_int64_t(m_task_start_finish_time, msg);
}

int Pool::v_calcWeight() const
{
	int weight = Node::v_calcWeight();
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
