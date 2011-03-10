#pragma once

#include "../libafanasy/af.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/client.h"

#include "afqueueitem.h"

class MsgQueue;

/// Afanasy server side of Msg.
class MsgAf: public af::Msg, public AfQueueItem
{
public:

/// Construct an empty or non data message
   MsgAf( int msgType = 0, int msgInt = 0);

/// Construct a message from class
   MsgAf( int msgType, af::Af * afClass );

/// Construct a message and set an address
   MsgAf( const struct sockaddr_storage & ss);

   ~MsgAf();

   inline bool addressIsEmpty() const { return address.isEmpty();}

/// Set message address to \c client .
   inline void setAddress( const af::Client* client)
      { address = client->getAddress();}

/// Add dispatch address.
   inline void addAddress( const af::Client* client)
      { addresses.push_back( client->getAddress());}

//// Set address to message, used to get address from socket.
//   inline void setAddress( af::Address* newAddress) { if( address ) delete address; address = newAddress;}

/// Get address constant pointer.
   inline const af::Address & getAddress() const { return address;}

/// Get addresses constant list pointer.
   inline const std::list<af::Address> * getAddresses() const { return &addresses;}

/// Set message to it's address, it stores in msgDispatchQueue, waiting for special thread.
   void dispatch();

/// Set message to it's address right now (in this tread) and wait for an \c answer , \c return true on success.
   bool request( MsgAf *answer);

private:
   af::Address address;               ///< Address, where message came from or will be send.

   std::list<af::Address> addresses;
};
