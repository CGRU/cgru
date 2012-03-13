#pragma once

#include <QtGui/QColor>

#include "attr.h"

class QByteArray;
class QDomDocument;

class afqt::AttrColor: public Attr
{
public:
   AttrColor( const QString & Name, const QString & Label, const QString & initString);
   ~AttrColor();

   bool readData();
   void writeData();

public:
   QColor c;

private:
};
