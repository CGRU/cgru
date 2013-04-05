#include "blockinfo.h"

#include <limits.h>

#include "../libafanasy/msg.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mctaskspos.h"

#include "../libafqt/qenvironment.h"

#include "actionid.h"
#include "item.h"
#include "listitems.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int BlockInfo::Height        = 40;
const int BlockInfo::HeightCompact = 12;

BlockInfo::BlockInfo( Item * qItem, int BlockNumber, int JobId):
	str_runtime("Run Time"),
	str_progress("Progress numbers"),
	str_percent("Percentage"),
	str_compact("(compact display)"),
	str_properties("Block Properties"),
	str_avoiderrors("ea"),

	tasksnum(1),
	tasksdone(0),
	taskserror(0),
	percentage(0),
	taskssumruntime( 0),

	runningtasksnumber(0),
	errors_retries(-1),
	errors_avoidhost(-1),
	errors_tasksamehost(-1),
	tasksmaxruntime( 0),

	item( qItem),
	blocknum( BlockNumber),
	jobid(JobId),

	injobslist( false)
{
	if(( blocknum == -1 ) && ( jobid == -1 )) injobslist = true;
}

BlockInfo::~BlockInfo()
{
}

bool BlockInfo::update( const af::BlockData* block, int type)
{
	switch( type)
	{
	case af::Msg::TJob:
	case af::Msg::TJobsList:
	case af::Msg::TBlocks:
	case af::Msg::TBlocksProperties:
		multihost            = block->isMultiHost();
		multihost_samemaster = block->canMasterRunOnSlaveHost();
		varcapacity          = block->canVarCapacity();
		numeric              = block->isNumeric();
		nonsequential      = block->isNonSequential();

		frame_first          = block->getFrameFirst();
		frame_last           = block->getFrameLast();
		frame_pertask        = block->getFramePerTask();
		frame_inc            = block->getFrameInc();

		tasksnum             = block->getTasksNum();
		tasksmaxruntime      = block->getTasksMaxRunTime();
		errors_retries       = block->getErrorsRetries();
		errors_avoidhost     = block->getErrorsAvoidHost();
		errors_tasksamehost  = block->getErrorsTaskSameHost();
		errors_forgivetime   = block->getErrorsForgiveTime();
		maxrunningtasks      = block->getMaxRunningTasks();
		maxruntasksperhost   = block->getMaxRunTasksPerHost();
		need_memory          = block->getNeedMemory();
		need_power           = block->getNeedPower();
		need_hdd             = block->getNeedHDD();
		need_properties      = afqt::stoq( block->getNeedProperties());
		hostsmask            = afqt::stoq(block->getHostsMask());
		hostsmask_exclude    = afqt::stoq(block->getHostsMaskExclude());
		need_properties      = afqt::stoq(block->getNeedProperties());
		dependmask           = afqt::stoq(block->getDependMask());
		tasksdependmask      = afqt::stoq(block->getTasksDependMask());
		capacity             = block->getCapacity();
		filesize_min         = block->getFileSizeMin();
		filesize_max         = block->getFileSizeMax();
		capcoeff_min         = block->getCapCoeffMin();
		capcoeff_max         = block->getCapCoeffMax();
		multihost_min        = block->getMultiHostMin();
		multihost_max        = block->getMultiHostMax();
		multihost_waitmax    = block->getMultiHostWaitMax();
		multihost_waitsrv    = block->getMultiHostWaitSrv();
		service              = afqt::stoq( block->getService());


		depends.clear();
		if( block->isDependSubTask()) depends += QString(" [SUB]");
		if( false == dependmask.isEmpty()) depends += QString(" D(%1)").arg( dependmask);
		if( false == tasksdependmask.isEmpty()) depends += QString(" T[%1]").arg( tasksdependmask);

		icon_large = Watch::getServiceIconLarge( service);
		icon_small = Watch::getServiceIconSmall( service);

	case af::Msg::TBlocksProgress:

		state                = block->getState();
		avoidhostsnum        = block->getProgressAvoidHostsNum();
		errorhostsnum        = block->getProgressErrorHostsNum();
		runningtasksnumber   = block->getRunningTasksNumber();
		taskssumruntime      = block->getProgressTasksSumRunTime();
		tasksready           = block->getProgressTasksReady();
		tasksdone            = block->getProgressTasksDone();
		taskserror           = block->getProgressTasksError();
		percentage           = block->getProgressPercentage();
		memcpy( progress,      block->getProgressBar(), AFJOB::ASCII_PROGRESS_LENGTH);

		break;

	default:
		AFERRAR("BlockInfo::update: Invalid message type = %s", af::Msg::TNAMES[type])
		return false;
	}

	refresh();

#ifdef AFOUTPUT
if( type == af::Msg::TBlocksProgress)
{
	printf("update:\n");
	stdOutFlags( (char*)progress_done, AFJOB::PROGRESS_BYTES);
	printf("\n");
	stdOutFlags( (char*)progress_running, AFJOB::PROGRESS_BYTES);
	printf("\n");
	printf("\n");
}
#endif

	if( runningtasksnumber || taskserror || ((tasksdone != 0) && (tasksdone != tasksnum))) return true;

	return false;
}

void BlockInfo::refresh()
{
	if( tasksdone) str_runtime = QString("RT: S%1/A%2")
		.arg( af::time2strHMS( taskssumruntime, true).c_str())
		.arg( af::time2strHMS( taskssumruntime/tasksdone, true).c_str());
	else str_runtime = "Run Time";

	str_properties.clear();

	if(( errors_avoidhost >= 0 ) || ( errors_tasksamehost >= 0 ) || ( errors_retries >= 0 ))
		str_properties += QString("E:%1b|%2t|%3r").arg( errors_avoidhost).arg( errors_tasksamehost).arg( errors_retries);
	if( errors_forgivetime >= 0 ) str_properties += QString(" F%1").arg( af::time2strHMS( errors_forgivetime, true).c_str());

	if( tasksmaxruntime) str_properties += QString(" Max%1").arg( af::time2strHMS( tasksmaxruntime, true).c_str());

	if( maxrunningtasks    != -1 ) str_properties += QString(" m%1").arg( maxrunningtasks);
	if( maxruntasksperhost != -1 ) str_properties += QString(" mph%1").arg( maxruntasksperhost);
	if( false == hostsmask.isEmpty()          ) str_properties += QString(" H(%1)").arg( hostsmask         );
	if( false == hostsmask_exclude.isEmpty()  ) str_properties += QString(" E(%1)").arg( hostsmask_exclude );
	if( false == need_properties.isEmpty()    ) str_properties += QString(" P(%1)").arg( need_properties   );
	if( need_memory > 0 ) str_properties += QString(" M>%1").arg( need_memory);
	if( need_hdd    > 0 ) str_properties += QString(" H>%1").arg( need_hdd);
	if( need_power  > 0 ) str_properties += QString(" P>%1").arg( need_power);
	if( multihost )
	{
		str_properties += QString(" MH(%1,%2)").arg( multihost_min).arg( multihost_max);
		if( multihost_samemaster) str_properties += 'S';
		if( multihost_waitmax) str_properties += QString(":%1wm").arg(multihost_waitmax);
		if( multihost_waitsrv) str_properties += QString(":%1ws").arg(multihost_waitsrv);
	}
	if((filesize_min != -1) || (filesize_max != -1))
		str_properties += QString(" F(%1,%2)").arg( filesize_min).arg( filesize_max);

	str_properties += " [";
	if( varcapacity   ) str_properties += QString("(%1-%2)*").arg( capcoeff_min).arg( capcoeff_max);
	str_properties += QString("%1]").arg( capacity);

	if( errorhostsnum ) str_avoiderrors  = QString( "e%1").arg( errorhostsnum);
	if( avoidhostsnum ) str_avoiderrors += QString(" %1A").arg( avoidhostsnum);

	QString tasksinfo = QString("t%1").arg( tasksnum);
	if( numeric )
	{
		tasksinfo += QString("(%1-%2").arg( frame_first).arg( frame_last);
		if(( frame_pertask > 1 ) || ( frame_inc > 1 ))
		{
			tasksinfo += QString(":%1").arg( frame_pertask);
			if( frame_inc > 1 )
				tasksinfo += QString("/%1").arg( frame_inc);
		}
		tasksinfo += ")";
	}
	else if( frame_pertask > 1)
	{
		tasksinfo += QString("*%1").arg( frame_pertask);
	}
	else if( frame_pertask < 0)
	{
		tasksinfo += QString("/%1").arg( -frame_pertask);
	}

	if( nonsequential )
		tasksinfo += "*";

	str_compact = QString("%1: ").arg( tasksinfo);
	if( tasksdone) str_compact += QString("%1: ").arg( str_runtime);
	str_compact += name;

	str_percent = QString::number( percentage) + "%";
	if( false == name.isEmpty()) str_percent += ' ' + name;

	str_progress = QString("%1: r%3 d%5 e%6")
		.arg( tasksinfo)
		.arg( runningtasksnumber)
		.arg( tasksdone)
		.arg( taskserror);
	if( jobid == AFJOB::SYSJOB_ID ) str_progress += QString(" ready:%1").arg( tasksready);

	if( false == depends.isEmpty())
	{
		str_percent += depends;
		str_compact += depends;
	}
}

void BlockInfo::stdOutFlags( char* data, int size) const
{
	for( int i = 0; i < size; i++)
	{
		uint8_t flags = 1;
		for( int b = 0; b < 8; b++)
		{
			if( data[i] & flags) printf("1");
			else                 printf("0");
			flags <<= 1;
		}
	}
}

void BlockInfo::paint( QPainter * painter, const QStyleOptionViewItem &option,
	int x, int y, int w,
	bool compact_display,
	const QColor * backcolor) const
{
	if( item == NULL)
	{
		AFERROR("BlockInfo::drawBars: Item is not set.")
		return;
	}

	static const int y_properties = -1;
	static const int y_progress   = 27;
	static const int y_bars       = 12;

	painter->setFont( afqt::QEnvironment::f_info);
	painter->setPen( Item::clrTextInfo( runningtasksnumber, option.state & QStyle::State_Selected, item->isLocked()));
	QRect rect_properties;
	painter->drawText( x, y+y_properties, w-5, 15, Qt::AlignRight | Qt::AlignTop, str_properties, &rect_properties );

	int xoffset = 1;

	if( compact_display)
	{
		if( icon_small )
		{
			painter->drawPixmap( x, y-2, *icon_small);
			xoffset += 15;
		}
		painter->drawText(  x+xoffset, y-2, w-rect_properties.width()-20, 15, Qt::AlignTop | Qt::AlignLeft, str_compact );
		return;
	}

	xoffset = 5;

	if( icon_large )
	{
		painter->drawPixmap( x, y-2, *icon_large);
		xoffset += 40;
	}

	painter->drawText( x+xoffset, y+y_properties, w-rect_properties.width()-20-xoffset, 15, Qt::AlignLeft  | Qt::AlignTop, str_runtime  );
	QRect rect_progress;
	painter->drawText( x+xoffset, y+y_progress,   w-5-xoffset, 15, Qt::AlignRight | Qt::AlignTop, str_progress, &rect_progress);
	painter->drawText( x+xoffset, y+y_progress,   w-rect_progress.width()-10-xoffset, 15, Qt::AlignLeft  | Qt::AlignTop, str_percent  );

	int progress_w_offset = 0;

	if( errorhostsnum )
	{
		progress_w_offset = 20;
		if( avoidhostsnum )
		{
			painter->setPen( afqt::QEnvironment::clr_error.c);
			progress_w_offset += 30;
		}
		painter->drawText( x, y+y_bars+2, w, 15, Qt::AlignRight | Qt::AlignTop, str_avoiderrors );
	}
	Item::drawPercent
	(
		painter, x+xoffset, y+y_bars, w-progress_w_offset-xoffset, 4,
		jobid == AFJOB::SYSJOB_ID ? runningtasksnumber + tasksready + taskserror : tasksnum,
		jobid == AFJOB::SYSJOB_ID ? 0 : tasksdone, taskserror, runningtasksnumber,
		false
	);
	Item::drawPercent
	(
		painter, x+xoffset, y+y_bars+4, w-progress_w_offset-xoffset, 4,
		100,
		percentage, 0, 0,
		false
	);
	drawProgress
	(
		painter, x+1+xoffset, y+y_bars+8, w-progress_w_offset-xoffset, 6,
		backcolor
	);

	painter->setPen( afqt::QEnvironment::clr_outline.c );
	painter->setBrush( Qt::NoBrush);
	painter->drawRect( x-1+xoffset, y+y_bars-1, w-progress_w_offset+1-xoffset, 15);
}

void BlockInfo::drawProgress(
			QPainter * painter,
			int posx, int posy, int width, int height,
			const QColor * backcolor
		) const
{
	painter->setPen( Qt::NoPen );

	int w = width / AFJOB::ASCII_PROGRESS_LENGTH;
	if( w < 1) w = 1;

	for( int last_x = 0, i = 0; i < AFJOB::ASCII_PROGRESS_LENGTH; i++)
	{
		int x = posx + i * width / AFJOB::ASCII_PROGRESS_LENGTH;
		int offset = 0; // offset to prevent extra pixel ( geomerty values are rounded to integer )
		if( last_x < x ) offset = 1;
		x = x - offset;

		switch( progress[i] )
		{
		case ' ': // 0
			painter->setBrush( QBrush( afqt::QEnvironment::qclr_black, Qt::SolidPattern ));
			painter->drawRect( x, posy, w+offset, height);
			break;
		case 'r': // STATE_READY_MASK
			break;
		case 'D': // STATE_DONE_MASK
			painter->setBrush( QBrush( afqt::QEnvironment::clr_done.c, Qt::SolidPattern ));
			painter->drawRect( x, posy, w+offset, height);
			break;
		case 'R': // STATE_RUNNING_MASK
			painter->setBrush( QBrush( afqt::QEnvironment::clr_running.c, Qt::SolidPattern ));
			painter->drawRect( x, posy, w+offset, height);
			break;
		case 'E': // STATE_ERROR_MASK
			painter->setBrush( QBrush( afqt::QEnvironment::clr_error.c, Qt::SolidPattern ));
			painter->drawRect( x, posy, w+offset, height);
			break;
		case 'W': // STATE_WAITDEP_MASK
			painter->setBrush( QBrush( afqt::QEnvironment::clr_itemjobwdep.c, Qt::SolidPattern ));
			painter->drawRect( x, posy, w+offset, height);
			break;
		case 'S': // STATE_SKIPPED_MASK | STATE_DONE_MASK
			painter->setBrush( QBrush( afqt::QEnvironment::clr_taskskipped.c, Qt::SolidPattern ));
			painter->drawRect( x, posy, w+offset, height);
			break;
		case 'N': // STATE_RUNNING_MASK_MASK | STATE_WARNING_MASK
			painter->setBrush( QBrush( afqt::QEnvironment::clr_taskwarningrun.c, Qt::SolidPattern ));
			painter->drawRect( x, posy, w+offset, height);
			break;
		case 'G': // STATE_DONE_MASK | STATE_WARNING_MASK
			painter->setBrush( QBrush( afqt::QEnvironment::clr_taskwarningdone.c, Qt::SolidPattern ));
			painter->drawRect( x, posy, w+offset, height);
			break;
		}
		last_x = x + w + offset;
//printf("%c", progress[i]);
	}
//printf("\n");
}

void BlockInfo::generateMenu( int id_block, QMenu * menu, QWidget * qwidget, QMenu * submenu)
{
	ActionIdString *action;

	// There is no need to reset error hosts for all job blocks here.
	// Job item has a special menuitem for it.
	if( id_block != -1 )
	{
		action = new ActionIdString( id_block, "reset_error_hosts", "Reset Error Hosts", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		menu->addSeparator();
	}

	// There is no way to skip and restart all job bloks.
	if( id_block != -1 )
	{

		action = new ActionIdString( id_block, "skip", "Skip Block", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		action = new ActionIdString( id_block, "restart", "Restart Block", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		menu->addSeparator();
	}

	if( submenu != NULL )
		 menu = submenu;

	action = new ActionIdString( id_block, "non_sequential", "Set Non-Sequential", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "non_sequential_unset", "Unset Non-Sequential", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	menu->addSeparator();

	action = new ActionIdString( id_block, "errors_avoid_host", "Set Errors Avoid Host", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "errors_task_same_host", "Set Task Errors Same Host", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "errors_retries", "Set Task Error Retries", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "tasks_max_run_time", "Set Tasks MaxRunTime", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "errors_forgive_time", "Set Errors Forgive time", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	menu->addSeparator();

	action = new ActionIdString( id_block, "depend_mask", "Set Depend Mask", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "tasks_depend_mask", "Set Tasks Depend Mask", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "hosts_mask", "Set Hosts Mask", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "hosts_mask_exclude", "Set Exclude Hosts Mask", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "max_running_tasks", "Set Max Running Tasks", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "max_running_tasks_per_host", "Set Max Tasks Per Host", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	menu->addSeparator();

	action = new ActionIdString( id_block, "capacity", "Set Capacity", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	if( varcapacity)
	{
		action = new ActionIdString( id_block, "capacity_coeff_min", "Set Capacity Min Coeff", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		action = new ActionIdString( id_block, "capacity_coeff_max", "Set Capacity Max Coeff", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);
	}

	menu->addSeparator();

	if( multihost)
	{
		action = new ActionIdString( id_block, "multihost_min", "Set Hosts Minimum", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		action = new ActionIdString( id_block, "multihost_max", "Set Hosts Maximum", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		action = new ActionIdString( id_block, "multihost_max_wait", "Set Maximum Hosts Wait Time", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		action = new ActionIdString( id_block, "multihost_service_wait", "Set Service Wait Time", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		menu->addSeparator();
	}

	action = new ActionIdString( id_block, "need_memory", "Set Needed Memory", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "need_hdd", "Set Needed HDD", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "need_power", "Set Needed Power", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);

	action = new ActionIdString( id_block, "need_properties", "Set Needed Properties", qwidget);
	QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
	menu->addAction( action);
}

bool BlockInfo::blockAction( std::ostringstream & i_str, int id_block, const QString & i_action, ListItems * listitems) const
{
	// Operations:

	if( i_action == "skip" ||
		i_action == "restart" ||
		i_action == "reset_error_hosts")
	{
		i_str << ",\n\"operation\":{\"type\":\"" << i_action.toUtf8().data() << "\"}";
		return true;
	}

	// Parameter change:

	i_str << ",\n\"params\":{\n";

	if( i_action == "non_sequential_unset" )
	{
		i_str << "\"non_sequential\":false}";
		return true;
	}

	i_str << '"' << i_action.toUtf8().data() << "\":";

	if( i_action == "non_sequential" )
	{
		i_str << "true}";
		return true;
	}

	// For other actions we should query some number or string:

	bool ok = true;
	int cur_number = 0;
	QString cur_string;
	int set_number = 0;
	QString set_string;

	if( i_action == "capacity" )
	{
		if( id_block == blocknum ) cur_number = capacity;
		set_number = QInputDialog::getInteger( listitems, "Change Capacity", "Enter Capacity", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "errors_retries" )
	{
		if( id_block == blocknum ) cur_number = errors_retries;
		set_number = QInputDialog::getInteger( listitems, "Set Retries Error", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "errors_avoid_host" )
	{
		if( id_block == blocknum ) cur_number = errors_avoidhost;
		set_number = QInputDialog::getInteger( listitems, "Set Errors Avoid Host", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "errors_task_same_host" )
	{
		if( id_block == blocknum ) cur_number = errors_tasksamehost;
		set_number = QInputDialog::getInteger( listitems, "Set Task Errors Same Host", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "errors_forgive_time" )
	{
		double cur = 0;
		if( id_block == blocknum ) cur = double(errors_forgivetime) / (60*60);
		double hours = QInputDialog::getDouble( listitems, "Set Errors forgive time", "Enter number of hours (0=infinite)", cur, -1, 365*24, 3, &ok);
		set_number = int( hours * 60*60 );
	}
	else if( i_action == "tasks_max_run_time" )
	{
		double cur = 0;
		if( id_block == blocknum ) cur = double(tasksmaxruntime) / (60*60);
		double hours = QInputDialog::getDouble( listitems, "Tasks Maximum Run Time", "Enter number of hours (0=infinite)", cur, 0, 365*24, 4, &ok);
		set_number = int( hours * 60*60 );
	}
	else if( i_action == "max_running_tasks" )
	{
		if( id_block == blocknum ) cur_number = maxrunningtasks;
		set_number = QInputDialog::getInteger( listitems, "Change Maximum Running Tasks", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "max_running_tasks_per_host" )
	{
		if( id_block == blocknum ) cur_number = maxruntasksperhost;
		set_number = QInputDialog::getInteger( listitems, "Change Maximum Running Tasks Per Host", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "hosts_mask" )
	{
		if( id_block == blocknum ) cur_string = hostsmask;
		set_string = QInputDialog::getText( listitems, "Change Hosts Mask", "Enter Mask", QLineEdit::Normal, cur_string, &ok);
	}
	else if( i_action == "hosts_mask_exclude" )
	{
		if( id_block == blocknum ) cur_string = hostsmask_exclude;
		set_string = QInputDialog::getText( listitems, "Change Exclude Hosts Mask", "Enter Mask", QLineEdit::Normal, cur_string, &ok);
	}
	else if( i_action == "depend_mask" )
	{
		if( id_block == blocknum ) cur_string = dependmask;
		set_string = QInputDialog::getText( listitems, "Change Depend Mask", "Enter Mask", QLineEdit::Normal, cur_string, &ok);
	}
	else if( i_action == "tasks_depend_mask" )
	{
		if( id_block == blocknum ) cur_string = tasksdependmask;
		set_string = QInputDialog::getText( listitems, "Change Tasks Depend Mask", "Enter Mask", QLineEdit::Normal, cur_string, &ok);
	}
	else if( i_action == "need_properties" )
	{
		if( id_block == blocknum ) cur_string = need_properties;
		set_string = QInputDialog::getText( listitems, "Change Needed Properties", "Enter Mask", QLineEdit::Normal, cur_string, &ok);
	}
	else if( i_action == "need_memory" )
	{
		if( id_block == blocknum ) cur_number = need_memory;
		set_number = QInputDialog::getInteger( listitems, "Change Needed Memory", "Enter MegaBytes", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "need_hdd" )
	{
		if( id_block == blocknum ) cur_number = need_hdd;
		set_number = QInputDialog::getInteger( listitems, "Change Maximum Hosts", "Enter GigaBytes", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "need_power" )
	{
		if( id_block == blocknum ) cur_number = need_power;
		set_number = QInputDialog::getInteger( listitems, "Change Maximum Hosts", "Enter Power", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "capacity_coeff_min" )
	{
		if( id_block == blocknum ) cur_number = capcoeff_min;
		set_number = QInputDialog::getInteger( listitems, "Change Capacity min coeff", "Enter Coefficient", cur_number, 0, INT_MAX, 1, &ok);
	}
	else if( i_action == "capacity_coeff_max" )
	{
		if( id_block == blocknum ) cur_number = capcoeff_max;
		set_number = QInputDialog::getInteger( listitems, "Change Capacity max coeff", "Enter Coefficient", cur_number, 0, INT_MAX, 1, &ok);
	}
	else if( i_action == "multihost_min" )
	{
		if( id_block == blocknum ) cur_number = multihost_min;
		set_number = QInputDialog::getInteger( listitems, "Change Hosts Minimun", "Enter Quantity", cur_number, 1, AFJOB::TASK_MULTIHOSTMAXHOSTS, 1, &ok);
	}
	else if( i_action == "multihost_max" )
	{
		if( id_block == blocknum ) cur_number = multihost_max;
		set_number = QInputDialog::getInteger( listitems, "Change Hosts Maximum", "Enter Quantity", cur_number, 1, AFJOB::TASK_MULTIHOSTMAXHOSTS, 1, &ok);
	}
	else if( i_action == "multihost_max_wait" )
	{
		if( id_block == blocknum ) cur_number = multihost_waitmax;
		set_number = QInputDialog::getInteger( listitems, "Set Hosts Maximum Wait Time", "Enter Seconds", cur_number, 0, INT_MAX, 1, &ok);
	}
	else if( i_action == "multihost_service_wait" )
	{
		if( id_block == blocknum ) cur_number = multihost_waitsrv;
		set_number = QInputDialog::getInteger( listitems, "Set Service Start Wait", "Enter Seconds", cur_number, 0, INT_MAX, 1, &ok);
	}
	else
	{
		AFERRAR("BlockInfo::blockAction: Unknown action = '%s'", i_action.toUtf8().data())
		return false;
	}

//printf("BlockInfo::blockAction: %s\n", af::Msg::TNAMES[id_action]);

	if( ok == false) return false;

	if( set_string.isEmpty() == false)
	{
		QRegExp rx( set_string, Qt::CaseInsensitive);
		if( rx.isValid() == false)
		{
			listitems->displayError( rx.errorString());
			return false;
		}
		i_str << '"' << set_string.toUtf8().data() << '"';
	}
	else
		i_str << set_number;

	i_str << '}';

//printf("%d %s\n", id_block, i_action.toUtf8().data());
	return true;
}
