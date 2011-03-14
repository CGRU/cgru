#ifdef WINNT
#include <winsock2.h>
#include <iphlpapi.h>
//#include <stdio.h>
#include <limits.h>
#include <ws2tcpip.h>
#pragma comment(lib, "IPHLPAPI.lib")
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#endif

#ifdef LINUX
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#endif

#include <string.h>

#include "netif.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

NetIF::NetIF( const char * Name, const unsigned char * MacAddr, const std::vector<Address> ifAddresses)
{
   name = Name;
   memcpy( macaddr, MacAddr, MacAddrLen);
   addresses = ifAddresses;
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
      sprintf( buffer, "%02x", macaddr[i]);
      stream << buffer;
   }
   if( addresses.size())
   {
      for( int i = 0; i < addresses.size(); i++)
      {
         if( full ) stream << "\n      ";
         else stream << " ";
         addresses[i].generateInfoStream( stream, full);
      }
   }
}

int NetIF::calcWeight() const
{
   int size = sizeof( NetIF);
   size += af::weigh( name);
   for( int i = 0; i < addresses.size(); i++) size += addresses[i].calcWeight();
   return size;
}

void NetIF::getNetIFs( std::vector<NetIF*> & netIFs, bool verbose)
{
#ifdef LINUX
#ifndef SIOCGIFADDR
   // The kernel does not support the required ioctls
   return;
#endif

   struct ifreq sIfReq;                      // Interface request
   struct if_nameindex *pIfList     = NULL;  // Ptr to interface name index
   struct if_nameindex *pIfListHead = NULL;  // Ptr to interface name index (not to eterate, always will point to list head, for structure deletion)
   struct ifaddrs * ifAddrStruct    = NULL;  // Interfaces addresses structure pointer

   // Create a socket that we can use for all of our ioctls
   int sd = socket( PF_INET, SOCK_STREAM, 0 );
   if( sd < 0 )
   {
      // Socket creation failed, this is a fatal error
      AFERRPE("Get if props socket:")
      return;
   }

   // Obtain a list of addresses
   if( getifaddrs( &ifAddrStruct) != 0 )
   {
      AFERRPE("getifaddrs:")
      ifAddrStruct = NULL;
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

      if( verbose)
      {
         if( isup)
            if( ioctl( sd, SIOCGIFADDR, &sIfReq) != 0 ) // Get the IP address for this interface
               AFERRPE("NetIF::getNetIFs: ioctl SIOCGIFADDR failed") // We failed to get the MAC address for the interface

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

      std::vector<Address> addresses;
      // Iterate through addresses:
      for( struct ifaddrs * ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
      {
         if( strncmp( ifa->ifa_name, pIfList->if_name, IF_NAMESIZE ) != 0 ) continue; // Skip to search this interface name

         struct sockaddr_storage * ss = (struct sockaddr_storage *)(ifa->ifa_addr);
         switch (ifa->ifa_addr->sa_family)
         {
            case AF_INET:
            case AF_INET6:
            {
               addresses.push_back( Address(*((struct sockaddr_storage *)(ifa->ifa_addr))));
               break;
            }
         }
         if( verbose)
            switch (ifa->ifa_addr->sa_family)
            {
               case AF_INET:
               {
                  void * tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                  char addressBuffer[INET_ADDRSTRLEN];
                  inet_ntop( AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                  printf("                  ");
                  printf("IPv4 Address: %s\n", addressBuffer);
                  break;
               }
               case AF_INET6:
               {
                  void * tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                  char addressBuffer[INET6_ADDRSTRLEN];
                  inet_ntop( AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
                  printf("                  ");
                  printf("IPv6 Address: %s\n", addressBuffer);
                  break;
               }
            }
      }

      netIFs.push_back( new NetIF( pIfList->if_name, cMacAddr, addresses));
   }

   // Clean up
   if_freenameindex( pIfListHead);
   freeifaddrs( ifAddrStruct);

   close( sd );

#endif

#ifdef WINNT

   /* Declare and initialize variables */

   // Set the flags to pass to GetAdaptersAddresses
   ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
   ULONG outBufLen = sizeof(IP_ADAPTER_ADDRESSES);
   PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES *) MALLOC(outBufLen);

   // Make an initial call to GetAdaptersAddresses to get the 
   // size needed into the outBufLen variable
   // AF_UNSPEC to unspecified address family (both)
   if( GetAdaptersAddresses( AF_UNSPEC, flags, NULL, pAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW)
   {
      FREE(pAddresses);
      pAddresses = (IP_ADAPTER_ADDRESSES *) MALLOC(outBufLen);
   }

   if( pAddresses == NULL)
   {
      AFERROR("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
      return;
   }
   // Make a second call to GetAdapters Addresses to get the
   // actual data we want

   DWORD dwRetVal = GetAdaptersAddresses( AF_UNSPEC, flags, NULL, pAddresses, &outBufLen);

   if( dwRetVal == NO_ERROR)
   {
      // If successful, output some information from the data we received
      for( PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses; pCurrAddresses != NULL; pCurrAddresses = pCurrAddresses->Next)
      {
         if( verbose)
         {
            printf("Adapter name: %s\n", pCurrAddresses->AdapterName);
            if( pCurrAddresses->PhysicalAddressLength != 0)
            {
               printf("\tPhysical address: ");
               for( unsigned i = 0; i < pCurrAddresses->PhysicalAddressLength; i++)
               {
                  if( i == (pCurrAddresses->PhysicalAddressLength - 1))
                     printf("%.2X\n", (int) pCurrAddresses->PhysicalAddress[i]);
                  else
                     printf("%.2X-", (int) pCurrAddresses->PhysicalAddress[i]);
               }
            }
         }

         // Skipping interface addresses without IP address(es) configured:
         bool skip_addresses = false;
         if((( pCurrAddresses->Flags & IP_ADAPTER_IPV4_ENABLED) == false)
            && (( pCurrAddresses->Flags & IP_ADAPTER_IPV6_ENABLED) == false))
         {
            if( verbose) printf("\tInterface without IP address(es) configured.\n");
            skip_addresses = true;
         }

         // Skipping down interface:
         if( pCurrAddresses->OperStatus != IfOperStatusUp )
         {
            if( verbose) printf("\tSkipping down interface.\n");
            continue;
         }

         std::vector<Address> addresses;
         for( IP_ADAPTER_PREFIX * pPrefix = pCurrAddresses->FirstPrefix; pPrefix != NULL; pPrefix = pPrefix->Next)
         {
            if( false == skip_addresses )
               addresses.push_back( Address(*((struct sockaddr_storage *)(pPrefix->Address.lpSockaddr))));

            if( verbose )
            {
               char szAddress[NI_MAXHOST];
               if( getnameinfo( pPrefix->Address.lpSockaddr,
		            pPrefix->Address.iSockaddrLength,
		            szAddress, sizeof(szAddress), NULL, 0,
		            NI_NUMERICHOST))
               {
	               fprintf(stderr, "can't convert network format to presentation format");
               }
               else
               {
                  switch((pPrefix->Address.lpSockaddr)->sa_family)
                  {
                     case AF_INET:
                        printf("\t\tFamily=IPv4, Addr=%s\n", szAddress);
                        break;
                     case AF_INET6:
                        printf("\t\tFamily=IPv6, Addr=%s/64\n", szAddress);
                        break;
                     default:
                        printf("\t\tFamily=Unknown, Addr=%s\n", szAddress);
                  }
               }
            }
         }

         // Skipping software loopback network interface:
         if( pCurrAddresses->IfType == IF_TYPE_SOFTWARE_LOOPBACK )
         {
            if( verbose) printf("\tSkipping software loopback network interface.\n");
            continue;
         }

         netIFs.push_back( new NetIF( pCurrAddresses->AdapterName, pCurrAddresses->PhysicalAddress, addresses));
      }
   }
   else
   {
      AFERRAR("Call to GetAdaptersAddresses failed with error(%d):\n", dwRetVal);
      if( dwRetVal == ERROR_NO_DATA)
         printf("\tNo addresses were found for the requested parameters\n");
      else
      {
         LPVOID lpMsgBuf = NULL;
         if( FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) & lpMsgBuf, 0, NULL))
         {
            printf("%s\n", lpMsgBuf);
            LocalFree(lpMsgBuf);
         }
      }
   }
   FREE(pAddresses);
#endif

}
