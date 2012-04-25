#pragma once

#include "watch.h"

#include <QtGui/QImage>
#include <QtGui/QWidget>

class ButtonMonitor : public QWidget
{
   Q_OBJECT

public:
   ButtonMonitor( int wType, QWidget *parent);
   ~ButtonMonitor();

   static void refreshImages();

   static void pushButton( int wType);

   static const int ButtonsHeight;

   static void unset();

protected:
   void contextMenuEvent( QContextMenuEvent *event);

   void paintEvent ( QPaintEvent * event );

   void enterEvent( QEvent * event);
   void leaveEvent( QEvent *event);
   void mousePressEvent( QMouseEvent * event );
   void mouseReleaseEvent( QMouseEvent * event );

private slots:
   void open_SLOT();

private:
   void createImage();
   void openMonitor( bool inSeparateWindow);

private:
   bool hovered;
   bool pressed;
   int type;

   int width;
   int height;

   bool useimages;
   QImage img;
   QImage img_h;
   QImage img_p;
   QImage img_t;

private:
   static ButtonMonitor *Buttons[Watch::WLAST];
   static ButtonMonitor *Current;
   static int CurrentType;
};
