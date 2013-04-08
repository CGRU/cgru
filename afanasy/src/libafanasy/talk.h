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
   Talk( Msg * msg);

   virtual ~Talk();

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

private:
   void v_readwrite( Msg * msg); ///< Read or write Talk in buffer.
};
}
