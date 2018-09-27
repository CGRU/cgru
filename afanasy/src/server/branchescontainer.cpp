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
#undef AFOUTPUT
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
//AF_DEBUG << i_path;
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
	if ((i_path == "/") || parent->isCreateChilds())
	{
		branch = new BranchSrv(parent, i_path);
		if (addBranchToContainer(branch) == 0)
		{
			AF_ERR << "Can't add branch to container: " << i_path;
			delete branch;
			return NULL;
		}

		AFCommon::QueueLog("New branch registered: " + branch->v_generateInfoString(false));

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
