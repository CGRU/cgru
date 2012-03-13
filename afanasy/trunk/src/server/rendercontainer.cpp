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
   if( newRender->isOnline())
   {
      int id = 0;
      RenderContainerIt rendersIt( this);
      for( RenderAf *render = rendersIt.render(); render != NULL; rendersIt.next(), render = rendersIt.render())
      {
         if( newRender->getName() == render->getName())
         {
            if( render->isOnline())
            {
               AFERROR("Online render with the same name exists:\n");
               printTime();
               printf("\nNew render:\n");
               render->stdOut( false);
               printf("\nExisting render:\n");
               newRender->stdOut( false);
               return new MsgAf( Msg::TRenderId, id);
            }
            else if( render->online( newRender, monitoring))
            {
               id = render->getId();
               printTime(); printf(" : Render online: "); render->stdOut( false );
               delete newRender;
               newRender = NULL;
               break;
            }
         }
      }

      if( id ) return new MsgAf( Msg::TRenderId, id);

      id = addClient( newRender);
      if( id != 0 )
      {
         newRender->getFarmHost();
         if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersAdd, id);
         printTime(); printf(" : New Render registered: "); newRender->stdOut( false );
         if( newRender->isOnline()) AFCommon::QueueDBAddItem( newRender);
      }
      return new MsgAf( Msg::TRenderId, id);
   }

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
