#include "ctrljobs.h"

#include <QBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>

#include "../libafqt/qenvironment.h"

#include "button.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const QStringList CtrlJobs::ms_thumbs_names = {"S", "M", "L", "XL"};
const QList<int> CtrlJobs::ms_thumbs_sizes = {25, 50, 100, 200};

CtrlJobs::CtrlJobs(QWidget *i_parent, ListJobs *i_listjobs, bool i_inworklist)
	: QFrame(i_parent), m_listjobs(i_listjobs), m_inworklist(i_inworklist)
{
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Raised);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetMaximumSize);

	QLabel *lThumbs = new QLabel("Thumbs:", this);
	lThumbs->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	layout->addWidget(lThumbs);

	for (int i = 0; i < ms_thumbs_names.size(); i++)
	{
		Button *btn = new Button(ms_thumbs_names[i], QString(), QString(), false, true);
		layout->addWidget(btn);
		connect(btn, SIGNAL(sig_Clicked(Button *)), this, SLOT(slot_ThumsButtonClicked(Button *)));

		if (m_inworklist)
		{
			if (ms_thumbs_sizes[i] == afqt::QEnvironment::thumb_work_height.n)
				btn->setActive(true);
		}
		else
		{
			if (ms_thumbs_sizes[i] == afqt::QEnvironment::thumb_jobs_height.n)
				btn->setActive(true);
		}

		m_thumbs_btns.append(btn);
	}

	QLabel *lMax = new QLabel("Max(hrs):", this);
	lMax->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	lMax->setToolTip(
		"Mark job blocks with tasks maximum running time above this value.\nType zero to disable.");
	layout->addWidget(lMax);

	m_max_runtime_edit = new QLineEdit(this);
	QDoubleValidator *dv = new QDoubleValidator(0, 24 * 10, 2, m_max_runtime_edit);
	dv->setNotation(QDoubleValidator::StandardNotation);
	m_max_runtime_edit->setValidator(dv);
	m_max_runtime_edit->setFixedWidth(32);
	connect(m_max_runtime_edit, SIGNAL(editingFinished()), this, SLOT(slot_MaxEditingFinished()));
	layout->addWidget(m_max_runtime_edit);
	int seconds = afqt::QEnvironment::jobs_run_time_max_secs.n;
	if (m_inworklist)
		seconds = afqt::QEnvironment::work_run_time_max_secs.n;
	if (seconds)
		m_max_runtime_edit->setText(QString::number(double(seconds) / 60.0 / 60.0, 'f', 2));

	CtrlJobsViewOptions *viewOpts = new CtrlJobsViewOptions(this, m_listjobs, m_inworklist);
	layout->addWidget(viewOpts);
}

CtrlJobs::~CtrlJobs() {}

void CtrlJobs::slot_ThumsButtonClicked(Button *i_btn)
{
	int size = 0;
	for (int i = 0; i < ms_thumbs_names.size(); i++)
		if (i_btn->getName() == ms_thumbs_names[i])
			size = ms_thumbs_sizes[i];

	if (m_inworklist)
	{
		if (size == afqt::QEnvironment::thumb_work_height.n)
			return;
	}
	else
	{
		if (size == afqt::QEnvironment::thumb_jobs_height.n)
			return;
	}

	for (int i = 0; i < m_thumbs_btns.size(); i++)
		if (m_thumbs_btns[i] != i_btn)
			m_thumbs_btns[i]->setActive(false);

	if (m_inworklist)
		afqt::QEnvironment::thumb_work_height.n = size;
	else
		afqt::QEnvironment::thumb_jobs_height.n = size;

	m_listjobs->jobsHeightRecalculate();
}

void CtrlJobs::slot_MaxEditingFinished()
{
	QString text = m_max_runtime_edit->text();
	double hours = text.toDouble();
	if (hours <= 0)
		m_max_runtime_edit->clear();
	int seconds = int(hours * 60 * 60);
	if (seconds < 0)
		seconds = 0;
	if (m_inworklist)
		afqt::QEnvironment::work_run_time_max_secs.n = seconds;
	else
		afqt::QEnvironment::jobs_run_time_max_secs.n = seconds;
	m_listjobs->repaintItems();
}

CtrlJobsViewOptions::CtrlJobsViewOptions(QWidget *i_parent, ListJobs *i_listjobs, bool i_inworklist)
	: QLabel("View Options", i_parent), m_listjobs(i_listjobs), m_inworklist(i_inworklist)
{
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Raised);

	setToolTip("Click RMB for options.");
}

CtrlJobsViewOptions::~CtrlJobsViewOptions() {}

void CtrlJobsViewOptions::contextMenuEvent(QContextMenuEvent *i_event)
{
	QMenu menu(this);
	QAction *action;
	ActionId *action_id;

	action = new QAction("Hide:", this);
	action->setEnabled(false);
	menu.addAction(action);
	menu.addSeparator();

	action_id = new ActionId(ListNodes::e_HideInvert, "Invert", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideInvert);
	connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
	menu.addAction(action_id);
	menu.addSeparator();

	action_id = new ActionId(ListNodes::e_HideHidden, "Hidden", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideHidden);
	connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideDone, "Done", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideDone);
	connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideError, "Error", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideError);
	connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideOffline, "Offline", this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideOffline);
	connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	if (m_inworklist)
	{
		action_id = new ActionId(ListNodes::e_HideEmpty, "Empty", this);
		action_id->setCheckable(true);
		action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideEmpty);
		connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
		menu.addAction(action_id);

		action_id = new ActionId(ListNodes::e_HideSystem, "System", this);
		action_id->setCheckable(true);
		action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideSystem);
		connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
		menu.addAction(action_id);
	}

	menu.addSeparator();

	action = new QAction("Collapse All Jobs", this);
	connect(action, SIGNAL(triggered()), m_listjobs, SLOT(slot_CollapseJobs()));
	menu.addAction(action);

	action = new QAction("Expand All Jobs", this);
	connect(action, SIGNAL(triggered()), m_listjobs, SLOT(slot_ExpandJobs()));
	menu.addAction(action);

	action = new QAction("Collapse New Jobs", this);
	action->setCheckable(true);
	action->setChecked(afqt::QEnvironment::collapseNewJobs());
	connect(action, SIGNAL(triggered(bool)), this, SLOT(slot_CollapseNewJobs(bool)));
	menu.addAction(action);

	menu.exec(i_event->globalPos());
}

void CtrlJobsViewOptions::slot_CollapseNewJobs(bool i_collapse)
{
	afqt::QEnvironment::setCollapseNewJobs(i_collapse);
}
