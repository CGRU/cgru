#pragma once

#include "../libafanasy/afnode.h"
#include "../libafanasy/name_af.h"

class AfNodeSrv;
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

	static bool SolveList( std::list<AfNodeSrv*> & i_list, af::Node::SolvingMethod i_method,
		RenderAf * i_render, MonitorContainer * i_monitoring);

private:
	JobContainer     * m_jobcontainer;
	RenderContainer  * m_rendercontainer;
	UserContainer    * m_usercontainer;
	MonitorContainer * m_monitorcontaier;

};

