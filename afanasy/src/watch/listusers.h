#pragma once

#include "../libafanasy/msg.h"

#include "listnodes.h"

class ListUsers : public ListNodes
{
	Q_OBJECT

public:
	ListUsers( QWidget* parent);
	~ListUsers();

	bool v_caseMessage( af::Msg * msg);

	ItemNode * v_createNewItem( af::Node * i_node, bool i_subscibed);

	virtual bool v_processEvents( const af::MonitorEvents & i_me);

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
	static int     ms_SortType1;
	static int     ms_SortType2;
	static bool    ms_SortAscending1;
	static bool    ms_SortAscending2;
	static int     ms_FilterType;
	static bool    ms_FilterInclude;
	static bool    ms_FilterMatch;
	static std::string ms_FilterString;
};
