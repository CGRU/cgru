#include "listtasks.h"

#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtGui/QBoxLayout>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QListWidget>
#include <QtGui/QMenu>

#include "../include/afanasy.h"

#include "../libafanasy/address.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/jobprogress.h"
#include "../libafanasy/service.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msgclasses/mctaskspos.h"
#include "../libafanasy/msgclasses/mctasksprogress.h"

#include "../libafqt/qmsg.h"

#include "actionid.h"
#include "dialog.h"
#include "itemjobblock.h"
#include "itemjobtask.h"
#include "modelitems.h"
#include "viewitems.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ListTasks::ListTasks( QWidget* parent, int JobId, const QString & JobName):
   ListItems( parent),
   jobid( JobId),
   jobname( JobName),
   job( NULL),
   progress( NULL),
   blocksnum(0),
   wblocks( NULL),
   tasksnum( NULL),
   wtasks( NULL),
   constructed( false)
{
   init();

   view->setSpacing( 1);
//   view->setUniformItemSizes( true);
//   view->setBatchSize( 10000);

   Watch::sendMsg( new afqt::QMsg( af::Msg::TJobRequestId, jobid, true));

   parentWindow->setWindowTitle( jobname);
}

ListTasks::~ListTasks()
{
   Watch::delJobId( jobid);

   if( tasksnum != NULL ) delete [] tasksnum;
   if( wblocks != NULL ) delete [] wblocks;
   if( wtasks != NULL ) delete [] wtasks;
   if( job ) delete job;
   if( progress ) delete progress;

   Watch::watchTasks_rem( jobid);
}

void ListTasks::connectionLost()
{
   if( parentWindow != (QWidget*)Watch::getDialog()) parentWindow->close();
}

void ListTasks::contextMenuEvent(QContextMenuEvent *event)
{
   Item* item = getCurrentItem();
   if( item == NULL) return;

   QMenu menu(this);
   QAction *action;

   int id = item->getId();
   switch( id)
   {
      case ItemJobBlock::ItemId:
      {
         ItemJobBlock *itemBlock = (ItemJobBlock*)item;
         itemBlock->generateMenu( itemBlock->getNumBlock(), &menu, this);

         menu.addSeparator();

         action = new QAction( "Set Command", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actBlockCommand() ));
         menu.addAction( action);

         action = new QAction( "Set Working Directory", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actBlockWorkingDir() ));
         menu.addAction( action);

         action = new QAction( "Set Post Command", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actBlockCmdPost() ));
         menu.addAction( action);

         action = new QAction( "Set Preview Command", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actBlockPreviewCmd() ));
         menu.addAction( action);

         action = new QAction( "Set Tasks Type", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actBlockTasksType() ));
         menu.addAction( action);

         action = new QAction( "Set Parser Type", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actBlockParserType() ));
         menu.addAction( action);

         break;
      }
      case ItemJobTask::ItemId:
      {
//printf("ListTasks::contextMenuEvent: task:\n");
         ActionId * actionid = new ActionId( 0, "Output", this);
         connect( actionid, SIGNAL( triggeredId( int ) ), this, SLOT( actTaskStdOut( int ) ));
         menu.addAction( actionid);

         int startCount = 1;
         startCount = ((ItemJobTask*)(item))->taskprogress.starts_count;
//printf("ListTasks::contextMenuEvent: startCount=%d\n", startCount);
         if( startCount > 1 )
         {
            QMenu * submenu = new QMenu( "outputs", this);
            for( int i = 1; i < startCount; i++)
            {
               actionid = new ActionId( i, QString(" %1 ").arg(i), this);
               connect( actionid, SIGNAL( triggeredId( int ) ), this, SLOT( actTaskStdOut( int ) ));
               submenu->addAction( actionid);
            }
            menu.addMenu( submenu);
         }

         action = new QAction( "Log", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actTaskLog() ));
         menu.addAction( action);

         action = new QAction( "Listen", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actTaskListen() ));
         menu.addAction( action);

         action = new QAction( "Error Hosts", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actTaskErrorHosts() ));
         menu.addAction( action);

         if( ((ItemJobTask*)(item))->genCmdView().isEmpty() == false )
         {
            QStringList images = ((ItemJobTask*)(item))->genCmdView().split(';');
            const QStringList * previewcmds = Watch::getPreviewCmds();
            if( previewcmds->size() > 0 )
            {
               menu.addSeparator();
               QMenu * submenu_cmd = new QMenu( "Preview", this);
               for( int p = 0; p < previewcmds->size(); p++)
               {
                  if( images.size() > 1)
                  {
                     QMenu * submenu_img = new QMenu( QString("%1").arg((*previewcmds)[p]), this);
                     for( int i = 0; i < images.size(); i++)
                     {
                        QString imgname = images[i].right(99);
                        ActionIdId * actionid = new ActionIdId( p, i, imgname, this);
                        connect( actionid, SIGNAL( triggeredId(int,int) ), this, SLOT( actTaskPreview(int,int) ));
                        submenu_img->addAction( actionid);
                     }
                     submenu_cmd->addMenu( submenu_img);
                  }
                  else
                  {
                     ActionIdId * actionid = new ActionIdId( p, 0, QString("%1").arg((*previewcmds)[p]), this);
                     connect( actionid, SIGNAL( triggeredId(int,int) ), this, SLOT( actTaskPreview(int,int) ));
                     submenu_cmd->addAction( actionid);
                  }
               }
               menu.addMenu( submenu_cmd);
            }
         }

         menu.addSeparator();

         action = new QAction( "Skip Tasks", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actTasksSkip() ));
         menu.addAction( action);

         action = new QAction( "Restart Tasks", this);
         connect( action, SIGNAL( triggered() ), this, SLOT( actTasksRestart() ));
         menu.addAction( action);

         break;
      }
      default:
      {
         AFERRAR("ListTasks::contextMenuEvent: unknown item id = %d.\n", id);
         return;
      }
   }

   menu.exec(event->globalPos());
}

bool ListTasks::caseMessage( af::Msg * msg)
{
#ifdef AFOUTPUT
printf("ListTasks::caseMessage:\n"); msg->stdOut();
#endif
   switch( msg->type())
   {
   case af::Msg::TJob:
   {
      af::Job * new_job = new af::Job( msg);
      if( new_job->getId() != jobid )
      {
         delete new_job;
         break;
      }

      if( job ) delete job;
      job = new_job;

      if( constructed == false)
      {
         construct();
         Watch::sendMsg( new afqt::QMsg( af::Msg::TJobProgressRequestId, jobid, true));
         Watch::addJobId( jobid);
      }
      else updateJob();
      break;
   }
   case af::Msg::TJobProgress:
   {
      if( constructed == false ) break;

      af::JobProgress * new_progress = new af::JobProgress( msg);
      if( jobid != new_progress->getJobId())
      {
         delete new_progress;
         break;
      }

      if( progress ) delete progress;
      progress = new_progress;

      if( updateProgress() == false)
      {
         printf("Tasks update error. Closing tasks window.\n");
         displayWarning( "Tasks update error.");
         parentWindow->close();
      }
      break;
   }
   case af::Msg::TTasksRun:
   {
      af::MCTasksProgress mctsp( msg);
      if( mctsp.getJobId() != jobid ) return false;
      return updateTasks( &mctsp);
   }
   case af::Msg::TMonitorJobsDel:
   {
      af::MCGeneral ids( msg);
      if( ids.hasId( jobid) == false) break;
   }
   case af::Msg::TJobRequestId:
   case af::Msg::TJobProgressRequestId:
   {  // this messages sent if where is no job with given id.
      printf("The job does not exist any more. Closing tasks window.\n");
      displayWarning( "The job does not exist any more.");
      parentWindow->close();
      break;
   }
   case af::Msg::TMonitorJobsAdd:
   {
      af::MCGeneral ids( msg);
      if( ids.hasId( jobid))
         Watch::sendMsg( new afqt::QMsg( af::Msg::TJobRequestId, jobid, true));
      break;
   }
   case af::Msg::TBlocks:
   case af::Msg::TBlocksProperties:
   case af::Msg::TBlocksProgress:
   {
      af::MCAfNodes mcblocks( msg);
      int count = mcblocks.getCount();
      for( int b = 0; b < count; b++)
      {
         af::BlockData * block = (af::BlockData*)mcblocks.getNode( b);
         if( block->getJobId() != jobid) continue;
         int blocknum = block->getBlockNum();
         if( blocknum >= blocksnum ) continue;

         wblocks[blocknum]->update( block, msg->type());

         if( msg->type() == af::Msg::TBlocks)
            model->emit_dataChanged();
         else
         {
            int row = getRow( b);
            if( row != -1 ) model->emit_dataChanged( getRow( blocknum));
         }
      }
      if( msg->type() == af::Msg::TBlocks) model->emit_dataChanged();
      break;
   }
   default:
      return false;
   }
   return true;
}

void ListTasks::construct()
{
   constructed = true;
   view->viewport()->hide();

   blocksnum = job->getBlocksNum();
   if( blocksnum == 0 ) return;

   tasksnum = new int[blocksnum];
   wblocks = new ItemJobBlock*[blocksnum];
   wtasks = new ItemJobTask**[blocksnum];
   int row = 0;
   for( int b = 0; b < blocksnum; b++)
   {
      const af::BlockData* block = job->getBlock( b);
      ItemJobBlock *wblock = new ItemJobBlock( block);
      wblock->tasksHidded = blocksnum > 1;
      wblocks[b] = wblock;
      model->addItem( wblock);
      row++;
      tasksnum[b] = block->getTasksNum();
      wtasks[b] = new ItemJobTask*[tasksnum[b]];
      for( int t = 0; t < tasksnum[b]; t++)
      {
         ItemJobTask *wtask =  new ItemJobTask( block, t);
         model->addItem( wtask);
         if( blocksnum > 1) view->setRowHidden( row , true);
         row++;
         wtasks[b][t] = wtask;
      }
   }

   view->viewport()->show();
}

int ListTasks::getRow( int block, int task)
{
   int row = -1;
   if( block < blocksnum )
   {
      if((task != -1) && (task >= tasksnum[block]))
      {
         AFERRAR("ListTasks::getRow: task >= tasksnum[block] : (%d>=%d[%d])\n", task, tasksnum[block], block);
      }
      else
      {
         row = 1;
         for( int b = 0; b < block; b++) row += 1 + tasksnum[b];
         row += task;
      }
   }
   else
      AFERRAR("ListTasks::getRow: block >= blocksnum : (%d>=%d)\n", block, blocksnum);
//printf("ListTasks::getRow: b[%d] t[%d] = %d\n", block, task, row);
   return row;
}

void ListTasks::updateJob()
{
//printf("ListTasks::updateJob:\n");
   if( blocksnum == 0 ) return;
   if( blocksnum != job->getBlocksNum())
   {
      AFERRAR("ListTasks::updateJob: blocksnum != job->getBlocksNum() ( %d != %d )\n", blocksnum, job->getBlocksNum());
      return;
   }
   for( int b = 0; b < blocksnum; b++)
   {
      const af::BlockData* block = job->getBlock( b);
      wblocks[b]->update( block, af::Msg::TJob);
   }
}

bool ListTasks::updateProgress(  bool blocksOnly)
{
   if( progress == NULL ) return true;
   if( blocksnum != progress->getBlocksNum())
   {
      AFERRAR("ListTasks::updateProgress: Blocks number mismatch (%d!=%d).\n", blocksnum, progress->getBlocksNum());
      return false;
   }

   for( int b = 0; b < blocksnum; b++)
   {
      af::BlockData * blockdata = job->getBlock(b);
      blockdata->updateProgress( progress);
      wblocks[b]->update( blockdata, af::Msg::TBlocksProgress);
      int row = getRow( b);
      if( row != -1 ) model->emit_dataChanged( row);

      if( blocksOnly ) continue;

      if( tasksnum[b] != progress->getTasksNum(b))
      {
         AFERRAR("ListTasks::updateProgress: Tasks number mismatch in block #%d (%d!=%d).\n",
            b, tasksnum[b], progress->getTasksNum(b));
         return false;
      }

      for( int t = 0; t < tasksnum[b]; t++)
      {
         wtasks[b][t]->upProgress( *(progress->tp[b][t]) );
      }
   }

   setWindowTitleProgress();

   return true;
}

bool ListTasks::updateTasks( af::MCTasksProgress * mctasksprogress)
{
   if( progress == NULL) return true;

   const std::list<int32_t> * tasks  = mctasksprogress->getTasks();
   const std::list<int32_t> * blocks = mctasksprogress->getBlocks();
   std::list<af::TaskProgress*> * tasksprogress = mctasksprogress->getTasksRun();

   std::list<int32_t>::const_iterator tIt = tasks->begin();
   std::list<int32_t>::const_iterator bIt = blocks->begin();
   std::list<af::TaskProgress*>::iterator trIt = tasksprogress->begin();

   int firstChangedRow = -1;
   int lastChangedRow = -1;
   int count = tasks->size();
   for( int i = 0; i < count; i++)
   {
      if( *bIt > blocksnum)
      {
         AFERRAR("ListTasks::updateTasks: block > blocksnum (%d>%d)\n", *bIt, blocksnum);
         return false;
      }
      if( *tIt > tasksnum[*bIt])
      {
         AFERRAR("ListTasks::updateTasks: task > tasksnum[%d] (%d>%d)\n", *bIt, *tIt, tasksnum[*bIt]);
         return false;
      }
      wtasks[*bIt][*tIt]->upProgress( **trIt );

      int row = getRow( *bIt, *tIt);
      if( row != -1 )
      {
         if((firstChangedRow == -1) || (firstChangedRow > row)) firstChangedRow = row;
         if(  lastChangedRow < row) lastChangedRow = row;
      }

      *(progress->tp[*bIt][*tIt]) = **trIt;

      tIt++; bIt++; trIt++;
   }

   if( firstChangedRow != -1 ) model->emit_dataChanged( firstChangedRow, lastChangedRow);

   updateProgress( true);

   return true;
}

void ListTasks::setWindowTitleProgress()
{
   int total_percent = 0;
   int total_tasks = 0;
   for( int b = 0; b < blocksnum; b++)
      for( int t = 0; t < tasksnum[b]; t++)
      {
         total_percent += wtasks[b][t]->taskprogress.percent;
         total_tasks++;
      }

   parentWindow->setWindowTitle( QString("%1% %2").arg(total_percent/total_tasks).arg(jobname));
}

void ListTasks::actTasksSkip()              { do_Skip_Restart( af::Msg::TTasksSkip,     ItemJobTask::ItemId); }
void ListTasks::actTasksRestart()           { do_Skip_Restart( af::Msg::TTasksRestart,  ItemJobTask::ItemId); }

void ListTasks::actTaskLog()               { do_Info_StdOut(  af::Msg::TTaskLogRequest,         0);}
void ListTasks::actTaskErrorHosts()        { do_Info_StdOut(  af::Msg::TTaskErrorHostsRequest,  0);}
void ListTasks::actTaskStdOut( int number ){ do_Info_StdOut(  af::Msg::TTaskOutputRequest, number);}

void ListTasks::doubleClicked( Item * item)
{
   if( item->getId() == ItemJobTask ::ItemId )
      do_Info_StdOut(  af::Msg::TTaskRequest, 0, item);
   else if( item->getId() == ItemJobBlock::ItemId )
   {
      ItemJobBlock * block = (ItemJobBlock*)item;
      int blockNum = block->getNumBlock();
      bool hide = false == wblocks[blockNum]->tasksHidded;
      wblocks[blockNum]->tasksHidded = hide;
      int row_start = getRow( blockNum, 0);
      int row_end   = getRow( blockNum, tasksnum[blockNum]-1);
      for( int row = row_start; row <= row_end; row++) view->setRowHidden( row, hide);
//   view->updateGeometries();
   }
}

void ListTasks::do_Skip_Restart( int type, int itemid)
{
   af::MCTasksPos taskspos( jobid, " (watch) ");
   const QList<Item*> items( getSelectedItems());
   for( int i = 0; i < items.count(); i++)
   {
      int id = items[i]->getId();
      if( id != itemid ) continue;
      switch( id)
      {
         case ItemJobBlock::ItemId:
         {
            ItemJobBlock *itemBlock = (ItemJobBlock*)items[i];
            if( taskspos.addBlock( itemBlock->getNumBlock()) == false) return;
            break;
         }
         case ItemJobTask::ItemId:
         {
            ItemJobTask *itemTask = (ItemJobTask*)items[i];
            if( taskspos.addTask( itemTask->getBlockNum(), itemTask->getTaskNum()) == false) return;
            break;
         }
         default:
         {
            AFERRAR("ListTasks::do_Skip_Restart: Invalid item id = %d.\n", id);
         }
      }
   }

   if( taskspos.getCount() < 1) return;

   afqt::QMsg * msg = new afqt::QMsg( type, &taskspos);
   switch ( type )
   {
      case af::Msg::TTasksSkip:
      {
         if( itemid == ItemJobBlock::ItemId ) displayInfo( "Skip blocks.");
         else                                 displayInfo( "Skip tasks.");
         break;
      }
      case af::Msg::TTasksRestart:
      {
         if( itemid == ItemJobBlock::ItemId ) displayInfo( "Restart blocks.");
         else                                 displayInfo( "Restart tasks.");
         break;
      }
   }
   Watch::sendMsg( msg);
}

void ListTasks::do_Info_StdOut( int type, int number, Item * item)
{
   if( item == NULL) item = getCurrentItem();
   if( item->getId() != ItemJobTask::ItemId) return;
   ItemJobTask *itemTask = (ItemJobTask*)item;
   af::MCTaskPos mctaskpos( jobid, itemTask->getBlockNum(), itemTask->getTaskNum(), number);
   afqt::QMsg * msg = new afqt::QMsg( type, &mctaskpos, true);
   Watch::sendMsg( msg);
}

void ListTasks::actBlockCommand()
{
   bool ok;
   QString cur = ((ItemJobBlock*)( getCurrentItem()))->command;
   QString str = QInputDialog::getText(this, "Change Command", "Enter Command", QLineEdit::Normal, cur, &ok);
   if( !ok) return;
   af::MCGeneral mcgeneral( str);
   setBlockProperty( af::Msg::TBlockCommand, mcgeneral);
}
void ListTasks::actBlockWorkingDir()
{
   bool ok;
   QString cur = ((ItemJobBlock*)( getCurrentItem()))->workingdir;
   QString str = QInputDialog::getText(this, "Change Working Directory", "Enter Directory", QLineEdit::Normal, cur, &ok);
   if( !ok) return;
   af::MCGeneral mcgeneral( str);
   setBlockProperty( af::Msg::TBlockWorkingDir, mcgeneral);
}
void ListTasks::actBlockPreviewCmd()
{
   bool ok;
   QString cur = ((ItemJobBlock*)( getCurrentItem()))->previewcmd;
   QString str = QInputDialog::getText(this, "Change Preview Command", "Enter Command", QLineEdit::Normal, cur, &ok);
   if( !ok) return;
   af::MCGeneral mcgeneral( str);
   setBlockProperty( af::Msg::TBlockPreviewCmd, mcgeneral);
}
void ListTasks::actBlockCmdPost()
{
   bool ok;
   QString cur = ((ItemJobBlock*)( getCurrentItem()))->cmdpost;
   QString str = QInputDialog::getText(this, "Change Post Command", "Enter Command", QLineEdit::Normal, cur, &ok);
   if( !ok) return;
   af::MCGeneral mcgeneral( str);
   setBlockProperty( af::Msg::TBlockCmdPost, mcgeneral);
}
void ListTasks::actBlockTasksType()
{
   bool ok;
   QString cur = ((ItemJobBlock*)( getCurrentItem()))->taskstype;
   QString str = QInputDialog::getText(this, "Change Tasks Type", "Enter Type", QLineEdit::Normal, cur, &ok);
   if( !ok) return;
   af::MCGeneral mcgeneral( str);
   setBlockProperty( af::Msg::TBlockTasksType, mcgeneral);
}
void ListTasks::actBlockParserType()
{
   bool ok;
   QString cur = ((ItemJobBlock*)( getCurrentItem()))->parsertype;
   QString str = QInputDialog::getText(this, "Change Parser Type", "Enter Type", QLineEdit::Normal, cur, &ok);
   if( !ok) return;
   af::MCGeneral mcgeneral( str);
   setBlockProperty( af::Msg::TBlockParserType, mcgeneral);
}

void ListTasks::setBlockProperty( int type, af::MCGeneral & mcgeneral)
{
//printf("ListTasks::setBlockProperty: %s\n", af::Msg::TNAMES[type]);
   Item* item = getCurrentItem();
   if( item == NULL )
   {
      displayError( "No items selected.");
      return;
   }
   if( item->getId() != ItemJobBlock::ItemId)
   {
      displayWarning( "This action for blocks only.");
      return;
   }
   mcgeneral.addId( jobid);
   mcgeneral.setId( ((ItemJobBlock*)item)->getNumBlock());
   afqt::QMsg * msg = new afqt::QMsg( type, &mcgeneral);
   Watch::sendMsg( msg);
}

void ListTasks::actTaskPreview( int num_cmd, int num_img)
{
   Item* item = getCurrentItem();
   if( item == NULL )
   {
      displayError( "No items selected.");
      return;
   }
   if( item->getId() != ItemJobTask::ItemId)
   {
      displayWarning( "This action for task only.");
      return;
   }

   ItemJobTask* taskitem = (ItemJobTask*)item;
   af::Service service( "service", taskitem->getWDir(), "", taskitem ->genCmdView());

   QStringList images = service.getFiles().split(';');
   if( num_img >= images.size())
   {
      displayError( "No such image nubmer.");
      return;
   }
   QString arg  = images[num_img];
   QString wdir = service.getWDir();

   if( arg.isEmpty()) return;
   const QStringList * previewcmds = Watch::getPreviewCmds();
   if( num_cmd >= previewcmds->size())
   {
      displayError( "No such command number.");
      return;
   }

   QString cmd((*previewcmds)[num_cmd]);
   cmd = cmd.replace( AFWATCH::CMDS_ARGUMENT, arg);

   printf("Starting '%s'\n in'%s'\n", cmd.toUtf8().data(), wdir.toUtf8().data());
   QProcess * process = new QProcess( Watch::getDialog());
   if( false == QDir( wdir).exists())
   {
      AFERROR("Working directory does not exists.\n");
   }
   else
   {
      process->setWorkingDirectory( wdir);
   }
   QStringList args;
#ifdef WINNT
   args << "/c" << cmd;
   process->start( "cmd.exe", args, QIODevice::ReadWrite);
#else
   args << "-c" << cmd;
   process->start( "/bin/bash", args, QIODevice::ReadWrite);
#endif
}

void ListTasks::actTaskListen()
{
   ItemJobTask *itemTask = (ItemJobTask*)getCurrentItem();
   Watch::listenTask( jobid, itemTask->getBlockNum(), itemTask->getTaskNum(),
      jobname + '(' + itemTask->getName() + ')');
}

void ListTasks::blockAction( int id_block, int id_action)
{
   if( id_block >= blocksnum)
   {
      AFERRAR("ListTasks::blockAction: id_block >= blocksnum (%d>=%d)\n", id_block, blocksnum);
      return;
   }
   wblocks[id_block]->blockAction( id_block, id_action, this);
}
