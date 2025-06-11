#include "rendercontainer.h"

#include "../include/afanasy.h"

#include "../libafanasy/msg.h"

#include "afcommon.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

RenderContainer::RenderContainer() : AfContainer("Renders", AFRENDER::MAXCOUNT)
{
	RenderAf::setRenderContainer(this);
}

RenderContainer::~RenderContainer(){AFINFO("RenderContainer::~RenderContainer:")}

af::Msg *RenderContainer::addRender(RenderAf *newRender, PoolsContainer *i_pools, JobContainer *i_jobs,
									MonitorContainer *monitoring)
{
	// Online render register request, from client, not from database:
	if (newRender->isOnline())
	{
		RenderContainerIt rendersIt(this);
		// Search for a render with the same hostname:
		for (RenderAf *render = rendersIt.render(); render != NULL;
			 rendersIt.next(), render = rendersIt.render())
		{
			if (newRender->getName() == render->getName())
			{
				// Online render with the same hostname found:
				if (render->isOnline())
				{
					std::string errLog = "Online render with the same name exists:";
					errLog += "\nNew render:\n";
					errLog += newRender->v_generateInfoString(false);
					errLog += "\nExisting render:\n";
					errLog += render->v_generateInfoString(false);
					AFCommon::QueueLogError(errLog);
					delete newRender;

					af::RenderEvents re(af::RenderEvents::RE_Status_Exit,
										"Render with the same name exists.");
					return new af::Msg(af::Msg::TRenderEvents, &re);
				}
				// Offline render with the same hostname found:
				else
				{
					render->online(newRender, i_jobs, monitoring);
					int id = render->getId();
					AFCommon::QueueLog("Render: " + render->v_generateInfoString(false));
					delete newRender;

					return render->writeConnectedMsg("Render became online.");
				}
			}
		}

		// Registering new render, no renders with this hostname exist:
		int id = add(newRender);
		if (id != 0)
		{
			newRender->setRegistered(i_pools);
			if (monitoring)
				monitoring->addEvent(af::Monitor::EVT_renders_add, id);

			AFCommon::QueueLog("New Render registered: " + newRender->v_generateInfoString());

			return newRender->writeConnectedMsg("New render registered.");
		}

		// Server is unable to add a new render to container.
		// This should never happen.
		delete newRender;

		af::RenderEvents re(af::RenderEvents::RE_Status_Exit,
							"Unable to add render. See server log for details.");
		return new af::Msg(af::Msg::TRenderEvents, &re);
	}

	// Adding offline render from database:
	if (add(newRender))
	{
		std::cout << "Render offline registered - \"" << newRender->getName() << "\"." << std::endl;
		newRender->setRegistered(i_pools);
	}
	else
	{
		// Server is unable to add a new render to container.
		// This should never happen.
		delete newRender;
	}

	return NULL;
}

// ##############################################################################

RenderContainerIt::RenderContainerIt(RenderContainer *container, bool skipZombies)
	: AfContainerIt((AfContainer *)container, skipZombies)
{
}

RenderContainerIt::~RenderContainerIt() {}
