#include "address.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "environment.h"

#include <QtNetwork/qhostaddress.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

uint16_t Address::firtstClientPort  = 0;

Address::Address( const Address* other)
{
   port = other->port;
   family = other->family;
   memcpy( &addr, &other->addr, AddrDataLength);
}

#ifndef WINNT
Address::Address( const struct sockaddr_storage * ss)
{
   bzero( addr, AddrDataLength);
   switch( ss->ss_family)
   {
   case AF_INET:
   {
      struct sockaddr_in * sa = (struct sockaddr_in*)ss;
      family = IPv4;
      port = ntohs( sa->sin_port);
      memcpy( addr, &(sa->sin_addr), 4);
      break;
   }
   case AF_INET6:
   {
      struct sockaddr_in6 * sa = (struct sockaddr_in6*)ss;

      // Check for mapped IPv4 address: 1-80 bits == 0, 81-96 bits == 1 and last 32 bits == IPv4
      family = IPv4;
      char * data = (char*)&(sa->sin6_addr);
      for( int i = 0; i < 10; i++) // Check first 10 bytes for zeros:
         if( data[i] != char(0))
         {
            family = IPv6;
            break;
         }
      if( family == IPv4 )
         for( int i = 10; i < 12; i++) // Check next 2 bytes for 0xFF:
            if( data[i] != char(0xFF))
            {
               family = IPv6;
               break;
            }

      if( family == IPv6)
      {
         port = ntohs( sa->sin6_port);
         memcpy( addr, &(sa->sin6_addr), 16);
      }
      else
      {
         struct sockaddr_in * sa = (struct sockaddr_in*)ss;
         port = ntohs( sa->sin_port);
         memcpy( addr, &(data[12]), 4);
      }
      break;
   }
   default:
      AFERROR("Address::Address: Unknown address family.\n");
      break;
   }
}
#endif

Address::Address( int first_port)
{
   firtstClientPort = first_port;

   port = first_port;
   family = 0;
   // There is no need to find out local host address.
   // Server will look at connected client address to register.
   memset( addr, 0, AddrDataLength);
}

bool Address::equal( const af::Address * other ) const
{
   if( other == NULL ) return false;
   if(( family == other->family) && ( port == other->port ))
   {
      if( memcmp( &addr, &(other->addr), AddrDataLength) == 0)
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
   rw_int8_t(   family, msg);
   rw_uint16_t( port,   msg);
   rw_data(     addr,   msg, AddrDataLength);
}

Address::~Address()
{
}

#ifndef WINNT
void Address::setSocketAddress( struct sockaddr_storage * ss) const
{
   bzero( ss, sizeof(sockaddr_storage));
   switch( family)
   {
      case IPv4:
      {
         struct sockaddr_in * sa = (struct sockaddr_in*)ss;
         sa->sin_family = AF_INET;
         sa->sin_port = htons( port);
         memcpy( &(sa->sin_addr), addr, 4);
         break;
      }
      case IPv6:
      {
         struct sockaddr_in6 * sa = (struct sockaddr_in6*)ss;
         sa->sin6_family = AF_INET6;
         sa->sin6_port = htons( port);
         memcpy( &(sa->sin6_addr), addr, 16);
         break;
      }
      default:
         AFERROR("Address::setSocketAddress: Unknown address family.\n");
         break;
   }
}
#endif

void Address::setIP( const af::Address * other)
{
   family = other->family;
   memcpy( &addr, &other->addr, AddrDataLength);
}

void Address::setQAddress( QHostAddress & qhostaddress) const
{
   switch( family)
   {
      case IPv4:
      {
         quint32 ipv4;
         memcpy( &ipv4, addr, 4);
         qhostaddress.setAddress( ipv4);
      }
      case IPv6:
         qhostaddress.setAddress( (quint8*)addr);
         break;
      default:
         AFERROR("Address::setQAddress: Unknown address family.\n");
   }
}

void Address::generateIPStream( std::ostringstream & stream) const
{
   switch( family)
   {
      case IPv4:
         stream << int( addr[0]) << "." << int( addr[1]) << "." << int( addr[2]) << "." << int( addr[3]);
         break;
      case IPv6:
         for( int i = 0; i < 8; i++)
         {
            if( i != 0 ) stream << ':';
            char buffer[8];
            sprintf( buffer, "%02x", uint8_t(addr[2*i  ]));
            stream << buffer;
            sprintf( buffer, "%02x", uint8_t(addr[2*i+1]));
            stream << buffer;
         }
         break;
      default:
         stream << "Unknown address family";
         break;
   }
}

const std::string Address::generateIPString() const
{
   std::ostringstream stream;
   generateIPStream( stream);
   return stream.str();
}

void Address::generatePortStream( std::ostringstream & stream) const
{
   stream << port;
}

const std::string Address::generatePortString() const
{
   std::ostringstream stream;
   generatePortStream( stream);
   return stream.str();
}

void Address::generateInfoStream( std::ostringstream & stream, bool full) const
{
   generateIPStream( stream);
   stream << ":";
   generatePortStream( stream);
}

int Address::calcWeight() const
{
   int weight = sizeof( Address);
   return weight;
}
