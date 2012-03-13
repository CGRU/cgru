#include "attrcolor.h"

#include <QtCore/QByteArray>
//#include <QtXml/QtXml>

//#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

using namespace afqt;

AttrColor::AttrColor(  const QString & Name, const QString & Label, const QString & initString):
      Attr( Name, Label, initString)
{
   readData();
}

AttrColor::~AttrColor()
{
}

bool AttrColor::readData()
{
   union { uint32_t num4; uint8_t num1[4];} four;
   bool ok;
   four.num4 = str.toInt( &ok, 16);
   if( false == ok)
   {
      AFERRAR("'%s' is not valid color string.\n", str.toUtf8().data());
      return false;
   }
   c = QColor( four.num1[2], four.num1[1], four.num1[0]);
AFINFA("ClrStr='%s': %02X %02X %02X %02X\n", str.toUtf8().data(), four.num1[3], four.num1[2], four.num1[1], four.num1[0]);
   return true;
}

void AttrColor::writeData()
{
   QString number; QChar ch_zero('0');
   number.append(QString("%1").arg( c.red(),   2, 0x10, ch_zero));
   number.append(QString("%1").arg( c.green(), 2, 0x10, ch_zero));
   number.append(QString("%1").arg( c.blue(),  2, 0x10, ch_zero));
   str =  number.toUpper();
}
