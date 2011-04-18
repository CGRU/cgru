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
   revision( 0)
{
   id = Id;
   if( flags & GetEnvironment )
   {
      bool verbose = false;
#ifdef WINNT
//      verbose = true;
#endif //WINNT
#ifdef MACOSX
//      verbose = true;
#endif //MACOSX
      af::NetIF::getNetIFs( netIFs, verbose);

      std::cout << "Network Interfaces:" << std::endl;
      for( int i = 0; i < netIFs.size(); i++)
      {
         std::cout << "   ";
         netIFs[i]->stdOut(true);
      }

      address.setPort( af::Environment::getClientPort());
      time_launch = time(NULL);
      username = af::Environment::getUserName();
      name = af::Environment::getHostName();
      revision = af::Environment::getAfanasyBuildVersion();
      version = af::Environment::getCGRUVersion();
   }
}

Client::~Client()
{
   clearNetIFs();
}

void Client::clearNetIFs()
{
   for( int i = 0; i < netIFs.size(); i++) if( netIFs[i]) delete netIFs[i];
   netIFs.clear();
}

void Client::grabNetIFs( std::vector<NetIF*> & otherNetIFs)
{
   clearNetIFs();
   for( int i = 0; i < otherNetIFs.size(); i++)
   {
      netIFs.push_back( otherNetIFs[i]);
      otherNetIFs[i] = NULL;
   }
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
   weight += address.calcWeight();
   for( int i = 0; i < netIFs.size(); i++) weight += netIFs[i]->calcWeight();
//printf("Client::calcWeight: %d bytes ( sizeof Client = %d)\n", weight, sizeof( Client));
   return weight;
}
