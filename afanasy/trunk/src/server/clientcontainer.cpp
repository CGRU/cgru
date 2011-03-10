#include "clientcontainer.h"

#include <stdio.h>
#include <stdlib.h>

#include "../libafanasy/address.h"

#include "afcommon.h"
#include "msgaf.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ClientContainer::ClientContainer( int maxcount):
   AfContainer( maxcount)
{
}

ClientContainer::~ClientContainer()
{
AFINFO("ClientContainer::~ClientContainer:\n");
}

int ClientContainer::addClient( af::Client *newClient, bool deleteSameAddress, MonitorContainer * monitoring, int msgEventType)
{
   int id = 0;

   if( newClient == NULL )
   {
      AFERROR("ClientContainer::addClient: Client is NULL (was not allocated).\n ");
      return 0;
   }

   {
      ClientContainerIt clientsIt( this);
      for( af::Client *client = clientsIt.Client(); client != NULL; clientsIt.next(), client = clientsIt.Client())
      {
         if( newClient->addrEqual( client))
         {
            if( deleteSameAddress)
            {
               client->setZombie();
               if( monitoring ) monitoring->addEvent( msgEventType, client->getId());
            }
            else
            {
               AFCommon::QueueLogError( std::string("Client with this address already exists: ") + newClient->getAddress().generateInfoString());
               delete newClient;
               return 0;
            }
         }
      }
   }

   id = add( newClient);
   if( id == 0)
   {
      delete newClient;
   }
   else
   {
      newClient->setRegisterTime();
   }

   return id;
}

bool ClientContainer::updateId( int id)
{
   ClientContainerIt clientsIt( this);
   af::Client* client = clientsIt.getClient( id);
   if( client == NULL ) return false;

   client->updateTime();
   return true;
}

//##############################################################################
ClientContainerIt::ClientContainerIt( ClientContainer* container, bool skipZombies):
   AfContainerIt( (AfContainer*)container, skipZombies)
{
}

ClientContainerIt::~ClientContainerIt()
{
}
