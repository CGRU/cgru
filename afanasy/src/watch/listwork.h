#pragma once

#include "listnodes.h"

#include "../libafanasy/msg.h"
#include "../libafanasy/taskexec.h"

class QItemSelection;

class ItemBranch;
class ItemJob;

class ListWork : public ListNodes
{
	Q_OBJECT

public:
	ListWork(QWidget* parent);
	~ListWork();

	bool v_caseMessage(af::Msg * msg);

	ItemNode * v_createNewItemNode(af::Node * i_afnode, Item::EType i_type, bool i_notify);

	virtual bool v_processEvents(const af::MonitorEvents & i_me);

	bool v_filesReceived(const af::MCTaskUp & i_taskup);

protected:
	void contextMenuEvent( QContextMenuEvent *event);

public:
	 enum EDisplaySize
	 {
		  EVariableSize,
		  EBigSize,
		  ENormalSize,
		  ESmallSize
	 };
	 static EDisplaySize getDisplaySize() { return ms_displaysize; }

private slots:

	void slot_ChangeSize( int i_size);

	void slot_ACC_Enable();
	void slot_ACC_Disable();
	void slot_SolveJobs();
	void slot_SolveUsers();
	void slot_SolvePiority();
	void slot_SolveOrder();
	void slot_SolveTasks();
	void slot_SolveCapacity();

	void slot_Start();
	void slot_Pause();

	void slot_JobSetBranch();
	void slot_Delete();
	void slot_MaxTasks();
	void slot_RequestLog();

private:
	void addBranch(int i_parent_id, const QString & i_child);
	void jobSetBranch(const QString & i_name);
	void editServiceDialog(const QString & i_mode, const QString & i_dialog_caption);
	void launchCmdExit( bool i_exit);
	void launchCmdStringExit(const QString & i_cmd, bool i_exit);
	void calcTitle();
	void setSpacing();

	void ticketEdit(bool i_host_ticket);
	void ticketEdit(const QString & i_name, bool i_host_ticket);

private:
	static EDisplaySize ms_displaysize;

	static int     ms_SortType1;
	static int     ms_SortType2;
	static bool    ms_SortAscending1;
	static bool    ms_SortAscending2;
	static int     ms_FilterType;
	static bool    ms_FilterInclude;
	static bool    ms_FilterMatch;
	static std::string ms_FilterString;

	static uint32_t ms_hide_flags;

	QTimer * timer;
};
