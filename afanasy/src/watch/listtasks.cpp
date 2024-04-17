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
#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/msgclasses/mctasksprogress.h"

#include "actionid.h"
#include "dialog.h"
#include "itemjobblock.h"
#include "itemjobtask.h"
#include "modelitems.h"
#include "monitorhost.h"
#include "paramspaneltask.h"
#include "viewitems.h"
#include "watch.h"
#include "wndtask.h"

#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtGui/QContextMenuEvent>
#include <QBoxLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ListTasks::ListTasks( QWidget* parent, int JobId, const QString & JobName):
	ListItems(parent, "tasks"),
	m_job_id(JobId),
	m_job_name(JobName),
	constructed(false)
{
	m_paramspanel_task = new ParamsPanelTask();
	m_paramspanel = m_paramspanel_task;
	initListItems();

	m_view->setSpacing( 1);
//   view->setUniformItemSizes( true);
//   view->setBatchSize( 10000);

	this->setWindowTitleWithPrefix( m_job_name);

	getJobFullData();

	displayInfo("Use Alt+LMB or MMB on a task to ask server for thumbnails.");
}

void ListTasks::getJobFullData()
{
	std::ostringstream str;
	str << "{\"get\":{\"type\":\"jobs\",\"mode\":\"full\",\"binary\":true";
	str << ",\"ids\":[" << m_job_id << "]}}";

	af::Msg * msg = af::jsonMsg( str);
	Watch::sendMsg(msg);
}

void ListTasks::construct(af::Job * i_job)
{
	constructed = true;
	m_view->viewport()->hide();

	if (i_job->getBlocksNum() == 0)
	{
		displayError("Job with a zero blocks number received.");
		return;
	}

	for (int b = 0; b < i_job->getBlocksNum(); b++)
		appendBlock(i_job->getBlockData(b));

	// If the job has several blocks, we hide tasks if block more than one task
	if (m_blocks.size() > 1)
	{
		int row = 0;
		for (int b = 0; b < m_blocks.size(); b++)
		{
			m_blocks[b]->tasksHidden = m_tasks[b].size() > 1;
			row++;

			for (int t = 0; t < m_tasks[b].size(); t++)
			{
				if (m_blocks[b]->tasksHidden)
					m_view->setRowHidden(row, true);
				row++;
			}
		}
	}

	m_view->viewport()->show();
}

void ListTasks::appendBlock(af::BlockData * i_block)
{
	ItemJobBlock * item_block = new ItemJobBlock(i_block, this);
	m_blocks.append(item_block);
	m_model->addItem(item_block);

	appendTasks(i_block, item_block);
}

void ListTasks::appendTasks(af::BlockData * i_block, ItemJobBlock * i_item_block)
{
	int start_task_num = 0;
	if (m_tasks.size() <= i_block->getBlockNum())
	{
		// A block was just created and has no task items
		m_tasks.resize(i_block->getBlockNum() + 1);
	}
	else
	{
		// We appending tasks to an existing block item
		start_task_num = m_tasks[i_block->getBlockNum()].size();
	}

	for (int t = start_task_num; t < i_block->getTasksNum(); t++)
	{
		ItemJobTask * item_task = new ItemJobTask(this, i_item_block, t, i_block);
		m_model->addItem(item_task);
		m_tasks[i_block->getBlockNum()].append(item_task);
	}
}

ListTasks::~ListTasks()
{
	for (int i = 0; i < m_wndtasks.size(); i++)
		m_wndtasks[i]->parentClosed();
	
	MonitorHost::delJobId(m_job_id);

	Watch::watchJobTasksWindowRem(m_job_id);
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
	generateMenu(menu, item);
	menu.exec(event->globalPos());
}

void ListTasks::generateMenu(QMenu &o_menu, Item * i_item)
{
	QAction *action;

	switch (i_item->getType())
	{
		case Item::TBlock:
		{
			ItemJobBlock *itemBlock = static_cast<ItemJobBlock*>(i_item);
			
			if (itemBlock->hasFiles())
			{
				action = new QAction("Browse Files...", this);
				connect(action, SIGNAL(triggered()), this, SLOT(actBrowseFolder()));
				o_menu.addAction(action);
				
				const std::vector<std::string> preview_cmds = af::Environment::getPreviewCmds();
				std::vector<std::string> files = itemBlock->getFiles();
                if (preview_cmds.size())
                {
                    QMenu * submenu_cmd = new QMenu("Launch", this);
                    for (int i = 0; i < files.size(); i++)
                    {
                        action = new QAction(afqt::stoq(files[i]).right(55), this);
                        action->setEnabled(false);
                        QFont f = action->font();
                        f.setItalic(true);
                        action->setFont(f);
                        submenu_cmd->addAction( action);
                        for (int p = 0; p < preview_cmds.size(); p++)
                        {
                            QString cmd = afqt::stoq( preview_cmds[p]);
                            QStringList cmdSplit = cmd.split("|");
                                
                            ActionIdId * actionid = new ActionIdId(p, i, QString("    " + cmdSplit.first()), this);
                            connect(actionid, SIGNAL(triggeredId(int,int)), this, SLOT(actBlockPreview(int,int)));
                            submenu_cmd->addAction(actionid);
                        }
                        submenu_cmd->addSeparator();
                    }
                    o_menu.addMenu(submenu_cmd);
                }
                o_menu.addSeparator();
			}
            
			QMenu * submenu = new QMenu( "Change Block", this);

			// Operations on the current block item
			itemBlock->getInfo()->generateMenu(&o_menu, submenu);

			o_menu.addMenu( submenu);
			
			o_menu.addSeparator();
			// Operations on all the selected blocks

			submenu = new QMenu( "Change Tasks", this);
			o_menu.addMenu( submenu);

			action = new QAction( "Set Command", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actBlockCommand() ));
			submenu->addAction( action);

			action = new QAction( "Set Working Directory", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actBlockWorkingDir() ));
			submenu->addAction( action);

			action = new QAction( "Set Environment", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actBlockEnvironment() ));
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
		case Item::TTask:
		{
			ItemJobTask *itemTask = static_cast<ItemJobTask*>(i_item);

			action = new QAction("Open Task", this);
			connect(action, SIGNAL(triggered() ), this, SLOT(actTaskOpen()));
			o_menu.addAction(action);

			o_menu.addSeparator();

			if (itemTask->hasFiles())
			{
				const std::vector<std::string> files = itemTask->getFiles();

				o_menu.addSeparator();

				action = new QAction("Browse Files...", this);
				connect(action, SIGNAL(triggered() ), this, SLOT(actBrowseFolder()));
				o_menu.addAction(action);

				const std::vector<std::string> preview_cmds = af::Environment::getPreviewCmds();
				if (preview_cmds.size())
                {
                    QMenu * submenu_cmd = new QMenu("Launch", this);
                    for (int i = 0; i < files.size(); i++)
                    {
                        action = new QAction(afqt::stoq(files[i]).right(55), this);
                        action->setEnabled(false);
                        QFont f = action->font();
                        f.setItalic(true);
                        action->setFont(f);
                        submenu_cmd->addAction(action);
                        for (int p = 0; p < preview_cmds.size(); p++)
                        {
                            QString cmd = afqt::stoq(preview_cmds[p]);
                            QStringList cmdSplit = cmd.split("|");
                                
                            ActionIdId * actionid = new ActionIdId(p, i, QString("    " + cmdSplit.first()), this);
                            connect(actionid, SIGNAL(triggeredId(int,int) ), this, SLOT(actTaskPreview(int,int)));
                            submenu_cmd->addAction(actionid);
                        }
                        submenu_cmd->addSeparator();
                    }
                    o_menu.addMenu(submenu_cmd);
                }
                o_menu.addSeparator();
			}

			if ((itemTask->taskprogress.state & AFJOB::STATE_READY_MASK) &&
				(false == (itemTask->taskprogress.state & AFJOB::STATE_TRYTHISTASKNEXT_MASK)))
			{
				action = new QAction("Try This Task Next", this);
				connect(action, SIGNAL(triggered()), this, SLOT(actTaskTryNext()));
				o_menu.addAction(action);

				o_menu.addSeparator();
			}
			if (itemTask->taskprogress.state & AFJOB::STATE_TRYTHISTASKNEXT_MASK)
			{
				action = new QAction("Do Not Try Next", this);
				connect(action, SIGNAL(triggered()), this, SLOT(actTaskDoNotTry()));
				o_menu.addAction(action);

				o_menu.addSeparator();
			}

			o_menu.addSeparator();
			
			action = new QAction( "Skip Tasks", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actTasksSkip() ));
			o_menu.addAction( action);

			if (af::Environment::VISOR())
			{
				action = new QAction("Done Tasks", this);
				connect(action, SIGNAL(triggered()), this, SLOT(actTasksDone()));
				o_menu.addAction(action);
			}

			action = new QAction( "Restart Tasks", this);
			connect( action, SIGNAL( triggered() ), this, SLOT( actTasksRestart() ));
			o_menu.addAction( action);

			if (itemTask->taskprogress.state & (AFJOB::STATE_READY_MASK | AFJOB::STATE_RUNNING_MASK | AFJOB::STATE_SKIPPED_MASK))
			{
				action = new QAction("Suspend Tasks", this);
				connect(action, SIGNAL(triggered()), this, SLOT(actTasksSuspend()));
				o_menu.addAction(action);
			}

			if (itemTask->taskprogress.state & AFJOB::STATE_SUSPENDED_MASK)
			{
				action = new QAction("Continue Tasks", this);
				connect(action, SIGNAL(triggered()), this, SLOT(actTasksContinue()));
				o_menu.addAction(action);
			}

			break;
		}
		default:
		{
			AF_ERR << "ListTasks::contextMenuEvent: unknown item type = " << i_item->getType();
		}
	}
}

bool ListTasks::v_caseMessage(af::Msg * msg)
{
	switch (msg->type())
	{
	case af::Msg::TJob:
	{
		af::MCAfNodes mcnodes(msg);
		af::Job * job = static_cast<af::Job*>(mcnodes.getNode(0));

		if(job->getId() != m_job_id)
		{
			// No job deletion needed.
			// af::MCAfNodes delete all node in dtor.
			return false;
		}

		if (constructed == false)
		{
			construct(job);

			std::ostringstream str;
			str << "{\"get\":{\"type\":\"jobs\",\"mode\":\"progress\",\"binary\":true";
			str << ",\"ids\":[" << m_job_id << "]}}";

			af::Msg * msg = af::jsonMsg( str);
			Watch::sendMsg( msg);

			MonitorHost::addJobId(m_job_id);
		}
		else
		{
			AF_ERR << "af::Msg::TJob: Job is already constructed.";
			displayError("af::Msg::TJob: Job is already constructed.");
		}
		break;
	}
	case af::Msg::TJobProgress:
	{
		if (constructed == false)
			break;

		af::JobProgress * progress = new af::JobProgress(msg);
		if (m_job_id == progress->getJobId())
		{
			if (updateProgress(progress) == false)
			{
				displayError("Tasks update error. Closing tasks window.");
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
		af::MCAfNodes mcblocks(msg);
		int count = int(mcblocks.getCount());
		for (int b = 0; b < count; b++)
		{
			af::BlockData * block = static_cast<af::BlockData*>(mcblocks.getNode(b));
			if (block->getJobId() != m_job_id)
			{
				AF_ERR << "ListTasks::v_caseMessage: block->getJobId() != m_job_id: "
					<< block->getJobId() << "!=" << m_job_id;
				continue;
			}

			int blocknum = block->getBlockNum();
			if (blocknum >= m_blocks.size())
			{
				if (msg->type() == af::Msg::TBlocks)
				{
					// A new block(s) was appended to the job
					appendBlock(block);
				}
				else
				{
					AF_ERR << "ListTasks::v_caseMessage: blocknum >= m_blocks.size(): "
						<< blocknum << ">=" << m_blocks.size();
				}
			}
			else
			{
				m_blocks[blocknum]->update(block, msg->type());

				if ((msg->type() == af::Msg::TBlocks) &&
					(block->getTasksNum() > m_tasks[blocknum].size()))
				{
					// New tasks were appended to block
					appendTasks(block, m_blocks[blocknum]);
				}
			}

			if (msg->type() == af::Msg::TBlocks)
				m_model->emit_dataChanged();
			else
			{
				int row = getRow(b);
				if (row != -1)
					m_model->emit_dataChanged(getRow( blocknum));
			}
		}
		if (msg->type() == af::Msg::TBlocks)
			m_model->emit_dataChanged();
		break;
	}
	default:
		return false;
	}
	return true;
}

bool ListTasks::v_processEvents( const af::MonitorEvents & i_me)
{
	bool found = false;

	if( i_me.m_tp.size())
	{
		for( int j = 0; j < i_me.m_tp.size(); j++ )
		{
			if( i_me.m_tp[j].job_id != m_job_id )
				continue;

			updateTasks( i_me.m_tp[j].blocks, i_me.m_tp[j].tasks, i_me.m_tp[j].tp);

			found = true;

			break;
		}
	}

	if( i_me.m_bids.size())
	{
		std::vector<int32_t> block_ids;
		std::vector<std::string> modes;

		for( int j = 0; j < i_me.m_bids.size(); j++ )
		{
			if( i_me.m_bids[j].job_id != m_job_id )
				continue;

			block_ids.push_back( i_me.m_bids[j].block_num);

			modes.push_back( af::BlockData::DataModeFromMsgType( i_me.m_bids[j].mode));
		}

		if( block_ids.size())
		{
			Watch::get("jobs", std::vector<int>(1, m_job_id), modes, block_ids);

			found = true;
		}
	}

	//
	// Check if this job deleted:
	for( int i = 0; i < i_me.m_events[af::Monitor::EVT_jobs_del].size(); i++)
	{
		if( i_me.m_events[af::Monitor::EVT_jobs_del][i] == m_job_id )
		{
			found = true;
			displayWarning( "The job does not exist any more.");
			m_parentWindow->close();
			break;
		}
	}

	return found;
}

int ListTasks::getRow(int i_block, int i_task)
{
	int row = -1;
	if (i_block < m_blocks.size())
	{
		if((i_task != -1) && (i_task >= m_tasks[i_block].size()))
		{
			AF_ERR << "ListTasks::getRow: i_task >= m_tasks[i_block].size(): "
				<< i_task << ">=" << m_tasks[i_block].size();
		}
		else
		{
			row = 1;
			for (int b = 0; b < i_block; b++)
				row += 1 + m_tasks[b].size();
			row += i_task;
		}
	}
	else
		AF_ERR << "ListTasks::getRow: i_block >= m_blocks.size(): "
			<< i_block << ">=" << m_blocks.size();

	return row;
}

bool ListTasks::updateProgress(const af::JobProgress * i_job_progress/*bool blocksOnly = false*/)
{
	if (m_blocks.size() != i_job_progress->getBlocksNum())
	{
		AF_ERR << "ListTasks::updateProgress: Blocks number mismatch: "
			<< m_blocks.size() << "!=" << i_job_progress->getBlocksNum();
		return false;
	}

	for (int b = 0; b < m_blocks.size(); b++)
	{
		if (m_tasks[b].size() != i_job_progress->getTasksNum(b))
		{
			AF_ERR << "ListTasks::updateProgress: Tasks number mismatch in block #"
				<< b << ": " << m_tasks[b].size() << "!=" << i_job_progress->getTasksNum(b);
			return false;
		}

		for (int t = 0; t < m_tasks[b].size(); t++)
		{
			m_tasks[b][t]->upProgress(*(i_job_progress->tp[b][t]) );
		}
	}

	setWindowTitleProgress();
	repaintItems();
	updateResources();

	return true;
}

void ListTasks::updateResources()
{
	QMap<QString, QVector<float>> resmap;
	for (int b = 0; b < m_blocks.size(); b++)
	{
		for (int t = 0; t < m_tasks[b].size(); t++)
		{
			QStringList pair = afqt::stoq(m_tasks[b][t]->taskprogress.resources).split(' ');
			for (int i = 0; i < pair.size(); i++)
			{
				QStringList res = pair[i].split(':');
				if (res.size() != 2)
					continue;

				resmap[res[0]].append(res[1].toFloat());
			}
		}
	}
	if (resmap.size())
		m_paramspanel_task->updateResources(resmap);
}

bool ListTasks::updateTasks(
	const std::vector<int32_t> & i_blocks,
	const std::vector<int32_t> & i_tasks,
	const std::vector<af::TaskProgress> & i_tps)
{
	if(( i_tps.size() != i_blocks.size()) && ( i_tps.size() != i_tasks.size()))
	{
		AF_ERR << "ListTasks::updateTasks: input sizes mismatch: "
			<< int(i_tasks.size()) << ", " << int(i_blocks.size()) << ", " << int(i_tps.size());
		return false;
	}

	int firstChangedRow = -1;
	int lastChangedRow = -1;
	for (int i = 0; i < i_tps.size(); i++)
	{
		if (i_blocks[i] >= m_blocks.size())
		{
			AF_ERR << "ListTasks::updateTasks: block >= m_blocks.size(): "
				<< i_blocks[i] << ">=" << m_blocks.size();
			return false;
		}
		if (i_tasks[i] >= m_tasks[i_blocks[i]].size())
		{
			AF_ERR << "ListTasks::updateTasks: i_tasks[i] >= m_tasks[i_blocks[i]].size():"
				<< i_tasks[i] << ">=" << m_tasks[i_blocks[i]].size();
			return false;
		}
		m_tasks[i_blocks[i]][i_tasks[i]]->upProgress(i_tps[i]);

		int row = getRow(i_blocks[i], i_tasks[i]);
		if (row != -1)
		{
			if ((firstChangedRow == -1) || (firstChangedRow > row))
				firstChangedRow = row;
			if (lastChangedRow < row)
				lastChangedRow = row;
		}

		/// Update opened tasks windows ( if any )
		for (int w = 0; w < m_wndtasks.size(); w++)
			if (m_wndtasks[w]->isSameTask(af::MCTaskPos(m_job_id, i_blocks[i], i_tasks[i])))
					m_wndtasks[w]->updateProgress(i_tps[i]);
	}

	if (firstChangedRow != -1)
		m_model->emit_dataChanged(firstChangedRow, lastChangedRow);

	setWindowTitleProgress();
	updateResources();

	return true;
}

void ListTasks::setWindowTitleProgress()
{
	int total_percent = 0;
	int total_tasks = 0;
	for (int b = 0; b < m_blocks.size(); b++)
		for (int t = 0; t < m_tasks[b].size(); t++)
		{
			if ((m_tasks[b][t]->taskprogress.state & AFJOB::STATE_DONE_MASK) ||
				(m_tasks[b][t]->taskprogress.state & AFJOB::STATE_SKIPPED_MASK))
				total_percent += 100;
			else if (m_tasks[b][t]->taskprogress.state & AFJOB::STATE_RUNNING_MASK)
				total_percent += m_tasks[b][t]->taskprogress.percent;
			total_tasks++;
		}

	this->setWindowTitleWithPrefix( QString("%1% %2").arg(total_percent/total_tasks).arg(m_job_name));
}

void ListTasks::actTaskOpen()
{
	Item * item = getCurrentItem();
	if (item->getType() != Item::TTask)
		return;

	openTask(static_cast<ItemJobTask*>(item));
}

void ListTasks::actTasksSkip()   {tasksOperation("skip");   }
void ListTasks::actTasksDone()   {tasksOperation("done");   }
void ListTasks::actTasksRestart(){tasksOperation("restart");}
void ListTasks::actTasksSuspend(){tasksOperation("suspend");}
void ListTasks::actTasksContinue(){tasksOperation("continue");}
void ListTasks::actTaskTryNext() {tasksOperation("trynext","append");}
void ListTasks::actTaskDoNotTry(){tasksOperation("trynext","remove");}

void ListTasks::openTask( ItemJobTask * i_itemTask)
{
	m_wndtasks.push_back( WndTask::openTask( i_itemTask->getTaskPos(), this));
}

void ListTasks::taskWindowClosed( WndTask * i_wndtask)
{
	std::vector<WndTask*>::iterator it = m_wndtasks.begin();
	while( it != m_wndtasks.end())
		if( *it == i_wndtask )
			it = m_wndtasks.erase( it);
		else
			it++;
}

void ListTasks::v_doubleClicked(Item * i_item)
{
	if (i_item->getType() == Item::TTask)
	{
		openTask(static_cast<ItemJobTask*>(i_item));
	}
	else if (i_item->getType() == Item::TBlock)
	{
		ItemJobBlock * block = static_cast<ItemJobBlock*>(i_item);
		int blockNum = block->getNumBlock();
		bool hide = false == m_blocks[blockNum]->tasksHidden;
		m_blocks[blockNum]->tasksHidden = hide;
		int row_start = getRow(blockNum, 0);
		int row_end   = getRow(blockNum, m_tasks[blockNum].size() - 1);
		for (int row = row_start; row <= row_end; row++)
			m_view->setRowHidden(row, hide);
//   view->updateGeometries();
		if (block->resetSortingParameters())
			sortBlock(block->getNumBlock());
	}
}

void ListTasks::tasksOperation(const std::string & i_type, const std::string & i_mode)
{
	std::ostringstream str;
	af::jsonActionStart( str, "jobs", "", std::vector<int>( 1, m_job_id));
	str << ",\n\"operation\":{\n\"type\":\"" << i_type << '"';
	if (i_mode.size())
		str << ",\n\"mode\":\"" << i_mode << '"';
	str << ",\n\"task_ids\":[";

	int blockId = -1;
	// Collect tasks of the same block:
	const QList<Item*> items(getSelectedItems());
	for (int i = 0; i < items.count(); i++)
	{
		if (items[i]->getType() != Item::TTask)
			continue;
		else
		{
			ItemJobTask * itemTask = static_cast<ItemJobTask*>(items[i]);
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
	blockAction(QString("\"params\":{\"command\":\"%1\"}").arg(str));
}
void ListTasks::actBlockWorkingDir()
{
	bool ok;
	QString cur = afqt::stoq(((ItemJobBlock*)(getCurrentItem()))->getWDirOriginal());
	QString str = QInputDialog::getText(this, "Change Working Directory", "Enter Directory", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	str = afqt::stoq( af::strEscape( afqt::qtos( str)));
	blockAction(QString("\"params\":{\"working_directory\":\"%1\"}").arg(str));
}
void ListTasks::actBlockEnvironment()
{
	bool ok;
	QString str = QInputDialog::getText( this,"Set Environment", "Enter name=value pair:", QLineEdit::Normal, QString(), &ok);
	if( !ok) return;
	str = afqt::stoq( af::strEscape( afqt::qtos( str)));
    QStringList list = str.split('=');
    if( list.size() != 2 )
    {
        displayError( QString("Invalid name=value pair: ") + str);
    }
	blockAction(QString("\"params\":{\"environment\":{\"%1\":\"%2\"}}").arg(list[0], list[1]));
}
void ListTasks::actBlockCmdPost()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->cmdpost;
	QString str = QInputDialog::getText(this, "Change Post Command", "Enter Command", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	str = afqt::stoq( af::strEscape( afqt::qtos( str)));
	blockAction(QString("\"params\":{\"command_post\":\"%1\"}").arg(str));
}
void ListTasks::actBlockFiles()
{
	bool ok;
	QString cur = afqt::stoq( af::strJoin(((ItemJobBlock*)( getCurrentItem()))->getFilesOriginal(), ";"));
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

	blockAction(params);
}
void ListTasks::actBlockService()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->service;
	QString str = QInputDialog::getText(this, "Change Service", "Enter Type", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	blockAction(QString("\"params\":{\"service\":\"%1\"}").arg(str.replace("\"","\\\"")));
}
void ListTasks::actBlockParser()
{
	bool ok;
	QString cur = ((ItemJobBlock*)( getCurrentItem()))->parser;
	QString str = QInputDialog::getText(this, "Change Parser", "Enter Type", QLineEdit::Normal, cur, &ok);
	if( !ok) return;
	blockAction(QString("\"params\":{\"parser\":\"%1\"}").arg(str.replace("\"","\\\"")));
}

void ListTasks::actBrowseFolder()
{
	Item* item = getCurrentItem();
	if( item == NULL )
		return;

	QString image;
	QString wdir;

	switch (item->getType())
	{
	case Item::TBlock:
	{
		ItemJobBlock * itemBlock = static_cast<ItemJobBlock*>(item);
		image = afqt::stoq(itemBlock->getFiles()[0]);
		wdir = afqt::stoq(itemBlock->getWDir());
		break;
	}
	case Item::TTask:
	{
		ItemJobTask* itemTask = static_cast<ItemJobTask*>(item);
		image = afqt::stoq(itemTask->getFiles()[0]);
		wdir = afqt::stoq(itemTask->getWDir());
		break;
	}
	default:
		return;
	}

	Watch::browseImages( image, wdir);
}

void ListTasks::actTaskPreview(int i_num_cmd, int i_num_img)
{
	Item* item = getCurrentItem();
	if (item == NULL)
	{
		displayError("No items selected.");
		return;
	}
	if (item->getType() != Item::TTask)
	{
		displayWarning("This action for task only.");
		return;
	}

	ItemJobTask* taskitem = (ItemJobTask*)item;
	std::vector<std::string> images = taskitem->getFiles();
	if (i_num_img >= images.size())
	{
		displayError("No such image number.");
		return;
	}

	QString arg = afqt::stoq(images[i_num_img]);
	if (arg.isEmpty())
		return;

	if (i_num_cmd >= af::Environment::getPreviewCmds().size())
	{
		displayError("No such command number.");
		return;
	}

	QString cmd(afqt::stoq(af::Environment::getPreviewCmds()[i_num_cmd]));
	cmd = cmd.replace(AFWATCH::CMDS_ARGUMENT, arg);
	QStringList cmdSplit = cmd.split("|");

	Watch::startProcess(cmdSplit.last(), afqt::stoq(taskitem->getWDir()));
}

void ListTasks::actBlockPreview( int num_cmd, int num_img)
{
    Item* item = getCurrentItem();
    if (item == NULL)
    {
        displayError("No items selected.");
        return;
    }
    if (item->getType() != Item::TBlock)
    {
        displayWarning("This action for block only.");
        return;
    }

    ItemJobBlock* blockitem = (ItemJobBlock*)item;
    std::vector<std::string> images = blockitem->getFiles();
    if (num_img >= images.size())
    {
        displayError("No such image number.");
        return;
    }

    QString arg = afqt::stoq(images[num_img]);
    if (arg.isEmpty())
		return;

    if (num_cmd >= af::Environment::getPreviewCmds().size())
    {
        displayError("No such command number.");
        return;
    }

    QString cmd(afqt::stoq(af::Environment::getPreviewCmds()[num_cmd]));
    cmd = cmd.replace(AFWATCH::CMDS_ARGUMENT, arg);
    QStringList cmdSplit = cmd.split("|");

    Watch::startProcess(cmdSplit.last(), afqt::stoq(blockitem->getWDir()));
}

// Block number will be ignored here. Selected blocks will be processed.
void ListTasks::slot_BlockAction(int i_bnum, QString i_json) {blockAction(i_json);}
void ListTasks::blockAction(const QString & i_json)
{
	std::ostringstream str;
	af::jsonActionStart( str, "jobs", "", std::vector<int>( 1, m_job_id));

	// Collect selected blocks ids:
	str << ",\n\"block_ids\":[";
	const QList<Item*> items( getSelectedItems());
	int counter = 0;
	for (int i = 0; i < items.count(); i++)
		if (items[i]->getType() == Item::TBlock)
		{
			if( counter ) str << ',';
			str << ((ItemJobBlock*)items[i])->getNumBlock();
			counter++;
		}

	str << "],\n" << afqt::qtos(i_json);

	af::jsonActionFinish( str);

	Watch::sendMsg( af::jsonMsg( str));
}

void ListTasks::sortBlock(int i_block_num)
{
	if (i_block_num >= m_blocks.size())
	{
		AF_ERR << "ListTasks::sortBlock: i_block_num >= m_blocks.size(): "
			<< i_block_num << ">=" << m_blocks.size();
		return;
	}

	int sort_type = m_blocks[i_block_num]->getSortType();
	bool sort_ascending = m_blocks[i_block_num]->isSortAsceding();

	ItemJobTask ** array = new ItemJobTask*[m_tasks[i_block_num].size()];
	for (int i = 0; i < m_tasks[i_block_num].size(); i++)
		array[i] = m_tasks[i_block_num][i];

	if (sort_type)
		for (int i = 0; i < m_tasks[i_block_num].size(); i++)
			for (int j = m_tasks[i_block_num].size() - 1; j > i; j--)
			{
				ItemJobTask * item_a = array[j-1];
				ItemJobTask * item_b = array[j];
				if (item_a->compare(sort_type, *item_b, sort_ascending))
				{
					array[j-1] = item_b;
					array[j]   = item_a;
				}
			}

	int start = i_block_num+1;
	for (int b = 0; b < i_block_num; b++)
		start += m_tasks[b].size();

	storeSelection();
	m_model->setItems(start, (Item**)array, m_tasks[i_block_num].size());
	reStoreSelection();

	delete [] array;
}

bool ListTasks::v_filesReceived( const af::MCTaskUp & i_taskup )
{
	if( i_taskup.getNumJob() != m_job_id )
		return false; // This is some for other job

	if(( i_taskup.getNumBlock() < 0 ) || ( i_taskup.getNumTask() < 0 ))
		return false; // This files are for an entire job

	if (i_taskup.getNumBlock() >= m_blocks.size())
	{
		AF_ERR << "ListTasks::taskFilesReceived: i_taskup.getNumBlock() >= m_blocks.size():"
			<< i_taskup.getNumBlock() << ">=" << m_blocks.size();
		return true;
	}

	if (i_taskup.getNumTask() >= m_tasks[i_taskup.getNumBlock()].size())
	{
		AF_ERR << "ListTasks::taskFilesReceived: i_taskup.getNumTask() >= m_tasks[i_taskup.getNumBlock()].size():"
			<< i_taskup.getNumTask() << ">=" << m_tasks[i_taskup.getNumBlock()].size();
		return true;
	}

	m_tasks[i_taskup.getNumBlock()][i_taskup.getNumTask()]->taskFilesReceived( i_taskup);

	return true;
}
