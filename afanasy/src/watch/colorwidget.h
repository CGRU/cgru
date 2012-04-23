#pragma once

#include "../libafqt/name_afqt.h"

#include <QtGui/QWidget>

class ColorWidget : public QWidget
{
Q_OBJECT
public:
   ColorWidget( QWidget * parent, afqt::AttrColor * color);
   ~ColorWidget();

protected:
   virtual void paintEvent( QPaintEvent * event);
   virtual void mouseDoubleClickEvent( QMouseEvent * event);

private slots:
   void currentColorChanged( const QColor & color);
   void finished( int);
   void accepted();

private:
   afqt::AttrColor * clr;
};
