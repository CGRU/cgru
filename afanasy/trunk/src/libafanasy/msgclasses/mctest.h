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
   MCTest( int Number, const QString & String );
   MCTest( Msg * msg);
   ~MCTest();

   void stdOut( bool full = false) const;

   void addString( const QString & String);

   inline const QString & getString() const { return string; }
   inline int32_t getNumber() const { return number; }

private:
   bool construct();

   QString  string;
   int32_t number;

   QStringList stringlist;
   int32_t * numberarray;
   QString * stringarray;

   void readwrite( Msg * msg);
};
}
