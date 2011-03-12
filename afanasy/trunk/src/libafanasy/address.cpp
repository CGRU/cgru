#include "address.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "environment.h"

#ifdef WINNT
#define sprintf sprintf_s
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Address::Address( int Port):
      port( Port),
      family( Empty)
{
   // There is no need to find out local host address.
   // Server will look at connected client address to register.
   memset( addr, 0, AddrDataLength);
}

Address::Address( const Address & other)
{
   copy( other);
}

Address::Address( const Address * other)
{
   if( other ) copy( *other);
}

Address & Address::operator=( const Address & other)
{
   if( this != &other) copy( other);
   return *this;
}

void Address::copy( const Address & other)
{
   port = other.port;
   family = other.family;
   memcpy( &addr, &other.addr, AddrDataLength);
}

#ifndef WINNT
Address::Address( const struct sockaddr_storage & ss)
{
   bzero( addr, AddrDataLength);
   switch( ss.ss_family)
   {
   case AF_INET:
   {
      struct sockaddr_in * sa = (struct sockaddr_in*)(&ss);
      family = IPv4;
      port = ntohs( sa->sin_port);
      memcpy( addr, &(sa->sin_addr), 4);
      break;
   }
   case AF_INET6:
   {
      struct sockaddr_in6 * sa = (struct sockaddr_in6*)(&ss);

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
         struct sockaddr_in * sa = (struct sockaddr_in*)(&ss);
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

bool Address::equal( const af::Address & other ) const
{
   if( isEmpty()) return false;
   if( other.isEmpty()) return false;

   if(( family == other.family) && ( port == other.port ))
   {
      if( memcmp( &addr, &(other.addr), AddrDataLength) == 0)
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

void Address::clear()
{
   port = 0,
   family = Empty;
   memset( addr, 0, AddrDataLength);
}

#ifndef WINNT
bool Address::setSocketAddress( struct sockaddr_storage & ss) const
{
   bzero( &ss, sizeof(sockaddr_storage));
   switch( family)
   {
      case IPv4:
      {
         struct sockaddr_in * sa = (struct sockaddr_in*)(&ss);
         sa->sin_family = AF_INET;
         sa->sin_port = htons( port);
         memcpy( &(sa->sin_addr), addr, 4);
         break;
      }
      case IPv6:
      {
         struct sockaddr_in6 * sa = (struct sockaddr_in6*)(&ss);
         sa->sin6_family = AF_INET6;
         sa->sin6_port = htons( port);
         memcpy( &(sa->sin6_addr), addr, 16);
         break;
      }
      case Empty:
         AFERROR("Address::setSocketAddress: Address is empty.\n");
         return false;
      default:
         AFERROR("Address::setSocketAddress: Unknown address family.\n");
         return false;
   }
   return true;
}
#endif

void Address::setIP( const af::Address & other)
{
   family = other.family;
   memcpy( &addr, &other.addr, AddrDataLength);
}

void Address::generateIPStream( std::ostringstream & stream) const
{
   switch( family)
   {
      case IPv4:
      {
         char buffer[64];
         sprintf( buffer, "%u.%u.%u.%u", uint8_t(addr[0]), uint8_t(addr[1]), uint8_t(addr[2]), uint8_t(addr[3]));
         stream << buffer;
         break;
      }
      case IPv6:
      {
         char buffer[64];
         for( int i = 0; i < 8; i++)
         {
            if( i != 0 ) stream << ':';
            uint8_t byte0 = uint8_t(addr[2*i]);
            uint8_t byte1 = uint8_t(addr[2*i+1]);
            if( byte0 ) sprintf( buffer, "%x%02x", byte0, byte1);
            else if( byte1 ) sprintf( buffer, "%x", byte1);
            if( byte0 || byte1 ) stream << buffer;
         }
         break;
      }
      case Empty:
         stream << "Empty address";
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
   if( full )
   {
      switch (family)
      {
         case IPv4:
            stream << "IPv4 Address: ";
            break;
         case IPv6:
            stream << "IPv6 Address: ";
            break;
      }
   }
   generateIPStream( stream);
   if( notEmpty())
   {
      stream << ":";
      generatePortStream( stream);
   }
}

int Address::calcWeight() const
{
   int weight = sizeof( Address);
   return weight;
}
