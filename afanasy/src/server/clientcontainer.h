#pragma once

#include "../libafanasy/client.h"

#include "afcontainer.h"
#include "afcontainerit.h"

/// Clients container (abstact class, clients are Render, Talk, Monitor).
class ClientContainer : public AfContainer
{
public:
   ClientContainer( std::string ContainerName, int MaximumSize);
   ~ClientContainer();

   bool updateId( int id);

protected:
/// Add new Client to container, new id returned on success, else return 0.
//	int addClient( af::Client *newClient, bool deleteSameAddress = false, MonitorContainer * monitoring = NULL, int msgEventType = 0);
	int addClient( AfNodeSrv * i_nodesrv, bool deleteSameAddress = false, MonitorContainer * monitoring = NULL, int msgEventType = 0);

private:

};
/*
/// Clients interator.
class ClientContainerIt : public AfContainerIt
{
public:
   ClientContainerIt( ClientContainer* container, bool skipZombies = true);
   ~ClientContainerIt();

	inline af::Client* Client() { return (af::Client*)(node->m_node); }
	inline af::Client* getClient( int id) { return (af::Client*)(get( id)->m_node); }

private:
};*/
