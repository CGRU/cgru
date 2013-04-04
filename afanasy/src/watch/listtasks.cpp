#include "listtasks.h"

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

#include "actionid.h"
#include "dialog.h"
#include "itemjobblock.h"
#include "itemjobtask.h"
#include "modelitems.h"
#include "viewitems.h"
#include "watch.h"

#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtGui/QBoxLayout>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QListWidget>
#include <QtGui/QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ListTasks::ListTasks( QWidget* parent, int JobId, const QString & JobName):
	ListItems( parent),
	jobid( JobId),
	jobname( JobName),
	blocksnum(0),
	wblocks( NULL),
	tasksnum( NULL),
	wtasks( NULL),
	constructed( false)
{
	init();

	m_view->setSpacing( 1);
//   view->setUniformItemSizes( true);
//   view->setBatchSize( 10000);

	m_view->setListItems( this);

	Watch::sendMsg( new af::Msg( af::Msg::TJobRequestId, jobid, true));

	m_parentWindow->setWindowTitle( jobname);
}

void ListTasks::construct( af::Job * job)
{
	constructed = true;
	m_view->viewport()->hide();

	blocksnum = job->getBlocksNum();
	if( blocksnum == 0 ) return;

	tasksnum = new int[blocksnum];
	wblocks = new ItemJobBlock*[blocksnum];
	wtasks = new ItemJobTask**[blocksnum];
	int row = 0;
	for( int b = 0; b < blocksnum; b++)
	{
		const af::BlockData* block = job->getBlock( b);
		wblocks[b] = new ItemJobBlock( block, this);
		tasksnum[b] = block->getTasksNum();
		wblocks[b]->tasksHidded = ((blocksnum > 1) && (tasksnum[b] > 1));
		m_model->addItem( wblocks[b]);
		row++;
		wtasks[b] = new ItemJobTask*[tasksnum[b]];
		for( int t = 0; t < tasksnum[b]; t++)
		{
			ItemJobTask *wtask =  new ItemJobTask( block, t);
			m_model->addItem( wtask);
			if( wblocks[b]->tasksHidded) m_view->setRowHidden( row , true);
			row++;
			wtasks[b][t] = wtask;
		}
	}

	m_view->viewport()->show();
}

ListTasks::~ListTasks()
{
	Watch::delJobId( jobid);

	for( int b = 0; b < blocksnum; b++) delete [] wtasks[b];
	delete [] tasksnum;
	delete [] wblocks;
	delete [] wtasks;

	Watch::watchJodTasksWindowRem( jobid);
}

void ListTasks::v_connectionLost()
{
	if( m_parentWindow != (QWidget*)Watch::getDialog()) m_parentWindow->close();
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
				if( false == itemBlock->files.isEmpty() )
				{
					 action = new QAction( "Browse Files...", this);
					 connect( action, SIGNAL( triggered() ), this, SLOT( actBrowseFolder() ));
					 menu.addAction( action);
					 menu.addSeparator();
				}

				QMenu * submenu = new QMenu( "Change Block", this);
				itemBlock->generateMenu( itemBlock->getNumBlock(), &menu, this, submenu);

				menu.addMenu( submenu);
				menu.addSeparator();

				submenu = new QMenu( "Change Tasks", this);
				menu.addMenu( submenu);

				action = new QAction( "Set Command", this);
				connect( action, SIGNAL( triggered() ), this, SLOT( actBlockCommand() ));
				submenu->addAction( action);

				action = new QAction( "Set Working Directory", this);
				connect( action, SIGNAL( triggered() ), this, SLOT( actBlockWorkingDir() ));
				submenu->addAction( action);

				action = new QAction( "Set Post Command", this);
				connect( action, SIGNAL( triggered() ), this, SLOT( actBlockCmdPost() ));
				submenu->addAction( action);

				action = new QAction( "Set Files", this);
				connect( action, SIGNAL( triggered() ), this, SLOT( actBlockFiles() ));
				submenu->addAction( action);

				action = new QAction( "Set Service Type", this);
				connect( action, SIGNAL( triggered() ), this, SLOT( actBlockService() ));
				submenu->addAction( action);

				action = new QAction( "Set Parser Type", this);
				connect( action, SIGNAL( triggered() ), this, SLOT( actBlockParser() ));
				submenu->addAction( action);

				break;
		  }
		case ItemJobTask::ItemId:
		{
			ActionId * actionid = new ActionId( 0, "Output", this);
			connect( actionid, SIGNAL( triggeredId( int ) ), this, SLOT( actTaskStdOut( int ) ));
			menu.addAction( actionid);

			if( jobid != AFJOB::SYSJOB_ID )
			{
				int startCount = ((ItemJobTask*)(item))->taskprogress.starts_count;
				if( startCount > 1 )
				{
					QMenu * submenu = new QMenu( "outputs", this);
					for( int i = 1; i < startCount; i++)
					{
						actionid = new ActionId( i, QString("session #%1").arg(i), this);
						connect( actionid, SIGNAL( triggeredId( int ) ), this, SLOT( actTaskStdOut( int ) ));
						submenu->addAction( actionid);
					}
					menu.addMenu( submenu);
				}
			}

			action = new QAction( "Log", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actTaskLog() ));
			menu.addAction( action);

			action = new QAction( "Info", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actTaskInfo() ));
			menu.addAction( action);

			action = new QAction( "Listen", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actTaskListen() ));
			menu.addAction( action);

			action = new QAction( "Error Hosts", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actTaskErrorHosts() ));
			menu.addAction( action);

			if( ((ItemJobTask*)(item))->genFiles().empty() == false )
			{
				QStringList files = QString::fromUtf8(((ItemJobTask*)(item))->genFiles().c_str()).split(';');
				if( af::Environment::getPreviewCmds().size() > 0 )
				{
					menu.addSeparator();

					action = new QAction( "Browse Files...", this);
					connect( action, SIGNAL( triggered() ), this, SLOT( actBrowseFolder() ));
					menu.addAction( action);

				if( ((ItemJobTask*)(item))->isBlockNumeric() )
				{
					QMenu * submenu_cmd = new QMenu( "Preview", this);
					int p = 0;
					for( std::vector<std::string>::const_iterator it = af::Environment::getPreviewCmds().begin(); it != af::Environment::getPreviewCmds().end(); it++, p++)
					{
						if( files.size() > 1)
						{
							QMenu * submenu_img = new QMenu( QString("%1").arg( QString::fromUtf8((*it).c_str())), this);
							for( int i = 0; i < files.size(); i++)
							{
								QString imgname = files[i].right(99);
								ActionIdId * actionid = new ActionIdId( p, i, imgname, this);
								connect( actionid, SIGNAL( triggeredId(int,int) ), this, SLOT( actTaskPreview(int,int) ));
								submenu_img->addAction( actionid);
							}
							submenu_cmd->addMenu( submenu_img);
						}
						else
						{
							ActionIdId * actionid = new ActionIdId( p, 0, QString("%1").arg( QString::fromUtf8((*it).c_str())), this);
							connect( actionid, SIGNAL( triggeredId(int,int) ), this, SLOT( actTaskPreview(int,int) ));
							submenu_cmd->addAction( actionid);
						}
					}
					menu.addMenu( submenu_cmd);
				}
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
			AFERRAR("ListTasks::contextMenuEvent: unknown item id = %d.", id)
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
		af::Job * job = new af::Job( msg);
		if( job->getId() != jobid )
		{
			AFERROR(     "ListTasks::caseMessage: af::Msg::TJob: Jobs ids mismatch.")
			displayError("ListTasks::caseMessage: af::Msg::TJob: Jobs ids mismatch.");
			delete job;
			break;
		}

		if( constructed == false)
		{
			construct( job);
			Watch::sendMsg( new af::Msg( af::Msg::TJobProgressRequestId, jobid, true));
			Watch::addJobId( jobid);
		}
		else
		{
			AFERROR(     "ListTasks::caseMessage: af::Msg::TJob: Job is already constructed.")
			displayError("ListTasks::caseMessage: af::Msg::TJob: Job is already constructed.");
		}
		break;
	}
	case af::Msg::TJobProgress:
	{
		if( constructed == false ) break;

		af::JobProgress * progress = new af::JobProgress( msg);
		if( jobid == progress->getJobId())
		{
			if( updateProgress( progress ) == false)
			{
				printf("Tasks update error. Closing tasks window.\n");
				displayWarning( "Tasks update error.");
				m_parentWindow->close();
			}
		}
		delete progress;
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
		m_parentWindow->close();
		break;
	}
	case af::Msg::TMonitorJobsAdd:
	{
		af::MCGeneral ids( msg);
		if( ids.hasId( jobid))
			Watch::sendMsg( new af::Msg( af::Msg::TJobRequestId, jobid, true));
		break;
	}
	case af::Msg::TBlocks:
	case af::Msg::TBlocksProperties:
	case af::Msg::TBlocksProgress:
	{
		af::MCAfNodes mcblocks( msg);
		int count = int( mcblocks.getCount());
		for( int b = 0; b < count; b++)
		{
			af::BlockData * block = (af::BlockData*)mcblocks.getNode( b);
			if( block->getJobId() != jobid) continue;
			int blocknum = block->getBlockNum();
			if( blocknum >= blocksnum ) continue;

			wblocks[blocknum]->update( block, msg->type());

			if( msg->type() == af::Msg::TBlocks)
				m_model->emit_dataChanged();
			else
			{
				int row = getRow( b);
				if( row != -1 ) m_model->emit_dataChanged( getRow( blocknum));
			}
		}
		if( msg->type() == af::Msg::TBlocks) m_model->emit_dataChanged();
		break;
	}
	default:
		return false;
	}
	return true;
}

int ListTasks::getRow( int block, int task)
{
	int row = -1;
	if( block < blocksnum )
	{
		if((task != -1) && (task >= tasksnum[block]))
		{
			AFERRAR("ListTasks::getRow: task >= tasksnum[block] : (%d>=%d[%d])", task, tasksnum[block], block)
		}
		else
		{
			row = 1;
			for( int b = 0; b < block; b++) row += 1 + tasksnum[b];
			row += task;
		}
	}
	else
		AFERRAR("ListTasks::getRow: block >= blocksnum : (%d>=%d)", block, blocksnum)
//printf("ListTasks::getRow: b[%d] t[%d] = %d\n", block, task, row);
	return row;
}

bool ListTasks::updateProgress( const af::JobProgress * progress/*bool blocksOnly = false*/)
{
	if( blocksnum != progress->getBlocksNum())
	{
		AFERRAR("ListTasks::updateProgress: Blocks number mismatch (%d!=%d).", blocksnum, progress->getBlocksNum())
		return false;
	}

	for( int b = 0; b < blocksnum; b++)
	{
		if( tasksnum[b] != progress->getTasksNum(b))
		{
			AFERRAR("ListTasks::updateProgress: Tasks number mismatch in block #%d (%d!=%d)", b, tasksnum[b], progress->getTasksNum(b))
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
	const std::list<int32_t> * tasks  = mctasksprogress->getTasks();
	const std::list<int32_t> * blocks = mctasksprogress->getBlocks();
	std::list<af::TaskProgress*> * tasksprogress = mctasksprogress->getTasksRun();

	std::list<int32_t>::const_iterator tIt = tasks->begin();
	std::list<int32_t>::const_iterator bIt = blocks->begin();
	std::list<af::TaskProgress*>::iterator trIt = tasksprogress->begin();

	int firstChangedRow = -1;
	int lastChangedRow = -1;
	int count = int( tasks->size());
	for( int i = 0; i < count; i++)
	{
		if( *bIt > blocksnum)
		{
			AFERRAR("ListTasks::updateTasks: block > blocksnum (%d>%d)", *bIt, blocksnum)
			return false;
		}
		if( *tIt > tasksnum[*bIt])
		{
			AFERRAR("ListTasks::updateTasks: task > tasksnum[%d] (%d>%d)", *bIt, *tIt, tasksnum[*bIt])
			return false;
		}
		wtasks[*bIt][*tIt]->upProgress( **trIt );

		int row = getRow( *bIt, *tIt);
		if( row != -1 )
		{
			if((firstChangedRow == -1) || (firstChangedRow > row)) firstChangedRow = row;
			if(  lastChangedRow < row) lastChangedRow = row;
		}

		tIt++; bIt++; trIt++;
	}

	if( firstChangedRow != -1 ) m_model->emit_dataChanged( firstChangedRow, lastChangedRow);

	setWindowTitleProgress();

	return true;
}

void ListTasks::setWindowTitleProgress()
{
	int total_percent = 0;
	int total_tasks = 0;
	for( int b = 0; b < blocksnum; b++)
		for( int t = 0; t < tasksnum[b]; t++)
		{
			if(( wtasks[b][t]->taskprogress.state & AFJOB::STATE_DONE_MASK) ||
				( wtasks[b][t]->taskprogress.state & AFJOB::STATE_SKIPPED_MASK))
				total_percent += 100;
			else if ( wtasks[b][t]->taskprogress.state & AFJOB::STATE_RUNNING_MASK )
				total_percent += wtasks[b][t]->taskprogress.percent;
			total_tasks++;
		}

	m_parentWindow->setWindowTitle( QString("%1% %2").arg(total_percent/total_tasks).arg(jobname));
}

void ListTasks::actTasksSkip()    { tasksOpeation("skip"); }
void ListTasks::actTasksRestart() { tasksOpeation("restart"); }

void ListTasks::actTaskLog()               { do_Info_StdOut(  af::Msg::TTaskLogRequest,         0);}
void ListTasks::actTaskInfo()              { do_Info_StdOut(  af::Msg::TTaskRequest,            0);}
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
		for( int row = row_start; row <= row_end; row++) m_view->setRowHidden( row, hide);
//   view->updateGeometries();
		if( block->resetSortingParameters()) sortBlock( block->getNumBlock());
	}
}

void ListTasks::tasksOpeation( const std::string & i_type)
{
	std::ostringstream str;
	af::jsonActionStart( str, "jobs", "", std::vector<int>( 1, jobid));
	str << ",\n\"operation\":{\n\"type\":\"" << i_type << '"';
	str << ",\n\"task_ids\":[";

	int blockId = -1;
	// Collect tasks of the same block:
	const QList<Item*> items( getSelectedItems());
	for( int i = 0; i < items.count(); i++)
	{
		if( items[i]->getId() != ItemJobTask::ItemId ) continue;
		{
			ItemJobTask *itemTask = (ItemJobTask*)items[i];
			if( blockId == -1 )
				blockId = itemTask->getBlockNum();
			else if( blockId != itemTask->getBlockNum())
				continue;
			else
				str << ',';
			str << itemTask->getTaskNum();
		}
	}
	// Add one tasks block:
	str << "]},\n\"block_ids\":[" << blockId << ']';

	af::jsonActionFinish( str);
	Watch::sendMsg( af::jsonMsg( str));
}

void ListTasks::do_Info_StdOut( int type, int number, Item * item)
{
	if( item == NULL) item = getCurrentItem();
	if( item->getId() != ItemJobTask::ItemId) return;
	ItemJobTask *itemTask = (ItemJobTask*)item;
	af::MCTaskPos mctaskpos( jobid, itemTask->getBlockNum(), itemTask->getTaskNum(), number);
	af::Msg * msg = new af::Msg( type, &mctaskpos, true);
	Watch::sendMsg( msg);
}

void ListTasks::actBlockCommand()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->command;
	QString str = QInputDialog::getText(this, "Change Command", "Enter Command", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	blockAction( 0, QString("\"params\":{\"command\":\"%1\"}").arg(str.replace("\"","\\\"")), false);
}
void ListTasks::actBlockWorkingDir()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->workingdir;
	QString str = QInputDialog::getText(this, "Change Working Directory", "Enter Directory", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	blockAction( 0, QString("\"params\":{\"working_directory\":\"%1\"}").arg(str.replace("\"","\\\"")), false);
}
void ListTasks::actBlockFiles()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->files;
	QString str = QInputDialog::getText(this, "Change Files", "Enter Files", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	blockAction( 0, QString("\"params\":{\"files\":\"%1\"}").arg(str.replace("\"","\\\"")), false);
}
void ListTasks::actBlockCmdPost()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->cmdpost;
	QString str = QInputDialog::getText(this, "Change Post Command", "Enter Command", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	blockAction( 0, QString("\"params\":{\"command_post\":\"%1\"}").arg(str.replace("\"","\\\"")), false);
}
void ListTasks::actBlockService()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->service;
	QString str = QInputDialog::getText(this, "Change Service", "Enter Type", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	blockAction( 0, QString("\"params\":{\"service\":\"%1\"}").arg(str.replace("\"","\\\"")), false);
}
void ListTasks::actBlockParser()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->parser;
	QString str = QInputDialog::getText(this, "Change Parser", "Enter Type", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	blockAction( 0, QString("\"params\":{\"parser\":\"%1\"}").arg(str.replace("\"","\\\"")), false);
}

void ListTasks::actBrowseFolder()
{
	 Item* item = getCurrentItem();
	 if( item == NULL )
		  return;

	 QString image;
	 QString wdir;

	 int id = item->getId();
	 switch( id)
	 {
	 case ItemJobBlock::ItemId:
	 {
		  ItemJobBlock *itemBlock = (ItemJobBlock*)item;
		  af::Service service( "service", afqt::qtos( itemBlock->workingdir), "", afqt::qtos( itemBlock->files));
		  image = afqt::stoq( service.getFiles()).split(';')[0];
		  wdir = afqt::stoq( service.getWDir());
		  break;
	 }
	 case ItemJobTask::ItemId:
	 {
		  ItemJobTask* taskitem = (ItemJobTask*)item;
		  af::Service service("service", taskitem->getWDir(), "", taskitem->genFiles());
		  image = afqt::stoq( service.getFiles()).split(';')[0];
		  wdir = afqt::stoq( service.getWDir());
		  break;
	 }
	 default:
		  return;
	 }

	Watch::browseImages( image, wdir);
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
	af::Service service( "service", taskitem->getWDir(), "", taskitem->genFiles());

	QStringList images = afqt::stoq( service.getFiles()).split(';');
	if( num_img >= images.size())
	{
		displayError( "No such image nubmer.");
		return;
	}
	QString arg  = images[num_img];
	QString wdir( afqt::stoq( service.getWDir()));

	if( arg.isEmpty()) return;
	if( num_cmd >= af::Environment::getPreviewCmds().size())
	{
		displayError( "No such command number.");
		return;
	}

	QString cmd( afqt::stoq( af::Environment::getPreviewCmds()[num_cmd]));
	cmd = cmd.replace( AFWATCH::CMDS_ARGUMENT, arg);

	Watch::startProcess( cmd, wdir);
}

void ListTasks::actTaskListen()
{
	ItemJobTask *itemTask = (ItemJobTask*)getCurrentItem();
	Watch::listenTask( jobid, itemTask->getBlockNum(), itemTask->getTaskNum(),
		jobname + '(' + itemTask->getName() + ')');
}

void ListTasks::blockAction( int id_block, QString i_action) { blockAction( id_block, i_action, true); }
void ListTasks::blockAction( int id_block, const QString & i_action, bool i_query)
{
	if( id_block >= blocksnum)
	{
		AFERRAR("ListTasks::blockAction: id_block >= blocksnum (%d>=%d)", id_block, blocksnum)
		return;
	}

	std::ostringstream str;
	af::jsonActionStart( str, "jobs", "", std::vector<int>( 1, jobid));

	// Collect selected blocks ids:
	str << ",\n\"block_ids\":[";
	const QList<Item*> items( getSelectedItems());
	int counter = 0;
	for( int i = 0; i < items.count(); i++)
		if( items[i]->getId() == ItemJobBlock::ItemId )
		{
			if( counter ) str << ',';
			str << ((ItemJobBlock*)items[i])->getNumBlock();
			counter++;
		}
	str << ']';

	if( i_query )
	{
		if( false == wblocks[id_block]->blockAction( str, id_block, i_action, this))
			return;
	}
	else
	{
		str << ",\n" << i_action.toUtf8().data();
	}

	af::jsonActionFinish( str);
	Watch::sendMsg( af::jsonMsg( str));
}

bool ListTasks::mousePressed( QMouseEvent * event)
{
	QModelIndex index = m_view->indexAt( event->pos());
	if( qVariantCanConvert<Item*>( index.data()))
	{
		Item * item = qVariantValue<Item*>( index.data());
		if( item->getId() == ItemJobBlock::ItemId)
			return ((ItemJobBlock*)item)->mousePressed( event->pos(), m_view->visualRect( index));
	}
	return false;
}

void ListTasks::sortBlock( int numblock)
{
	if( numblock >= blocksnum )
	{
		AFERRAR("ListTasks::sortBlock: numblock >= blocksnum (%d>=%d)", numblock, blocksnum)
		return;
	}

	int sort_type = wblocks[numblock]->getSortType();
	bool sort_ascending = wblocks[numblock]->isSortAsceding();

	ItemJobTask ** array = new ItemJobTask*[tasksnum[numblock]];
	for( int i = 0; i < tasksnum[numblock]; i++) array[i] = wtasks[numblock][i];

	if( sort_type)\
		for( int i = 0; i < tasksnum[numblock]; i++)
			for( int j = tasksnum[numblock]-1; j > i; j--)
			{
				ItemJobTask * item_a = array[j-1];
				ItemJobTask * item_b = array[j];
				if( item_a->compare( sort_type, *item_b, sort_ascending ))
				{
					array[j-1] = item_b;
					array[j]   = item_a;
				}
			}

	int start = numblock+1;
	for( int b = 0; b < numblock; b++) start += tasksnum[b];

	const QList<Item*> selection = getSelectedItems();
	m_model->setItems( start, (Item**)array, tasksnum[numblock]);
	setSelectedItems( selection);

	delete [] array;
}
