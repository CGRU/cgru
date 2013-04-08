#include "rendercontainer.h"

#include "../include/afanasy.h"

#include "../libafanasy/msg.h"

#include "afcommon.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

RenderContainer::RenderContainer():
   ClientContainer( "Renders", AFRENDER::MAXCOUNT)
{
}

RenderContainer::~RenderContainer()
{
AFINFO("RenderContainer::~RenderContainer:")
}

af::Msg * RenderContainer::addRender( RenderAf *newRender, MonitorContainer * monitoring)
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
            // Online render with the same hostname founded:
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
            // Offline render with the same hostname founded:
            else if( render->online( newRender, monitoring))
            {
               int id = render->getId();
               AFCommon::QueueLog("Render: " + render->v_generateInfoString( false));
               delete newRender;
               // Return new render ID to render to tell that it was successfully registered:
               return new af::Msg( af::Msg::TRenderId, id);
            }
         }
      }

      // Registering new render, no renders with this hostname exist:
      int id = addClient( newRender);
      if( id != 0 )
      {
         newRender->getFarmHost();
         if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersAdd, id);
         AFCommon::QueueLog("New Render registered: " + newRender->v_generateInfoString());
         if( newRender->isOnline()) AFCommon::QueueDBAddItem( newRender);
      }
      // Return new render ID to render to tell that it was successfully registered:
      return new af::Msg( af::Msg::TRenderId, id);
   }

   // Adding offline render from database:
   if( addClient( newRender))
   {
      std::cout << "Render offline registered - \"" << newRender->getName() << "\"." << std::endl;
      newRender->getFarmHost();
   }

   return NULL;
}

//##############################################################################

RenderContainerIt::RenderContainerIt( RenderContainer* container, bool skipZombies):
   AfContainerIt( (AfContainer*)container, skipZombies)
{
}

RenderContainerIt::~RenderContainerIt()
{
}
