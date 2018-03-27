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
	Job branches server container.
*/
#include "branchescontainer.h"

#include <stdio.h>
#include <string.h>

#include "../include/afanasy.h"

#include "../libafanasy/msgclasses/mcafnodes.h"

#include "action.h"
#include "afcommon.h"
#include "renderaf.h"
#include "monitorcontainer.h"

#define AFOUTPUT
//#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

BranchesContainer::BranchesContainer():
	m_root_branch(NULL),
	AfContainer("Branches", AFBRANCH::MAXCOUNT)
{
	BranchSrv::setBranchesContainer(this);
}

BranchesContainer::~BranchesContainer()
{
AFINFO("BranchesContainer::~BranchesContainer:\n");
}

BranchSrv * BranchesContainer::addBranchFromPath(const std::string & i_path, MonitorContainer * i_monitors)
{
AF_DEBUG << i_path;
//Filter branch path
//std::string path = Branch::FilterPath(i_path);

	// Look for an existing branch
	BranchSrv * branch = getBranch(i_path);
	if (branch)
		return branch;

	// Look for a parent branch if it is not the root
	BranchSrv * parent = NULL;
	if (i_path != "/")
	{
		std::string up_path = af::pathUp(i_path);
		if (up_path != i_path)
			parent = addBranchFromPath(up_path, i_monitors);
		if (NULL == parent)
		{
			AF_ERR << "Can't process parent branch:\n" << i_path << "\n" << up_path;
			delete branch;
			return NULL;
		}
	}

	// Create new Branch if parent can,
	// or we should to create the first root branch
	if (parent->isCreateChilds() || (i_path == "/"))
	{
		branch = new BranchSrv(parent, i_path);
		if (addBranchToContainer(branch) == 0)
		{
			AF_ERR << "Can't add branch to container: " << i_path;
			delete branch;
			return NULL;
		}

		// Store root branch if it was not:
		if ((m_root_branch == NULL) && (i_path == "/"))
		{
			m_root_branch = branch;
			AF_LOG << "Root branch created.";
		}

		if (i_monitors)
			i_monitors->addEvent(af::Monitor::EVT_branches_add, branch->getId());
	}
	else
		branch = parent;

	AFCommon::QueueLog("New branch registered: " + branch->v_generateInfoString(false));
	return branch;
}

bool BranchesContainer::addBranchFromStore(BranchSrv * i_branch)
{
	std::string path = i_branch->getName();
	if (path != "/")
	{
		std::string up_path = af::pathUp(path);
		BranchSrv * parent = getBranch(up_path);
		if (NULL == parent)
		{
			AF_ERR << "Can't process parent of a stored branch:\n" << path << "\n" << up_path;
			return false;
		}
		i_branch->setParent(parent);
	}
	else if (m_root_branch == NULL)
	{
		// Store root branch if it was not:
		m_root_branch = i_branch;
		AF_LOG << "Root branch created from store.";
	}

	return (addBranchToContainer(i_branch) != 0);
}

int BranchesContainer::addBranchToContainer(BranchSrv * i_branch)
{
	// Add node to container
	if (false == add(i_branch))
		return 0;

	// Initialize branch
	if (false == i_branch->initialize())
		return 0;

	return i_branch->getId();
}

BranchSrv * BranchesContainer::getBranch(const std::string & i_path)
{
	BranchesContainerIt it(this);
	for (BranchSrv * branch = it.branch(); branch != NULL; it.next(), branch = it.branch())
		if (branch->getName() == i_path)
			return branch;
	return NULL;
}

af::Msg* BranchesContainer::generateJobsList(int id)
{
	BranchesContainerIt it(this);
	BranchSrv* branch = it.getBranch(id);
	if (branch == NULL)
		return af::jsonMsgError("Branch with such ID does not exist.");

	af::MCAfNodes mcjobs;
	branch->jobsinfo(mcjobs);

	return new af::Msg(af::Msg::TJobsList, &mcjobs);
}

af::Msg * BranchesContainer::generateJobsList(const std::vector<int32_t> & ids, const std::string & i_type_name, bool i_json)
{
	af::MCAfNodes mcjobs;
	std::ostringstream stream;
	bool has_jobs = false;

	if (i_json)
	{
		stream << "{\"" << i_type_name << "\":[\n";
	}

	BranchesContainerIt it(this);
	std::vector<BranchSrv*> branches;
	for (int i = 0; i < ids.size(); i++)
		branches.push_back(it.getBranch(ids[i]));

	for (int i = 0; i < branches.size(); i++)
	{
		if (branches[i] == NULL) continue;
		if (i_json)
		{
			if ((i != 0) && (has_jobs))
				stream << ",\n";
			has_jobs = branches[i]->getJobs(stream);
		}
		else
			branches[i]->jobsinfo(mcjobs);
	}

	if (i_json)
	{
		stream << "\n]}";
		return af::jsonMsg(stream);
	}

	af::Msg * msg = new af::Msg();
	msg->set(af::Msg::TJobsList, &mcjobs);
	return msg;
}

//############################################################################
//                               BranchesContainerIt
//############################################################################

BranchesContainerIt::BranchesContainerIt(BranchesContainer * i_container, bool i_skipZombies):
	AfContainerIt((AfContainer*)i_container, i_skipZombies)
{
}

BranchesContainerIt::~BranchesContainerIt()
{
}
