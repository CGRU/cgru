#include "attr.h"

#include <QtCore/QByteArray>

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afqt;

Attr::Attr( const QString & Name, const QString & Label, const QString & initString):
      str( initString),
      name( Name),
      label( Label)
{
}

Attr::~Attr()
{
}

bool Attr::readData()   { return true; }
void Attr::writeData()  {}

bool Attr::read( const QDomDocument & doc)
{
   std::string value;
   if( af::Environment::getVar( doc, value, name.toUtf8().data()) == false) return false;
   str = afqt::stoq( value);
   return readData();
}

void Attr::write( QByteArray & data)
{
   writeData();
   data.append(QString("   <%1").arg( name));
   for( int i = 0; i < 20 - name.size(); i++) data.append(' ');
   data.append('>');
   data.append(str);
   data.append(QString("</%1>\n").arg( name));
}
