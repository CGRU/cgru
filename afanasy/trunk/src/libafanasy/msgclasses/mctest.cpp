#include "mctest.h"

#include <stdio.h>

#include "../msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTest::MCTest( int Number, const QString & String):
   string( String),
   number( Number),
   numberarray( NULL),
   stringarray( NULL)
{
   construct();
}

MCTest::MCTest( Msg * msg)
{
   read( msg);
}

bool MCTest::construct()
{
   numberarray = new int32_t[ number];
   stringarray = new QString[ number];
   if((numberarray == NULL) || (stringarray == NULL))
   {
      AFERROR("MCTest::MCTest: Memory allocation failed.");
      number = 0;
      return false;
   }
   return true;
}

MCTest::~MCTest()
{
   if( numberarray != NULL) delete [] numberarray;
   if( stringarray != NULL) delete [] stringarray;
}

void MCTest::readwrite( Msg * msg)
{
   MsgClassUserHost::readwrite( msg);

   rw_int32_t(    number,     msg);
   rw_QString(     string,     msg);
   rw_QStringList( stringlist, msg);

   if( msg->isReading() )
      if( construct() == false)
         return;

   for( int i = 0; i < number; i++)
   {
      rw_int32_t( numberarray[i], msg);
      rw_QString(  stringarray[i], msg);
   }
}

void MCTest::addString( const QString & String)
{
   numberarray[ stringlist.size()] = stringlist.size();
   stringarray[ stringlist.size()] = String;
   stringlist << String;
}

void MCTest::stdOut( bool full) const
{
   MsgClassUserHost::stdOut( full);
   printf(": String = \"%s\", Number = %d.\n",
            string.toUtf8().data(), number);

   if( full == false ) return;

   if( string[0] == '/') return;

   for( int i = 0; i < number; i++)
   {
      printf("a\"%s\" = l\"%s\" : n%d\n",
         stringarray[i].toUtf8().data(),
         stringlist[i].toUtf8().data(),
         numberarray[i]);
   }
}

