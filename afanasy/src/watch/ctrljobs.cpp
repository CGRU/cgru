#include "ctrljobs.h"

#include <QtCore/QEvent>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>

CtrlJobs::CtrlJobs( QWidget * i_parent, ListJobs * i_listjobs):
	QLabel( "[O]", i_parent ),
	m_listjobs( i_listjobs)
{
	setFixedHeight(16);
}

CtrlJobs::~CtrlJobs()
{
}

void CtrlJobs::contextMenuEvent( QContextMenuEvent * i_event)
{
	QMenu menu(this);
	QAction * action;
	ActionId * action_id;

	action = new QAction("Hide:", this);
	action->setEnabled( false);
	menu.addAction( action);
    menu.addSeparator();

	action_id = new ActionId( ListNodes::e_HideInvert, "Invert", this);
	action_id->setCheckable( true);
	action_id->setChecked( m_listjobs->getFlagsHideShow() & ListNodes::e_HideInvert);
	connect( action_id, SIGNAL( triggeredId( int ) ), m_listjobs, SLOT( actHideShow( int) ));
	menu.addAction( action_id);
    menu.addSeparator();

	action_id = new ActionId( ListNodes::e_HideHidden, "Hidden", this);
	action_id->setCheckable( true);
	action_id->setChecked( m_listjobs->getFlagsHideShow() & ListNodes::e_HideHidden);
	connect( action_id, SIGNAL( triggeredId( int ) ), m_listjobs, SLOT( actHideShow( int) ));
	menu.addAction( action_id);

	action_id = new ActionId( ListNodes::e_HideDone, "Done", this);
	action_id->setCheckable( true);
	action_id->setChecked( m_listjobs->getFlagsHideShow() & ListNodes::e_HideDone);
	connect( action_id, SIGNAL( triggeredId( int ) ), m_listjobs, SLOT( actHideShow( int) ));
	menu.addAction( action_id);

	action_id = new ActionId( ListNodes::e_HideError, "Error", this);
	action_id->setCheckable( true);
	action_id->setChecked( m_listjobs->getFlagsHideShow() & ListNodes::e_HideError);
	connect( action_id, SIGNAL( triggeredId( int ) ), m_listjobs, SLOT( actHideShow( int) ));
	menu.addAction( action_id);

	action_id = new ActionId( ListNodes::e_HideOffline, "Offline", this);
	action_id->setCheckable( true);
	action_id->setChecked( m_listjobs->getFlagsHideShow() & ListNodes::e_HideOffline);
	connect( action_id, SIGNAL( triggeredId( int ) ), m_listjobs, SLOT( actHideShow( int) ));
	menu.addAction( action_id);

	menu.exec( i_event->globalPos());
}
