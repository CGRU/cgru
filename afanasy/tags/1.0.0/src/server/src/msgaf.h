#pragma once

#include <af.h>
#include <msg.h>
#include <client.h>

#include "afqueueitem.h"

class MsgQueue;

/// Afanasy server side of Msg.
class MsgAf: public af::Msg, public AfQueueItem
{
public:

/// Constructor.
   MsgAf( int msgType = 0, int msgInt = 0);

/// Constructor.
   MsgAf( int msgType, af::Af * afClass );

   ~MsgAf();

/// Set message address to \c client .
   inline void setAddress( const af::Client* client)
      { if(address) delete address; const af::Address *addr = client->getAddress(); if(addr) address = new af::Address(addr); else address = NULL;}

/// Add dispatch address.
   inline void addAddress( const af::Client* client)
      { const af::Address *addr = client->getAddress(); if( addr) addresses.push_back( new af::Address( addr));}

/// Set new address to message.
   inline void setAddress( af::Address* newAddress) { address = newAddress;}

/// Get address constant pointer.
   inline const af::Address* getAddress() const { return address;}

/// Get addresses constant list pointer.
   inline const std::list<af::Address*> * getAddresses() const { return &addresses;}

/// Set message to it's address, it stores in msgDispatchQueue, waiting for special thread.
   void dispatch();

/// Set message to it's address right now (in this tread) and wait for an \c answer , \c return true on success.
   bool request( MsgAf *answer);

private:
   af::Address *address;               ///< Address, where message came from or will be send.

   std::list<af::Address*> addresses;
};
