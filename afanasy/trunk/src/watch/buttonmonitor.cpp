#include "buttonmonitor.h"

#include <QtCore/QEvent>
#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>

#include "wndlist.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ButtonMonitor::ButtonsHeight = 25;
ButtonMonitor * ButtonMonitor::Buttons[Watch::WLAST] = {0,0,0,0,0,0};
ButtonMonitor * ButtonMonitor::Current = NULL;
int ButtonMonitor::CurrentType = Watch::WNONE;

ButtonMonitor::ButtonMonitor( int wType, QWidget *parent, int width, int height):
   QPushButton( parent),
   type( Watch::WNONE)
{
   if((wType <= Watch::WNONE) || (wType >= Watch::WLAST))
   {
      AFERRAR("ButtonMonitor::ButtonMonitor: Invalid type = %d\n", wType);
      return;
   }
   type = wType;
   if( Buttons[type] != NULL )
   {
      AFERRAR("ButtonMonitor::ButtonMonitor: Type = %d, already exists.\n", wType);
      return;
   }
   Buttons[type] = this;
   setText( Watch::BtnName[type]);
   setMaximumHeight( ButtonsHeight);
   if( width ) setMaximumWidth( width);
   if( height ) setMaximumHeight( height);
   connect( this, SIGNAL( pressed()), this, SLOT( pressed_SLOT()));
}

void ButtonMonitor::contextMenuEvent( QContextMenuEvent *event)
{
   if( Watch::isConnected() == false ) return;
   if((type == CurrentType) || (type == Watch::WJobs)) return;
   QString itemname;
   if( Watch::opened[type]) itemname = "Raise";
   else itemname = "Open";

   QMenu menu(this);
   QAction *action;
   action = new QAction( itemname, this);
   connect( action, SIGNAL( triggered() ), this, SLOT( open_SLOT() ));
   menu.addAction( action);
   menu.exec( event->globalPos());
}

ButtonMonitor::~ButtonMonitor()
{
   Buttons[type] = NULL;
   if( Current == this)
   {
      Current = NULL;
      CurrentType = Watch::WNONE;
   }
}

void ButtonMonitor::pressed_SLOT()
{
   if( Watch::opened[type])
   {
      Watch::raiseWindow( Watch::opened[type], &Watch::WndName[type]);
      return;
   }
   unset();
   if( Watch::isConnected() == false ) return;

   setFlat(true);
   Current = this;
   CurrentType = type;

   Watch::openMonitor( type, false);
}

void ButtonMonitor::open_SLOT()
{
   if( Watch::opened[type])
   {
      Watch::raiseWindow( Watch::opened[type], &Watch::WndName[type]);
      return;
   }
   if( Watch::isConnected() == false ) return;
   Watch::openMonitor( type, true);
}

void ButtonMonitor::unset()
{
   for( int b = 0; b < Watch::WLAST; b++) if( Buttons[b]) Buttons[b]->setFlat( false);
   Current = NULL;
   CurrentType = Watch::WNONE;
}
