#pragma once

#include "../libafanasy/msg.h"

#include "listnodes.h"

class MCMonitorOnlineList;

class ListMonitors : public ListNodes
{
   Q_OBJECT

public:
   ListMonitors( QWidget* parent);
   ~ListMonitors();

	bool caseMessage( af::Msg * msg);

	ItemNode* v_createNewItem( af::Node *node, bool i_subscibed);

	virtual bool processEvents( const af::MonitorEvents & i_me);

protected:
   void contextMenuEvent( QContextMenuEvent *event);

private slots:
//	void actSendMessage();
	void actRequestLog();
	void actExit();

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
