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

#include <math.h>

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
//#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

BranchesContainer * BranchSrv::ms_branches = NULL;

BranchSrv::BranchSrv(BranchSrv * i_parent, const std::string & i_path):
	af::Branch(i_path),
	m_parent(i_parent),
	AfNodeSolve(this)
{
	appendLog("Created from job.");
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

bool BranchSrv::setParent(BranchSrv * i_parent)
{
	if (NULL != m_parent)
	{
		AF_ERR << "BranchSrv::setParent: Branch['" << m_name << "'] already has a parent.";
		return false;
	}

	if (m_name == "/")
	{
		AF_ERR << "BranchSrv::setParent: Root branch should not have any parent.";
		return false;
	}

	m_parent = i_parent;
}

bool BranchSrv::initialize()
{
	// Non root branch should have a parent
	if ((m_name != "/") && (NULL == m_parent))
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
		if (m_time_creation == 0 )
		{
			m_time_creation = time(NULL);
			store();
		}
		appendLog("Initialized from store.");
	}
	else
	{
		if (NULL == m_parent)
			setCreateChilds(true);

		m_time_creation = time(NULL);
		m_time_empty = 0;

		setStoreDir(AFCommon::getStoreDirBranch(*this));
		store();
		appendLog("Initialized.");
	}

	return true;
}

BranchSrv::~BranchSrv()
{
}

void BranchSrv::v_action(Action & i_action)
{
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
	}

	const JSON & params = (*i_action.data)["params"];
	if (params.IsObject())
		jsonRead(params, &i_action.log);

	if (i_action.log.size())
	{
		store();
		i_action.monitors->addEvent(af::Monitor::EVT_branches_change, m_id);
	}
}

void BranchSrv::logAction(const Action & i_action, const std::string & i_node_name)
{
	if (i_action.log.empty())
		return;

	appendLog(std::string("Action[") + i_action.type + "][" +  i_node_name + "]: " + i_action.log);
}

void BranchSrv::deleteBranch(Action & o_action, MonitorContainer * i_monitoring)
{
	if (NULL == m_parent)
	{
		o_action.answer_kind = "error";
		o_action.answer = "Can`t delete ROOT branch.";
		return;
	}

	if (m_branches_num || m_jobs_num)
	{
		o_action.answer_kind = "error";
		o_action.answer = "Branch['" + m_name + "'] has child branches/jobs.";
		return;
	}

	appendLog(std::string("Deleted by ") + o_action.author);
	setZombie();

	if (i_monitoring)
		i_monitoring->addEvent(af::Monitor::EVT_branches_del, m_id);
}

void BranchSrv::addBranch(BranchSrv * i_branch)
{
	appendLog(std::string("Adding a branch: ") + i_branch->getName());

	m_branches_list.add(i_branch);

	m_branches_num++;
}

void BranchSrv::removeBranch(BranchSrv * i_branch)
{
	appendLog(std::string("Removing a branch: ") + i_branch->getName());

	m_branches_list.remove(i_branch);

	m_branches_num--;
}

void BranchSrv::addJob(JobAf * i_job, UserAf * i_user)
{
	if (m_jobs_list.has(i_job))
	{
		AF_ERR << "Branch[" << getName() << "] already has a job[" << i_job->getName() << "]";
		return;
	}

	appendLog(std::string("Adding a job: ") + i_job->getName());

	m_jobs_list.add(i_job);

	i_job->setBranch(this);

	// Add job to user (create new branch user if not exists)
	std::map<UserAf*, BranchSrvUserData*>::iterator it = m_users.find(i_user);
	if (it == m_users.end())
		m_users[i_user] = new BranchSrvUserData(i_job);
	else
		((*it).second)->jobs.add(i_job);

	m_jobs_num++;
}

void BranchSrv::removeJob(JobAf * i_job, UserAf * i_user)
{
	appendLog(std::string("Removing a job: ") + i_job->getName());

	m_jobs_list.remove(i_job);

	// Remove job from user (and remove user if it has not any more jobs)
	std::map<UserAf*, BranchSrvUserData*>::iterator it = m_users.find(i_user);
	if (it != m_users.end())
	{
		(*it).second->jobs.remove(i_job);
		 if ((*it).second->jobs.getCount() == 0)
		 {
			 delete (*it).second;
			 m_users.erase(it);
		 }
	}
	else
		AF_ERR << "Branch[" << getName() << "] already has no user[" << i_user->getName() << "]";

	m_jobs_num--;
}

bool BranchSrv::getJobs(std::ostringstream & o_str)
{
	AfListIt jobsListIt(&m_jobs_list);
	bool first = true;
	bool has_jobs = false;
	for (AfNodeSrv *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
	{
		if (false == first)
			o_str << ",\n";
		first = false;
		((JobAf*)(job))->v_jsonWrite(o_str, af::Msg::TJobsList);
		has_jobs = true;
	}
	return has_jobs;
}

void BranchSrv::jobsinfo(af::MCAfNodes &mcjobs)
{
	AfListIt jobsListIt(&m_jobs_list);
	for (AfNodeSrv *job = jobsListIt.node(); job != NULL; jobsListIt.next(), job = jobsListIt.node())
		mcjobs.addNode(job->node());
}

void BranchSrv::v_refresh(time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
	bool changed = false;

	int32_t _branches_num = 0;
	int32_t _branches_total = 0;
	int32_t _jobs_num = 0;
	int32_t _jobs_total = 0;

	// Iterate branches
	AfListIt bIt(&m_branches_list);
	for (AfNodeSrv * node = bIt.node(); node != NULL; bIt.next(), node = bIt.node())
	{
		BranchSrv * branch = (BranchSrv*)node;

		_branches_num++;
		_branches_total++;
		_branches_total += branch->m_branches_total;
		_jobs_total += branch->m_jobs_total;
	}

	// Iterate jobs
	AfListIt jIt(&m_jobs_list);
	for (AfNodeSrv * node = jIt.node(); node != NULL; jIt.next(), node = jIt.node())
	{
		_jobs_num++;
		_jobs_total++;
	}

	// Compare changes
	if ((_branches_num   != m_branches_num  ) ||
		(_branches_total != m_branches_total) ||
		(_jobs_num       != m_jobs_num      ) ||
		(_jobs_total     != m_jobs_total    ))
		changed = true;

	// Store new calculations
	m_branches_num   = _branches_num;
	m_branches_total = _branches_total;
	m_jobs_num       = _jobs_num;
	m_jobs_total     = _jobs_total;


	if (changed && monitoring)
		monitoring->addEvent(af::Monitor::EVT_branches_change, m_id);
}

void BranchSrv::v_calcNeed()
{
	calcNeedResouces( m_running_capacity_total);
}

bool BranchSrv::v_canRun()
{
	if (m_jobs_total == 0)
	{
		// Nothing to run
		return false;
	}

	return true;
}

bool BranchSrv::v_canRunOn(RenderAf * i_render)
{
	return true;
}

// Functor for sorting algorithm
struct GreaterNeed : public std::binary_function<AfNodeSolve*,AfNodeSolve*,bool>
{
	inline bool operator()(const AfNodeSolve * a, const AfNodeSolve * b)
	{
		return a->greaterNeed( b);
	}
};
RenderAf * BranchSrv::v_solve(std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring)
{
	std::list<AfNodeSolve*> solve_list;

	if (m_branches_num)
	{
		// Iterate child branches
		std::list<AfNodeSolve*> & list(m_branches_list.getStdList());
		for (std::list<AfNodeSolve*>::iterator it = list.begin(); it != list.end(); it++)
		{
			if (false == (*it)->canRun())
				continue;

			(*it)->v_calcNeed();

			solve_list.push_back(*it);
		}
	}

	if (m_jobs_num)
	{
		// Iterate child jobs
		std::list<AfNodeSolve*> & jList(m_jobs_list.getStdList());
		for (std::list<AfNodeSolve*>::iterator it = jList.begin(); it != jList.end(); it++)
		{
			if (false == (*it)->canRun())
				continue;

			(*it)->v_calcNeed();

			solve_list.push_back(*it);
		}
	}

	solve_list.sort( GreaterNeed());


//	std::list<AfNodeSolve*> solve_list(m_branches_list.getStdList());
//	std::list<AfNodeSolve*> jobs_list(m_jobs_list.getStdList());
	// Add jobs list to a branches list:
//	solve_list.splice(solve_list.end(), jobs_list);


	return Solver::SolveList(solve_list, i_renders_list);
}

void BranchSrv::v_postSolve(time_t i_curtime, MonitorContainer * i_monitoring)
{
	// Generate a new active jobs list:
	m_active_jobs_list.clear();
	AfListIt jIt(&m_jobs_list);
	for (AfNodeSrv * node = jIt.node(); node != NULL; jIt.next(), node = jIt.node())
	{
		JobAf * job = (JobAf*)node;
		if(job->getRunningTasksNum() == 0)
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

