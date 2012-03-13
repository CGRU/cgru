#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "../libafqt/name_afqt.h"

#include "listitems.h"

class QListWidget;
class QListWidgetItem;

class ItemJobBlock;
class ItemJobTask;

class ListTasks : public ListItems
{
   Q_OBJECT

public:
   ListTasks( QWidget* parent, int JobId, const QString & JobName);
   ~ListTasks();

   bool caseMessage( af::Msg * msg);

signals:
   void changeMonitor( int type, int id);

protected:
   void contextMenuEvent(QContextMenuEvent *event);
   void doubleClicked( Item * item);
   void connectionLost();

private slots:

   void actBlockCommand();
   void actBlockWorkingDir();
   void actBlockPreviewCmd();
   void actBlockCmdPost();
   void actBlockTasksType();
   void actBlockParserType();

   void blockAction( int id_block, int id_action);

   void actTaskLog();
   void actTaskStdOut( int number );
   void actTaskErrorHosts();
   void actTaskPreview( int number);
   void actTaskListen();
   void actTasksRestart();
   void actTasksSkip();

private:
   int jobid;
   QString jobname;
   af::Job * job;
   af::JobProgress * progress;

   int blocksnum;
   ItemJobBlock **wblocks;
   int *tasksnum;
   ItemJobTask ***wtasks;

   bool constructed;

private:
   void construct();
   void updateJob();

   bool updateProgress( bool blocksOnly = false);
   bool updateTasks( af::MCTasksProgress * mctasksprogress);

   int getRow( int block, int task = -1);

   void do_Skip_Restart(  int type, int itemid);
   void do_Info_StdOut(   int type, int number, Item * item = NULL);
   void setBlockProperty( int type, af::MCGeneral & mcgeneral);
   void setWindowTitleProgress();
};
