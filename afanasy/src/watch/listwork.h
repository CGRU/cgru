#pragma once

#include "listjobs.h"

#include "../libafanasy/msg.h"
#include "../libafanasy/taskexec.h"

class QItemSelection;

class ItemBranch;
class ItemJob;

class ListWork : public ListJobs
{
	Q_OBJECT

  public:
	ListWork(QWidget *i_parent);
	~ListWork();

	bool v_caseMessage(af::Msg *msg);

	ItemNode *v_createNewItemNode(af::Node *i_afnode, Item::EType i_type, bool i_notify);

	virtual bool v_processEvents(const af::MonitorEvents &i_me);

	bool v_filesReceived(const af::MCTaskUp &i_taskup);

	virtual void v_hideChanged();

  protected:
	void contextMenuEvent(QContextMenuEvent *event);
	void v_doubleClicked(Item *i_item);

  private slots:
	void slot_ACC_Enable();
	void slot_ACC_Disable();
	void slot_SolveJobs();
	void slot_SolveUsers();
	void slot_SolvePiority();
	void slot_SolveOrder();
	void slot_SolveTasks();
	void slot_SolveCapacity();
	void slot_DelDoneJobs();

	void slot_JobSetBranch();

  private:
	void addBranch(int i_parent_id, const QString &i_child);
	void jobSetBranch(const QString &i_name);
	void editServiceDialog(const QString &i_mode, const QString &i_dialog_caption);
	void launchCmdExit(bool i_exit);
	void launchCmdStringExit(const QString &i_cmd, bool i_exit);
	void calcTitle();

	void ticketEdit(bool i_host_ticket);
	void ticketEdit(const QString &i_name, bool i_host_ticket);

  private:
	static uint32_t ms_hide_flags;
};
