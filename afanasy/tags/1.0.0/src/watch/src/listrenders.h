#pragma once

#include "listnodes.h"

#include <msg.h>

class QItemSelection;

class ListRenders : public ListNodes
{
   Q_OBJECT

public:
   ListRenders( QWidget* parent);
   ~ListRenders();

   bool caseMessage( af::Msg * msg);
   ItemNode* createNewItem( af::Node *node);

protected:
   void contextMenuEvent( QContextMenuEvent *event);

public:
   static bool ConstHeight;

private slots:

   void renderAdded( ItemNode * node, const QModelIndex & index);
   void selectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );

   void actPriority();
   void actNIMBY();
   void actNimby();
   void actFree();
   void actUser();
   void actRequestLog();
   void actRequestServices();

   void actEject();

   void actExit();
   void actDelete();
   void actRestart();
   void actStart();

   void actReboot();
   void actShutdown();

   void requestResources();

private:
   QTimer * timer;

private:
   void calcTitle();

private:
   static int     SortType;
   static bool    SortAscending;
   static QString FilterString;
   static int     FilterType;
   static bool    FilterInclude;
};
