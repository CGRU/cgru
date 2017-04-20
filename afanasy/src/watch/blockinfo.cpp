#include "blockinfo.h"

#include <limits.h>

#include "../libafanasy/msg.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "../libafqt/qenvironment.h"

#include "actionid.h"
#include "item.h"
#include "listitems.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QInputDialog>
#include <QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int BlockInfo::Height        = 44;
const int BlockInfo::HeightCompact = 14;

BlockInfo::BlockInfo( Item * qItem, int BlockNumber, int JobId):
	tasksnum(1),

	p_percentage(0),
	p_tasksready(0),
	p_tasksrunning(0),
	p_tasksdone(0),
	p_taskserror(0),
	p_tasksskipped(0),
	p_taskswarning(0),
	p_taskswaitrec(0),
	p_errorhosts(0),
	p_avoidhosts(0),
	p_taskssumruntime(0),

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
		multihost                    = block->isMultiHost();
		multihost_samemaster         = block->canMasterRunOnSlaveHost();
		varcapacity                  = block->canVarCapacity();
		numeric                      = block->isNumeric();

		frame_first                  = block->getFrameFirst();
		frame_last                   = block->getFrameLast();
		frame_pertask                = block->getFramePerTask();
		frame_inc                    = block->getFrameInc();
		sequential                   = block->getSequential();

		tasksnum                     = block->getTasksNum();
		tasksmaxruntime              = block->getTasksMaxRunTime();
		errors_retries               = block->getErrorsRetries();
		errors_avoidhost             = block->getErrorsAvoidHost();
		errors_tasksamehost          = block->getErrorsTaskSameHost();
		errors_forgivetime           = block->getErrorsForgiveTime();
		task_progress_change_timeout = block->getTaskProgressChangeTimeout();
		maxrunningtasks              = block->getMaxRunningTasks();
		maxruntasksperhost           = block->getMaxRunTasksPerHost();
		need_memory                  = block->getNeedMemory();
		need_power                   = block->getNeedPower();
		need_hdd                     = block->getNeedHDD();
		need_properties              = afqt::stoq( block->getNeedProperties());
		hostsmask                    = afqt::stoq(block->getHostsMask());
		hostsmask_exclude            = afqt::stoq(block->getHostsMaskExclude());
		need_properties              = afqt::stoq(block->getNeedProperties());
		dependmask                   = afqt::stoq(block->getDependMask());
		tasksdependmask              = afqt::stoq(block->getTasksDependMask());
		capacity                     = block->getCapacity();
		filesize_min                 = block->getFileSizeMin();
		filesize_max                 = block->getFileSizeMax();
		capcoeff_min                 = block->getCapCoeffMin();
		capcoeff_max                 = block->getCapCoeffMax();
		multihost_min                = block->getMultiHostMin();
		multihost_max                = block->getMultiHostMax();
		multihost_waitmax            = block->getMultiHostWaitMax();
		multihost_waitsrv            = block->getMultiHostWaitSrv();
		service                      = afqt::stoq( block->getService());


		depends.clear();
		if( Watch::isPadawan())
		{
			if( block->isDependSubTask()) depends += QString(" [Sub-Task Depends]");
			if( false == dependmask.isEmpty()) depends += QString(" Depends(%1)").arg( dependmask);
			if( false == tasksdependmask.isEmpty()) depends += QString(" TasksDepends[%1]").arg( tasksdependmask);
		}
		else if( Watch::isJedi())
		{
			if( block->isDependSubTask()) depends += QString(" [SUB]");
			if( false == dependmask.isEmpty()) depends += QString(" Dep(%1)").arg( dependmask);
			if( false == tasksdependmask.isEmpty()) depends += QString(" TDep[%1]").arg( tasksdependmask);
		}
		else
		{
			if( block->isDependSubTask()) depends += QString(" [sub]");
			if( false == dependmask.isEmpty()) depends += QString(" d(%1)").arg( dependmask);
			if( false == tasksdependmask.isEmpty()) depends += QString(" t[%1]").arg( tasksdependmask);
		}

		icon_large = Watch::getServiceIconLarge( service);
		icon_small = Watch::getServiceIconSmall( service);

	case af::Msg::TBlocksProgress:

		state = block->getState();

		p_percentage      = block->getProgressPercentage();
		p_tasksready      = block->getProgressTasksReady();
		p_tasksrunning    = block->getRunningTasksNumber();
		p_tasksdone       = block->getProgressTasksDone();
		p_taskserror      = block->getProgressTasksError();
		p_tasksskipped    = block->getProgressTasksSkipped();
		p_taskswarning    = block->getProgressTasksWarning();
		p_taskswaitrec    = block->getProgressTasksWaitReconn();
		p_avoidhosts      = block->getProgressAvoidHostsNum();
		p_errorhosts      = block->getProgressErrorHostsNum();
		p_taskssumruntime = block->getProgressTasksSumRunTime();

		memcpy( progress, block->getProgressBar(), AFJOB::ASCII_PROGRESS_LENGTH);

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

	if( p_tasksrunning || p_taskserror || ((p_tasksdone != 0) && (p_tasksdone != tasksnum))) return true;

	return false;
}

void BlockInfo::refresh()
{
	// General information:
	if( Watch::isPadawan())
	{
		str_info = QString("Frames[%1]").arg( tasksnum);
		if( numeric )
		{
			str_info += QString("( %1 - %2 ").arg( frame_first).arg( frame_last);
			if(( frame_pertask > 1 ) || ( frame_inc > 1 ))
			{
				str_info += QString(" : PerTask(%1)").arg( frame_pertask);
				if( frame_inc > 1 )
					str_info += QString(" / Increment(%1)").arg( frame_inc);
			}
			if( sequential != 1 )
				str_info += QString(" % Sequential(%1)").arg( sequential);
			str_info += ")";
		}
		else if( frame_pertask > 1)
		{
			str_info += QString(" * PerTask:%1").arg( frame_pertask);
		}
		else if( frame_pertask < 0)
		{
			str_info += QString(" / PerTask:%1").arg( -frame_pertask);
		}
	}
	else if( Watch::isJedi())
	{
		str_info = QString("Tasks[%1]").arg( tasksnum);
		if( numeric )
		{
			str_info += QString("(%1-%2").arg( frame_first).arg( frame_last);
			if(( frame_pertask > 1 ) || ( frame_inc > 1 ))
			{
				str_info += QString(":%1").arg( frame_pertask);
				if( frame_inc > 1 )
					str_info += QString("/inc(%1)").arg( frame_inc);
			}
			if( sequential != 1 )
				str_info += "seq(%)" + QString::number( sequential);
			str_info += ")";
		}
		else if( frame_pertask > 1)
		{
			str_info += QString("*%1").arg( frame_pertask);
		}
		else if( frame_pertask < 0)
		{
			str_info += QString("/%1").arg( -frame_pertask);
		}
	}
	else
	{
		str_info = QString("t%1").arg( tasksnum);
		if( numeric )
		{
			str_info += QString("(%1-%2").arg( frame_first).arg( frame_last);
			if(( frame_pertask > 1 ) || ( frame_inc > 1 ))
			{
				str_info += QString(":%1").arg( frame_pertask);
				if( frame_inc > 1 )
					str_info += QString("/%1").arg( frame_inc);
			}
			if( sequential != 1 )
				str_info += "%" + QString::number( sequential);
			str_info += ")";
		}
		else if( frame_pertask > 1)
		{
			str_info += QString("*%1").arg( frame_pertask);
		}
		else if( frame_pertask < 0)
		{
			str_info += QString("/%1").arg( -frame_pertask);
		}
	}

	str_info += QString(": %1").arg( name);

	if( false == depends.isEmpty())
		str_info += depends;


	// Parameters:
	str_params.clear();
	if( Watch::isPadawan())
	{
		if( p_tasksdone) str_params += QString(" Render Timings: Sum:%1 / Average:%2")
			.arg( af::time2strHMS( p_taskssumruntime, true).c_str())
			.arg( af::time2strHMS( p_taskssumruntime/p_tasksdone, true).c_str());

		if(( errors_avoidhost >= 0 ) || ( errors_tasksamehost >= 0 ) || ( errors_retries >= 0 ))
			str_params += Item::generateErrorsSolvingInfo( errors_avoidhost, errors_tasksamehost, errors_retries);
		if( errors_forgivetime >= 0 ) str_params += QString(" ErrorsForgiveTime:%1").arg( af::time2strHMS( errors_forgivetime, true).c_str());

		if( tasksmaxruntime) str_params += QString(" MaxRunTime:%1").arg( af::time2strHMS( tasksmaxruntime, true).c_str());

		if( maxrunningtasks    != -1 ) str_params += QString(" MaxRunTasks:%1").arg( maxrunningtasks);
		if( maxruntasksperhost != -1 ) str_params += QString(" MaxPerHost:%1").arg( maxruntasksperhost);
		if( false == hostsmask.isEmpty()          ) str_params += QString(" HostsMask(%1)").arg( hostsmask         );
		if( false == hostsmask_exclude.isEmpty()  ) str_params += QString(" ExcludeHosts(%1)").arg( hostsmask_exclude );
		if( false == need_properties.isEmpty()    ) str_params += QString(" Properties(%1)").arg( need_properties   );
		if( need_memory > 0 ) str_params += QString(" Mem>%1").arg( need_memory);
		if( need_hdd    > 0 ) str_params += QString(" HDD>%1").arg( need_hdd);
		if( need_power  > 0 ) str_params += QString(" Power>%1").arg( need_power);
		if( multihost )
		{
			str_params += QString(" Multi-Host(%1,%2)").arg( multihost_min).arg( multihost_max);
			if( multihost_samemaster) str_params += "SameMaster";
			if( multihost_waitmax) str_params += QString(":WaitMax(%1)").arg(multihost_waitmax);
			if( multihost_waitsrv) str_params += QString(":WaitService(%1)").arg(multihost_waitsrv);
		}
		if((filesize_min != -1) || (filesize_max != -1))
			str_params += QString(" FileSize(%1,%2)").arg( filesize_min).arg( filesize_max);

		str_params += " Capacity:";
		if( varcapacity   ) str_params += QString("(%1-%2)*").arg( capcoeff_min).arg( capcoeff_max);
		str_params += QString("%1").arg( capacity);
		
		if( task_progress_change_timeout != -1) str_params += QString(" NoProgessFor:%1").arg(af::time2strHMS( task_progress_change_timeout, true).c_str());
	}
	else if( Watch::isJedi())
	{
		if( p_tasksdone) str_params += QString(" Timings: Sum:%1/Avg:%2")
			.arg( af::time2strHMS( p_taskssumruntime, true).c_str())
			.arg( af::time2strHMS( p_taskssumruntime/p_tasksdone, true).c_str());

		if(( errors_avoidhost >= 0 ) || ( errors_tasksamehost >= 0 ) || ( errors_retries >= 0 ))
			str_params += Item::generateErrorsSolvingInfo( errors_avoidhost, errors_tasksamehost, errors_retries);
		if( errors_forgivetime >= 0 ) str_params += QString(" Forgive:%1").arg( af::time2strHMS( errors_forgivetime, true).c_str());

		if( tasksmaxruntime) str_params += QString(" MaxTime:%1").arg( af::time2strHMS( tasksmaxruntime, true).c_str());

		if( maxrunningtasks    != -1 ) str_params += QString(" Max:%1").arg( maxrunningtasks);
		if( maxruntasksperhost != -1 ) str_params += QString(" PerHost:%1").arg( maxruntasksperhost);
		if( false == hostsmask.isEmpty()          ) str_params += QString(" Hosts(%1)").arg( hostsmask         );
		if( false == hostsmask_exclude.isEmpty()  ) str_params += QString(" Exclude(%1)").arg( hostsmask_exclude );
		if( false == need_properties.isEmpty()    ) str_params += QString(" Props(%1)").arg( need_properties   );
		if( need_memory > 0 ) str_params += QString(" Mem>%1").arg( need_memory);
		if( need_hdd    > 0 ) str_params += QString(" HDD>%1").arg( need_hdd);
		if( need_power  > 0 ) str_params += QString(" Pow>%1").arg( need_power);
		if( multihost )
		{
			str_params += QString(" MH(%1,%2)").arg( multihost_min).arg( multihost_max);
			if( multihost_samemaster) str_params += 'S';
			if( multihost_waitmax) str_params += QString(":%1WM").arg(multihost_waitmax);
			if( multihost_waitsrv) str_params += QString(":%1WS").arg(multihost_waitsrv);
		}
		if((filesize_min != -1) || (filesize_max != -1))
			str_params += QString(" FSize(%1,%2)").arg( filesize_min).arg( filesize_max);

		str_params += " Cap[";
		if( varcapacity   ) str_params += QString("(%1-%2)*").arg( capcoeff_min).arg( capcoeff_max);
		str_params += QString("%1]").arg( capacity);
		
		if( task_progress_change_timeout != -1) str_params += QString(" NPF:%1").arg(af::time2strHMS( task_progress_change_timeout, true).c_str());
	}
	else
	{
		if( p_tasksdone) str_params += QString(" rt:s%1/a%2")
			.arg( af::time2strHMS( p_taskssumruntime, true).c_str())
			.arg( af::time2strHMS( p_taskssumruntime/p_tasksdone, true).c_str());

		if(( errors_avoidhost >= 0 ) || ( errors_tasksamehost >= 0 ) || ( errors_retries >= 0 ))
			str_params += Item::generateErrorsSolvingInfo( errors_avoidhost, errors_tasksamehost, errors_retries);
		if( errors_forgivetime >= 0 ) str_params += QString(" f%1").arg( af::time2strHMS( errors_forgivetime, true).c_str());

		if( tasksmaxruntime) str_params += QString(" mrt%1").arg( af::time2strHMS( tasksmaxruntime, true).c_str());

		if( maxrunningtasks    != -1 ) str_params += QString(" m%1").arg( maxrunningtasks);
		if( maxruntasksperhost != -1 ) str_params += QString(" mph%1").arg( maxruntasksperhost);
		if( false == hostsmask.isEmpty()          ) str_params += QString(" h(%1)").arg( hostsmask         );
		if( false == hostsmask_exclude.isEmpty()  ) str_params += QString(" e(%1)").arg( hostsmask_exclude );
		if( false == need_properties.isEmpty()    ) str_params += QString(" p(%1)").arg( need_properties   );
		if( need_memory > 0 ) str_params += QString(" m>%1").arg( need_memory);
		if( need_hdd    > 0 ) str_params += QString(" h>%1").arg( need_hdd);
		if( need_power  > 0 ) str_params += QString(" p>%1").arg( need_power);
		if( multihost )
		{
			str_params += QString(" mh(%1,%2)").arg( multihost_min).arg( multihost_max);
			if( multihost_samemaster) str_params += 's';
			if( multihost_waitmax) str_params += QString(":%1wm").arg(multihost_waitmax);
			if( multihost_waitsrv) str_params += QString(":%1ws").arg(multihost_waitsrv);
		}
		if((filesize_min != -1) || (filesize_max != -1))
			str_params += QString(" fs(%1,%2)").arg( filesize_min).arg( filesize_max);

		str_params += " [";
		if( varcapacity   ) str_params += QString("(%1-%2)*").arg( capcoeff_min).arg( capcoeff_max);
		str_params += QString("%1]").arg( capacity);
		
		if( task_progress_change_timeout != -1) str_params += QString(" npf%1").arg(af::time2strHMS( task_progress_change_timeout, true).c_str());
	}


	// Progress:
	str_progress = QString::number( p_percentage) + "%";
	if( Watch::isPadawan())
	{
		if( p_tasksrunning ) str_progress += QString(" Running:%1" ).arg( p_tasksrunning);
		if( p_tasksdone    ) str_progress += QString(" Done:%1").arg( p_tasksdone);
		if( p_taskserror   ) str_progress += QString(" Errors:%1" ).arg( p_taskserror);
		if( p_tasksskipped ) str_progress += QString(" Skipped:%1" ).arg( p_tasksskipped);
		if( p_taskswarning ) str_progress += QString(" Warnings:%1" ).arg( p_taskswarning);
		if( p_taskswaitrec ) str_progress += QString(" WaitingReconnect:%1" ).arg( p_taskswaitrec);
	}
	else if( Watch::isJedi())
	{
		if( p_tasksrunning ) str_progress += QString(" Run:%1" ).arg( p_tasksrunning);
		if( p_tasksdone    ) str_progress += QString(" Done:%1").arg( p_tasksdone);
		if( p_taskserror   ) str_progress += QString(" Err:%1" ).arg( p_taskserror);
		if( p_tasksskipped ) str_progress += QString(" Skp:%1" ).arg( p_tasksskipped);
		if( p_taskswarning ) str_progress += QString(" Wrn:%1" ).arg( p_taskswarning);
		if( p_taskswaitrec ) str_progress += QString(" WRC:%1" ).arg( p_taskswaitrec);
	}
	else
	{
		if( p_tasksrunning ) str_progress += QString(" r%1" ).arg( p_tasksrunning);
		if( p_tasksdone    ) str_progress += QString(" d%1").arg( p_tasksdone);
		if( p_taskserror   ) str_progress += QString(" e%1" ).arg( p_taskserror);
		if( p_tasksskipped ) str_progress += QString(" s%1" ).arg( p_tasksskipped);
		if( p_taskswarning ) str_progress += QString(" w%1" ).arg( p_taskswarning);
		if( p_taskswaitrec ) str_progress += QString(" wrc%1" ).arg( p_taskswaitrec);
	}

	if( jobid == AFJOB::SYSJOB_ID ) str_progress += QString(" Ready:%1").arg( p_tasksready);


	// Error Hosts:
	if( Watch::isPadawan())
	{
		if( p_errorhosts ) str_avoiderrors  = QString("Error Hosts:%1").arg( p_errorhosts);
		if( p_avoidhosts ) str_avoiderrors += QString("/%1:Avoiding").arg( p_avoidhosts);
	}
	else if( Watch::isJedi())
	{
		if( p_errorhosts ) str_avoiderrors  = QString("ErrHosts:%1").arg( p_errorhosts);
		if( p_avoidhosts ) str_avoiderrors += QString("/%1:Avoid").arg( p_avoidhosts);
	}
	else
	{
		if( p_errorhosts ) str_avoiderrors  = QString("eh:%1").arg( p_errorhosts);
		if( p_avoidhosts ) str_avoiderrors += QString("/%1:a").arg( p_avoidhosts);
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

	static const int y_info     =  0;
	static const int y_bars     = 14;
	static const int y_progress = 30;


	// Paint an icon and calculate an offset for further info:
	int xoffset = 3;
	if( compact_display)
	{
		if( icon_small )
		{
			painter->drawPixmap( x, y, *icon_small);
			xoffset += 16;
		}
	}
	else
	{
		xoffset = 7;

		if( icon_large )
		{
			painter->drawPixmap( x, y, *icon_large);
			xoffset += 42;
		}
	}


	// Paint border:
	painter->setPen( afqt::QEnvironment::clr_outline.c );
	painter->setBrush( Qt::NoBrush);
	painter->drawRoundedRect( x+xoffset-3, y-1, w+4-xoffset, compact_display ? HeightCompact : Height, 3, 3);


	// Setup font size and color:
	painter->setFont( afqt::QEnvironment::f_info);
	QPen pen( Item::clrTextInfo( p_tasksrunning, option.state & QStyle::State_Selected, item->isLocked()));
	painter->setPen( pen);


	// Paint parameters:
	QRect rect_params;
	painter->drawText( x, y+y_info, w-5, 15, Qt::AlignRight | Qt::AlignTop, str_params, &rect_params );

	// Paint general information:
	painter->drawText( x+xoffset+5, y+y_info, w-rect_params.width()-10-xoffset, 15, Qt::AlignLeft | Qt::AlignTop, str_info);


	// On compact display we dont show progress, error hosts and progress bars.
	if( compact_display )
		return;


	// Paint error hosts:
	int error_hosts_text_width = 0;
	if( p_errorhosts )
	{
		if( p_avoidhosts )
			painter->setPen( afqt::QEnvironment::clr_error.c);
		else
			painter->setPen( afqt::QEnvironment::clr_errorready.c);

		QRect rect_errorhosts;
		painter->drawText( x+xoffset+5, y+y_progress, w-10-xoffset, 15, Qt::AlignRight | Qt::AlignTop, str_avoiderrors, &rect_errorhosts);
		error_hosts_text_width = rect_errorhosts.width() + 10;
	}


	// Paint progress:
	painter->setPen( pen);
	painter->drawText( x+xoffset+5, y+y_progress, w-15-xoffset-error_hosts_text_width, 15, Qt::AlignLeft | Qt::AlignTop, str_progress);


	xoffset += 2;
	// Paint progress bars:
	Item::drawPercent
	(
		painter, x+xoffset, y+y_bars, w-xoffset-2, 4,
		jobid == AFJOB::SYSJOB_ID ? p_tasksrunning + p_tasksready + p_taskserror : tasksnum,
		jobid == AFJOB::SYSJOB_ID ? 0 : p_tasksdone, p_taskserror, p_tasksrunning,
		false
	);
	Item::drawPercent
	(
		painter, x+xoffset, y+y_bars+4, w-xoffset-2, 4,
		100,
		p_percentage, 0, 0,
		false
	);
	drawProgress
	(
		painter, x+1+xoffset, y+y_bars+8, w-xoffset-2, 6,
		backcolor
	);

	painter->setPen( afqt::QEnvironment::clr_outline.c );
	painter->setBrush( Qt::NoBrush);
	painter->drawRoundedRect( x-1+xoffset, y+y_bars-1, w-1-xoffset, 15, 2, 2);
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
		case 'Y': // STATE_READY_MASK | STATE_ERROR_READY_MASK
			painter->setBrush( QBrush( afqt::QEnvironment::clr_errorready.c, Qt::SolidPattern ));
			painter->drawRect( x, posy, w+offset, height);
			break;
		case 'W': // STATE_WAITDEP_MASK
			painter->setBrush( QBrush( afqt::QEnvironment::clr_itemjobwdep.c, Qt::SolidPattern ));
			painter->drawRect( x, posy, w+offset, height);
			break;
		case 'C': // STATE_WAITRECONNECT_MASK
			painter->setBrush( QBrush( afqt::QEnvironment::clr_taskwaitreconn.c, Qt::SolidPattern ));
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
			painter->setBrush( QBrush( afqt::QEnvironment::clr_itemjobwarning.c, Qt::SolidPattern ));
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

		action = new ActionIdString( id_block, "restart_running", "Restart Running", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		action = new ActionIdString( id_block, "restart_skipped", "Restart Skipped", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		action = new ActionIdString( id_block, "restart_done", "Restart Done", qwidget);
		QObject::connect( action, SIGNAL( triggeredId( int, QString) ), qwidget, SLOT( blockAction( int, QString) ));
		menu->addAction( action);

		menu->addSeparator();
	}

	if( submenu != NULL )
		 menu = submenu;

	action = new ActionIdString( id_block, "sequential", "Set Sequential", qwidget);
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
	
	action = new ActionIdString( id_block, "task_progress_change_timeout", "Set Task Progress Change Timeout", qwidget);
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
		i_action == "restart_running" ||
		i_action == "restart_skipped" ||
		i_action == "restart_done" ||
		i_action == "reset_error_hosts")
	{
		i_str << ",\n\"operation\":{\"type\":\"" << i_action.toUtf8().data() << "\"}";
		return true;
	}

	// Parameter change:

	i_str << ",\n\"params\":{\n";
	i_str << '"' << i_action.toUtf8().data() << "\":";

	// We should query some number or string:

	bool ok = true;
	int cur_number = 0;
	QString cur_string;
	int set_number = 0;
	QString set_string;

	if( i_action == "capacity" )
	{
		if( id_block == blocknum ) cur_number = capacity;
		set_number = QInputDialog::getInt( listitems, "Change Capacity", "Enter Capacity", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "sequential" )
	{
		if( id_block == blocknum ) cur_number = sequential;
		set_number = QInputDialog::getInt( listitems, "Change Sequential", "Enter Sequential", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "errors_retries" )
	{
		if( id_block == blocknum ) cur_number = errors_retries;
		set_number = QInputDialog::getInt( listitems, "Set Retries Error", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "errors_avoid_host" )
	{
		if( id_block == blocknum ) cur_number = errors_avoidhost;
		set_number = QInputDialog::getInt( listitems, "Set Errors Avoid Host", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "errors_task_same_host" )
	{
		if( id_block == blocknum ) cur_number = errors_tasksamehost;
		set_number = QInputDialog::getInt( listitems, "Set Task Errors Same Host", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "errors_forgive_time" )
	{
		double cur = 0;
		if( id_block == blocknum ) cur = double(errors_forgivetime) / (60*60);
		double hours = QInputDialog::getDouble( listitems, "Set Errors forgive time", "Enter number of hours (0=infinite)", cur, -1, 365*24, 3, &ok);
		set_number = int( hours * 60*60 );
	}
	else if( i_action == "task_progress_change_timeout" )
	{
		double cur = 0;
		if( id_block == blocknum ) cur = double(task_progress_change_timeout) / (60*60);
		double hours = QInputDialog::getDouble( listitems, "Set Task Progress Change Timeout", "Enter number of hours (0=infinite)", cur, -1, 365*24, 3, &ok);
		set_number = int( hours * 60*60 );
		if( set_number <= 0) set_number = -1;
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
		set_number = QInputDialog::getInt( listitems, "Change Maximum Running Tasks", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "max_running_tasks_per_host" )
	{
		if( id_block == blocknum ) cur_number = maxruntasksperhost;
		set_number = QInputDialog::getInt( listitems, "Change Maximum Running Tasks Per Host", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
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
		set_number = QInputDialog::getInt( listitems, "Change Needed Memory", "Enter MegaBytes", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "need_hdd" )
	{
		if( id_block == blocknum ) cur_number = need_hdd;
		set_number = QInputDialog::getInt( listitems, "Change Maximum Hosts", "Enter GigaBytes", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "need_power" )
	{
		if( id_block == blocknum ) cur_number = need_power;
		set_number = QInputDialog::getInt( listitems, "Change Maximum Hosts", "Enter Power", cur_number, -1, INT_MAX, 1, &ok);
	}
	else if( i_action == "capacity_coeff_min" )
	{
		if( id_block == blocknum ) cur_number = capcoeff_min;
		set_number = QInputDialog::getInt( listitems, "Change Capacity min coeff", "Enter Coefficient", cur_number, 0, INT_MAX, 1, &ok);
	}
	else if( i_action == "capacity_coeff_max" )
	{
		if( id_block == blocknum ) cur_number = capcoeff_max;
		set_number = QInputDialog::getInt( listitems, "Change Capacity max coeff", "Enter Coefficient", cur_number, 0, INT_MAX, 1, &ok);
	}
	else if( i_action == "multihost_min" )
	{
		if( id_block == blocknum ) cur_number = multihost_min;
		set_number = QInputDialog::getInt( listitems, "Change Hosts Minimun", "Enter Quantity", cur_number, 1, AFJOB::TASK_MULTIHOSTMAXHOSTS, 1, &ok);
	}
	else if( i_action == "multihost_max" )
	{
		if( id_block == blocknum ) cur_number = multihost_max;
		set_number = QInputDialog::getInt( listitems, "Change Hosts Maximum", "Enter Quantity", cur_number, 1, AFJOB::TASK_MULTIHOSTMAXHOSTS, 1, &ok);
	}
	else if( i_action == "multihost_max_wait" )
	{
		if( id_block == blocknum ) cur_number = multihost_waitmax;
		set_number = QInputDialog::getInt( listitems, "Set Hosts Maximum Wait Time", "Enter Seconds", cur_number, 0, INT_MAX, 1, &ok);
	}
	else if( i_action == "multihost_service_wait" )
	{
		if( id_block == blocknum ) cur_number = multihost_waitsrv;
		set_number = QInputDialog::getInt( listitems, "Set Service Start Wait", "Enter Seconds", cur_number, 0, INT_MAX, 1, &ok);
	}
	else
	{
		AFERRAR("BlockInfo::blockAction: Unknown action = '%s'", i_action.toUtf8().data())
		return false;
	}

//printf("BlockInfo::blockAction: %s\n", af::Msg::TNAMES[id_action]);

	if( ok == false) return false;

	if( set_string.isNull())
	{
		i_str << set_number;
	}
	else
	{
		std::string err;
		if( false == af::RegExp::Validate( afqt::qtos( set_string), &err))
		{
			listitems->displayError( afqt::stoq( err));
			return false;
		}
		i_str << '"' << afqt::qtos( set_string) << '"';
	}

	i_str << '}';

//printf("%d %s\n", id_block, i_action.toUtf8().data());
	return true;
}
