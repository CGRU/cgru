#pragma once

#include "attr.h"

#include <QtGui/QColor>

class QByteArray;
class QDomDocument;

class afqt::AttrColor: public Attr
{
public:
   AttrColor( const QString & Name, const QString & Label, const QString & initString);
   ~AttrColor();

	virtual bool v_read( const JSON & i_obj);
	const QString v_writeData();

public:
   QColor c;

private:
	bool readColor();
};
