#pragma once

#include "attr.h"

#include <QtGui/QColor>

class QByteArray;
class QDomDocument;

class afqt::AttrNumber: public Attr
{
public:
   AttrNumber( const QString & Name, const QString & Label, int initNumber);
   ~AttrNumber();

	virtual bool v_read( const JSON & i_obj);
	const QString v_writeData();

public:
   int n;

private:
};
