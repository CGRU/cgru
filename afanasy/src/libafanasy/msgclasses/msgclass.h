#pragma once

#include "../af.h"

namespace af
{

class MsgClass : public Af
{
public:
   MsgClass();
   virtual ~MsgClass();

   virtual void v_generateInfoStream( std::ostringstream & stream, bool full = false) const = 0;

protected:

};
}
