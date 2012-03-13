#pragma once

#include <QtGui/QWidget>

#include <name_afqt.h>

class NumberWidget : public QWidget
{
Q_OBJECT
public:
   NumberWidget( QWidget * parent, afqt::AttrNumber * attrNumber);
   ~NumberWidget();

private slots:
   void valueChanged( int value);

private:
   afqt::AttrNumber * num;
};
