#pragma once

#include "../libafanasy/msg.h"

#include "listnodes.h"

class MCMonitorOnlineList;

class ListMonitors : public ListNodes
{
	Q_OBJECT

  public:
	ListMonitors(QWidget *parent);
	~ListMonitors();

	bool v_caseMessage(af::Msg *msg);

	ItemNode *v_createNewItemNode(af::Node *i_afnode, Item::EType i_type, bool i_notify);

	virtual bool v_processEvents(const af::MonitorEvents &i_me);

  protected:
	void contextMenuEvent(QContextMenuEvent *event);

  private slots:
	void actSendMessage();
	void actRequestLog();
	void actExit();

  private:
	void calcTitle();

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
};
