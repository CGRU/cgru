#include "ctrlrenders.h"

#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>
#include <QLabel>
#include <QBoxLayout>
#include <QMenu>

#include "actionid.h"
#include "button.h"
#include "listrenders.h"

const QStringList CtrlRenders::ms_sizes_names = {"VR","S","M","L"};
const QList<int>  CtrlRenders::ms_sizes_enums = {  0 , 1 , 2 , 3 };

CtrlRenders::CtrlRenders(QWidget * i_parent, ListRenders * i_listrenders):
	QFrame(i_parent),
	m_listrenders(i_listrenders)
{
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Raised);

	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMaximumSize);

	QLabel * lSize = new QLabel("Size:", this);
	lSize->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	layout->addWidget(lSize);

	for (int i = 0; i < ms_sizes_names.size(); i++)
	{
		Button * btn = new Button(ms_sizes_names[i], QString(), QString(), false, true);
		layout->addWidget(btn);
		connect(btn, SIGNAL(sig_Clicked(Button*)), this, SLOT(slot_ThumsButtonClicked(Button*)));

		if (ms_sizes_enums[i] == afqt::QEnvironment::render_item_size.n)
			btn->setActive(true);

		m_thumbs_btns.append(btn);

		if (i == 0)
			btn->setToolTip("Variable size.");
	}

	QLabel * lMax = new QLabel("Max(hrs):", this);
	lMax->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	lMax->setToolTip("Mark tasks maximum running time above this value.\nType zero to disable.");
	layout->addWidget(lMax);

	m_max_runtime_edit = new QLineEdit(this);
	QDoubleValidator * dv = new QDoubleValidator(0, 24*10, 2, m_max_runtime_edit);
	dv->setNotation(QDoubleValidator::StandardNotation);
	m_max_runtime_edit->setValidator(dv);
	m_max_runtime_edit->setFixedWidth(32);
	connect(m_max_runtime_edit, SIGNAL(editingFinished()), this, SLOT(slot_MaxEditingFinished()));
	layout->addWidget(m_max_runtime_edit);
	int seconds = afqt::QEnvironment::renders_run_time_max_secs.n;
	if (seconds)
		m_max_runtime_edit->setText(QString::number(double(seconds) / 60.0 / 60.0, 'f', 2));

	CtrlRendersViewOptions * viewOpts = new CtrlRendersViewOptions(this, m_listrenders);
	layout->addWidget(viewOpts);
}

CtrlRenders::~CtrlRenders()
{
}

void CtrlRenders::slot_ThumsButtonClicked(Button * i_btn)
{
	int size = 0;
	for (int i = 0; i < ms_sizes_names.size(); i++)
		if (i_btn->getName() == ms_sizes_names[i])
			size = ms_sizes_enums[i];

	if (size == afqt::QEnvironment::thumb_work_height.n)
		return;

	for (int i = 0; i < m_thumbs_btns.size(); i++)
		if (m_thumbs_btns[i] != i_btn)
			m_thumbs_btns[i]->setActive(false);

	afqt::QEnvironment::render_item_size.n = size;

	m_listrenders->itemsSizeChanged();
}

void CtrlRenders::slot_MaxEditingFinished()
{
	QString text = m_max_runtime_edit->text();
	double hours = text.toDouble();
	if (hours <= 0)
		m_max_runtime_edit->clear();
	int seconds = int(hours * 60 * 60);
	if (seconds < 0)
		seconds = 0;
	afqt::QEnvironment::renders_run_time_max_secs.n = seconds;
	m_listrenders->repaintItems();
}

CtrlRendersViewOptions::CtrlRendersViewOptions(QWidget * i_parent, ListRenders * i_listrenders):
	QLabel("View Options", i_parent),
	m_listrenders(i_listrenders)
{
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Raised);
}

CtrlRendersViewOptions::~CtrlRendersViewOptions()
{
}

void CtrlRendersViewOptions::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	ActionId * action_id;
	QAction * action;

	action = new QAction("Hide:", this);
	action->setEnabled(false);
	menu.addAction(action);
	menu.addSeparator();

	action_id = new ActionId(ListNodes::e_HideInvert, "Invert", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listrenders->getFlagsHideShow() & ListNodes::e_HideInvert);
	connect(action_id, SIGNAL(triggeredId(int)), m_listrenders, SLOT(actHideShow(int)));
	menu.addAction(action_id);
	menu.addSeparator();

	action_id = new ActionId(ListNodes::e_HideHidden, "Hidden", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listrenders->getFlagsHideShow() & ListNodes::e_HideHidden);
	connect(action_id, SIGNAL(triggeredId(int)), m_listrenders, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideOffline, "Offline", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listrenders->getFlagsHideShow() & ListNodes::e_HideOffline);
	connect(action_id, SIGNAL(triggeredId(int)), m_listrenders, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HidePools, "Pools", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listrenders->getFlagsHideShow() & ListNodes::e_HidePools);
	connect(action_id, SIGNAL(triggeredId(int)), m_listrenders, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	menu.exec(event->globalPos());
}
