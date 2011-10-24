#include "ctrlrenders.h"

#include <QtCore/QEvent>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>

#include "listrenders.h"

CtrlRenders::CtrlRenders( QWidget * parent, ListRenders * renderslist):
   QLabel( "[O]", parent ),
   list( renderslist)
{
   setFixedHeight(16);
}

CtrlRenders::~CtrlRenders()
{
}

void CtrlRenders::contextMenuEvent(QContextMenuEvent *event)
{
   QMenu menu(this);
   QAction *action;

   action = new QAction( "Constants Height", this);
   action->setCheckable( true);
   action->setChecked( ListRenders::ConstHeight);
   connect( action, SIGNAL( triggered() ), this, SLOT( actExpandTasks() ));
   menu.addAction( action);

   menu.exec( event->globalPos());
}

void CtrlRenders::actExpandTasks()
{
   ListRenders::ConstHeight = false == ListRenders::ConstHeight;

   list->itemsHeightCahnged();
   list->revertModel();
   list->repaintItems();
}
