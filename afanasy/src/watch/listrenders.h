#pragma once

#include "listnodes.h"

#include "../libafanasy/msg.h"
#include "../libafanasy/taskexec.h"

class QItemSelection;

class ItemPool;
class ItemRender;

class ListRenders : public ListNodes
{
	Q_OBJECT

public:
	ListRenders( QWidget* parent);
	~ListRenders();

	bool v_caseMessage( af::Msg * msg);

	ItemNode * v_createNewItemNode(af::Node * i_afnode, Item::EType i_type, bool i_notify);

	virtual bool v_processEvents( const af::MonitorEvents & i_me);

	void offsetHierarchy(ItemPool   * i_item_pool);
	void offsetHierarchy(ItemRender * i_item_render);

	void removeRender(ItemRender * i_item_render);
	void removePool(ItemPool * i_item_pool);

protected:
	void contextMenuEvent( QContextMenuEvent *event);

	void doubleClicked( Item * item);

	virtual void v_connectionLost();

	virtual void v_itemToBeDeleted(Item * i_item);

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
	void rendersSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );

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
	void actSetPaused();
	void actUnsetPaused();
	void actLaunchCmd();
	void actLaunchCmdExit();
	void actLaunchCmdString(QString i_cmd);
	void actLaunchCmdExitString(QString i_cmd);

	void actEjectTasks();
	void actEjectNotMyTasks();

	void actServiceAdd();
	void actServiceRemove();
	void actServiceEnable();
	void actServiceDisable();
	void editService(QString i_mode, QString i_service);
	void actClearServices();

	void actExit();
	void actDelete();

	void actCommand( int number);
	void actReboot();
	void actShutdown();
	void actWOLSleep();
	void actWOLWake();

	void requestResources();

private:
	void editServiceDialog(const QString & i_mode, const QString & i_dialog_caption);
	void launchCmdExit( bool i_exit);
	void launchCmdStringExit(const QString & i_cmd, bool i_exit);
	void calcTitle();
	void setSpacing();

private:
	QMap<QString, ItemPool*> m_pools;
	QMap<QString, QList<ItemRender*>> m_pool_renders;

	static EDisplaySize ms_displaysize;

	static int     ms_SortType1;
	static int     ms_SortType2;
	static bool    ms_SortAscending1;
	static bool    ms_SortAscending2;
	static int     ms_FilterType;
	static bool    ms_FilterInclude;
	static bool    ms_FilterMatch;
	static std::string ms_FilterString;

	QTimer * timer;
};
