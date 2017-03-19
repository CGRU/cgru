#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/monitor.h"
#include "../libafanasy/monitorevents.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "listnodes.h"

class ListJobs : public ListNodes
{
	Q_OBJECT

public:
	ListJobs( QWidget* parent);
	~ListJobs();

	bool v_caseMessage( af::Msg * msg);

	ItemNode* v_createNewItem( af::Node * i_node, bool i_subscibed);

	virtual bool v_processEvents( const af::MonitorEvents & i_me);

	bool v_filesReceived( const af::MCTaskUp & i_taskup );

protected:
	void contextMenuEvent(QContextMenuEvent *event);

	void v_showFunc();

	void doubleClicked( Item * item);

//	void v_connectionLost();

	void v_resetSorting();

private slots:
	void actMoveUp();
	void actMoveDown();
	void actMoveTop();
	void actMoveBottom();

	void actSetUser();
	void actHostsMask();
	void actHostsMaskExclude();
	void actMaxRunningTasks();
	void actMaxRunTasksPerHost();
	void actDependMask();
	void actDependMaskGlobal();
	void actWaitTime();
	void actNeedOS();
	void actNeedProperties();
	void actPostCommand();
	void actLifeTime();
	void actSetHidden();
	void actUnsetHidden();
	void actPreviewApproval();
	void actNoPreviewApproval();

	void blockAction( int id_block, QString i_action);

	void actStart();
	void actStop();
	void actRestart();
	void actRestartErrors();
	void actRestartWarnings();
	void actRestartRunning();
	void actRestartSkipped();
	void actRestartDone();
	void actResetErrorHosts();
	void actPause();
	void actRestartPause();
	void actDelete();
	void actDeleteDone();

	void actRequestLog();
	void actRequestErrorHostsList();

	void actListenJob();

	void actBrowseFolder( QString i_folder);
	void actOpenRULES();

private:

	void moveJobs( const std::string & i_operation);

	void getUserJobsOrder();

	void calcTotals();

private:
	// Sorting filtering settings ordinary user:
	static int     ms_SortType1;
	static int     ms_SortType2;
	static bool    ms_SortAscending1;
	static bool    ms_SortAscending2;
	static int     ms_FilterType;
	static bool    ms_FilterInclude;
	static bool    ms_FilterMatch;
	static std::string ms_FilterString;

	// Sorting filtering settings for super user:
	static int     ms_SortType1_SU;
	static int     ms_SortType2_SU;
	static bool    ms_SortAscending1_SU;
	static bool    ms_SortAscending2_SU;
	static int     ms_FilterType_SU;
	static bool    ms_FilterInclude_SU;
	static bool    ms_FilterMatch_SU;
	static std::string ms_FilterString_SU;
};
