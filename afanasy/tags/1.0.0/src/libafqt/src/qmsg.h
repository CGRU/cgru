#pragma once

#include <msg.h>

#include "name_afqt.h"

/// Afanasy server side of Msg.
class afqt::QMsg : public af::Msg
{
public:

/// Constructor.
   QMsg( int msgType = 0, int msgInt = 0, bool toRecieve = false, const af::Address * sendTo = NULL);

/// Constructor.
   QMsg( int msgType, af::Af * afClass, bool toRecieve = false, const af::Address * sendTo = NULL);

   ~QMsg();

/// Get whether to recieve an answer.
   inline const bool          getRecieving() const { return recieve;}

/// Get address constant pointer.
   inline const af::Address * getAddress()   const { return address;}

   mutable QMsg *next_ptr;  ///< Next message pointer.

private:
   af::Address *address;   ///< Address to send message to.
   bool         recieve;   ///< Whether to recieve an answer.
};
