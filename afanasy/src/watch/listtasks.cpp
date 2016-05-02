#include "listtasks.h"

#include "../include/afanasy.h"

#include "../libafanasy/address.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/jobprogress.h"
#include "../libafanasy/monitorevents.h"
#include "../libafanasy/service.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msgclasses/mctaskspos.h"
#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/msgclasses/mctasksprogress.h"

#include "actionid.h"
#include "dialog.h"
#include "itemjobblock.h"
#include "itemjobtask.h"
#include "modelitems.h"
#include "monitorhost.h"
#include "viewitems.h"
#include "watch.h"

#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtGui/QContextMenuEvent>
#include <QApplication>
#include <QBoxLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ListTasks::ListTasks( QWidget* parent, int JobId, const QString & JobName):
	ListItems( parent),
	m_job_id( JobId),
	m_job_name( JobName),
	m_blocks_num(0),
	m_blocks( NULL),
	m_tasks_num( NULL),
	m_tasks( NULL),
	constructed( false)
{
	init();

	m_view->setSpacing( 1);
//   view->setUniformItemSizes( true);
//   view->setBatchSize( 10000);

	std::ostringstream str;
	str << "{\"get\":{\"type\":\"jobs\",\"mode\":\"full\",\"binary\":true";
	str << ",\"ids\":[" << m_job_id << "]}}";

	af::Msg * msg = af::jsonMsg( str);
	Watch::sendMsg( msg);

	m_parentWindow->setWindowTitle( m_job_name);
}

void ListTasks::construct( af::Job * job)
{
	constructed = true;
	m_view->viewport()->hide();

	m_blocks_num = job->getBlocksNum();
	if( m_blocks_num == 0 ) return;

	m_tasks_num = new int[m_blocks_num];
	m_blocks = new ItemJobBlock*[m_blocks_num];
	m_tasks = new ItemJobTask**[m_blocks_num];
	int row = 0;
	for( int b = 0; b < m_blocks_num; b++)
	{
		const af::BlockData* block = job->getBlock( b);
		m_blocks[b] = new ItemJobBlock( block, this);
		m_tasks_num[b] = block->getTasksNum();
		m_blocks[b]->tasksHidded = ((m_blocks_num > 1) && (m_tasks_num[b] > 1));
		m_model->addItem( m_blocks[b]);
		row++;
		m_tasks[b] = new ItemJobTask*[m_tasks_num[b]];
		for( int t = 0; t < m_tasks_num[b]; t++)
		{
			ItemJobTask *wtask =  new ItemJobTask( this, m_blocks[b], t, block);
			m_model->addItem( wtask);
			if( m_blocks[b]->tasksHidded) m_view->setRowHidden( row , true);
			row++;
			m_tasks[b][t] = wtask;
		}
	}

	m_view->viewport()->show();
}

ListTasks::~ListTasks()
{
	MonitorHost::delJobId( m_job_id);

	for( int b = 0; b < m_blocks_num; b++) delete [] m_tasks[b];
	delete [] m_tasks_num;
	delete [] m_blocks;
	delete [] m_tasks;

	Watch::watchJodTasksWindowRem( m_job_id);
}

void ListTasks::v_connectionLost()
{
//	if( m_parentWindow != (QWidget*)Watch::getDialog())
		m_parentWindow->close();
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
				if( itemBlock->files.size() )
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

			if( m_job_id != AFJOB::SYSJOB_ID )
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

			std::vector<std::string> files = ((ItemJobTask*)(item))->genFiles();
			if( files.size())
			{
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
							QString file = afqt::stoq((*it).c_str());
							QMenu * submenu_img = new QMenu( QString("%1").arg( file), this);
							for( int i = 0; i < files.size(); i++)
							{
								QString imgname = file.right(99);
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
printf("ListTasks::caseMessage:\n"); msg->v_stdOut();
#endif
	switch( msg->type())
	{
	case af::Msg::TJob:
	{
		af::MCAfNodes mcnodes(msg);
		af::Job * job = (af::Job*)(mcnodes.getNode(0));
		if( job->getId() != m_job_id )
		{
			AFERROR(     "ListTasks::caseMessage: af::Msg::TJob: Jobs ids mismatch.")
			displayError("ListTasks::caseMessage: af::Msg::TJob: Jobs ids mismatch.");
			delete job;
			break;
		}

		if( constructed == false)
		{
			construct( job);

			std::ostringstream str;
			str << "{\"get\":{\"type\":\"jobs\",\"mode\":\"progress\",\"binary\":true";
			str << ",\"ids\":[" << m_job_id << "]}}";

			af::Msg * msg = af::jsonMsg( str);
			Watch::sendMsg( msg);

			MonitorHost::addJobId( m_job_id);
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
		if( m_job_id == progress->getJobId())
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
	case af::Msg::TBlocks:
	case af::Msg::TBlocksProperties:
	case af::Msg::TBlocksProgress:
	{
		af::MCAfNodes mcblocks( msg);
		int count = int( mcblocks.getCount());
		for( int b = 0; b < count; b++)
		{
			af::BlockData * block = (af::BlockData*)mcblocks.getNode( b);
			if( block->getJobId() != m_job_id) continue;
			int blocknum = block->getBlockNum();
			if( blocknum >= m_blocks_num ) continue;

			m_blocks[blocknum]->update( block, msg->type());

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

bool ListTasks::processEvents( const af::MonitorEvents & i_me)
{
	bool founded = false;

	if( i_me.m_tp.size())
	{
		for( int j = 0; j < i_me.m_tp.size(); j++ )
		{
			if( i_me.m_tp[j].job_id != m_job_id )
				continue;

			updateTasks( i_me.m_tp[j].blocks, i_me.m_tp[j].tasks, i_me.m_tp[j].tp);

			founded = true;

			break;
		}
	}

	if( i_me.m_bids.size())
	{
		std::vector<int32_t> job_ids;
		std::vector<int32_t> block_ids;
		std::vector<std::string> modes;

		for( int j = 0; j < i_me.m_bids.size(); j++ )
		{
			if( i_me.m_bids[j].job_id != m_job_id )
				continue;

			job_ids.push_back( m_job_id);
			block_ids.push_back( i_me.m_bids[j].block_num);

			modes.push_back( af::BlockData::DataModeFromMsgType( i_me.m_bids[j].mode));
		}

		if( block_ids.size())
		{
			Watch::get( "jobs", job_ids, modes, block_ids);

			founded = true;
		}
	}

	//
	// Check if this job deleted:
	for( int i = 0; i < i_me.m_events[af::Monitor::EVT_jobs_del].size(); i++)
	{
		if( i_me.m_events[af::Monitor::EVT_jobs_del][i] == m_job_id )
		{
			founded = true;
			displayWarning( "The job does not exist any more.");
			m_parentWindow->close();
			break;
		}
	}

	return founded;
}

int ListTasks::getRow( int block, int task)
{
	int row = -1;
	if( block < m_blocks_num )
	{
		if((task != -1) && (task >= m_tasks_num[block]))
		{
			AFERRAR("ListTasks::getRow: task >= m_tasks_num[block] : (%d>=%d[%d])", task, m_tasks_num[block], block)
		}
		else
		{
			row = 1;
			for( int b = 0; b < block; b++) row += 1 + m_tasks_num[b];
			row += task;
		}
	}
	else
		AFERRAR("ListTasks::getRow: block >= m_blocks_num : (%d>=%d)", block, m_blocks_num)
//printf("ListTasks::getRow: b[%d] t[%d] = %d\n", block, task, row);
	return row;
}

bool ListTasks::updateProgress( const af::JobProgress * progress/*bool blocksOnly = false*/)
{
	if( m_blocks_num != progress->getBlocksNum())
	{
		AFERRAR("ListTasks::updateProgress: Blocks number mismatch (%d!=%d).", m_blocks_num, progress->getBlocksNum())
		return false;
	}

	for( int b = 0; b < m_blocks_num; b++)
	{
		if( m_tasks_num[b] != progress->getTasksNum(b))
		{
			AFERRAR("ListTasks::updateProgress: Tasks number mismatch in block #%d (%d!=%d)", b, m_tasks_num[b], progress->getTasksNum(b))
			return false;
		}

		for( int t = 0; t < m_tasks_num[b]; t++)
		{
			m_tasks[b][t]->upProgress( *(progress->tp[b][t]) );
		}
	}

	setWindowTitleProgress();

	return true;
}

bool ListTasks::updateTasks(
	const std::vector<int32_t> & i_blocks,
	const std::vector<int32_t> & i_tasks,
	const std::vector<af::TaskProgress> & i_tps)
{
	if(( i_tps.size() != i_blocks.size()) && ( i_tps.size() != i_tasks.size()))
	{
		AFERRAR("ListTasks::updateTasks: input sizes mismatch: %d, %d, %d", int(i_tasks.size()), int(i_blocks.size()), int(i_tps.size()))
		return false;
	}

	int firstChangedRow = -1;
	int lastChangedRow = -1;
	for( int i = 0; i < i_tps.size(); i++)
	{
		if( i_blocks[i] > m_blocks_num)
		{
			AFERRAR("ListTasks::updateTasks: block > m_blocks_num (%d>%d)", i_blocks[i], m_blocks_num)
			return false;
		}
		if( i_tasks[i] > m_tasks_num[i_blocks[i]])
		{
			AFERRAR("ListTasks::updateTasks: task > m_tasks_num[%d] (%d>%d)", i_blocks[i], i_tasks[i], m_tasks_num[i_blocks[i]])
			return false;
		}
		m_tasks[i_blocks[i]][i_tasks[i]]->upProgress( i_tps[i]);

		int row = getRow( i_blocks[i], i_tasks[i]);
		if( row != -1 )
		{
			if((firstChangedRow == -1) || (firstChangedRow > row)) firstChangedRow = row;
			if(  lastChangedRow < row) lastChangedRow = row;
		}
	}

	if( firstChangedRow != -1 ) m_model->emit_dataChanged( firstChangedRow, lastChangedRow);

	setWindowTitleProgress();

	return true;
}

void ListTasks::setWindowTitleProgress()
{
	int total_percent = 0;
	int total_tasks = 0;
	for( int b = 0; b < m_blocks_num; b++)
		for( int t = 0; t < m_tasks_num[b]; t++)
		{
			if(( m_tasks[b][t]->taskprogress.state & AFJOB::STATE_DONE_MASK) ||
				( m_tasks[b][t]->taskprogress.state & AFJOB::STATE_SKIPPED_MASK))
				total_percent += 100;
			else if ( m_tasks[b][t]->taskprogress.state & AFJOB::STATE_RUNNING_MASK )
				total_percent += m_tasks[b][t]->taskprogress.percent;
			total_tasks++;
		}

	m_parentWindow->setWindowTitle( QString("%1% %2").arg(total_percent/total_tasks).arg(m_job_name));
}

void ListTasks::actTasksSkip()    { tasksOpeation("skip"); }
void ListTasks::actTasksRestart() { tasksOpeation("restart"); }

void ListTasks::actTaskLog()        { getTaskInfo("log");}
void ListTasks::actTaskInfo()       { getTaskInfo("info");}
void ListTasks::actTaskErrorHosts() { getTaskInfo("error_hosts");}
void ListTasks::actTaskStdOut( int i_number ) { getTaskInfo("output", i_number);}

void ListTasks::getTaskInfo( const std::string & i_mode, int i_number, Item * item)
{
//{"get":{"type":"jobs","mode":"files","ids":[2],"block_ids":[0],"task_ids":[3],"binary":true}}
//{"get":{"type":"jobs","ids":[2],"mode":"output","number":11,"block_ids":[0],"task_ids":[4]}}
	if( item == NULL) item = getCurrentItem();
	if( item->getId() != ItemJobTask::ItemId )
		return;

	ItemJobTask *itemTask = (ItemJobTask*)item;

	std::ostringstream str;
	str << "{\"get\":{\"type\":\"jobs\"";
	str << ",\"mode\":\"" << i_mode << "\"";
	str << ",\"ids\":[" << m_job_id << "]";
	str << ",\"block_ids\":[" << itemTask->getBlockNum() << "]";
	str << ",\"task_ids\":[" << itemTask->getTaskNum() << "]";
	if( i_number != -1 )
		str << ",\"number\":" << i_number;
	str << ",\"mon_id\":" << MonitorHost::id();
	str << ",\"binary\":true}}";

	af::Msg * msg = af::jsonMsg( str);
	Watch::sendMsg( msg);
}

void ListTasks::doubleClicked( Item * item)
{
	if( item->getId() == ItemJobTask ::ItemId )
		getTaskInfo("info", -1, item);
	else if( item->getId() == ItemJobBlock::ItemId )
	{
		ItemJobBlock * block = (ItemJobBlock*)item;
		int blockNum = block->getNumBlock();
		bool hide = false == m_blocks[blockNum]->tasksHidded;
		m_blocks[blockNum]->tasksHidded = hide;
		int row_start = getRow( blockNum, 0);
		int row_end   = getRow( blockNum, m_tasks_num[blockNum]-1);
		for( int row = row_start; row <= row_end; row++) m_view->setRowHidden( row, hide);
//   view->updateGeometries();
		if( block->resetSortingParameters()) sortBlock( block->getNumBlock());
	}
}

void ListTasks::tasksOpeation( const std::string & i_type)
{
	std::ostringstream str;
	af::jsonActionStart( str, "jobs", "", std::vector<int>( 1, m_job_id));
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

void ListTasks::actBlockCommand()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->command;
	QString str = QInputDialog::getText(this, "Change Command", "Enter Command", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	str = afqt::stoq( af::strEscape( afqt::qtos( str)));
	blockAction( 0, QString("\"params\":{\"command\":\"%1\"}").arg( str), false);
}
void ListTasks::actBlockWorkingDir()
{
	bool ok;
	QString cur = afqt::stoq(((ItemJobBlock*)( getCurrentItem()))->workingdir);
	QString str = QInputDialog::getText(this, "Change Working Directory", "Enter Directory", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	str = afqt::stoq( af::strEscape( afqt::qtos( str)));
	blockAction( 0, QString("\"params\":{\"working_directory\":\"%1\"}").arg( str), false);
}
void ListTasks::actBlockFiles()
{
	bool ok;
	QString cur = afqt::stoq( af::strJoin(((ItemJobBlock*)( getCurrentItem()))->files, ";"));
	QString str = QInputDialog::getText(this, "Change Files", "Enter Files", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	QString params = QString("\"params\":{\"files\":[");
	QStringList files = str.split(";");
	for( int i = 0; i < files.size(); i++ )
	{
		if( i ) params += ",";
		params += "\"" + afqt::stoq( af::strEscape( afqt::qtos( files[i]))) + "\"";
	}
	params += "]}";

	blockAction( 0, params, false);
}
void ListTasks::actBlockCmdPost()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->cmdpost;
	QString str = QInputDialog::getText(this, "Change Post Command", "Enter Command", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	str = afqt::stoq( af::strEscape( afqt::qtos( str)));
	blockAction( 0, QString("\"params\":{\"command_post\":\"%1\"}").arg(str), false);
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
		  af::Service service( "service", itemBlock->workingdir, "", itemBlock->files);
		  image = afqt::stoq( service.getFiles()[0]);
		  wdir = afqt::stoq( service.getWDir());
		  break;
	 }
	 case ItemJobTask::ItemId:
	 {
		  ItemJobTask* taskitem = (ItemJobTask*)item;
		  af::Service service("service", taskitem->getWDir(), "", taskitem->genFiles());
		  image = afqt::stoq( service.getFiles()[0]);
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

	std::vector<std::string> images = service.getFiles();
	if( num_img >= images.size())
	{
		displayError( "No such image nubmer.");
		return;
	}
	QString arg = afqt::stoq( images[num_img]);
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
	Watch::listenTask( m_job_id, itemTask->getBlockNum(), itemTask->getTaskNum(),
		m_job_name + '(' + itemTask->getName() + ')');
}

void ListTasks::blockAction( int id_block, QString i_action) { blockAction( id_block, i_action, true); }
void ListTasks::blockAction( int id_block, const QString & i_action, bool i_query)
{
	if( id_block >= m_blocks_num)
	{
		AFERRAR("ListTasks::blockAction: id_block >= m_blocks_num (%d>=%d)", id_block, m_blocks_num)
		return;
	}

	std::ostringstream str;
	af::jsonActionStart( str, "jobs", "", std::vector<int>( 1, m_job_id));

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
		if( false == m_blocks[id_block]->blockAction( str, id_block, i_action, this))
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
	if( Item::isItemP( index.data()) == false ) return false;

	Item * item = Item::toItemP( index.data());
	if( item->getId() == ItemJobBlock::ItemId)
		return ((ItemJobBlock*)item)->mousePressed( event->pos(), m_view->visualRect( index));

	if(( QApplication::mouseButtons() == Qt::MidButton ) || ( QApplication::keyboardModifiers() == Qt::AltModifier ))
		((ItemJobTask*)item)->showThumbnail();

	return false;
}

void ListTasks::sortBlock( int i_block_num)
{
	if( i_block_num >= m_blocks_num )
	{
		AFERRAR("ListTasks::sortBlock: i_block_num >= m_blocks_num (%d>=%d)", i_block_num, m_blocks_num)
		return;
	}

	int sort_type = m_blocks[i_block_num]->getSortType();
	bool sort_ascending = m_blocks[i_block_num]->isSortAsceding();

	ItemJobTask ** array = new ItemJobTask*[m_tasks_num[i_block_num]];
	for( int i = 0; i < m_tasks_num[i_block_num]; i++) array[i] = m_tasks[i_block_num][i];

	if( sort_type)
		for( int i = 0; i < m_tasks_num[i_block_num]; i++)
			for( int j = m_tasks_num[i_block_num]-1; j > i; j--)
			{
				ItemJobTask * item_a = array[j-1];
				ItemJobTask * item_b = array[j];
				if( item_a->compare( sort_type, *item_b, sort_ascending ))
				{
					array[j-1] = item_b;
					array[j]   = item_a;
				}
			}

	int start = i_block_num+1;
	for( int b = 0; b < i_block_num; b++) start += m_tasks_num[b];

	const QList<Item*> selection = getSelectedItems();
	m_model->setItems( start, (Item**)array, m_tasks_num[i_block_num]);
	setSelectedItems( selection);

	delete [] array;
}

bool ListTasks::v_filesReceived( const af::MCTaskUp & i_taskup )
{
	if( i_taskup.getNumJob() != m_job_id )
		return false; // This is some for other job

	if(( i_taskup.getNumBlock() < 0 ) || ( i_taskup.getNumTask() < 0 ))
		return false; // This files are for an entire job

	if( i_taskup.getNumBlock() >= m_blocks_num )
	{
		AFERRAR("ListTasks::taskFilesReceived: i_taskup.getNumBlock() >= m_blocks_num ( %d >= %d )", i_taskup.getNumBlock(), m_blocks_num)
		return true;
	}

	if( i_taskup.getNumTask() >= m_tasks_num[i_taskup.getNumBlock()] )
	{
		AFERRAR("ListTasks::taskFilesReceived: i_taskup.getNumBlock() >= m_blocks_num ( %d >= %d )", i_taskup.getNumBlock(), m_blocks_num)
		return true;
	}

	m_tasks[i_taskup.getNumBlock()][i_taskup.getNumTask()]->taskFilesReceived( i_taskup);

	return true;
}

