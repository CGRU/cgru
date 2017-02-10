#include "rendercontainer.h"

#include "../include/afanasy.h"

#include "../libafanasy/msg.h"

#include "afcommon.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

RenderContainer::RenderContainer():
	AfContainer( "Renders", AFRENDER::MAXCOUNT)
{
	RenderAf::setRenderContainer( this);
}

RenderContainer::~RenderContainer()
{
AFINFO("RenderContainer::~RenderContainer:")
}

af::Msg * RenderContainer::addRender( RenderAf *newRender, JobContainer * i_jobs, MonitorContainer * monitoring)
{
   // Online render register request, from client, not from database:
   if( newRender->isOnline())
   {
      RenderContainerIt rendersIt( this);
      // Search for a render with the same hostname:
      for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
      {
         if( newRender->getName() == render->getName())
         {
            // Online render with the same hostname found:
            if( render->isOnline())
            {
               std::string errLog = "Online render with the same name exists:";
               errLog += "\nNew render:\n";
               errLog += newRender->v_generateInfoString( false);
               errLog += "\nExisting render:\n";
               errLog += render->v_generateInfoString( false);
               AFCommon::QueueLogError( errLog);
               delete newRender;
               // Return -1 ID to render to tell that there is already registered render with the same name:
               return new af::Msg( af::Msg::TRenderId, -1);
            }
            // Offline render with the same hostname found:
            else
			{
				render->online( newRender, i_jobs, monitoring);
               int id = render->getId();
               AFCommon::QueueLog("Render: " + render->v_generateInfoString( false));
               delete newRender;
               // Return new render ID to render to tell that it was successfully registered:
               return new af::Msg( af::Msg::TRenderId, id);
            }
         }
      }

		// Registering new render, no renders with this hostname exist:
		int id = add( newRender);
		if( id != 0 )
		{
			newRender->setRegistered();
			if( monitoring )
				monitoring->addEvent( af::Monitor::EVT_renders_add, id);

			AFCommon::QueueLog("New Render registered: " + newRender->v_generateInfoString());
		}
		else
		{
			delete newRender;
		}

		// Return new render ID to render to tell that it was successfully registered:
		return new af::Msg( af::Msg::TRenderId, id);
	}

	// Adding offline render from database:
	if( add( newRender))
	{
		std::cout << "Render offline registered - \"" << newRender->getName() << "\"." << std::endl;
		newRender->setRegistered();
	}
	else
	{
		delete newRender;
	}

   return NULL;
}

bool RenderContainer::farmLoad( std::string & o_status, MonitorContainer * i_monitors)
{
	AF_LOG << "Reloading farm.";

	if( false == af::loadFarm( af::VerboseOn))
	{
		o_status = "Failed, see server logs fo details. Check farm with \"afcmd fcheck\" at first.";
		AF_ERR << o_status;

		return false;
	}

	RenderContainerIt rendersIt( this);
	for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
	{
		render->getFarmHost();

		if( i_monitors )
			i_monitors->addEvent( af::Monitor::EVT_renders_change, render->getId());
	}

	o_status = "Farm reloaded successfully.";
	AF_LOG << o_status;

	return true;
}

//##############################################################################

RenderContainerIt::RenderContainerIt( RenderContainer* container, bool skipZombies):
   AfContainerIt( (AfContainer*)container, skipZombies)
{
}

RenderContainerIt::~RenderContainerIt()
{
}
