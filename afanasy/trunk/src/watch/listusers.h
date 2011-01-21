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
   ItemNode* createNewItem( af::Node *node);

protected:
   void contextMenuEvent( QContextMenuEvent *event);

private slots:

   void userAdded( ItemNode * node, const QModelIndex & index);

   void actAnnotate();
   void actHostsMask();
   void actHostsMaskExclude();
   void actMaxHosts();
   void actPriority();
   void actErrorsAvoidHost();
   void actErrorRetries();
   void actErrorsSameHost();
   void actErrorsForgiveTime();
   void actRequestLog();

   void actAdd();
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
