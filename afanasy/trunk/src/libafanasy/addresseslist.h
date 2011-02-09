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

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   bool addAddress( const Address * address );

   inline int getAddressesNum() const { return addresses.size(); }

   inline const std::list<Address*> * getAddresses() const { return &addresses;}

   bool removeAddress( const Address * address);

   int calcWeight() const;
   inline int getWeight() const { return weight;}

private:
   std::list<Address*> addresses;
   mutable int weight;

private:

   void readwrite( Msg * msg); ///< Read or write task in message buffer.
};
}
