#pragma once

#include "name_af.h"
#include "client.h"
#include "address.h"

namespace af
{
/// Talk - Afanasy communication client.
class Talk: public Client
{
public:

/// Construct client, getting values from environment.
   Talk();

/// Construct Talk data from buffer.
   Talk( Msg * msg, const af::Address * addr = NULL);

   virtual ~Talk();

   void stdOut( bool full = false) const;       ///< Print Talk general information.

private:
   void readwrite( Msg * msg); ///< Read or write Talk in buffer.
};
}
