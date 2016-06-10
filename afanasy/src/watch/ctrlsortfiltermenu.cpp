#include "ctrlsortfiltermenu.h"

#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QPainter>
#include <QMenu>

#include "actionid.h"
#include "ctrlsortfilter.h"

CtrlSortFilterMenu::CtrlSortFilterMenu( QWidget * i_parent, const int * i_checked_type):
	QLabel( i_parent),
	m_checked_type(i_checked_type)
{
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Raised);

	setText("Menu");
}

CtrlSortFilterMenu::~CtrlSortFilterMenu()
{
}

void CtrlSortFilterMenu::contextMenuEvent(QContextMenuEvent * i_event)
{
	QMenu menu(this);
	QAction *action;

	for( int i = 0; i < m_types.size(); i++)
	{
		ActionId * action = new ActionId( m_types[i], CtrlSortFilter::TNAMES[m_types[i]], this);
		action->setCheckable( true);
		action->setChecked( m_types[i] == *m_checked_type);
		connect( action, SIGNAL( triggeredId( int) ), this, SLOT( slot_changed( int ) ));
		menu.addAction( action);
	}

	menu.exec( i_event->globalPos());
}

void CtrlSortFilterMenu::mouseDoubleClickEvent( QMouseEvent * i_event)
{
	emit sig_dblclick();
}

void CtrlSortFilterMenu::slot_changed( int i_type)
{
	emit sig_changed( i_type);
}
