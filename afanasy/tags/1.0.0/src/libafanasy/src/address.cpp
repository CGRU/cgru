#include "address.h"

//#inclide <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "environment.h"

#include <QtNetwork/qhostinfo.h>
#include <QtNetwork/qhostaddress.h>

//#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

using namespace af;

uint16_t Address::firtstClientPort  = 0;

Address::Address( const Address* address)
{
    sin_addr=address->sin_addr;
//     memcpy( &sin_addr, &address->sin_addr, 4);
    sin_port = address->sin_port;
}

Address::Address( const Address &address)
{
    sin_addr=address.sin_addr;
//     memcpy( &sin_addr, &address.sin_addr, 4);
    sin_port = address.sin_port;
}

Address::Address( uint16_t port, in_addr addr)
{
    sin_addr=addr;
//     memcpy( &sin_addr, &addr, 4);
    sin_port = port;
}

Address::Address( const QString &hostname, int first_port)
{
   sin_port = first_port;
   firtstClientPort = sin_port;

//
// Get host address
   QList<QHostAddress> addresseslist = QHostInfo::fromName( hostname).addresses();
   for( int a = 0; a < addresseslist.size(); a++)
   {
      QHostAddress qhostaddress = addresseslist[a];
//      if( qhostaddress == QHostAddress("127.0.0.1")) continue;
      uint32_t addr = htonl( qhostaddress.toIPv4Address());
      sin_addr.s_addr = addr;
//       memcpy( &sin_addr, &addr, 4);
   }
}

bool Address::equal( const af::Address * other ) const
{
   if( other == NULL ) return false;
   if( sin_port == other->sin_port )
   {
      if( memcmp( &sin_addr, &(other->sin_addr), 4) == 0)
      return true;
   }
   return false;
}

Address::Address( Msg * msg)
{
   read( msg);
}

void Address::readwrite( Msg * msg)
{
   rw_in_addr(  sin_addr, msg);
   rw_uint16_t( sin_port, msg);
}

Address::~Address()
{
}

#ifndef WINNT
void Address::setAddress( struct sockaddr_in &saddr_in) const
{
   saddr_in.sin_port = htons( sin_port);
   memcpy( &saddr_in.sin_addr, &sin_addr, 4);
}
#endif

void Address::setIP( const af::Address * other)
{
    sin_addr=other->sin_addr;
//    memcpy( &sin_addr, &other->sin_addr, 4);
}

void Address::setAddress( QHostAddress &qhostaddress) const
{
   uint32_t addr;
   addr=sin_addr.s_addr;
//    memcpy( &addr, &sin_addr, 4);
   qhostaddress.setAddress( addr);
}

const QString Address::getIPString() const
{
   uint32_t addr;
   addr=sin_addr.s_addr;
//    memcpy( &addr, &sin_addr, 4);
   QHostAddress qhostaddress( ntohl( addr));
   return qhostaddress.toString();
}

const QString Address::getPortString() const
{
   return QString::number( sin_port);
}

const QString Address::getAddressString() const
{
   return QString("%1:%2").arg( getIPString(), getPortString());
}

void Address::printIP() const
{
   printf("%s", getIPString().toUtf8().data());
}

int Address::calcWeight() const
{
   weight = sizeof( Address);
   return weight;
}

void Address::stdOut( bool full ) const
{
   printf("%s", getAddressString().toUtf8().data());
}
