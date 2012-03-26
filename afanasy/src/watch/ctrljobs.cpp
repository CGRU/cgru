#include "ctrljobs.h"

#include <QtCore/QEvent>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>

#include "actionid.h"
#include "listjobs.h"

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
	ActionId * action_id;

	action_id = new ActionId( 0, "Show Hidden", this);
	action_id->setCheckable( true);
	action_id->setChecked( m_listjobs->isShowingHidden());
	connect( action_id, SIGNAL( triggeredId( int ) ), m_listjobs, SLOT( actShowHidden( int) ));
	menu.addAction( action_id);

	menu.exec( i_event->globalPos());
}
