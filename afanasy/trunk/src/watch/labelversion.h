#pragma once

#include <QtGui/QWidget>

class LabelVersion : public QWidget
{
public:
   LabelVersion ( QWidget *parent);
   ~LabelVersion ();

protected:
   void paintEvent( QPaintEvent * event);

private:
   QFont font;
   QString text;
//   QPixmap logo;
};
