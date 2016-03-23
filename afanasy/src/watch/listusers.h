#pragma once

#include "../libafanasy/msg.h"

#include "listnodes.h"

class ListUsers : public ListNodes
{
	Q_OBJECT

public:
	ListUsers( QWidget* parent);
	~ListUsers();

	bool caseMessage( af::Msg * msg);

	ItemNode* v_createNewItem( af::Node *node, bool i_subscibed);

	virtual bool processEvents( const af::MonitorEvents & i_me);

protected:
	void contextMenuEvent( QContextMenuEvent *event);

private slots:

	void userAdded( ItemNode * node, const QModelIndex & index);

	void actHostsMask();
	void actHostsMaskExclude();
	void actMaxRunningTasks();
	void actErrorsAvoidHost();
	void actErrorRetries();
	void actErrorsSameHost();
	void actErrorsForgiveTime();
	void actJobsLifeTime();
	void actRequestLog();

	void actSolveJobsByOrder();
	void actSolveJobsParallel();

	void actDelete();

private:
	void calcTitle();

private:
	static int     SortType;
	static bool    SortAscending;
	static QString FilterString;
	static int     FilterType;
	static bool    FilterInclude;
	static bool    FilterMatch;
};
