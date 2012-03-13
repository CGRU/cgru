#pragma once

#include <QtGui/QColor>

#include "attr.h"

class QByteArray;
class QDomDocument;

class afqt::AttrNumber: public Attr
{
public:
   AttrNumber( const QString & Name, const QString & Label, int initNumber);
   ~AttrNumber();

   bool readData();
   void writeData();

public:
   int n;

private:
};
