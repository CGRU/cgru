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
	Solver class.
	Designed to encapsulate functions to solve jobs on renders.
*/
#pragma once

#include "../libafanasy/afwork.h"
#include "../libafanasy/name_af.h"

class AfNodeSolve;
class BranchesContainer;
class BranchSrv;
class JobContainer;
class MonitorContainer;
class RenderAf;
class RenderContainer;
class UserContainer;

class Solver
{
public:
	Solver(
			BranchesContainer * i_branchescontainer,
			JobContainer      * i_jobcontainer,
			RenderContainer   * i_rendercontainer,
			UserContainer     * i_usercontainer,
			MonitorContainer  * i_monitorcontainer
		);

	~Solver();

	void solve();

	static void SortList(std::list<AfNodeSolve*> & i_list, int i_solving_flags);
	static RenderAf * SolveList(std::list<AfNodeSolve*> & i_list, std::list<RenderAf*> & i_renders, BranchSrv * i_branch);

private:
	static BranchesContainer * ms_branchescontainer;
	static JobContainer      * ms_jobcontainer;
	static RenderContainer   * ms_rendercontainer;
	static UserContainer     * ms_usercontainer;
	static MonitorContainer  * ms_monitorcontaier;

	static uint64_t ms_run_cycle;
	static const int ms_solve_cycles_limit;
	static int ms_awaken_renders;
	static const int ms_awaken_renders_max;
};

