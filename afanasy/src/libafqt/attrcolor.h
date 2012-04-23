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

   bool readData();
   void writeData();

public:
   QColor c;

private:
};
