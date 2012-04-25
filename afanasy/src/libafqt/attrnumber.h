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

   bool readData();
   void writeData();

public:
   int n;

private:
};
