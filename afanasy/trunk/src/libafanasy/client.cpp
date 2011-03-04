#include "client.h"

#include <stdio.h>
#include <memory.h>

#include "environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Client::Client( int flags, int Id):
   time_launch( 0),
   time_register( 0),
   time_update( 0),
   revision( 0),
   address( NULL)
{
   id = Id;
   if( flags & GetEnvironment )
   {
      address = new Address( af::Environment::getAddress());
      time_launch = time(NULL);
      username = af::Environment::getUserName();
      name = af::Environment::getHostName();
      revision = af::Environment::getAfanasyBuildVersion();
      version = af::Environment::getCGRUVersion();
   }
}

Client::~Client()
{
   if( address != NULL ) delete address;
}

void Client::setRegisterTime()
{
   time_register = time( NULL);
   time_update = time_register;
}

int Client::calcWeight() const
{
   int weight = Node::calcWeight();
//printf("Client::calcWeight: Node::calcWeight: %d bytes\n", weight);
   weight += sizeof(Client) - sizeof( Node);
   weight += weigh( username);
   weight += weigh( version );
   if( address != NULL) weight += address->calcWeight();
//printf("Client::calcWeight: %d bytes ( sizeof Client = %d)\n", weight, sizeof( Client));
   return weight;
}
