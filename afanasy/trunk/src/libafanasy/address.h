#pragma once

#include "af.h"

#ifdef WINNT
struct sockaddr_storage;
#endif

namespace af
{

class Address : public Af
{
public:
   Address( int Port = 0);                         ///< Construct an empty address.
   Address( const Address & other);                ///< Copy given address.
   Address( const Address * other);                ///< Copy given address.
   Address & operator=( const Address & other);    ///< Assignment operator
   Address( const struct sockaddr_storage & ss);   ///< Set address from address storage structure.
   Address( Msg * msg);                            ///< Construct address using raw data.

   ~Address();

   enum Family
   {
      IPv4,
      IPv6,
      Empty
   };

   void copy( const Address & other);

   void clear();

   inline bool  isEmpty()    const  { return family == Empty;}
   inline bool notEmpty()    const  { return family != Empty;}

   inline int getFamily()    const  { return family;      }
   inline int getPortHBO()   const  { return port;        } ///< Get address port in host byte order.
   inline int getPortNBO()   const  { return htons(port); } ///< Get address port in network byte order.
   inline const char * getAddrData() const { return addr; }

   inline void setPort( int value ) { port = value;       } ///< Set address port in host byte order.

   bool equal( const Address & other ) const;   ///< Compare address with other.

#ifndef WINNT
/// Set sockaddr_in structure address.
   bool setSocketAddress( struct sockaddr_storage & ss) const;
#endif

/// Set new IP address.
   void setIP( const Address & other);


   void generateIPStream( std::ostringstream & stream) const;
   const std::string generateIPString() const;
   void generatePortStream( std::ostringstream & stream) const;
   const std::string generatePortString() const;
   void generateInfoStream( std::ostringstream & stream, bool full = false) const;

   int calcWeight() const;                   ///< Calculate and return memory size.

//protected:
/// Read or write address in buffer.
   void readwrite( Msg * msg);

private:

   static const int AddrDataLength = 16;

   int8_t   family;                  ///< Address family.
   uint16_t port;                    ///< Address port.
   char     addr[AddrDataLength];    ///< Address IP.
};
}
