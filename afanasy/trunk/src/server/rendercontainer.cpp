#include "rendercontainer.h"

#include "../include/afanasy.h"

#include "afcommon.h"
#include "msgaf.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

RenderContainer::RenderContainer():
   ClientContainer( AFRENDER::MAXCOUNT)
{
}

RenderContainer::~RenderContainer()
{
AFINFO("RenderContainer::~RenderContainer:\n");
}

MsgAf * RenderContainer::addRender( RenderAf *newRender, MonitorContainer * monitoring)
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
               AFERROR("Online render with the same name exists:\n");
               printTime();
               printf("\nNew render:\n");
               newRender->stdOut( false);
               printf("\nExisting render:\n");
               render->stdOut( false);
               delete newRender;
               // Return -1 ID to render to tell that there is already registered render with the same name:
               return new MsgAf( Msg::TRenderId, -1);
            }
            // Offline render with the same hostname founded:
            else if( render->online( newRender, monitoring))
            {
               int id = render->getId();
               printTime(); printf(" : Render online: "); render->stdOut( false );
               delete newRender;
               // Return new render ID to render to tell that it was successfully registered:
               return new MsgAf( Msg::TRenderId, id);
            }
         }
      }

      // Registering new render, no renders with this hostname exist:
      int id = addClient( newRender);
      if( id != 0 )
      {
         newRender->getFarmHost();
         if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersAdd, id);
         printTime(); printf(" : New Render registered: "); newRender->stdOut( false );
         if( newRender->isOnline()) AFCommon::QueueDBAddItem( newRender);
      }
      // Return new render ID to render to tell that it was successfully registered:
      return new MsgAf( Msg::TRenderId, id);
   }

   // Adding offline render from database:
   if( addClient( newRender))
   {
      printf("Render offline registered - \"%s\".\n", newRender->getName().toUtf8().data());
      newRender->getFarmHost();
   }
   else delete newRender;

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
