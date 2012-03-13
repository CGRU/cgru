#pragma once

#include "../af.h"

namespace af
{

class MsgClass : public Af
{
public:
   MsgClass();
   virtual ~MsgClass();

   virtual void stdOut( bool full = false) const = 0;

protected:

};
}
