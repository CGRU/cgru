#pragma once

#include "../libafanasy/monitor.h"
#include "../libafanasy/monitorevents.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "listnodes.h"

class ListJobs : public ListNodes
{
	Q_OBJECT

  public:
	ListJobs(QWidget *i_parent, bool i_listwork = false, const std::string &i_name = "jobs");
	~ListJobs();

	bool v_caseMessage(af::Msg *msg);

	ItemNode *v_createNewItemNode(af::Node *i_afnode, Item::EType i_type, bool i_notify);

	virtual bool v_processEvents(const af::MonitorEvents &i_me);

	bool v_filesReceived(const af::MCTaskUp &i_taskup);

	virtual void v_hideChanged();

	// Called on delete ids event.
	// Not called on list dtor.
	void v_itemToBeDeleted(Item *i_item);

	void jobsHeightRecalculate();

  protected:
	void contextMenuEvent(QContextMenuEvent *event);

	void v_showFunc();

	void v_doubleClicked(Item *i_item);

	void v_resetSorting();

  private slots:
	void actMoveUp();
	void actMoveDown();
	void actMoveTop();
	void actMoveBottom();

	void actSetUser();
	void actChangeBranch();
	void actPostCommand();
	void actSetHidden();
	void actUnsetHidden();
	void actPreviewApproval();
	void actNoPreviewApproval();
	void actHostsMaskFind();
	void actHostsMaskRegEx();

	void actStart();
	void actStop();
	void actRestart();
	void actRestartErrors();
	void actRestartWarnings();
	void actRestartRunning();
	void actRestartSkipped();
	void actRestartDone();
	void actResetErrorHosts();
	void actResetTryingNextTasks();
	void actPause();
	void actRestartPause();
	void actDelete();
	void actDeleteDone();

	void actRequestLog();
	void actRequestErrorHostsList();

	void actListenJob();

	void actBrowseFolder(QString i_folder);
	void actOpenRULES();

	void slot_CollapseJobs();
	void slot_ExpandJobs();

	// Needed to store where a menu for all blocks manipulation opened
	void slot_BlocksMenuForAll();
	void slot_BlocksMenuNotAll();

	void slot_BlockAction(int i_bum, QString i_json);

  private:
	void moveJobs(const std::string &i_operation);

	void getUserJobsOrder();

	void calcTotals();

	void collapseJobs(bool i_collapse);

  private:
	const bool m_listwork;

	// Sorting filtering settings ordinary user:
	static int ms_SortType1;
	static int ms_SortType2;
	static bool ms_SortAscending1;
	static bool ms_SortAscending2;
	static int ms_FilterType;
	static bool ms_FilterInclude;
	static bool ms_FilterMatch;
	static bool ms_FilterRegEx;
	static std::string ms_FilterString;

	// Sorting filtering settings for super user:
	static int ms_SortType1_SU;
	static int ms_SortType2_SU;
	static bool ms_SortAscending1_SU;
	static bool ms_SortAscending2_SU;
	static int ms_FilterType_SU;
	static bool ms_FilterInclude_SU;
	static bool ms_FilterMatch_SU;
	static bool ms_FilterRegEx_SU;
	static std::string ms_FilterString_SU;

	// To store what to hide
	static uint32_t ms_hide_flags;

	// Needed to store where a menu for all blocks manipulation opened
	bool m_all_blocks_menu_shown;
};
