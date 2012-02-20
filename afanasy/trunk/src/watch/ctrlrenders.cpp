#include "ctrlrenders.h"

#include <QtCore/QEvent>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>

#include "actionid.h"
#include "listrenders.h"

CtrlRenders::CtrlRenders( QWidget * i_parent, ListRenders * i_renderslist):
   QLabel( "[O]", i_parent ),
   m_list( i_renderslist)
{
   setFixedHeight(16);
}

CtrlRenders::~CtrlRenders()
{
}

void CtrlRenders::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    ActionId * action_id;

    action_id = new ActionId( int(ListRenders::EVariableSize), "Variable Size", this);
    action_id->setCheckable( true);
    action_id->setChecked( ListRenders::getDisplaySize() == ListRenders::EVariableSize);
    connect( action_id, SIGNAL( triggeredId( int ) ), m_list, SLOT( actChangeSize( int) ));
    menu.addAction( action_id);

    action_id = new ActionId( int(ListRenders::EBigSize), "Big Size", this);
    action_id->setCheckable( true);
    action_id->setChecked( ListRenders::getDisplaySize() == ListRenders::EBigSize);
    connect( action_id, SIGNAL( triggeredId( int ) ), m_list, SLOT( actChangeSize( int) ));
    menu.addAction( action_id);

    menu.exec( event->globalPos());
}
