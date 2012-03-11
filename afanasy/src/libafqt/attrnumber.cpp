#include "attrnumber.h"

#include <QtCore/QByteArray>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afqt;

AttrNumber::AttrNumber(  const QString & Name, const QString & Label, int initNumber):
      Attr( Name, Label, ""),
      n( initNumber)
{
}

AttrNumber::~AttrNumber()
{
}

bool AttrNumber::readData()
{
   bool ok;
   int number = str.toInt( &ok);
   if( false == ok)
   {
      AFERRAR("'%s' is not valid number string.\n", str.toUtf8().data());
      return false;
   }
   n = number;
   return true;
}

void AttrNumber::writeData()
{
   str = QString::number(n);
}
