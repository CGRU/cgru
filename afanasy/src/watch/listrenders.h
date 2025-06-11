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
	ListRenders(QWidget *parent);
	~ListRenders();

	bool v_caseMessage(af::Msg *msg);

	ItemNode *v_createNewItemNode(af::Node *i_afnode, Item::EType i_type, bool i_notify);

	virtual bool v_processEvents(const af::MonitorEvents &i_me);

	virtual void v_hideChanged();

	void itemsSizeChanged();

  public slots:
	void slot_ServiceAdd();
	void slot_ServiceDisable();
	void slot_ServiceEdit(QString i_mode, QString i_service);
	void slot_TicketPoolEdit();
	void slot_TicketHostEdit();
	void slot_TicketPoolEdit(const QString &i_name);
	void slot_TicketHostEdit(const QString &i_name);

  protected:
	void contextMenuEvent(QContextMenuEvent *event);
	void generateRenderMenu(QMenu *i_menu);
	void generatePoolMenu(QMenu *i_menu);
	void generateCommonMenuItems(QMenu *i_menu);

	void v_doubleClicked(Item *i_item);

	virtual void v_connectionLost();

  public:
	enum EDisplaySize
	{
		EVariableSize = 0,
		ESmallSize = 1,
		ENormalSize = 2,
		EBigSize = 3
	};

  private slots:

	void renderAdded(ItemNode *node, const QModelIndex &index);

	void actAddPool();

	void actRenderSetPool();
	void actRenderReAssing();

	void actNewRenderNimby();
	void actNewRenderFree();
	void actNewRenderPaused();
	void actNewRenderReady();

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

	void actServiceRemove();
	void actServiceEnable();
	void actClearServices();

	void actHealSick();
	void actExit();
	void actDelete();
	void actDeleteRenders();

	void actCommand(int number);
	void actReboot();
	void actShutdown();
	void actWOLSleep();
	void actWOLWake();

  private:
	void addPool(int i_parent_id, const QString &i_child);
	void renderSetPool(const QString &i_name);
	void editServiceDialog(const QString &i_mode, const QString &i_dialog_caption);
	void launchCmdExit(bool i_exit);
	void launchCmdStringExit(const QString &i_cmd, bool i_exit);
	void calcTitle();
	void setSpacing();

	void ticketEdit_DialogName(bool i_host_ticket);
	void ticketEdit_DialogCount(const QString &i_name, bool i_host_ticket);

  private:
	static int ms_SortType1;
	static int ms_SortType2;
	static bool ms_SortAscending1;
	static bool ms_SortAscending2;
	static int ms_FilterType;
	static bool ms_FilterInclude;
	static bool ms_FilterMatch;
	static bool ms_FilterRegEx;
	static std::string ms_FilterString;

	// To store what to hide
	static uint32_t ms_hide_flags;
};
