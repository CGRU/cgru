#include "blockinfo.h"

#include <limits.h>

#include "../libafanasy/msg.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "../libafqt/qenvironment.h"

#include "actionid.h"
#include "listitems.h"
#include "param.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QInputDialog>
#include <QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int BlockInfo::Height        = 48;
const int BlockInfo::HeightCompact = 16;

BlockInfo::BlockInfo(const af::BlockData * i_data, Item * i_item, ListItems * i_listitems):
	m_item(i_item),
	m_listitems(i_listitems),

	p_percentage(0),
	p_tasksready(0),
	p_tasksrunning(0),
	p_capacitytotal(0),
	p_tasksdone(0),
	p_taskserror(0),
	p_tasksskipped(0),
	p_taskswarning(0),
	p_taskswaitrec(0),
	p_errorhosts(0),
	p_avoidhosts(0),
	p_taskssumruntime(0),

	errors_retries(-1),
	errors_avoid_host(-1),
	errors_task_same_host(-1),
	task_max_run_time(0),
	task_min_run_time(0),

	m_blocknum(i_data->getBlockNum()),
	m_jobid(i_data->getJobId()),
	m_name(afqt::stoq(i_data->getName())),

	tasksnum(1)
{
	addParam_Num("sequential",                   "Sequential",            "Tasks solving order", -(1<<30));
	addParam_Num("capacity",                     "Capacity",              "Tasks capacity");
	addParam_Num("max_running_tasks",            "Maximum Running",       "Maximum running tasks number");
	addParam_Num("max_running_tasks_per_host",   "Max Run Per Host",      "Max run tasks on the same host");
	addParam_separator();
	addParam_Hrs("task_max_run_time",            "Task Max Run Time",     "Task maximum running time");
	addParam_Hrs("task_min_run_time",            "Task Min Run Time",     "Task minimum running time");
	addParam_Hrs("task_progress_change_timeout", "Task Progress Timeout", "Task progress change timeout");
	addParam_separator();
	addParam_Num("errors_retries",               "Errors Retries",        "Number of times to retry error tasks");
	addParam_Num("errors_avoid_host",            "Errors Avoid Host",     "Number of errors for block to avoid host");
	addParam_Num("errors_task_same_host",        "Errors Task Avoid",     "Number of errors for each task to avoid host");
	addParam_Hrs("errors_forgive_time",          "Errors Forgive Time",   "After this time host errors will be forgived");
	addParam_separator();
	addParam_REx("hosts_mask",                   "Hosts Mask",            "Host names pattern that block can run on");
	addParam_REx("hosts_mask_exclude",           "Hosts Mask Exclude",    "Host names pattern that block will not run");
	addParam_Num("need_memory",                  "Need Memory",           "Host free memory needed to run tasks");
	addParam_Num("need_hdd",                     "Need HDD Space",        "Host free HDD space needed to run tasks");
	addParam_REx("need_properties",              "Need Properties",       "Host \"Properties\" needed to run tasks");
	addParam_Num("need_power",                   "Need Power",            "Host \"Power\" needed to run tasks");
	addParam_separator();
	addParam_REx("depend_mask",                  "Depend Mask",           "Blocks name mask to wait to finish totally");
	addParam_REx("tasks_depend_mask",            "Tasks Depend Mask",     "Blocks name mask to wait to finish tasks");
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

		tasksnum                     = block->getTasksNum();
		numeric                      = block->isNumeric();

		frame_first                  = block->getFrameFirst();
		frame_last                   = block->getFrameLast();
		frame_pertask                = block->getFramePerTask();
		frame_inc                    = block->getFrameInc();

		service                      = afqt::stoq( block->getService());
		multihost                    = block->isMultiHost();
		multihost_samemaster         = block->canMasterRunOnSlaveHost();
		varcapacity                  = block->canVarCapacity();

		// Ediable parameters stored in map
		           sequential                     = block->getSequential();
		m_var_map["sequential"]                   = int(block->getSequential());
		           capacity                       = block->getCapacity();
		m_var_map["capacity"]                     = block->getCapacity();
		           max_running_tasks              = block->getMaxRunningTasks();
		m_var_map["max_running_tasks"]            = block->getMaxRunningTasks();
		           max_running_tasks_per_host     = block->getMaxRunTasksPerHost();
		m_var_map["max_running_tasks_per_host"]   = block->getMaxRunTasksPerHost();

		           task_max_run_time              = block->getTaskMaxRunTime();
		m_var_map["task_max_run_time"]            = block->getTaskMaxRunTime();
		           task_min_run_time              = block->getTaskMinRunTime();
		m_var_map["task_min_run_time"]            = block->getTaskMinRunTime();
		           task_progress_change_timeout   = block->getTaskProgressChangeTimeout();
		m_var_map["task_progress_change_timeout"] = block->getTaskProgressChangeTimeout();

		           errors_retries                 = block->getErrorsRetries();
		m_var_map["errors_retries"]               = block->getErrorsRetries();
		           errors_avoid_host              = block->getErrorsAvoidHost();
		m_var_map["errors_avoid_host"]            = block->getErrorsAvoidHost();
		           errors_task_same_host          = block->getErrorsTaskSameHost();
		m_var_map["errors_task_same_host"]        = block->getErrorsTaskSameHost();
		           errors_forgive_time            = block->getErrorsForgiveTime();
		m_var_map["errors_forgive_time"]          = block->getErrorsForgiveTime();

		           hosts_mask                     = afqt::stoq(block->getHostsMask());
		m_var_map["hosts_mask"]                   = afqt::stoq(block->getHostsMask());
		           hosts_mask_exclude             = afqt::stoq(block->getHostsMaskExclude());
		m_var_map["hosts_mask_exclude"]           = afqt::stoq(block->getHostsMaskExclude());
		           need_memory                    = block->getNeedMemory();
		m_var_map["need_memory"]                  = block->getNeedMemory();
		           need_hdd                       = block->getNeedHDD();
		m_var_map["need_hdd"]                     = block->getNeedHDD();
		           need_properties                = afqt::stoq(block->getNeedProperties());
		m_var_map["need_properties"]              = afqt::stoq(block->getNeedProperties());
		           need_power                     = block->getNeedPower();
		m_var_map["need_power"]                   = block->getNeedPower();

		           depend_mask                    = afqt::stoq(block->getDependMask());
		m_var_map["depend_mask"]                  = afqt::stoq(block->getDependMask());
		           tasks_depend_mask              = afqt::stoq(block->getTasksDependMask());
		m_var_map["tasks_depend_mask"]            = afqt::stoq(block->getTasksDependMask());


		filesize_min      = block->getFileSizeMin();
		filesize_max      = block->getFileSizeMax();
		capcoeff_min      = block->getCapCoeffMin();
		capcoeff_max      = block->getCapCoeffMax();
		multihost_min     = block->getMultiHostMin();
		multihost_max     = block->getMultiHostMax();
		multihost_waitmax = block->getMultiHostWaitMax();
		multihost_waitsrv = block->getMultiHostWaitSrv();


		tickets.clear();
		for (auto const& it : block->getTickets())
			tickets[afqt::stoq(it.first)] = it.second;

		depends.clear();
		if( Watch::isPadawan())
		{
			if( block->isDependSubTask()) depends += QString(" [Sub-Task Depends]");
			if( false == depend_mask.isEmpty()) depends += QString(" Depends(%1)").arg( depend_mask);
			if( false == tasks_depend_mask.isEmpty()) depends += QString(" TasksDepends[%1]").arg( tasks_depend_mask);
		}
		else if( Watch::isJedi())
		{
			if( block->isDependSubTask()) depends += QString(" [SUB]");
			if( false == depend_mask.isEmpty()) depends += QString(" Dep(%1)").arg( depend_mask);
			if( false == tasks_depend_mask.isEmpty()) depends += QString(" TDep[%1]").arg( tasks_depend_mask);
		}
		else
		{
			if( block->isDependSubTask()) depends += QString(" [sub]");
			if( false == depend_mask.isEmpty()) depends += QString(" d(%1)").arg( depend_mask);
			if( false == tasks_depend_mask.isEmpty()) depends += QString(" t[%1]").arg( tasks_depend_mask);
		}

		m_icon_large = Watch::getServiceIconLarge( service);
		m_icon_small = Watch::getServiceIconSmall( service);

	case af::Msg::TBlocksProgress:

		state = block->getState();

		p_percentage      = block->getProgressPercentage();
		p_tasksready      = block->getProgressTasksReady();
		p_tasksrunning    = block->getRunningTasksNumber();
		p_capacitytotal   = block->getRunningCapacityTotal();
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

	str_info += QString(": %1").arg(m_name);

	if( false == depends.isEmpty())
		str_info += depends;


	// Parameters:
	str_params.clear();
	if( Watch::isPadawan())
	{
		if( p_tasksdone) str_params += QString(" Render Timings: Sum:%1 / Average:%2")
			.arg( af::time2strHMS( p_taskssumruntime, true).c_str())
			.arg( af::time2strHMS( p_taskssumruntime/p_tasksdone, true).c_str());

		if(( errors_avoid_host >= 0 ) || ( errors_task_same_host >= 0 ) || ( errors_retries >= 0 ))
			str_params += Item::generateErrorsSolvingInfo( errors_avoid_host, errors_task_same_host, errors_retries);
		if( errors_forgive_time >= 0 ) str_params += QString(" ErrorsForgiveTime:%1").arg( af::time2strHMS( errors_forgive_time, true).c_str());

		if(task_max_run_time > 0) str_params += QString(" MaxRunTime:%1").arg(af::time2strHMS(task_max_run_time, true).c_str());
		if(task_min_run_time > 0) str_params += QString(" MinRunTime:%1").arg(af::time2strHMS(task_min_run_time, true).c_str());

		if( max_running_tasks    != -1 ) str_params += QString(" MaxRunTasks:%1").arg( max_running_tasks);
		if( max_running_tasks_per_host != -1 ) str_params += QString(" MaxPerHost:%1").arg( max_running_tasks_per_host);
		if( false == hosts_mask.isEmpty()          ) str_params += QString(" HostsMask(%1)").arg( hosts_mask         );
		if( false == hosts_mask_exclude.isEmpty()  ) str_params += QString(" ExcludeHosts(%1)").arg( hosts_mask_exclude );
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
		
		if( task_progress_change_timeout != -1) str_params += QString(" TasksProgessTimeout:%1").arg(af::time2strHMS( task_progress_change_timeout, true).c_str());
	}
	else if( Watch::isJedi())
	{
		if( p_tasksdone) str_params += QString(" Timings: Sum:%1/Avg:%2")
			.arg( af::time2strHMS( p_taskssumruntime, true).c_str())
			.arg( af::time2strHMS( p_taskssumruntime/p_tasksdone, true).c_str());

		if(( errors_avoid_host >= 0 ) || ( errors_task_same_host >= 0 ) || ( errors_retries >= 0 ))
			str_params += Item::generateErrorsSolvingInfo( errors_avoid_host, errors_task_same_host, errors_retries);
		if( errors_forgive_time >= 0 ) str_params += QString(" Forgive:%1").arg( af::time2strHMS( errors_forgive_time, true).c_str());

		if(task_max_run_time > 0) str_params += QString(" MaxTime:%1").arg(af::time2strHMS(task_max_run_time, true).c_str());
		if(task_min_run_time > 0) str_params += QString(" MinTime:%1").arg(af::time2strHMS(task_min_run_time, true).c_str());

		if( max_running_tasks    != -1 ) str_params += QString(" Max:%1").arg( max_running_tasks);
		if( max_running_tasks_per_host != -1 ) str_params += QString(" PerHost:%1").arg( max_running_tasks_per_host);
		if( false == hosts_mask.isEmpty()          ) str_params += QString(" Hosts(%1)").arg( hosts_mask         );
		if( false == hosts_mask_exclude.isEmpty()  ) str_params += QString(" Exclude(%1)").arg( hosts_mask_exclude );
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
		
		if( task_progress_change_timeout != -1) str_params += QString(" NoProgress:%1").arg(af::time2strHMS( task_progress_change_timeout, true).c_str());
	}
	else
	{
		if( p_tasksdone) str_params += QString(" rt:s%1/a%2")
			.arg( af::time2strHMS( p_taskssumruntime, true).c_str())
			.arg( af::time2strHMS( p_taskssumruntime/p_tasksdone, true).c_str());

		if(( errors_avoid_host >= 0 ) || ( errors_task_same_host >= 0 ) || ( errors_retries >= 0 ))
			str_params += Item::generateErrorsSolvingInfo( errors_avoid_host, errors_task_same_host, errors_retries);
		if( errors_forgive_time >= 0 ) str_params += QString(" f%1").arg( af::time2strHMS( errors_forgive_time, true).c_str());

		if(task_max_run_time > 0) str_params += QString(" Mrt%1").arg(af::time2strHMS(task_max_run_time, true).c_str());
		if(task_min_run_time > 0) str_params += QString(" mrt%1").arg(af::time2strHMS(task_min_run_time, true).c_str());

		if( max_running_tasks    != -1 ) str_params += QString(" m%1").arg( max_running_tasks);
		if( max_running_tasks_per_host != -1 ) str_params += QString(" mph%1").arg( max_running_tasks_per_host);
		if( false == hosts_mask.isEmpty()          ) str_params += QString(" h(%1)").arg( hosts_mask         );
		if( false == hosts_mask_exclude.isEmpty()  ) str_params += QString(" e(%1)").arg( hosts_mask_exclude );
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
		
		if( task_progress_change_timeout != -1) str_params += QString(" tpt%1").arg(af::time2strHMS( task_progress_change_timeout, true).c_str());
	}


	// Progress:
	str_progress = QString::number( p_percentage) + "%";
	if( Watch::isPadawan())
	{
		if (p_tasksrunning ) str_progress += QString(" Running:%1"         ).arg(p_tasksrunning);
		if (p_capacitytotal) str_progress += QString(" Capacity:%1"        ).arg(af::toKMG(p_capacitytotal).c_str());
		if (p_tasksdone    ) str_progress += QString(" Done:%1"            ).arg(p_tasksdone);
		if (p_taskserror   ) str_progress += QString(" Errors:%1"          ).arg(p_taskserror);
		if (p_tasksskipped ) str_progress += QString(" Skipped:%1"         ).arg(p_tasksskipped);
		if (p_taskswarning ) str_progress += QString(" Warnings:%1"        ).arg(p_taskswarning);
		if (p_taskswaitrec ) str_progress += QString(" WaitingReconnect:%1").arg(p_taskswaitrec);
	}
	else if( Watch::isJedi())
	{
		if (p_tasksrunning ) str_progress += QString(" Run:%1" ).arg(p_tasksrunning);
		if (p_capacitytotal) str_progress += QString(" Cap:%1" ).arg(af::toKMG(p_capacitytotal).c_str());
		if (p_tasksdone    ) str_progress += QString(" Done:%1").arg(p_tasksdone);
		if (p_taskserror   ) str_progress += QString(" Err:%1" ).arg(p_taskserror);
		if (p_tasksskipped ) str_progress += QString(" Skp:%1" ).arg(p_tasksskipped);
		if (p_taskswarning ) str_progress += QString(" Wrn:%1" ).arg(p_taskswarning);
		if (p_taskswaitrec ) str_progress += QString(" WRC:%1" ).arg(p_taskswaitrec);
	}
	else
	{
		if (p_tasksrunning ) str_progress += QString(" r%1"  ).arg(p_tasksrunning);
		if (p_capacitytotal) str_progress += QString(" c%1"  ).arg(af::toKMG(p_capacitytotal).c_str());
		if (p_tasksdone    ) str_progress += QString(" d%1"  ).arg(p_tasksdone);
		if (p_taskserror   ) str_progress += QString(" e%1"  ).arg(p_taskserror);
		if (p_tasksskipped ) str_progress += QString(" s%1"  ).arg(p_tasksskipped);
		if (p_taskswarning ) str_progress += QString(" w%1"  ).arg(p_taskswarning);
		if (p_taskswaitrec ) str_progress += QString(" wrc%1").arg(p_taskswaitrec);
	}

	if (m_jobid == AFJOB::SYSJOB_ID ) str_progress += QString(" Ready:%1").arg( p_tasksready);


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

void BlockInfo::paint( QPainter * i_painter, const QStyleOptionViewItem &option,
	int x, int y, int w,
	bool i_compact_display,
	const QColor * i_backcolor) const
{
	if (m_item == NULL)
	{
		AF_ERR << "BlockInfo::drawBars: Item is not set.";
		return;
	}

	static const int y_info     =  0;
	static const int y_bars     = 14;
	static const int y_progress = 30;


	// Paint an icon and calculate an offset for further info:
	int xoffset = 3;
	if (i_compact_display)
	{
		if (m_icon_small)
		{
			i_painter->drawPixmap(x, y, *m_icon_small);
			xoffset += 16;
		}
	}
	else
	{
		xoffset = 7;

		if (m_icon_large)
		{
			i_painter->drawPixmap(x, y, *m_icon_large);
			xoffset += 42;
		}
	}


	// Paint border:
	i_painter->setPen(afqt::QEnvironment::clr_outline.c );
	i_painter->setBrush(Qt::NoBrush);
	i_painter->drawRoundedRect(x+xoffset-3, y-1, w+4-xoffset, i_compact_display ? HeightCompact : Height, 3, 3);


	// Setup font size and color:
	i_painter->setFont(afqt::QEnvironment::f_info);
	QPen pen(Item::clrTextInfo(p_tasksrunning, option.state & QStyle::State_Selected, m_item->isLocked()));

	// Draw tickets:
	int tk_w = 0;
	int tk_y = i_compact_display ? y+y_info : y+y_progress;
	QMapIterator<QString, int> it(tickets);
	while (it.hasNext())
	{
		it.next();

		tk_w += Item::drawTicket(i_painter, pen, x+5 + xoffset, tk_y, w-10 - xoffset - tk_w,
				Item::TKD_LEFT,
				it.key(), it.value());

		tk_w += 8;
	}

	i_painter->setPen(pen);

	// Paint parameters:
	QRect rect_params;
	i_painter->drawText(x, y+y_info, w-5-(i_compact_display?tk_w:0), 15, Qt::AlignRight | Qt::AlignTop, str_params, &rect_params);

	// Paint general information:
	i_painter->drawText(x+xoffset+5, y+y_info, w-rect_params.width()-10-xoffset, 15, Qt::AlignLeft | Qt::AlignTop, str_info);


	// On compact display we dont show progress, error hosts and progress bars.
	if (i_compact_display)
		return;


	// Paint error hosts:
	int error_hosts_text_width = 0;
	if (p_errorhosts)
	{
		if (p_avoidhosts)
			i_painter->setPen(afqt::QEnvironment::clr_error.c);
		else
			i_painter->setPen(afqt::QEnvironment::clr_errorready.c);

		QRect rect_errorhosts;
		i_painter->drawText(x+xoffset+5, y+y_progress, w-10-xoffset-tk_w, 15, Qt::AlignRight | Qt::AlignTop, str_avoiderrors, &rect_errorhosts);
		error_hosts_text_width = rect_errorhosts.width() + 10;
	}


	// Paint progress:
	i_painter->setPen(pen);
	i_painter->drawText(x+xoffset+5, y+y_progress, w-15-xoffset-error_hosts_text_width, 15, Qt::AlignLeft | Qt::AlignTop, str_progress);


	xoffset += 2;
	// Paint progress bars:
	Item::drawPercent
	(
		i_painter, x+xoffset, y+y_bars, w-xoffset-2, 4,
		m_jobid == AFJOB::SYSJOB_ID ? p_tasksrunning + p_tasksready + p_taskserror : tasksnum,
		m_jobid == AFJOB::SYSJOB_ID ? 0 : p_tasksdone, p_taskserror, p_tasksrunning,
		false
	);
	Item::drawPercent
	(
		i_painter, x+xoffset, y+y_bars+4, w-xoffset-2, 4,
		100,
		p_percentage, 0, 0,
		false
	);
	drawProgress
	(
		i_painter, x+1+xoffset, y+y_bars+8, w-xoffset-2, 6,
		i_backcolor
	);

	i_painter->setPen(afqt::QEnvironment::clr_outline.c);
	i_painter->setBrush(Qt::NoBrush);
	i_painter->drawRoundedRect(x-1+xoffset, y+y_bars-1, w-1-xoffset, 15, 2, 2);
}

void BlockInfo::drawProgress(
			QPainter * i_painter,
			int posx, int posy, int width, int height,
			const QColor * i_backcolor
		) const
{
	i_painter->setPen( Qt::NoPen );

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
			i_painter->setBrush( QBrush( afqt::QEnvironment::qclr_black, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'r': // STATE_READY_MASK
			break;
		case 'D': // STATE_DONE_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_done.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'R': // STATE_RUNNING_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_running.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'E': // STATE_ERROR_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_error.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'Y': // STATE_READY_MASK | STATE_ERROR_READY_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_errorready.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'W': // STATE_WAITDEP_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_itemjobwdep.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'C': // STATE_WAITRECONNECT_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_taskwaitreconn.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'S': // STATE_SKIPPED_MASK | STATE_DONE_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_taskskipped.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'N': // STATE_RUNNING_MASK_MASK | STATE_WARNING_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_taskwarningrun.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'G': // STATE_DONE_MASK | STATE_WARNING_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_itemjobwarning.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		}
		last_x = x + w + offset;
//printf("%c", progress[i]);
	}
//printf("\n");
}

void BlockInfo::generateMenu(QMenu * i_menu, QMenu * i_params_submenu) const
{
	static const QStringList operations = {
		"reset_error_hosts", "Reset Error Hosts",
		"skip",              "Skip Block Tasks",
		"restart",           "Restart Block Tasks",
		"restart_running",   "Restart Running",
		"restart_skipped",   "Restart Skipped",
		"restart_done",      "Restart Done"};

	QStringListIterator it(operations);
	while (it.hasNext())
	{
		ActionString * action = new ActionString(it.next(), it.next(), m_listitems);
		connect(action, SIGNAL(triggeredString(QString)), this, SLOT(slot_BlockOperation(QString)));
		i_menu->addAction(action);
	}

	i_menu->addSeparator();

	// Change block parameters menu (ListTasks shows it in other submenu):
	if (i_params_submenu != NULL)
		i_menu = i_params_submenu;

	QList<Param*>::const_iterator pIt;
	for (pIt = m_params.begin(); pIt != m_params.end(); pIt++)
	{
		if ((*pIt)->isSeparator())
		{
			i_menu->addSeparator();
			continue;
		}

		ActionParam * action = new ActionParam(*pIt);
		connect(action, SIGNAL(triggeredParam(const Param *)), this, SLOT(slot_BlockChangeParam(const Param *)));
		i_menu->addAction(action);
	}
}

void BlockInfo::slot_BlockOperation(QString i_operation)
{
	QString json = QString("\"operation\":{\"type\":\"%1\"}").arg(i_operation);

	emit sig_BlockAction(m_blocknum, json);
}

void BlockInfo::slot_BlockChangeParam(const Param * i_param)
{
	if (false ==  m_var_map.contains(i_param->name))
	{
		AF_ERR << "No such parameter: " << i_param->name.toUtf8().data();
		return;
	}

	QVariant cur_val = m_var_map[i_param->name];

	QString new_val_str;
	if (false == i_param->getInputDialog(cur_val, new_val_str, m_listitems))
	{
		if (new_val_str.size())
			m_listitems->displayError(new_val_str);
		return;
	}

	QString json = QString("\"params\":{\"%1\":%2}").arg(i_param->name, new_val_str);

	emit sig_BlockAction(m_blocknum, json);
}

void BlockInfo::slot_BlockTicketAdd()
{
	bool ok;
	QString name = QInputDialog::getText(m_listitems, "Add Ticket", "Enter a new ticket name", QLineEdit::Normal, "", &ok);
	if (false == ok)
		return;

	slot_BlockTicketEdit(name);
}

void BlockInfo::slot_BlockTicketEdit(QString i_name)
{
	if (i_name.isEmpty())
	{
		// If name is not provided, new ticket name should be asked
		m_listitems->displayError("Empty ticket name.");
		return;
	}

	int cur_count = 1;
	// Find this ticket to get current cur_count
	QMapIterator<QString, int> it(tickets);
	while (it.hasNext())
	{
		it.next();
		if (it.key() == i_name)
		{
			cur_count = it.value();
			break;
		}
	}

	bool ok;
	int new_count = QInputDialog::getInt(m_listitems, "Edit Ticket",
			QString("Enter %1 cur_count.\nType -1 to remove.").arg(i_name),
			cur_count, -1, 1<<30, 1, &ok);

	QString json = QString("\"operation\":{\"type\":\"tickets\",\"name\":\"%1\",\"count\":%2}")
		.arg(i_name).arg(new_count);

	emit sig_BlockAction(m_blocknum, json);
}

void BlockInfo::addParam_separator() {
	m_params.append(new Param(Param::tsep, Item::TAny, "separator", "Separator", "This is parameters separator."));}

void BlockInfo::addParam_Num(const QString & i_name, const QString & i_label, const QString & i_tip, int i_min, int i_max) {
	m_params.append(new Param(Param::TNum, Item::TAny, i_name, i_label, i_tip, i_min, i_max));}

void BlockInfo::addParam_Str(const QString & i_name, const QString & i_label, const QString & i_tip) {
	m_params.append(new Param(Param::TStr, Item::TAny, i_name, i_label, i_tip));}

void BlockInfo::addParam_REx(const QString & i_name, const QString & i_label, const QString & i_tip) {
	m_params.append(new Param(Param::TREx, Item::TAny, i_name, i_label, i_tip));}

void BlockInfo::addParam_Hrs(const QString & i_name, const QString & i_label, const QString & i_tip) {
	m_params.append(new Param(Param::THrs, Item::TAny, i_name, i_label, i_tip));}

