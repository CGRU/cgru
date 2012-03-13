#pragma once

#include "listnodes.h"

#include <msg.h>

class MCMonitorOnlineList;

class ListMonitors : public ListNodes
{
   Q_OBJECT

public:
   ListMonitors( QWidget* parent);
   ~ListMonitors();

   bool caseMessage( af::Msg * msg);
   ItemNode* createNewItem( af::Node *node);

protected:
   void contextMenuEvent( QContextMenuEvent *event);

private slots:
   void actSendMessage();

private:
   void calcTitle();

private:
   static int     SortType;
   static bool    SortAscending;
   static QString FilterString;
   static int     FilterType;
   static bool    FilterInclude;
};
