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
	poolscontainer.h - Job pools server container.
*/
#pragma once

#include "afcontainer.h"
#include "afcontainerit.h"
#include "aflist.h"
#include "poolsrv.h"

class Action;
class MsgAf;
class RenderAf;

/// Users container.
class PoolsContainer: public AfContainer
{
public:
	PoolsContainer();
	~PoolsContainer();

	void addRootPool();

	/// Add user, called when job registering, if user with this name exists it's hostname set to \c hostname only.
	PoolSrv * addPoolFromPath(const std::string & i_path, MonitorContainer * i_monitors);

	/// Add user, called on start with user created from batadase
	bool addPoolFromStore(PoolSrv * i_pool);

	PoolSrv * getPool(const std::string & i_path);

	PoolSrv * getRootPool() {return m_root_pool;}

private:
	PoolSrv * m_root_pool;
	int addPoolToContainer(PoolSrv * i_pool);
};

//########################## Iterator ##############################

/// Users iterator.
class PoolsContainerIt: public AfContainerIt
{
public:
	PoolsContainerIt(PoolsContainer * i_container, bool i_skipZombies = true);
	~PoolsContainerIt();

	inline PoolSrv * pool() { return (PoolSrv*)(getNode()); }
	inline PoolSrv * getPool(int id) { return (PoolSrv*)(get(id)); }

private:
};

