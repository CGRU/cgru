#pragma once

#include <stdlib.h>
#include <vector>

#include <QtCore/QString>

#include "../../include/aftypes.h"

#include "../name_af.h"

#include "msgclassuserhost.h"

namespace af
{

class MCTest : public MsgClassUserHost
{
public:
   MCTest( int Number, const std::string & String );
   MCTest( Msg * msg);
   ~MCTest();

   void stdOut( bool full = false) const;

   void addString( const std::string & String);

   inline const std::string & getString() const { return string; }
   inline int32_t getNumber() const { return number; }

private:
   bool construct();

   std::string string;
   int32_t number;

   std::list<std::string> stringlist;
   int32_t * numberarray;
   std::string * stringarray;

   void readwrite( Msg * msg);
};
}
