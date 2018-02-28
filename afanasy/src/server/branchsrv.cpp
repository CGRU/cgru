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
#include "jobaf.h"
#include "renderaf.h"
#include "solver.h"
#include "monitorcontainer.h"
#include "branchescontainer.h"

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
//if( m_jobs_num != 0 ) return;
			appendLog(std::string("Deleted by ") + i_action.author);
			deleteNode(i_action.monitors);
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

void BranchSrv::jobPriorityChanged(JobAf * i_job, MonitorContainer * i_monitoring)
{
	AF_DEBUG << "BranchSrv::jobPriorityChanged:";
	m_jobs_list.sortPriority(i_job);
//i_monitoring->addUser( this);
}

void BranchSrv::logAction(const Action & i_action, const std::string & i_node_name)
{
	if (i_action.log.empty())
		return;

	appendLog(std::string("Action[") + i_action.type + "][" +  i_node_name + "]: " + i_action.log);
}

void BranchSrv::deleteNode(MonitorContainer * i_monitoring)
{
	setZombie();

	if( i_monitoring ) i_monitoring->addEvent( af::Monitor::EVT_branches_del, m_id);
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

void BranchSrv::addJob(JobAf * i_job)
{
	appendLog(std::string("Adding a job: ") + i_job->getName());

	m_jobs_list.add(i_job);

	i_job->setBranch(this);

	m_jobs_num++;
}

void BranchSrv::removeJob(JobAf * i_job)
{
	appendLog(std::string("Removing a job: ") + i_job->getName());

	m_jobs_list.remove(i_job);

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

af::Msg * BranchSrv::writeJobdsOrder(bool i_binary) const
{
	if (i_binary)
	{
		af::MCGeneral ids;
		ids.setId(getId());
		ids.setList(generateJobsIds());
		return new af::Msg(af::Msg::TUserJobsOrder, &ids);
	}


	std::vector<int32_t> jids = m_jobs_list.generateIdsList();
	std::ostringstream str;

	str << "{\"events\":{\"jobs_order\":{\"uids\":[";
	str << getId();
	str << "],\"jids\":[[";
	for (int j = 0; j < jids.size(); j++)
	{
		if (j > 0) str << ",";
		str << jids[j];
	}
	str << "]]}}}";

	return af::jsonMsg(str);
}

void BranchSrv::v_refresh(time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
	bool changed = false;

	std::list<af::Job*> _active_jobs_list;

	int32_t _running_tasks_num = 0;
	int64_t _running_capacity_total = 0;

	AfListIt it(&m_jobs_list);
	for (AfNodeSrv *node = it.node(); node != NULL; it.next(), node = it.node())
	{
		JobAf * job = (JobAf*)node;

		if(job->getRunningTasksNum() == 0)
			continue;

		_running_tasks_num += job->getRunningTasksNum();
		_running_capacity_total += job->getRunningCapacityTotal();

		_active_jobs_list.push_back((af::Job*)job);
	}

	if ((_running_capacity_total != m_running_capacity_total) ||
		(     _running_tasks_num != m_running_tasks_num     ) ||
		(      _active_jobs_list != m_active_jobs_list      ))
		changed = true;

	m_running_tasks_num      = _running_tasks_num;
	m_running_capacity_total = _running_capacity_total;
	m_active_jobs_list       = _active_jobs_list;

	if (changed && monitoring)
		monitoring->addEvent(af::Monitor::EVT_branches_change, m_id);

	// Update solving parameters:
//	v_calcNeed();
}

void BranchSrv::v_calcNeed()
{
}

bool BranchSrv::v_canRun()
{
	return true;
}

bool BranchSrv::v_canRunOn(RenderAf * i_render)
{
	return true;
}

RenderAf * BranchSrv::v_solve(std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring)
{
	// Node was not solved
	return NULL;
}

int BranchSrv::v_calcWeight() const
{
	int weight = af::Branch::v_calcWeight();
	weight += sizeof(BranchSrv) - sizeof(af::Branch);
	return weight;
}

