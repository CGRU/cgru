#pragma once

#include "af.h"

#ifdef WINNT
#include <ws2tcpip.h>
#include <winsock2.h>
#endif

namespace af
{

struct AddressMask;

class Address : public Af
{
public:
   Address( int Port = 0);                         ///< Construct an empty address.
   Address( const Address & other);                ///< Copy given address.
   Address( const Address * other);                ///< Copy given address.
   Address & operator=( const Address & other);    ///< Assignment operator
   Address( const struct sockaddr_storage * ss);   ///< Set address from address storage structure.
   Address( const std::string & str);              ///< Construct address parsing a string.
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

/// Set sockaddr_in structure address.
   bool setSocketAddress( struct sockaddr_storage * ss) const;

/// return the correct size of a 'sockaddr'. This size is needed by "connect" for example.
   inline size_t sizeofAddr( void ) const
   {
      if( family == IPv6 )
         return sizeof(sockaddr_in6);
      /// Assume IPv4. if family is empty nothing works anyway.
      return sizeof(sockaddr_in);
   }

/// Set new IP address.
   void setIP( const Address & other);

   void generateIPStream( std::ostringstream & stream, bool full = false) const;
   const std::string generateIPString( bool full = false) const;
   void generatePortStream( std::ostringstream & stream, bool full = false) const;
   const std::string generatePortString( bool full = false) const;
   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

   int calcWeight() const;                   ///< Calculate and return memory size.

//protected:
/// Read or write address in buffer.
   void v_readwrite( Msg * msg);

   static bool readIpMask( const std::vector<std::string> & i_masks, bool i_verbose);

   bool matchIpMask() const;

   static const int ms_addrdatalength = 16;

	void jsonWrite( std::ostringstream & o_str) const;

private:

   static std::list<AddressMask> ms_addr_masks;

   int8_t   family;                  ///< Address family.
   uint16_t port;                    ///< Address port.
   char     addr[ms_addrdatalength];    ///< Address IP.
};

struct AddressMask
{
    AddressMask( int i_len, const char * i_bytes, Address::Family i_family, bool i_verbose);

    Address::Family m_family;
    int  m_len;
    char m_bytes[Address::ms_addrdatalength];    ///< IP Mask.
};
}
