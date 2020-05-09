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
	Branch can contain jobs and other branches. Branch is designed to control a pack of jobs.
*/

#include "../include/afanasy.h"

#include "branch.h"
#include "job.h"
#include "user.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

using namespace af;

Branch::Branch(const std::string &i_path)
{
	initDefaultValues();
	m_name = i_path;
}

Branch::Branch(Msg *msg)
{
	initDefaultValues();
	read(msg);
}

Branch::Branch(int i_id)
{
	initDefaultValues();
	m_id = i_id;
}

void Branch::initDefaultValues()
{
	m_flags_branch = 0;

	setSolveJobs(false);
	setSolvePriority();
	setSolveTasksNum();

	m_branches_num = 0;
	m_branches_total = 0;
	m_jobs_num = 0;
	m_jobs_total = 0;

	m_time_creation = 0;
	m_time_empty = 0;
}

Branch::~Branch()
{
}

void Branch::v_readwrite(Msg *msg)
{
	Node::v_readwrite(msg);
	Work::readwrite(msg);

	rw_int8_t(m_flags_branch, msg);

	rw_int64_t(m_time_creation, msg);
	rw_int64_t(m_time_empty,    msg);

	rw_String(m_parent_path, msg);

	rw_int32_t(m_branches_num, msg);
	rw_int32_t(m_branches_total, msg);
	rw_int32_t(m_jobs_num,       msg);
	rw_int32_t(m_jobs_total,     msg);

	// NEW_VERSION
	// RW Active jobs and active users.
}

void Branch::v_jsonWrite(std::ostringstream &o_str, int i_type) const
{
	o_str << "{";

	Node::v_jsonWrite(o_str, i_type);

	Work::jsonWrite(o_str, i_type);

	if (isCreateChilds()) o_str << ",\n\"create_childs\":true";
	if (isSolveJobs()) o_str << ",\n\"solve_jobs\":true";

	o_str << ",\n\"time_creation\":" << m_time_creation;
	if (m_time_empty)
		o_str << ",\n\"time_empty\":" << m_time_empty;

	o_str << ",\n\"parent\":\"" << m_parent_path << "\"";

	if (m_branches_num > 0)
		o_str << ",\n\"branches_num\":" << m_branches_num;
	if (m_branches_total > 0)
		o_str << ",\n\"branches_total\":" << m_branches_total;
	if (m_jobs_num > 0)
		o_str << ",\n\"jobs_num\":" << m_jobs_num;
	if (m_jobs_total > 0)
		o_str << ",\n\"jobs_total\":" << m_jobs_total;

	o_str << ",\n\"active_jobs\":[";
	for (std::list<Job *>::const_iterator it = m_active_jobs_list.begin(); it != m_active_jobs_list.end(); it++)
	{
		if (it != m_active_jobs_list.begin()) o_str << ",";

		o_str << "\n{";
		o_str << "\n\"id\":" << (*it)->getId();
		o_str << ",\n\"name\":\"" << (*it)->getName() << "\"";
		o_str << ",\n\"user_name\":\"" << (*it)->getUserName() << "\"";
		o_str << ",\n"; jw_state((*it)->getState(), o_str);

		if ((*it)->getRunningTasksNum() > 0)
			o_str << ",\n\"running_tasks_num\":" << (*it)->getRunningTasksNum();

		if ((*it)->getRunningCapacityTotal() > 0)
			o_str << ",\n\"running_capacity_total\":" << (*it)->getRunningCapacityTotal();

		o_str << "\n}";
	}
	o_str << "\n]";

	o_str << ",\n\"active_users\":[";
	for (std::list<BranchUserData*>::const_iterator it = m_active_users_list.begin(); it != m_active_users_list.end(); it++)
	{
		if (it != m_active_users_list.begin()) o_str << ",";

		o_str << "\n{";
		o_str << "\n\"id\":" << (*it)->user->getId();;
		o_str << ",\n\"name\":\"" << (*it)->user->getName() << "\"";

		if ((*it)->running_tasks_num > 0)
			o_str << ",\n\"running_tasks_num\":" << (*it)->running_tasks_num;

		if ((*it)->running_capacity_total > 0)
			o_str << ",\n\"running_capacity_total\":" << (*it)->running_capacity_total;

		o_str << "\n}";
	}
	o_str << "\n]";

	o_str << "\n}";
}

bool Branch::jsonRead(const JSON &i_object, std::string *io_changes)
{
	if (false == i_object.IsObject())
	{
		AF_ERR << "Branch::jsonRead: Not a JSON object.";
		return false;
	}

	Work::jsonRead(i_object, io_changes);

	bool _createChilds;
	if (jr_bool("create_childs", _createChilds, i_object, io_changes))
		setCreateChilds(_createChilds);

	bool _solveJobs;
	if (jr_bool("solve_jobs", _solveJobs, i_object, io_changes))
		setSolveJobs(_solveJobs);

	//
	// Paramers below are not editable and are read only on creation.
	if (io_changes) return true;
	// Log provided to store changes, on parameters editing.
	//

	jr_int64("time_creation", m_time_creation, i_object);
	jr_int64("time_empty",    m_time_empty,    i_object);

	jr_string("parent", m_parent_path, i_object);

	Node::jsonRead(i_object);

	m_name = FilterPath(m_name);

	return true;
}

void Branch::v_generateInfoStream(std::ostringstream &stream, bool full) const
{
	if (full)
	{
		stream << "Branch:\"" << m_name << "\" (id=" << getId() << ")";
	}
	else
	{
		stream << m_name << "[" << m_id << "]";
	}
}

const std::string Branch::FilterPath(const std::string &i_path)
{
	// Zero path will be treated as root "/"
	if (i_path.size() == 0) return "/";

	std::string o_path(i_path);

	// All slashes to UNIX
	o_path = af::strReplace(o_path, '\\', '/');

	// Split path to folders array
	std::vector<std::string> folders = af::strSplit(o_path, "/");

	// Join folders back with each folder check
	o_path.clear();
	for (int i = 0; i < folders.size(); i++)
	{
		if (folders[i].size() == 0) continue;

		if (folders[i] == ".") continue;

		if (folders[i] == "..") continue;

		folders[i] = af::pathFilterFileName(folders[i]);

		o_path += "/" + folders[i];
	}

	// Zero path will be treated as root "/"
	if (o_path.size() == 0) o_path = "/";

	return o_path;
}
