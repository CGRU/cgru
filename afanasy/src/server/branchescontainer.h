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
	branchescontainer.h - Job branches server container.
*/
#pragma once

#include "afcontainer.h"
#include "afcontainerit.h"
#include "aflist.h"
#include "branchsrv.h"

class Action;
class MsgAf;
class RenderAf;

/// Users container.
class BranchesContainer: public AfContainer
{
public:
	BranchesContainer();
	~BranchesContainer();

	/// Add user, called when job registering, if user with this name exists it's hostname set to \c hostname only.
	BranchSrv * addBranchFromPath(const std::string & i_path, MonitorContainer * i_monitors);

	/// Add user, called on start with user created from batadase
	bool addBranchFromStore(BranchSrv * i_branch);

	BranchSrv * getBranch(const std::string & i_path);

	BranchSrv * getRootBranch() {return m_root_branch;}

private:
	BranchSrv * m_root_branch;
	int addBranchToContainer(BranchSrv * i_branch);
};

//########################## Iterator ##############################

/// Users iterator.
class BranchesContainerIt: public AfContainerIt
{
public:
	BranchesContainerIt(BranchesContainer * i_container, bool i_skipZombies = true);
	~BranchesContainerIt();

	inline BranchSrv * branch() { return (BranchSrv*)(getNode()); }
	inline BranchSrv * getBranch(int id) { return (BranchSrv*)(get(id)); }

private:
};

