#pragma once

#include "afcontainer.h"
#include "afcontainerit.h"
#include "renderaf.h"

#include "../libafanasy/msgclasses/mctaskup.h"

class PoolsContainer;

/// Renders container.
class RenderContainer : public AfContainer
{
  public:
	RenderContainer();
	~RenderContainer();

	/// Add new Render to container, new id returned on success, else return 0.
	af::Msg *addRender(RenderAf *newRender, PoolsContainer *i_pools, JobContainer *i_jobs,
					   MonitorContainer *monitoring);
};

/// Renders iterator.
class RenderContainerIt : public AfContainerIt
{
  public:
	RenderContainerIt(RenderContainer *m_container, bool skipZombies = true);
	~RenderContainerIt();

	inline RenderAf *render() { return (RenderAf *)(getNode()); }
	inline RenderAf *getRender(int i_id, const af::Msg *i_msg = NULL)
	{
		return static_cast<RenderAf *>(get(i_id, i_msg));
	}

  private:
};
