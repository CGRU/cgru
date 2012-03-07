#include "attrrect.h"

#include <QtCore/QByteArray>
#include <QtXml/QDomElement>

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afqt;

const QString AttrRect::WndTagName = "wndrect";

AttrRect::AttrRect(  const QString & Name, const QRect & initRect):
    Attr( Name, "", ""),
    r( initRect)
{
}

AttrRect::~AttrRect()
{
}

AttrRect * AttrRect::readNode( QDomNode node)
{
    QDomElement element;
    int x, y, w, h;
    element = node.toElement();
    if( element.isNull()) return NULL;
    if( getXMLAttribute( element, "x", x) == false) return NULL;
    if( getXMLAttribute( element, "y", y) == false) return NULL;
    if( getXMLAttribute( element, "w", w) == false) return NULL;
    if( getXMLAttribute( element, "h", h) == false) return NULL;
    AFINFA("AttrRect::readNode: %s %d %d %d %d\n", element.text().toUtf8().data(), x,y,w,h);
    return new AttrRect( element.text(), QRect( x, y, w, h));
}

void AttrRect::write( QByteArray & data)
{
   data.append(QString(
         "        <%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\">%6</%7>\n")
                .arg( WndTagName).arg( r.x()).arg( r.y()).arg( r.width()).arg( r.height()).arg( getName()).arg( WndTagName));
}
/*
   <wndrect x="10" y="20" w="100" h="200">jobs</wndrect>
   <wndrect x="20" y="30" w="200" h="300">renders</wndrect>
*/
