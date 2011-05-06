#pragma once

#include <QtGui/QWidget>

class LabelVersion : public QWidget
{
public:
   LabelVersion ( QWidget *parent);
   ~LabelVersion ();

   void showMessage( const std::string & str);

protected:
   void paintEvent( QPaintEvent * event);
   void mousePressEvent( QMouseEvent * event);

private:
   QFont font;
   QString text;
   QString tooltip;
   QString message;
   //   QPixmap logo;

   bool alarm;
};
