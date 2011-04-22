#pragma once

#include "af.h"
#include "address.h"
#include "name_af.h"

namespace af
{
/// Network interface parameters class.
class NetIF : public Af
{
public:

/// Constructor.
   NetIF( const char * Name, const unsigned char * MacAddr, const std::vector<Address> ifAddresses = std::vector<Address>());

/// Construct from message.
   NetIF( Msg * msg);

   ~NetIF();

   void generateInfoStream( std::ostringstream & stream, bool full = false) const;

   const std::string getMACAddrString( bool withSeparators = false) const;
   const std::string & getName() const { return name;}

   int calcWeight() const;

   static const int MacAddrLen = 6;

   /// Network interface addresses
   std::vector<Address> addresses;

   /// Search for interfaces and assinged addresses
   static void getNetIFs( std::vector<NetIF*> & netIFs, bool verbose = false);

   void readwrite( Msg * msg);

private:

   std::string name;

   unsigned char macaddr[MacAddrLen];
};
}
