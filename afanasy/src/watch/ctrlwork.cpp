#include "ctrlwork.h"

#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>
#include <QLabel>
#include <QMenu>

#include "listwork.h"

CtrlWork::CtrlWork(QWidget * i_parent, ListWork * i_listwork):
	QLabel("View Options", i_parent),
	m_listwork(i_listwork)
{
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Raised);
}

CtrlWork::~CtrlWork()
{
}

void CtrlWork::contextMenuEvent(QContextMenuEvent * i_event)
{
	QMenu menu(this);
	QAction * action;
	ActionId * action_id;

	action = new QAction("Hide:", this);
	action->setEnabled(false);
	menu.addAction(action);
	menu.addSeparator();

	action_id = new ActionId(ListNodes::e_HideInvert, "Invert", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listwork->getFlagsHideShow() & ListNodes::e_HideInvert);
	connect(action_id, SIGNAL(triggeredId(int)), m_listwork, SLOT(actHideShow(int)));
	menu.addAction(action_id);
	menu.addSeparator();

	action_id = new ActionId(ListNodes::e_HideHidden, "Hidden", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listwork->getFlagsHideShow() & ListNodes::e_HideHidden);
	connect(action_id, SIGNAL(triggeredId(int)), m_listwork, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideDone, "Done", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listwork->getFlagsHideShow() & ListNodes::e_HideDone);
	connect(action_id, SIGNAL(triggeredId(int)), m_listwork, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideEmpty, "Empty", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listwork->getFlagsHideShow() & ListNodes::e_HideEmpty);
	connect(action_id, SIGNAL(triggeredId(int)), m_listwork, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideError, "Error", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listwork->getFlagsHideShow() & ListNodes::e_HideError);
	connect(action_id, SIGNAL(triggeredId(int)), m_listwork, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideOffline, "Offline", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listwork->getFlagsHideShow() & ListNodes::e_HideOffline);
	connect(action_id, SIGNAL(triggeredId(int)), m_listwork, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideSystem, "System", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listwork->getFlagsHideShow() & ListNodes::e_HideSystem);
	connect(action_id, SIGNAL(triggeredId(int)), m_listwork, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	menu.exec(i_event->globalPos());
}
