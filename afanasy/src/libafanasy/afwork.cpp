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
#include "afwork.h"

#include "environment.h"

#define AFOUTPUT
//#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

using namespace af;

Work::Work():
	m_solve_method(SolveByOrder)
{
	m_max_running_tasks = af::Environment::getMaxRunningTasksNumber();
	m_max_running_tasks_per_host = -1;

	m_hosts_mask.setCaseInsensitive();
	m_hosts_mask_exclude.setCaseInsensitive();
	m_hosts_mask_exclude.setExclude();
}

Work::~Work()
{
}

void Work::readwrite(Msg * msg)
{
	/*
	   NEW VERSION
	*/
}

void Work::jsonRead( const JSON & i_object, std::string * io_changes)
{
	jr_int32 ("max_running_tasks",          m_max_running_tasks,          i_object, io_changes);
	jr_int32 ("max_running_tasks_per_host", m_max_running_tasks_per_host, i_object, io_changes);

	jr_regexp("hosts_mask",         m_hosts_mask,         i_object, io_changes);
	jr_regexp("hosts_mask_exclude", m_hosts_mask_exclude, i_object, io_changes);

	jr_intmap("pools",              m_pools,              i_object, io_changes);

	std::string solve_method;
	jr_string("solve_method", solve_method, i_object, io_changes);
	if (solve_method == "priority")
		m_solve_method = SolveByPriority;
	else
		m_solve_method = SolveByOrder;
}

void Work::jsonWrite( std::ostringstream & o_str, int i_type) const
{
	if (m_max_running_tasks != -1)
		o_str << ",\n\"max_running_tasks\":" << m_max_running_tasks;
	if( m_max_running_tasks_per_host != -1 )
		o_str << ",\n\"max_running_tasks_per_host\":" << m_max_running_tasks_per_host;

	if (hasHostsMask())
		o_str << ",\n\"hosts_mask\":\""  << af::strEscape(m_hosts_mask.getPattern()) << "\"";
	if (hasHostsMaskExclude())
		o_str << ",\n\"hosts_mask_exclude\":\""  << af::strEscape(m_hosts_mask_exclude.getPattern()) << "\"";

	if (m_pools.size())
		af::jw_intmap("pools", m_pools, o_str);

	o_str << ",\n\"solve_method\":\"";
	switch (m_solve_method)
	{
		case SolveByOrder:    o_str << "order";    break;
		case SolveByPriority: o_str << "priority"; break;
	}
	o_str << "\"";
}

void Work::generateInfoStream( std::ostringstream & o_str, bool full) const
{
	if (false == full)
		return;

	if (m_pools.size())
	{
		o_str << "\nPools:";
		for (std::map<std::string,int32_t>::const_iterator it = m_pools.begin(); it != m_pools.end(); it++)
		{
			if (it != m_pools.begin())
				o_str << ",";
			o_str << " \"" << (*it).first << "\": "<< (*it).second;
		}
	}
}

int Work::calcWeight() const
{
	int weight = Node::v_calcWeight();

	weight += sizeof(Work) - sizeof( Node);

	weight += m_hosts_mask.weigh();
	weight += m_hosts_mask_exclude.weigh();

	weight += weigh(m_pools);

	return weight;
}
