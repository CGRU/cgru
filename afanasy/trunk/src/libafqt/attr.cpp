#include "attr.h"

#include <QtCore/QByteArray>
#include <QtXml/QDomDocument>

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
   if( getXMLElement( doc, name, str) == false) return false;
   str = str.trimmed();
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

bool Attr::getXMLElement( const QDomDocument & doc, const QString & name, QString & text)
{
   QDomNodeList list = doc.elementsByTagName( name);
   int size = list.size();
   if( size < 1) return false;
   if( size > 1)
   {
      AFERRAR("Found %d '%s' elements in document, using the last.\n", size, name.toUtf8().data())
   }
   QDomElement element = list.at(size-1).toElement();
   if( element.isNull())
   {
      AFERRAR("Invalid element [Line %d - Col %d]: '%s'\n",
         element.lineNumber(), element.columnNumber(), name.toUtf8().data())
      return false;
   }
   text = element.text();
   return true;
}

bool Attr::getXMLAttribute( QDomElement & element, const QString & name, int & value)
{
   if( element.isNull()) return false;
   QDomAttr attribute = element.attributeNode( name);
   if( attribute.isNull()) return false;
   bool ok;
   int number = attribute.value().toInt( &ok);
   if( false == ok )
   {
      AFERRAR("Element '%s': attribute '%s' has not a number value '%s'\n",
              element.text().toUtf8().data(), name.toUtf8().data(), attribute.value().toUtf8().data())
      return false;
   }
   value = number;
   return true;
}
