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

	void v_write( QByteArray & o_data);

public:
    static AttrRect * readObj( const JSON & i_obj);

public:
    static const QString WndTagName;

public:
    QRect r;

private:
};
