#include "netif.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

NetIF::NetIF( const char * Name, const unsigned char * MacAddr)
{
   name = Name;
   memcpy( macaddr, MacAddr, MacAddrLen);
}

NetIF::~NetIF()
{
}

void NetIF::generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << name << ": ";
   char buffer[4];
   for( int i = 0; i < MacAddrLen; i++)
   {
      if( i != 0 ) stream << ":";
      sprintf( buffer, "%02X", macaddr[i]);
      stream << buffer;
   }
}

int NetIF::weigh() const
{
   int size = sizeof( NetIF);
   size += af::weigh( name);
   return size;
}

void NetIF::getNetIFs( std::vector<NetIF*> & netIFs, bool verbose)
{
#ifndef SIOCGIFADDR
   // The kernel does not support the required ioctls
   return;
#endif

   struct ifreq sIfReq;                      // Interface request
   struct if_nameindex *pIfList     = NULL;  // Ptr to interface name index
   struct if_nameindex *pIfListHead = NULL;  // Ptr to interface name index (not to eterate, always will point to list head, for structure deletion)

   // Create a socket that we can use for all of our ioctls
   int sd = socket( PF_INET, SOCK_STREAM, 0 );
   if( sd < 0 )
   {
      // Socket creation failed, this is a fatal error
      AFERRPE("Get if props socket:")
      return;
   }

   // Obtain a list of dynamically allocated structures
   pIfList = pIfListHead = if_nameindex();

   if( verbose) printf("Searching for network interfaces:\n");

   // Walk thru the array returned and query for each interface's address
   for ( pIfList; *(char *)pIfList != 0; pIfList++ )
   {
      strncpy( sIfReq.ifr_name, pIfList->if_name, IF_NAMESIZE );

      // Get the MAC address for this interface
      if( ioctl( sd, SIOCGIFHWADDR, &sIfReq) != 0 )
      {
         // We failed to get the MAC address for the interface
         AFERRPE("NetIF::getNetIFs: ioctl SIOCGIFHWADDR failed")
         continue;
      }

      unsigned char cMacAddr[8]; // Server's MAC address
      bzero( (void *)&cMacAddr[0], sizeof(cMacAddr) );
      memcpy( (void *)&cMacAddr[0], (void *)&sIfReq.ifr_ifru.ifru_hwaddr.sa_data[0], 6 );

      bool isup = false;
      // Get the status of this interface
      if( ioctl( sd, SIOCGIFFLAGS, &sIfReq) != 0 )
      {
         // We failed to get the MAC address for the interface
         AFERRPE("NetIF::getNetIFs: ioctl SIOCGIFFLAGS failed")
      }
      else
      {
         if(( sIfReq.ifr_flags & IFF_UP ) && ( sIfReq.ifr_flags & IFF_RUNNING ))
            isup = true;
      }

      if( isup)
      {
         // Get the IP address for this interface
         if( ioctl( sd, SIOCGIFADDR, &sIfReq) != 0 )
         {
            // We failed to get the MAC address for the interface
            AFERRPE("NetIF::getNetIFs: ioctl SIOCGIFADDR failed")
         }
      }

      if( verbose)
      {
         printf("   %10s: %02X:%02X:%02X:%02X:%02X:%02X", pIfList->if_name,
               cMacAddr[0], cMacAddr[1], cMacAddr[2],
               cMacAddr[3], cMacAddr[4], cMacAddr[5]);
         if( isup) printf(" UP");
         if( isup) printf(" %s", inet_ntoa(((struct sockaddr_in *)&sIfReq.ifr_addr)->sin_addr));
         printf("\n");
      }

      bool skip = true;
//      if( strncmp( pIfList->if_name, "lo", IF_NAMESIZE) != 0 ) skip = false;
      for( int i = 0; i < MacAddrLen; i++)
         if( cMacAddr[i] != 0 )
         {
            skip = false;
            break;
         }
      if( skip) continue;

      netIFs.push_back( new NetIF( pIfList->if_name, cMacAddr));
   }

   // Clean up list head
   if_freenameindex( pIfListHead);

   close( sd );


   struct ifaddrs * ifAddrStruct=NULL;
   void * tmpAddrPtr=NULL;

   getifaddrs(&ifAddrStruct);

   while (ifAddrStruct!=NULL)
   {
      if (ifAddrStruct->ifa_addr->sa_family==AF_INET)
      {
         // is a valid IP4 Address
         tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
         char addressBuffer[INET_ADDRSTRLEN];
         inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
         printf("   %s IP4 Address %s\n", ifAddrStruct->ifa_name, addressBuffer);
      }
      else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6)
      {
         // is a valid IP6 Address
         tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
         char addressBuffer[INET6_ADDRSTRLEN];
         inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
         printf("   %s IP6 Address %s\n", ifAddrStruct->ifa_name, addressBuffer);
      }
      ifAddrStruct=ifAddrStruct->ifa_next;
   }
}
