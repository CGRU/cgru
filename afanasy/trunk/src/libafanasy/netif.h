#pragma once

#include "af.h"
#include "name_af.h"

namespace af
{
/// Network interface parameters class.
class NetIF : public Af
{
public:

/// Constructor.
   NetIF( const char * Name, const unsigned char * MacAddr);

   ~NetIF();

   void generateInfoStream( std::ostringstream & stream, bool full = false) const;

   const std::string getName() const { return name;}

   int weigh() const;

   static const int MacAddrLen = 6;

   static void getNetIFs( std::vector<NetIF*> & netIFs, bool verbose = false);

private:

   std::string name;

   unsigned char macaddr[MacAddrLen];
};
}
