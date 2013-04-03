#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "listnodes.h"

class ListJobs : public ListNodes
{
	Q_OBJECT

public:
	ListJobs( QWidget* parent);
	~ListJobs();

	bool caseMessage( af::Msg * msg);
	ItemNode* createNewItem( af::Node *node);

protected:
	void contextMenuEvent(QContextMenuEvent *event);

	void doubleClicked( Item * item);

	void v_shownFunc();

	void v_connectionLost();

	void resetSorting();

private slots:
	void actMoveUp();
	void actMoveDown();
	void actMoveTop();
	void actMoveBottom();

	void actAnnotate();
	void actSetUser();
	void actPriority();
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

	void blockAction( int id_block, QString i_action);

	void actStart();
	void actStop();
	void actRestart();
	void actRestartErrors();
	void actRestartRunning();
	void actResetErrorHosts();
	void actPause();
	void actRestartPause();
	void actDelete();

	void actRequestLog();
	void actRequestErrorHostsList();

	void actListenJob();

private:
	void moveJobs( const std::string & i_operation);

private:
	// Sorting filtering settings ordinary user:
	static int     SortType;
	static bool    SortAscending;
	static QString FilterString;
	static int     FilterType;
	static bool    FilterInclude;
	static bool    FilterMatch;

	// Sorting filtering settings for super user:
	static int     SortType_SU;
	static bool    SortAscending_SU;
	static QString FilterString_SU;
	static int     FilterType_SU;
	static bool    FilterInclude_SU;
	static bool    FilterMatch_SU;

	void calcTotals();
};
