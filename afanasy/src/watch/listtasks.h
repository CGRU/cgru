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

   bool mousePressed( QMouseEvent * event);

   void sortBlock( int numblock);

signals:
   void changeMonitor( int type, int id);

protected:
   void contextMenuEvent(QContextMenuEvent *event);
   void doubleClicked( Item * item);
   void v_connectionLost();

private slots:

   void actBlockCommand();
   void actBlockWorkingDir();
   void actBlockFiles();
   void actBlockCmdPost();
   void actBlockService();
   void actBlockParser();

   void blockAction( int id_block, int id_action, int i_number);

   void actTaskInfo();
   void actTaskLog();
   void actTaskStdOut( int number );
   void actTaskErrorHosts();
   void actTaskPreview( int num_cmd, int num_img);
   void actTaskListen();
   void actTasksRestart();
   void actTasksSkip();

   void actBrowseFolder();

private:
   int jobid;
   QString jobname;

   int blocksnum;
   ItemJobBlock **wblocks;
   int *tasksnum;
   ItemJobTask ***wtasks;

   bool constructed;

private:
   void construct( af::Job * job);

   bool updateProgress( const af::JobProgress * progress);
   bool updateTasks( af::MCTasksProgress * mctasksprogress);

   int getRow( int block, int task = -1);

   void do_Skip_Restart(  int type, int itemid);
   void do_Info_StdOut(   int type, int number, Item * item = NULL);
   void setBlockProperty( int type, af::MCGeneral & mcgeneral);
   void setWindowTitleProgress();
};
