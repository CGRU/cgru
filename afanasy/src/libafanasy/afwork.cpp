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
	af::Work is a base class for any node that can be solved (branch, user, job).
*/

#include "../include/afanasy.h"

#include "afwork.h"

#include "environment.h"
#include "taskexec.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

using namespace af;

Work::Work()
{
	m_solving_flags = 0;
	m_max_tasks_per_second = -1;

	m_max_running_tasks = af::Environment::getMaxRunningTasksNumber();
	m_max_running_tasks_per_host = -1;

	m_hosts_mask.setCaseInsensitive();
	m_hosts_mask_exclude.setCaseInsensitive();
	m_hosts_mask_exclude.setExclude();

	m_need_os.setCaseInsensitive();
	m_need_os.setContain();
	m_need_properties.setCaseSensitive();
	m_need_properties.setContain();
	m_need_power  = -1;
	m_need_memory = -1;
	m_need_hdd    = -1;

	m_running_tasks_num = 0;
	m_running_capacity_total = 0;
}

Work::~Work()
{
}

void Work::readwrite(Msg *msg)
{
	rw_int8_t(m_solving_flags, msg);

	rw_int32_t(m_max_tasks_per_second, msg);

	rw_int32_t(m_max_running_tasks,          msg);
	rw_int32_t(m_max_running_tasks_per_host, msg);

	rw_RegExp(m_hosts_mask,         msg);
	rw_RegExp(m_hosts_mask_exclude, msg);

	rw_RegExp(m_need_os,         msg);
	rw_RegExp(m_need_properties, msg);
	rw_int32_t(m_need_power,     msg);
	rw_int32_t(m_need_memory,    msg);
	rw_int32_t(m_need_hdd,       msg);

	rw_int32_t(m_running_tasks_num,      msg);
	rw_int64_t(m_running_capacity_total, msg);

	if (msg->isReading())
		m_pools.clear();
	rw_IntMap(m_pools, msg);
	if (msg->isReading())
		checkPools();
}

void Work::checkPools()
{
	if (m_pools.empty())
		return;

	std::map<std::string, int32_t>::iterator it = m_pools.begin();
	while (it != m_pools.end())
	{
		// Remove empty name and zero value pools
		if (it->first.empty() || (it->second == 0))
		{
			it = m_pools.erase(it);
			continue;
		}

		// Clamp value between -100 and 100
		if (it->second >  100) it->second =  100;
		if (it->second < -100) it->second = -100;

		it++;
	}
}

void Work::jsonRead(const JSON &i_object, std::string *io_changes)
{
	jr_int32("max_tasks_per_second", m_max_tasks_per_second, i_object, io_changes);
	if (m_max_tasks_per_second  > AFBRANCH::TASKSPERSECOND_MAX)
		m_max_tasks_per_second == AFBRANCH::TASKSPERSECOND_MAX;

	jr_int32("max_running_tasks", m_max_running_tasks, i_object, io_changes);
	jr_int32("max_running_tasks_per_host", m_max_running_tasks_per_host, i_object, io_changes);

	jr_regexp("hosts_mask",         m_hosts_mask,         i_object, io_changes);
	jr_regexp("hosts_mask_exclude", m_hosts_mask_exclude, i_object, io_changes);

	jr_regexp("need_os",         m_need_os,         i_object, io_changes);
	jr_regexp("need_properties", m_need_properties, i_object, io_changes);
	jr_int32( "need_power",      m_need_power,      i_object, io_changes);
	jr_int32( "need_memory",     m_need_memory,     i_object, io_changes);
	jr_int32( "need_hdd",        m_need_hdd,        i_object, io_changes);

	m_pools.clear();
	jr_intmap("pools", m_pools, i_object, io_changes);
	checkPools();

	std::string solve_method;
	if (jr_string("solve_method", solve_method, i_object, io_changes))
	{
		if (solve_method == "solve_priority")
			setSolvePriority();
		else
			setSolveOrder();
	}

	std::string solve_need;
	if (jr_string("solve_need", solve_need, i_object, io_changes))
	{
		if (solve_need == "solve_capacity")
			setSolveCapacity();
		else
			setSolveTasksNum();
	}
}

void Work::jsonWrite(std::ostringstream &o_str, int i_type) const
{
	if (m_max_tasks_per_second > -1)
		o_str << ",\n\"max_tasks_per_second\":" << m_max_tasks_per_second;

	if (m_max_running_tasks != -1) o_str << ",\n\"max_running_tasks\":" << m_max_running_tasks;
	if (m_max_running_tasks_per_host != -1)
		o_str << ",\n\"max_running_tasks_per_host\":" << m_max_running_tasks_per_host;

	if (hasHostsMask())
		o_str << ",\n\"hosts_mask\":\""         << af::strEscape(m_hosts_mask.getPattern()        ) << "\"";
	if (hasHostsMaskExclude())
		o_str << ",\n\"hosts_mask_exclude\":\"" << af::strEscape(m_hosts_mask_exclude.getPattern()) << "\"";
	if (hasNeedOS())
		o_str << ",\n\"need_os\":\""            << af::strEscape( m_need_os.getPattern()          ) << "\"";
	if (hasNeedProperties())
		o_str << ",\n\"need_properties\":\""    << af::strEscape( m_need_properties.getPattern()  ) << "\"";
	if (m_need_power  > 0) o_str << ",\n\"need_power\":"  << m_need_power;
	if (m_need_memory > 0) o_str << ",\n\"need_memory\":" << m_need_memory;
	if (m_need_hdd    > 0) o_str << ",\n\"need_hdd\":"    << m_need_hdd;

	if (m_pools.size()) af::jw_intmap("pools", m_pools, o_str);

	o_str << ",\n\"solve_method\":\"" << (isSolvePriority() ? "solve_priority" : "solve_order")    << "\"";
	o_str << ",\n\"solve_need\":\""   << (isSolveCapacity() ? "solve_capacity" : "solve_tasksnum") << "\"";

	if (m_running_tasks_num > 0) o_str << ",\n\"running_tasks_num\":" << m_running_tasks_num;
	if (m_running_capacity_total > 0) o_str << ",\n\"running_capacity_total\":" << m_running_capacity_total;
}

int Work::getPoolPriority(const std::string & i_pool, bool & o_canrunon) const
{
	o_canrunon = true;
	if (m_pools.empty())
		return 0;

	int priority = 0;

	bool found_some = false;
	bool found_100 = false;

	for (auto const & it : m_pools)
	{
		if (it.first.empty())
			continue;

		bool found_cur = false;

		if (it.first[0] == '/')
		{
			// This is an absolute path
			if (i_pool.find(it.first) == 0)
				found_cur = true;
		}
		else
		{
			// This is a relative path
			if (i_pool.find(it.first) != std::string::npos)
				found_cur = true;
		}

		if (found_cur)
		{
			//std::cout<<getName()<<" * "<<i_pool<<"/: "<<it.first<<':'<< it.second;
			found_some = true;

			if (it.second <= -100)
				o_canrunon = false;
			else
				priority += it.second;
			//std::cout << " - " << priority << "\n";
		}

		if (it.second >= 100)
			found_100 = true;
	}

	if (found_100 && (false == found_some))
		o_canrunon = false;

	return priority;
}

void Work::addRunTasksCounts(const TaskExec *i_exec)
{
	m_running_tasks_num++;
	m_running_capacity_total += i_exec->getCapResult();

	incrementService(i_exec->getServiceType());
}

void Work::remRunTasksCounts(const TaskExec *i_exec)
{
	m_running_tasks_num--;
	m_running_capacity_total -= i_exec->getCapResult();
	checkNegativeRunningCounts();

//	if (m_running_tasks_num == 0)
//		clearRunningServices();
//	else
		decrementService(i_exec->getServiceType());
}

void Work::checkNegativeRunningCounts()
{
	if (m_running_tasks_num < 0)
	{
		AF_ERR << getName() << "[" << getId() << "]: Tasks counter is negative: " << m_running_tasks_num;
		m_running_tasks_num = 0;
	}

	if (m_running_capacity_total < 0)
	{
		AF_ERR << getName() << "[" << getId() << "]: Tasks capacity counter is negative: " << m_running_capacity_total;
		m_running_capacity_total = 0;
	}
}

void Work::addRunningCounts(const af::Work & i_other)
{
	m_running_tasks_num += i_other.m_running_tasks_num;
	m_running_capacity_total += i_other.m_running_capacity_total;
}

void Work::remRunningCounts(const af::Work & i_other)
{
	m_running_tasks_num -= i_other.m_running_tasks_num;
	m_running_capacity_total -= i_other.m_running_capacity_total;
	checkNegativeRunningCounts();
}

void Work::generateInfoStream(std::ostringstream &o_str, bool full) const
{
	if (false == full) return;

	if (m_pools.size())
	{
		o_str << "\nPools:";
		for (std::map<std::string, int32_t>::const_iterator it = m_pools.begin(); it != m_pools.end(); it++)
		{
			if (it != m_pools.begin()) o_str << ",";
			o_str << " \"" << (*it).first << "\": " << (*it).second;
		}
	}

	o_str << "\n Hosts mask: \"" << m_hosts_mask.getPattern() << "\"";
	if (m_hosts_mask.empty()) o_str << " (any host)";
	if (m_hosts_mask_exclude.notEmpty()) o_str << "\n Exclude hosts mask: \"" << m_hosts_mask_exclude.getPattern() << "\"";

	if (m_need_os.notEmpty()        ) o_str << "\n Needed OS: \"" << m_need_os.getPattern() << "\"";
	if (m_need_properties.notEmpty()) o_str << "\n Needed properties: \"" << m_need_properties.getPattern() << "\"";
	if (m_need_memory > 0           ) o_str << "\n Needed Memory = " << m_need_memory;
	if (m_need_power  > 0           ) o_str << "\n Need Power = " << m_need_power;
	if (m_need_hdd    > 0           ) o_str << "\n Need HDD = " << m_need_hdd;
}

int Work::calcWeight() const
{
	int weight = Node::v_calcWeight();

	weight += sizeof(Work) - sizeof(Node);

	weight += m_hosts_mask.weigh();
	weight += m_hosts_mask_exclude.weigh();

	weight += m_need_os.weigh();
	weight += m_need_properties.weigh();

	weight += weigh(m_pools);

	return weight;
}
