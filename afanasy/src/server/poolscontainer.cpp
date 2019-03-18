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
	Job pools server container.
*/
#include "poolscontainer.h"

#include <stdio.h>
#include <string.h>

#include "../include/afanasy.h"

#include "../libafanasy/msgclasses/mcafnodes.h"

#include "action.h"
#include "afcommon.h"
#include "monitorcontainer.h"
#include "renderaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

PoolsContainer::PoolsContainer():
	m_root_pool(NULL),
	AfContainer("Pools", AFBRANCH::MAXCOUNT)
{
	PoolSrv::setPoolsContainer(this);
}

PoolsContainer::~PoolsContainer()
{
AFINFO("PoolsContainer::~PoolsContainer:\n");
}

void PoolsContainer::addRootPool()
{
	if (m_root_pool)
		return;

	m_root_pool = new PoolSrv(NULL,"/");

	if (addPoolToContainer(m_root_pool) == 0)
	{
		AF_ERR << "Can't add root pool to container.";
		delete m_root_pool;
		m_root_pool = NULL;
		return;
	}

	AF_LOG << "Root pool created: " << m_root_pool;
}

PoolSrv * PoolsContainer::addPoolFromPath(const std::string & i_path, MonitorContainer * i_monitors)
{
//AF_DEBUG << i_path;
//Filter pool path
//std::string path = Pool::FilterPath(i_path);

	// Look for an existing pool
	PoolSrv * pool = getPool(i_path);
	if (pool)
		return pool;

	// Look for a parent pool if it is not the root
	PoolSrv * parent = NULL;
	if (i_path != "/")
	{
		std::string up_path = af::pathUp(i_path);
		if (up_path != i_path)
			parent = addPoolFromPath(up_path, i_monitors);
		if (NULL == parent)
		{
			AF_ERR << "Can't process parent pool:\n" << i_path << "\n" << up_path;
			delete pool;
			return NULL;
		}
	}

	// Create new Pool if parent can,
	// or we should to create the first root pool
	if ((i_path == "/")/* || parent->isCreateChilds()*/)
	{
		pool = new PoolSrv(parent, i_path);
		if (addPoolToContainer(pool) == 0)
		{
			AF_ERR << "Can't add pool to container: " << i_path;
			delete pool;
			return NULL;
		}

		AFCommon::QueueLog("New pool registered: " + pool->v_generateInfoString(false));

		// Store root pool if it was not:
		if ((m_root_pool == NULL) && (i_path == "/"))
		{
			m_root_pool = pool;
			AF_LOG << "Root pool created.";
		}

		if (i_monitors)
			i_monitors->addEvent(af::Monitor::EVT_pools_add, pool->getId());
	}
	else
		pool = parent;

	return pool;
}

bool PoolsContainer::addPoolFromStore(PoolSrv * i_pool)
{
	std::string path = i_pool->getName();
	if (path != "/")
	{
		std::string up_path = af::pathUp(path);
		PoolSrv * parent = getPool(up_path);
		if (NULL == parent)
		{
			AF_ERR << "Can't process parent of a stored pool:\n" << path << "\n" << up_path;
			return false;
		}
		i_pool->setParent(parent);
	}
	else if (m_root_pool == NULL)
	{
		// Store root pool if it was not:
		m_root_pool = i_pool;
		AF_LOG << "Root pool created from store.";
	}

	return (addPoolToContainer(i_pool) != 0);
}

int PoolsContainer::addPoolToContainer(PoolSrv * i_pool)
{
	// Add node to container
	if (false == add(i_pool))
		return 0;

	// Initialize pool
	if (false == i_pool->initialize())
		return 0;

	return i_pool->getId();
}

PoolSrv * PoolsContainer::getPool(const std::string & i_path)
{
	PoolsContainerIt it(this);
	for (PoolSrv * pool = it.pool(); pool != NULL; it.next(), pool = it.pool())
		if (pool->getName() == i_path)
			return pool;
	return NULL;
}

//############################################################################
//                               PoolsContainerIt
//############################################################################

PoolsContainerIt::PoolsContainerIt(PoolsContainer * i_container, bool i_skipZombies):
	AfContainerIt((AfContainer*)i_container, i_skipZombies)
{
}

PoolsContainerIt::~PoolsContainerIt()
{
}
