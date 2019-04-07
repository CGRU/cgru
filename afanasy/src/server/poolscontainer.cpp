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

void PoolsContainer::createRootPool()
{
	if (m_root_pool)
		return;

	addRootPool(new PoolSrv(NULL,"/"));

	AF_LOG << "Root pool created: " << m_root_pool;
}

bool PoolsContainer::addRootPool(PoolSrv * i_root_pool)
{
	if (m_root_pool)
	{
		AF_ERR << "Root pool already exists.";
		delete i_root_pool;
		return false;
	}

	m_root_pool = i_root_pool;

	if (addPoolToContainer(m_root_pool) == 0)
	{
		AF_ERR << "Can't add root pool to container.";
		delete m_root_pool;
		m_root_pool = NULL;
		return false;
	}

	return true;
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

bool PoolsContainer::addPoolFromStore(PoolSrv * i_pool)
{
	std::string path = i_pool->getName();

	if (path == "/")
	{
		if (NULL != m_root_pool)
		{
			AF_ERR << "An attempt to create a second root pool from store.";
			return false;
		}

		if (false == addRootPool(i_pool))
			return false;

		AF_LOG << "Root pool created from store.";
		return true;
	}

	std::string up_path = af::pathUp(path);
	PoolSrv * parent = getPool(up_path);
	if (NULL == parent)
	{
		AF_ERR << "Can't find a parent of a stored pool:\n" << path << "\n" << up_path;
		return false;
	}

	return parent->addPool(i_pool);
}

PoolSrv * PoolsContainer::getPool(const std::string & i_path)
{
	PoolsContainerIt it(this);
	for (PoolSrv * pool = it.pool(); pool != NULL; it.next(), pool = it.pool())
		if (pool->getName() == i_path)
			return pool;
	return NULL;
}

void PoolsContainer::assignRender(RenderAf * i_render)
{
	if (false == m_root_pool->assignRender(i_render))
		AF_ERR << "Can`t assign render '" << i_render->getName() << "' to any pool.";
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
