#pragma once

#include "af.h"

class QHostAddress;

namespace af
{

class Address : public Af
{
public:
   Address( int first_port);           ///< Construct local address with port, in host byte order (zero IP).
   Address( const Address * other);    ///< Copy given address.
#ifndef WINNT
   Address( const struct sockaddr_storage * ss);  ///< Set address from address storage structure.
#endif
   Address( Msg * msg);                   ///< Construct address using raw data.
   ~Address();

   enum Family
   {
      IPv4,
      IPv6
   };

   inline int getPortHBO()   const  { return port;        } ///< Get address port in host byte order.
   inline int getPortNBO()   const  { return htons(port); } ///< Get address port in network byte order.
   inline void setPort( int value ) { port = value;       } ///< Set address port in host byte order.

   bool equal( const Address * other ) const;   ///< Compare address with other.

   void setQAddress( QHostAddress &qhostaddress) const;      ///< Set QHostAddress class address.

#ifndef WINNT
/// Set sockaddr_in structure address.
   void setSocketAddress( struct sockaddr_storage * ss) const;
#endif

/// Set new IP address.
   void setIP( const Address * other);


   void generateIPStream( std::ostringstream & stream) const;
   const std::string generateIPString() const;
   void generatePortStream( std::ostringstream & stream) const;
   const std::string generatePortString() const;
   virtual void generateInfoStream( std::ostringstream & stream, bool full = false) const;

   virtual int calcWeight() const;                   ///< Calculate and return memory size.

protected:
/// Read or write address in buffer.
   void readwrite( Msg * msg);

private:
   static uint16_t firtstClientPort;      ///< First allowed client port.

   static const int AddrDataLength = 16;

   int8_t   family;                  ///< Address family.
   uint16_t port;                    ///< Address port.
   char     addr[AddrDataLength];    ///< Address IP.
};
}
