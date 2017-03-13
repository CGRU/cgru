#pragma once

#include "../libafanasy/afwork.h"
#include "../libafanasy/name_af.h"

class AfNodeSolve;
class JobContainer;
class MonitorContainer;
class RenderAf;
class RenderContainer;
class UserContainer;

class Solver
{
public:
	Solver(
			JobContainer     * i_jobcontainer,
			RenderContainer  * i_rendercontainer,
			UserContainer    * i_usercontainer,
			MonitorContainer * i_monitorcontainer
		);

	~Solver();

	void solve();

	static RenderAf * SolveList( std::list<AfNodeSolve*> & i_list, std::list<RenderAf*> & i_renders, af::Work::SolvingMethod i_method);

private:
	static JobContainer     * ms_jobcontainer;
	static RenderContainer  * ms_rendercontainer;
	static UserContainer    * ms_usercontainer;
	static MonitorContainer * ms_monitorcontaier;

	static int ms_solve_cycles_limit;
	static int ms_awaken_renders;
};

