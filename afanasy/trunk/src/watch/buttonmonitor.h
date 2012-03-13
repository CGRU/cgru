#pragma once

#include <QtGui/QPushButton>

#include "watch.h"

class ButtonMonitor : public QPushButton
{
   Q_OBJECT

public:
   ButtonMonitor( int wType, QWidget *parent, int width = 0, int height = 0);
   ~ButtonMonitor();

   static const int ButtonsHeight;

   static void unset();

public slots:
   void pressed_SLOT();
   void open_SLOT();

protected:
   void contextMenuEvent( QContextMenuEvent *event);

private:
   int type;

   static ButtonMonitor *Buttons[Watch::WLAST];
   static ButtonMonitor *Current;
   static int CurrentType;
};
