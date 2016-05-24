#pragma once

#include "listnodes.h"

#include "../libafanasy/msg.h"
#include "../libafanasy/taskexec.h"

class QItemSelection;

class ListRenders : public ListNodes
{
	Q_OBJECT

public:
	ListRenders( QWidget* parent);
	~ListRenders();

	bool caseMessage( af::Msg * msg);

	ItemNode* v_createNewItem( af::Node *node, bool i_subscibed);

	virtual bool processEvents( const af::MonitorEvents & i_me);

protected:
	void contextMenuEvent( QContextMenuEvent *event);

	void doubleClicked( Item * item);

public:
	 enum EDisplaySize
	 {
		  EVariableSize,
		  EBigSize,
		  ENormalSize,
		  ESMallSize
	 };
	 static EDisplaySize getDisplaySize() { return ms_displaysize; }

private slots:

	void actChangeSize( int i_size);

	void renderAdded( ItemNode * node, const QModelIndex & index);
	void selectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );

	void actCapacity();
	void actMaxTasks();
	void actNIMBY();
	void actNimby();
	void actFree();
	void actUser();
	void actRequestLog();
	void actRequestTasksLog();
	void actRequestInfo();
	void actRequestTaskInfo(int jid, int bnum, int tnum);
	void actEnableService();
	void actDisableService();
	void actRestoreDefaults();
	void actSetHidden();
	void actUnsetHidden();
	void actLaunchCmd();
	void actLaunchCmdExit();

	void actEjectTasks();
	void actEjectNotMyTasks();

	void actExit();
	void actDelete();

	void actCommand( int number);
	void actReboot();
	void actShutdown();
	void actWOLSleep();
	void actWOLWake();

	void requestResources();

private:
	QTimer * timer;

private:
	void setService( bool enable);
	void launchCmdExit( bool i_exit);
	void calcTitle();
	void setSpacing();

private:
	static EDisplaySize ms_displaysize;

	static int     SortType;
	static bool    SortAscending;
	static QString FilterString;
	static int     FilterType;
	static bool    FilterInclude;
	static bool    FilterMatch;
};
