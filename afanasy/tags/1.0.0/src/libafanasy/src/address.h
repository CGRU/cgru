#ifndef AF_ADDRESS_H
#define AF_ADDRESS_H

#include "af.h"

class QHostAddress;

union bytes4 { uint32_t num32; uint8_t num8[4];};

namespace af
{

class Address : public Af
{
public:
   Address( const QString &hostname, int first_port);     ///< Construct address looking up given hostname.
   Address( const Address *address);      ///< Copy given address.
   Address( const Address &address);      ///< Copy given address.
   Address( uint16_t port, in_addr addr); ///< set exact values.
   Address( Msg * msg);                   ///< Construct address using raw data.
   ~Address();

   inline void setPort( uint16_t port ) { sin_port = port; } ///< Set address port.

   inline uint16_t port() const { return sin_port; } ///< Get address port.
   inline in_addr  addr() const { return sin_addr; } ///< Get address IP.
   inline uint32_t IP4()  const { uint32_t ip4; ip4=sin_addr.s_addr/*memcpy( &ip4, &sin_addr, 4)*/; return ip4; } ///< Get address IP 4.

   bool equal( const Address * other ) const;   ///< Compare address with other.

#ifndef WINNT
/// Set sockaddr_in structure address.
   void setAddress( struct sockaddr_in &saddr_in) const;
#endif
/// Set new IP address.
   void setIP( const Address * other);

   void setAddress( QHostAddress &qhostaddress) const;      ///< Set QHostAddress class address.

   const QString getIPString() const;
   const QString getPortString() const;
   const QString getAddressString() const;

/// Print address info (port, IP).
   void printIP() const;                                          ///< Print IP address.

   void stdOut( bool full = false ) const;

   virtual int calcWeight() const;                   ///< Calculate and return memory size.
   inline  int  getWeight() const { return weight; } ///< Get memory size.

protected:
/// Read or write address in buffer.
   void readwrite( Msg * msg);

private:
   static uint16_t firtstClientPort;      ///< First allowed client port.
   uint16_t  sin_port;                    ///< Address port.
   in_addr   sin_addr;                    ///< Address IP.

   mutable int weight; ///< Memory size
};
}
#endif
