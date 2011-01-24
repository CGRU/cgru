#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "listnodes.h"

//class ItemJobBrief;

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

   void shownFunc();
   void connectionLost();

private slots:
   void actMoveUp();
   void actMoveDown();
   void actMoveTop();
   void actMoveBottom();

   void actAnnotate();
   void actPriority();
   void actHostsMask();
   void actHostsMaskExclude();
   void actMaxRunningTasks();
   void actDependMask();
   void actDependMaskGlobal();
   void actWaitTime();
   void actNeedOS();
   void actNeedProperties();
   void actPostCommand();

   void blockAction( int id_block, int id_action);

   void actStart();
   void actStop();
   void actRestart();
   void actRestartErrors();
   void actResetErrorHosts();
   void actPause();
   void actRestartPause();
   void actDelete();

   void actRequestLog();
   void actRequestErrorHostsList();

   void actListenJob();

private:
   QTimer * timer;
   static int     SortType;
   static bool    SortAscending;
   static QString FilterString;
   static int     FilterType;
   static bool    FilterInclude;
   static bool    FilterMatch;

   void calcTotals();
};
