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
	Server side jobs branch class.
*/
#include "branchsrv.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "action.h"
#include "afcommon.h"
#include "branchescontainer.h"
#include "jobaf.h"
#include "monitorcontainer.h"
#include "renderaf.h"
#include "solver.h"
#include "useraf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

BranchesContainer * BranchSrv::ms_branches = NULL;

BranchSrv::BranchSrv(BranchSrv * i_parent, const std::string & i_path):
	af::Branch(i_path),
	m_parent(i_parent),
	AfNodeSolve(this)
{
	appendBranchLog("Created from job.");
}

BranchSrv::BranchSrv(const std::string & i_store_dir):
	af::Branch(),
	m_parent(NULL),
	AfNodeSolve(this, i_store_dir)
{
	int size;
	char * data = af::fileRead(getStoreFile(), &size);
	if (data == NULL) return;

	rapidjson::Document document;
	char * res = af::jsonParseData(document, data, size);
	if (res == NULL)
	{
		delete [] data;
		return;
	}

	if (jsonRead(document))
		setStoredOk();

	delete [] res;
	delete [] data;
}

void BranchSrv::setParent(BranchSrv * i_parent)
{
	if (NULL != m_parent)
	{
		AF_ERR << "BranchSrv::setParent: Branch['" << m_name << "'] already has a parent.";
		return;
	}

	if (isRoot())
	{
		AF_ERR << "BranchSrv::setParent: Root branch should not have any parent.";
		return;
	}

	m_parent = i_parent;
}

bool BranchSrv::initialize()
{
	// Non root branch should have a parent
	if (isNotRoot() && (NULL == m_parent))
	{
		AF_ERR << "BranchSrv::initialize: Branch['" << m_name << "'] has NULL parent.";
		return false;
	}

	if (NULL != m_parent)
	{
		m_parent_path = m_parent->getName();
		m_parent->addBranch(this);
	}

	if (isFromStore())
	{
		if (m_time_creation == 0)
		{
			m_time_creation = time(NULL);
			store();
		}
		appendBranchLog("Initialized from store.", false);
	}
	else
	{
		if (NULL == m_parent)
		{
			// The root branch is just created for the first time (not from store)
			setCreateChilds(true);
			setSolveJobs(true);
			m_max_tasks_per_second = AFBRANCH::TASKSPERSECOND_ROOT;
			AF_LOG << "Root branch constructed.";
		}
		else
		{
			// This is a new non-root branch.
			// We should inherit some parameters from parent branch.
			if (m_parent->isSolveCapacity()) setSolveCapacity();
			if (m_parent->isSolveTasksNum()) setSolveTasksNum();

			if (m_parent->isSolvePriority()) setSolvePriority();
			if (m_parent->isSolveOrder())    setSolveOrder();

			if (m_parent->isNotRoot())
			{
				setSolveJobs(m_parent->isSolveJobs());
			}
		}

		m_time_creation = time(NULL);

		setStoreDir(AFCommon::getStoreDirBranch(*this));
		store();
		appendBranchLog("Initialized.", false);
	}

	return true;
}

BranchSrv::~BranchSrv()
{
}

void BranchSrv::v_action(Action & i_action)
{
	i_action.log.type = "branches";

	const JSON & operation = (*i_action.data)["operation"];
	if (operation.IsObject())
	{
		std::string type;
		af::jr_string("type", type, operation);

		if (type == "delete")
		{
			deleteBranch(i_action, i_action.monitors);
			return;
		}
		else if (type == "delete_done_jobs")
		{
			deleteDoneJobs(i_action, i_action.monitors);
		}
		else if (type == "pause")
		{
			if (isPaused())
				return;
			setPaused(true);
		}
		else if (type == "start")
		{
			if (false == isPaused())
				return;
			setPaused(false);
		}
		else
		{
			i_action.answerError("Unknown operation '" + type + "'");
			return;
		}

		//appendLog("Operation \"" + type + "\" by " + i_action.author);
		i_action.monitors->addEvent(af::Monitor::EVT_branches_change, m_id);
		store();
		return;
	}

	const JSON & params = (*i_action.data)["params"];
	if (params.IsObject())
		jsonRead(params, &i_action.log.info);

	if (i_action.log.info.size())
	{
		store();
		i_action.answerLog("Branch(es) parameter(s) changed.");
		i_action.monitors->addEvent(af::Monitor::EVT_branches_change, m_id);
	}
}

void BranchSrv::deleteBranch(Action & o_action, MonitorContainer * i_monitoring)
{
	if (isRoot())
	{
		o_action.answerError("Can`t delete ROOT branch.");
		return;
	}

	if (m_branches_total || m_jobs_total)
	{
		o_action.answerError("Branch['" + m_name + "'] has child branches/jobs.");
		return;
	}

	//appendLog(std::string("Deleted by ") + o_action.author);
	setZombie();

	if (i_monitoring)
		i_monitoring->addEvent(af::Monitor::EVT_branches_del, m_id);
}

void BranchSrv::deleteDoneJobs(Action & o_action, MonitorContainer * i_monitoring)
{
	std::list<JobAf*> jobs;
	AfListIt jIt(&m_jobs_list);
	for (AfNodeSrv * node = jIt.node(); node != NULL; jIt.next(), node = jIt.node())
	{
		JobAf * job = static_cast<JobAf*>(node);
		if (job->isDone())
			jobs.push_back(job);
	}

	o_action.answerLog("Deleting " + af::itos(jobs.size()) + " jobs.");

	for (auto & job : jobs)
		job->deleteNode(NULL, i_monitoring);

	return;
}

void BranchSrv::addBranch(BranchSrv * i_branch)
{
	appendBranchLog(std::string("Adding a branch: ") + i_branch->getName(), false);

	m_branches_list.add(i_branch);
}

void BranchSrv::removeBranch(BranchSrv * i_branch)
{
	appendBranchLog(std::string("Removing a branch: ") + i_branch->getName(), false);

	m_branches_list.remove(i_branch);
}

void BranchSrv::addJob(JobAf * i_job, UserAf * i_user)
{
	if (m_jobs_list.has(i_job))
	{
		AF_ERR << "Branch[" << getName() << "] already has a job[" << i_job->getName() << "]";
		return;
	}

	appendBranchLog(std::string("Adding a job: ") + i_job->getName(), false);

	m_jobs_list.add(i_job);

	i_job->setBranch(this);

	// Add solve counts, this function will add it to all parents
	// (running tasts number, capacity total, renders counts for max tasks per host)
	addJobCounts(*i_job);

	// Add job to user (create new branch user if not exists)
	addUserJob(i_job, i_user);
}

void BranchSrv::addUserJob(JobAf * i_job, UserAf * i_user)
{
	std::map<UserAf*, BranchSrvUserData*>::iterator it = m_users.find(i_user);
	if (it == m_users.end())
		m_users[i_user] = new BranchSrvUserData(i_job);
	else
	{
		BranchSrvUserData * udata = (*it).second;
		udata->jobs.add(i_job);
		udata->running_tasks_num += i_job->getRunningTasksNum();
		udata->running_capacity_total += i_job->getRunningCapacityTotal();
	}

}

void BranchSrv::removeJob(JobAf * i_job, UserAf * i_user)
{
	appendBranchLog(std::string("Removing a job: ") + i_job->getName(), false);

	m_jobs_list.remove(i_job);

	// Remove solve counts, this function will remove it to all parents
	// (running tasts number, capacity total, renders counts for max tasks per host)
	remJobCounts(*i_job);

	// Remove job from m_users
	remUserJob(i_job, i_user);
}

void BranchSrv::remUserJob(JobAf * i_job, UserAf * i_user)
{
	// Remove job from user (and remove user if it has not any more jobs)
	std::map<UserAf*, BranchSrvUserData*>::iterator it = m_users.find(i_user);
	if (it == m_users.end())
	{
		AF_ERR << "Branch[" << getName() << "] has no user[" << i_user->getName() << "]";
		return;
	}

	BranchSrvUserData * udata = (*it).second;

	// Decrement running counts:
	udata->jobs.remove(i_job);
	udata->running_tasks_num -= i_job->getRunningTasksNum();
	udata->running_capacity_total -= i_job->getRunningCapacityTotal();

	// Check negative running counts:
	if (udata->running_tasks_num < 0)
	{
		AF_WARN << "Branch [" << getName() << "] user[" << (*it).first->getName()
			<< "] has running_tasks_num = " << udata->running_tasks_num;
		udata->running_tasks_num = 0;
	}
	if (udata->running_capacity_total < 0)
	{
		AF_WARN << "Branch [" << getName() << "] user[" << (*it).first->getName()
			<< "] has running_capacity_total = " << udata->running_capacity_total;
		udata->running_capacity_total = 0;
	}

	// Remove user data if it has no more jobs:
	if (udata->jobs.getCount() == 0)
	{
		delete udata;
		m_users.erase(it);
	}
}

void BranchSrv::changeJobUser(UserAf * i_old_user, JobAf * i_job, UserAf * i_new_user)
{
	remUserJob(i_job, i_old_user);
	addUserJob(i_job, i_new_user);
}

void BranchSrv::addJobCounts(const JobAf & i_job)
{
	// Add running counts (runnig tasks num and capacity total) to Af::Work
	addRunningCounts(i_job);

	// Add renders counts (for max tasks per host) to AfNodeSolve
	addRendersCounts(i_job);

	// Add solve counts to the parent (if any, branch can be the root)
	if (m_parent)
		m_parent->addJobCounts(i_job);
}

void BranchSrv::remJobCounts(const JobAf & i_job)
{
	// Remove running counts (runnig tasks num and capacity total) from Af::Work
	remRunningCounts(i_job);

	// Remove renders counts (for max tasks per host) from AfNodeSolve
	remRendersCounts(i_job);

	// Remove solve counts to the parent (if any, branch can be the root)
	if (m_parent)
		m_parent->remJobCounts(i_job);
}

void BranchSrv::v_refresh(time_t i_currentTime, AfContainer * i_container, MonitorContainer * i_monitoring)
{
	bool changed = false;
	bool tostore = false;

	// Init counters:
	int32_t _branches_total = 0;
	int32_t _jobs_total     = 0;
	int32_t _jobs_running   = 0;
	int32_t _jobs_done      = 0;
	int32_t _jobs_error     = 0;
	int32_t _jobs_ready     = 0;
	int32_t _tasks_ready    = 0;
	int32_t _tasks_error    = 0;

	// Store values:
	int64_t _time_empty = m_time_empty;

	// Iterate branches
	AfListIt bIt(&m_branches_list);
	for (AfNodeSrv * node = bIt.node(); node != NULL; bIt.next(), node = bIt.node())
	{
		BranchSrv * branch = static_cast<BranchSrv*>(node);

		_branches_total++;
		_branches_total += branch->m_branches_total;

		_jobs_total     += branch->m_jobs_total;
		_jobs_running   += branch->m_jobs_running;
		_jobs_done      += branch->m_jobs_done;
		_jobs_ready     += branch->m_jobs_ready;
		_jobs_error     += branch->m_jobs_error;

		_tasks_ready    += branch->m_tasks_ready;
		_tasks_error    += branch->m_tasks_error;
	}

	// Iterate jobs
	AfListIt jIt(&m_jobs_list);
	for (AfNodeSrv * node = jIt.node(); node != NULL; jIt.next(), node = jIt.node())
	{
		JobAf * job = static_cast<JobAf*>(node);

		_jobs_total++;

		if (job->isRunning()) _jobs_running++;
		if (job->isDone())    _jobs_done++;
		if (job->isReady())   _jobs_ready++;
		if (job->isError())   _jobs_error++;

		_tasks_ready += job->getTasksReady();
		_tasks_error += job->getTasksError();
	}

	// Store empty time (total jobs == 0)
	if (_jobs_total == 0)
	{
		// Store only if it was not stored later
		if (m_time_empty == 0)
			_time_empty = i_currentTime;
	}
	else
	{
		// If there is some jobs (branch not empty)
		// time empty should be zero
		_time_empty = 0;
	}

	// Compare changes
	if ((_branches_total != m_branches_total) ||

		(_jobs_total     != m_jobs_total    ) ||
		(_jobs_running   != m_jobs_running  ) ||
		(_jobs_done      != m_jobs_done     ) ||
		(_jobs_ready     != m_jobs_ready    ) ||
		(_jobs_error     != m_jobs_error    ) ||

		(_tasks_ready    != m_tasks_ready   ) ||
		(_tasks_error    != m_tasks_error   ) ||

		(_time_empty     != m_time_empty    ))
		changed = true;

	if (_time_empty != m_time_empty)
		tostore = true;

	// Store new calculations
	m_branches_total = _branches_total;

	m_jobs_total     = _jobs_total;
	m_jobs_running   = _jobs_running;
	m_jobs_done      = _jobs_done;
	m_jobs_ready     = _jobs_ready;
	m_jobs_error     = _jobs_error;

	m_tasks_ready    = _tasks_ready;
	m_tasks_error    = _tasks_error;

	m_time_empty     = _time_empty;

	// Emit events on changes
	if (changed && i_monitoring)
		i_monitoring->addEvent(af::Monitor::EVT_branches_change, m_id);

	// Store if needed
	if (tostore)
		store();
}

bool BranchSrv::v_canRun()
{
	if (m_jobs_total == 0)
	{
		// Nothing to run
		return false;
	}

	if (isPaused())
		return false;

	return true;
}

bool BranchSrv::v_canRunOn(RenderAf * i_render)
{
	return true;
}

RenderAf * BranchSrv::v_solve(std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring, BranchSrv * i_branch)
{
	std::list<AfNodeSolve*> solve_list;

	if (m_branches_list.getCount())
	{
		// Iterate child branches
		AfListIt it(&m_branches_list);
		for (AfNodeSolve * node = it.node(); node != NULL; it.next(), node = it.node())
		{
			if (false == node->canRun())
				continue;

			node->calcNeed(m_work_flags);

			solve_list.push_back(node);
		}
	}

	if (isSolveJobs() && m_jobs_list.getCount())
	{
		// Iterate child jobs
		AfListIt it(&m_jobs_list);
		for (AfNodeSolve * node = it.node(); node != NULL; it.next(), node = it.node())
		{
			if (false == node->canRun())
				continue;

			node->calcNeed(m_work_flags);

			solve_list.push_back(node);
		}
	}
	else if(m_users.size())
	{
		std::map<UserAf*,BranchSrvUserData*>::iterator it = m_users.begin();
		for(; it != m_users.end(); it++)
		{
			UserAf * user = (*it).first;
			if (false == user->canRun())
				continue;

			if (isSolveCapacity())
				user->calcNeed(-1,(*it).second->running_capacity_total);
			else
				user->calcNeed(-1,(*it).second->running_tasks_num);

			solve_list.push_back(user);
		}
	}

	Solver::SortList(solve_list, m_work_flags);

	return Solver::SolveList(solve_list, i_renders_list, this);
}

void BranchSrv::v_postSolve(time_t i_curtime, MonitorContainer * i_monitoring)
{
	// Generate a new active jobs list:
	m_active_jobs_list.clear();
	AfListIt jIt(&m_jobs_list);
	for (AfNodeSrv * node = jIt.node(); node != NULL; jIt.next(), node = jIt.node())
	{
		JobAf * job = (JobAf*)node;
		if(job->isDone())
			continue;
		m_active_jobs_list.push_back(job);
	}


	// Clear active users (and delete users data)
	for (std::list<BranchUserData*>::iterator it = m_active_users_list.begin(); it != m_active_users_list.end(); it++)
		delete (*it);
	m_active_users_list.clear();

	// Generate a new active users list:
	std::map<UserAf*, BranchSrvUserData*>::iterator uIt = m_users.begin();
	for (; uIt != m_users.end(); uIt++)
	{
		if ((*uIt).second->running_tasks_num == 0)
			continue;

		m_active_users_list.push_back(
			new BranchUserData((*uIt).first, (*uIt).second->running_tasks_num, (*uIt).second->running_capacity_total));
	}
}

void BranchSrv::addSolveCounts(MonitorContainer * i_monitoring, af::TaskExec * i_exec, RenderAf * i_render, UserAf * i_user)
{
	if (i_user)
	{
		std::map<UserAf*, BranchSrvUserData*>::iterator it = m_users.find(i_user);
		if (it == m_users.end())
		{
			AF_ERR << "Branch [" << getName() << "] has no user[" << i_user->getName() << "]";
		}
		else
		{
			(*it).second->running_tasks_num++;
			(*it).second->running_capacity_total += i_exec->getCapResult();
		}
	}

	AfNodeSolve::addSolveCounts(i_exec, i_render);

	i_monitoring->addEvent(af::Monitor::EVT_branches_change, getId());

	if (m_parent)
		m_parent->addSolveCounts(i_monitoring, i_exec, i_render, NULL);
}

void BranchSrv::remSolveCounts(MonitorContainer * i_monitoring, af::TaskExec * i_exec, RenderAf * i_render, UserAf * i_user)
{
	if (i_user)
	{
		std::map<UserAf*, BranchSrvUserData*>::iterator it = m_users.find(i_user);
		if (it == m_users.end())
		{
			AF_ERR << "Branch [" << getName() << "] has no user[" << i_user->getName() << "]";
		}
		else
		{
			BranchSrvUserData * udata = (*it).second;
			udata->running_tasks_num--;
			udata->running_capacity_total -= i_exec->getCapResult();

			if (udata->running_tasks_num < 0)
			{
				AF_WARN << "Branch [" << getName() << "] user[" << i_user->getName()
					<< "] has running_tasks_num = " << udata->running_tasks_num;
				udata->running_tasks_num = 0;
			}
			if (udata->running_capacity_total < 0)
			{
				AF_WARN << "Branch [" << getName() << "] user[" << i_user->getName()
					<< "] has running_capacity_total = " << udata->running_capacity_total;
				udata->running_capacity_total = 0;
			}
		}
	}

	AfNodeSolve::remSolveCounts(i_exec, i_render);

	i_monitoring->addEvent(af::Monitor::EVT_branches_change, getId());

	if (m_parent)
		m_parent->remSolveCounts(i_monitoring, i_exec, i_render, NULL);
}


int BranchSrv::v_calcWeight() const
{
	int weight = af::Branch::v_calcWeight();
	weight += sizeof(BranchSrv) - sizeof(af::Branch);
	return weight;
}

