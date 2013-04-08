#pragma once

#include "af.h"

#include "address.h"

namespace af
{
/// Afanasy adresses list.
class AddressesList : public Af
{
public:
   AddressesList();
   AddressesList( Msg * msg); ///< Read adresses from message.
   ~AddressesList();

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   bool addAddress( const Address & address );

   inline size_t getAddressesNum() const { return addresses.size(); }

   inline const std::list<Address> * getAddresses() const { return &addresses;}

   bool removeAddress( const Address & address);

   int calcWeight() const;
   inline int getWeight() const { return weight;}

   void v_readwrite( Msg * msg); ///< Read or write task in message buffer.

private:
   std::list<Address> addresses;
   mutable int weight;
};
}
