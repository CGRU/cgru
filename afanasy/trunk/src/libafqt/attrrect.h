#pragma once

#include <QtCore/QRect>

#include "name_afqt.h"
#include "attr.h"

class QByteArray;
class QDomNode;

class afqt::AttrRect: public Attr
{
public:
    AttrRect( const QString & Name, const QRect & initRect);
    ~AttrRect();

    void write( QByteArray & data);

public:
    static AttrRect * readNode( QDomNode node);

public:
    static const QString WndTagName;

public:
    QRect r;

private:
};
