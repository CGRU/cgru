#include "ctrlrenders.h"

#include "actionid.h"
#include "listrenders.h"

#include <QtCore/QEvent>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>

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
	QAction * action;

	action = new QAction("Hide:", this);
	action->setEnabled( false);
	menu.addAction( action);
    menu.addSeparator();

	action_id = new ActionId( ListNodes::e_HideInvert, "Invert", this);
	action_id->setCheckable( true);
	action_id->setChecked( m_list->getFlagsHideShow() & ListNodes::e_HideInvert);
	connect( action_id, SIGNAL( triggeredId( int ) ), m_list, SLOT( actHideShow( int) ));
	menu.addAction( action_id);
    menu.addSeparator();

	action_id = new ActionId( ListNodes::e_HideHidden, "Hidden", this);
	action_id->setCheckable( true);
	action_id->setChecked( m_list->getFlagsHideShow() & ListNodes::e_HideHidden);
	connect( action_id, SIGNAL( triggeredId( int ) ), m_list, SLOT( actHideShow( int) ));
	menu.addAction( action_id);

	action_id = new ActionId( ListNodes::e_HideOffline, "Offline", this);
	action_id->setCheckable( true);
	action_id->setChecked( m_list->getFlagsHideShow() & ListNodes::e_HideOffline);
	connect( action_id, SIGNAL( triggeredId( int ) ), m_list, SLOT( actHideShow( int) ));
	menu.addAction( action_id);

    menu.addSeparator();

	action = new QAction("Size:", this);
	action->setEnabled( false);
	menu.addAction( action);
    menu.addSeparator();

    action_id = new ActionId( int(ListRenders::EVariableSize), "Variable", this);
    action_id->setCheckable( true);
    action_id->setChecked( ListRenders::getDisplaySize() == ListRenders::EVariableSize);
    connect( action_id, SIGNAL( triggeredId( int ) ), m_list, SLOT( actChangeSize( int) ));
    menu.addAction( action_id);

    action_id = new ActionId( int(ListRenders::EBigSize), "Big", this);
    action_id->setCheckable( true);
    action_id->setChecked( ListRenders::getDisplaySize() == ListRenders::EBigSize);
    connect( action_id, SIGNAL( triggeredId( int ) ), m_list, SLOT( actChangeSize( int) ));
    menu.addAction( action_id);

    action_id = new ActionId( int(ListRenders::ENormalSize), "Normal", this);
    action_id->setCheckable( true);
    action_id->setChecked( ListRenders::getDisplaySize() == ListRenders::ENormalSize);
    connect( action_id, SIGNAL( triggeredId( int ) ), m_list, SLOT( actChangeSize( int) ));
    menu.addAction( action_id);

    action_id = new ActionId( int(ListRenders::ESMallSize), "Small", this);
    action_id->setCheckable( true);
    action_id->setChecked( ListRenders::getDisplaySize() == ListRenders::ESMallSize);
    connect( action_id, SIGNAL( triggeredId( int ) ), m_list, SLOT( actChangeSize( int) ));
    menu.addAction( action_id);

    menu.exec( event->globalPos());
}
