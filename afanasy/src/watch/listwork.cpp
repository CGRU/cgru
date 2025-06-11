#include "listwork.h"

#include "../include/afanasy.h"

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/monitor.h"
#include "../libafanasy/monitorevents.h"
#include "../libafanasy/msgclasses/mctaskpos.h"

#include "actionid.h"
#include "buttonpanel.h"
#include "ctrlsortfilter.h"
#include "dialog.h"
#include "itembranch.h"
#include "itemjob.h"
#include "modelnodes.h"
#include "paramspanelfarm.h"
#include "viewitems.h"
#include "watch.h"
#include "wndtask.h"

#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QContextMenuEvent>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

uint32_t ListWork::ms_hide_flags = e_HideHidden | e_HideSystem | e_HideDone | e_HideOffline | e_HideEmpty;

ListWork::ListWork(QWidget *i_parent) : ListJobs(i_parent, true, "work")
{
	m_node_types.clear();
	m_node_types.push_back("branches");
	m_node_types.push_back("jobs");

	m_hide_flags = ms_hide_flags;

	this->setWindowTitleWithPrefix("Work");

	// Add left panel buttons:
	ButtonPanel *bp;

	if (af::Environment::VISOR())
	{
		addButtonsMenu(Item::TBranch, "Create Childs", "Create branch childs or not.");

		bp = addButtonPanel(Item::TBranch, "AUTO", "branch_childs_create",
							"Automatically create branch childs.");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_ACC_Enable()));
		bp = addButtonPanel(Item::TBranch, "DISABLED", "branch_childs_nocreate",
							"Do not create branch childs.");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_ACC_Disable()));

		addButtonsMenu(Item::TBranch, "Solve Method", "Solve by prority or order.");

		bp = addButtonPanel(Item::TBranch, "PRIORITY", "branch_solve_priority", "Solve by priority.");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_SolvePiority()));
		bp = addButtonPanel(Item::TBranch, "ORDER", "branch_solve_order", "Solve by order.");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_SolveOrder()));

		addButtonsMenu(Item::TBranch, "Solve Type", "Solve users or jobs.");

		bp = addButtonPanel(Item::TBranch, "JOBS", "branch_solve_jobs", "Solve branch jobs.");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_SolveJobs()));
		bp = addButtonPanel(Item::TBranch, "USERS", "branch_solve_users", "Solve branch users.");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_SolveUsers()));

		resetButtonsMenu();

		addButtonsMenu(Item::TBranch, "Solve Need", "Solve need parameter (by capacity or tasks).");

		bp = addButtonPanel(Item::TBranch, "TASKS", "branch_solve_tasks", "Solve by running tasks number.");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_SolveTasks()));
		bp = addButtonPanel(Item::TBranch, "CAPACITY", "branch_solve_capacity",
							"Solve by running capacity total.");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_SolveCapacity()));

		resetButtonsMenu();

		bp = addButtonPanel(Item::TJob, "SET BRANCH", "job_change_branch", "Change job branch.");
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_JobSetBranch()));

		bp = addButtonPanel(Item::TBranch, "DEL DONE JOBS", "branch_del_done_jobs",
							"Delete done jobs from branch.", "", true);
		connect(bp, SIGNAL(sigClicked()), this, SLOT(slot_DelDoneJobs()));
	}
	else
		m_paramspanel->v_setEditable(false);
}

ListWork::~ListWork() {}

void ListWork::v_hideChanged()
{
	// Store hide flags:
	ms_hide_flags = m_hide_flags;
}

void ListWork::contextMenuEvent(QContextMenuEvent *event)
{
	if (false == af::Environment::VISOR())
		return;

	Item *item = getCurrentItem();
	if (item == NULL)
		return;

	if (item->getType() == Item::TJob)
	{
		ListJobs::contextMenuEvent(event);
		return;
	}

	ItemBranch *itembranch = static_cast<ItemBranch *>(item);

	QMenu menu(this);
	QAction *action;

	action = new QAction("Show Log", this);
	connect(action, SIGNAL(triggered()), this, SLOT(slot_RequestLog()));
	menu.addAction(action);

	menu.exec(event->globalPos());
}

void ListWork::v_doubleClicked(Item *i_item)
{
	if (false == af::Environment::VISOR())
		return;

	ListJobs::v_doubleClicked(i_item);
}

bool ListWork::v_caseMessage(af::Msg *msg)
{
#ifdef AFOUTPUT
	msg->stdOut();
#endif
	switch (msg->type())
	{
		case af::Msg::TBranchesList:
		{
			updateItems(msg, Item::TBranch);
			if (false == isSubscribed())
				get("jobs");
			calcTitle();
			break;
		}
		case af::Msg::TJobsList:
		{
			subscribe();
			updateItems(msg, Item::TJob);
			calcTitle();
			break;
		}
		default: return false;
	}
	return true;
}

bool ListWork::v_processEvents(const af::MonitorEvents &i_me)
{
	bool processed = false;

	// Delete jobs by ids:
	if (i_me.m_events[af::Monitor::EVT_jobs_del].size())
	{
		deleteItems(i_me.m_events[af::Monitor::EVT_jobs_del], Item::TJob);
		calcTitle();
		processed = true;
	}

	// Delete branches by ids:
	if (i_me.m_events[af::Monitor::EVT_branches_del].size())
	{
		deleteItems(i_me.m_events[af::Monitor::EVT_branches_del], Item::TBranch);
		calcTitle();
		processed = true;
	}

	// Get new and changed branches ids:
	std::vector<int> pids;
	for (int i = 0; i < i_me.m_events[af::Monitor::EVT_branches_change].size(); i++)
		af::addUniqueToVect(pids, i_me.m_events[af::Monitor::EVT_branches_change][i]);
	for (int i = 0; i < i_me.m_events[af::Monitor::EVT_branches_add].size(); i++)
		af::addUniqueToVect(pids, i_me.m_events[af::Monitor::EVT_branches_add][i]);
	if (pids.size())
	{
		get(pids, "branches");
		processed = true;
	}

	// Get new and changed jobs ids:
	std::vector<int> rids;
	for (int i = 0; i < i_me.m_events[af::Monitor::EVT_jobs_change].size(); i++)
		af::addUniqueToVect(rids, i_me.m_events[af::Monitor::EVT_jobs_change][i]);
	for (int i = 0; i < i_me.m_events[af::Monitor::EVT_jobs_add].size(); i++)
		af::addUniqueToVect(rids, i_me.m_events[af::Monitor::EVT_jobs_add][i]);
	if (rids.size())
	{
		get(rids, "jobs");
		processed = true;
	}

	return processed;
}

ItemNode *ListWork::v_createNewItemNode(af::Node *i_afnode, Item::EType i_type, bool i_notify)
{
	switch (i_type)
	{
		case Item::TJob:
			return new ItemJob(this, true /*in work list*/, (af::Job *)i_afnode, m_ctrl_sf, i_notify);
		case Item::TBranch: return new ItemBranch(this, (af::Branch *)i_afnode, m_ctrl_sf);
		default: AF_ERR << "Invalid Item::EType: " << i_type; return NULL;
	}
}

bool ListWork::v_filesReceived(const af::MCTaskUp &i_taskup)
{
	if ((i_taskup.getNumBlock() != -1) || (i_taskup.getNumTask() != -1))
		return false; // This is for a task (not for an entire job)

	for (int i = 0; i < count(); i++)
	{
		Item *item = (ItemJob *)(m_model->item(i));
		if (NULL == item)
			continue;
		if (item->getType() != Item::TJob)
			continue;
		if (item->getId() != i_taskup.getNumJob())
			continue;

		ItemJob *itemjob = static_cast<ItemJob *>(item);
		itemjob->v_filesReceived(i_taskup);
		return true;
	}

	return false;
}

void ListWork::calcTitle()
{
	int btotal = 0;
	int bempty = 0;

	int jtotal = 0;
	int jpercent = 0;
	int jdone = 0;
	int jrunning = 0;
	int jerror = 0;
	int jblocksrun = 0;

	for (int i = 0; i < count(); i++)
	{
		Item *item = m_model->item(i);

		if (item->getType() == Item::TBranch)
		{
			btotal++;
			ItemBranch *branch = static_cast<ItemBranch *>(item);

			if (branch->jobs_total == 0)
				bempty++;

			continue;
		}

		jtotal++;
		ItemJob *job = static_cast<ItemJob *>(item);

		if (job->state & AFJOB::STATE_DONE_MASK)
		{
			jdone++;
		}
		else
		{
			for (int b = 0; b < job->getBlocksNum(); b++)
			{
				jpercent += job->getBlockPercent(b);
				jblocksrun++;
			}
		}
		if (job->state & AFJOB::STATE_RUNNING_MASK)
			jrunning++;
		if (job->state & AFJOB::STATE_ERROR_MASK)
			jerror++;
	}

	QString branches = QString("Branches: %1, Empty %2").arg(btotal).arg(bempty);

	QString jobs;
	if (jblocksrun)
		jobs = QString("Jobs: %1, Run %2 (%3%), Error %4, Done %5")
				   .arg(jtotal)
				   .arg(jrunning)
				   .arg(jpercent / jblocksrun)
				   .arg(jerror)
				   .arg(jdone);
	else
		jobs = QString("Jobs: %1 Done").arg(jtotal);

	this->setWindowTitleWithPrefix(jobs + "; " + branches);
}

void ListWork::slot_ACC_Enable() { setParameter(Item::TBranch, "create_childs", "true"); }
void ListWork::slot_ACC_Disable() { setParameter(Item::TBranch, "create_childs", "false"); }
void ListWork::slot_SolveJobs() { setParameter(Item::TBranch, "solve_jobs", "true"); }
void ListWork::slot_SolveUsers() { setParameter(Item::TBranch, "solve_jobs", "false"); }
void ListWork::slot_SolvePiority() { setParameterStr(Item::TBranch, "solve_method", "solve_priority"); }
void ListWork::slot_SolveOrder() { setParameterStr(Item::TBranch, "solve_method", "solve_order"); }
void ListWork::slot_SolveTasks() { setParameterStr(Item::TBranch, "solve_need", "solve_tasks"); }
void ListWork::slot_SolveCapacity() { setParameterStr(Item::TBranch, "solve_need", "solve_capacity"); }
void ListWork::slot_DelDoneJobs() { operation(Item::TBranch, "delete_done_jobs"); }

void ListWork::slot_JobSetBranch()
{
	Item *item = getCurrentItem();
	if (NULL == item)
		return;

	if (item->getType() != Item::TJob)
		return;

	QString branch((static_cast<ItemJob *>(item))->branch);

	bool ok;
	branch = QInputDialog::getText(this, "Set Job Branch", "Enter job(s) new branch branch",
								   QLineEdit::Normal, branch, &ok);
	if (false == ok)
		return;

	branch = QString("%1").arg(branch);

	setParameterQStr(Item::TJob, "branch", branch);

	displayInfo(QString("Setting branch to %1").arg(branch));
}

void ListWork::jobSetBranch(const QString &i_name)
{
	Item::EType type = Item::TJob;
	std::vector<int> ids(getSelectedIds(type));
	std::ostringstream str;
	af::jsonActionOperationStart(str, "jobs", "set_branch", "", ids);
	str << ",\n\"name\":\"" << afqt::qtos(i_name) << "\"";
	af::jsonActionOperationFinish(str);
	Watch::sendMsg(af::jsonMsg(str));
}
