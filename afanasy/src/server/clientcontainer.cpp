#include "clientcontainer.h"

#include <stdio.h>
#include <stdlib.h>

#include "../libafanasy/address.h"
#include "../libafanasy/msgqueue.h"

#include "afcommon.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ClientContainer::ClientContainer( std::string ContainerName, int MaximumSize):
   AfContainer( ContainerName, MaximumSize)
{
}

ClientContainer::~ClientContainer()
{
AFINFO("ClientContainer::~ClientContainer:\n");
}

int ClientContainer::addClient( AfNodeSrv * i_nodesrv, bool deleteSameAddress, MonitorContainer * monitoring, int msgEventType)
{
   int id = 0;

	if( i_nodesrv == NULL )
   {
      AFERROR("ClientContainer::addClient: Client is NULL (was not allocated).\n ");
      return 0;
   }

//printf("ClientContainer::addClient: newCLient = %p, newClient->m_node = %p\n", i_nodesrv, i_nodesrv->m_node);
	af::Client * newClient = (af::Client*)(i_nodesrv->node());

	if( newClient->isOnline())
	{
//      ClientContainerIt clientsIt( this);
		AfContainerIt nodesIt( this);
//af::Client *client = clientsIt.Client();
//client != NULL;
//clientsIt.next();
//client = clientsIt.Client();
//printf("###############\n");
//      for( af::Client *client = clientsIt.Client(); client != NULL; clientsIt.next(), client = clientsIt.Client())
		for( AfNodeSrv * node = nodesIt.getNode(); node != NULL; nodesIt.next(), node = nodesIt.getNode())
		{
			af::Client * client = (af::Client*)(node->node());
			if( newClient->addrEqual( client))
			{
				if( deleteSameAddress)
				{
					node->v_setZombie();
					if( monitoring ) monitoring->addEvent( msgEventType, client->getId());
				}
				else
				{
					AFCommon::QueueLogError( std::string("Client with this address already exists: ") + newClient->getAddress().v_generateInfoString());
					AFINFO( std::string("Client with this address already exists: ") + newClient->getAddress().v_generateInfoString());
					delete newClient;
					AFINFO("ClientContainer::addClient: Client deleted. Returning zero...");
					return 0;
				}
			}
		}
	}

   id = add( i_nodesrv);
   AFINFA("ClientContainer::addClient: id = %d.", id);
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
//   ClientContainerIt clientsIt( this);
//   af::Client* client = clientsIt.getClient( id);
	AfContainerIt nodesIt( this);
	AfNodeSrv * node = nodesIt.get( id);
	if( node == NULL ) return false;
	af::Client * client = (af::Client*)(node->node());
	if( client == NULL )
	{
		AFERROR("ClientContainer::updateId: node is NULL")
		return false;
	}

   client->updateTime();
   return true;
}
/*
//##############################################################################
ClientContainerIt::ClientContainerIt( ClientContainer* container, bool skipZombies):
   AfContainerIt( (AfContainer*)container, skipZombies)
{
}

ClientContainerIt::~ClientContainerIt()
{
}*/
