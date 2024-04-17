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
	p_tasks_ready(0),
	p_tasks_running(0),
	p_capacity_total(0),
	p_tasks_done(0),
	p_tasks_error(0),
	p_tasks_skipped(0),
	p_tasks_waitdep(0),
	p_tasks_warning(0),
	p_tasks_waitrec(0),
	p_error_hosts(0),
	p_avoid_hosts(0),
	p_tasks_sumruntime(0),

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
	addParam_MiB("need_memory",                  "Need Memory",           "Host free memory needed to run tasks (GB)");
	addParam_MiB("need_gpu_mem_mb",              "Need GPU Memory",       "Host free GPU memory needed to run tasks (GB)");
	addParam_Meg("need_cpu_freq_mgz",            "Need CPU Frequency",    "Host CPU freqency to run tasks (GHz)");
	addParam_Num("need_cpu_cores",               "Need CPU Cores",        "Host CPU cores number to run tasks");
	addParam_Meg("need_cpu_freq_cores",          "Need CPU Cores*Freq",   "Host CPU cores*freqency to run tasks (GHz)");
	addParam_GiB("need_hdd",                     "Need HDD Space",        "Host free HDD space needed to run tasks (GB)");
	addParam_REx("need_properties",              "Need Properties",       "Host \"Properties\" needed to run tasks");
	addParam_Num("need_power",                   "Need Power",            "Host \"Power\" needed to run tasks");
	addParam_separator();
	addParam_REx("depend_mask",                  "Depend Mask",           "Blocks name mask to wait to finish totally");
	addParam_REx("tasks_depend_mask",            "Tasks Depend Mask",     "Blocks name mask to wait to finish tasks");
	addParam_separator();
	addParam_MSS("environment",                  "Tasks Environment",     "Blocks tasks process extra environment variables");
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

		           need_gpu_mem_mb                = block->getNeedGPUMemMb();
		m_var_map["need_gpu_mem_mb"]              = block->getNeedGPUMemMb();
		           need_cpu_freq_mgz              = block->getNeedCPUFreqMHz();
		m_var_map["need_cpu_freq_mgz"]            = block->getNeedCPUFreqMHz();
		           need_cpu_cores                 = block->getNeedCPUCores();
		m_var_map["need_cpu_cores"]               = block->getNeedCPUCores();
		           need_cpu_freq_cores            = block->getNeedCPUFreqCores();
		m_var_map["need_cpu_freq_cores"]          = block->getNeedCPUFreqCores();

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

		// Collect environment
		{
			environment.clear();
			QMap<QString, QVariant> qv_environment;
			for (auto const & it : block->getEnvironment())
			{
				if (environment.size()) environment += ",";
				environment += QString("%1:%2").arg(afqt::stoq(it.first)).arg(afqt::stoq(it.second));

				qv_environment[afqt::stoq(it.first)] = afqt::stoq(it.second);
			}
			if (environment.size())
				environment = QString("{%1}").arg(environment);
			m_var_map["environment"] = qv_environment;
		}


		skip_existing_files  = block->isSkippingExistingFiles();
		check_rendered_files = block->isCheckingRenderedFiles();
		filesize_min = block->getFileSizeMin();
		filesize_max = block->getFileSizeMax();

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

		p_percentage       = block->getProgressPercentage();
		p_tasks_ready      = block->getProgressTasksReady();
		p_tasks_running    = block->getRunningTasksNumber();
		p_capacity_total   = block->getRunningCapacityTotal();
		p_tasks_done       = block->getProgressTasksDone();
		p_tasks_error      = block->getProgressTasksError();
		p_tasks_skipped    = block->getProgressTasksSkipped();
		p_tasks_waitdep    = block->getProgressTasksWaitDep();
		p_tasks_warning    = block->getProgressTasksWarning();
		p_tasks_waitrec    = block->getProgressTasksWaitReconn();
		p_avoid_hosts      = block->getProgressAvoidHostsNum();
		p_error_hosts      = block->getProgressErrorHostsNum();
		p_tasks_sumruntime = block->getProgressTasksSumRunTime();

		server_info = afqt::stoq(block->getSrvInfo());

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

	if (p_tasks_running || p_tasks_error || ((p_tasks_done != 0) && (p_tasks_done != tasksnum))) return true;

	return false;
}

void BlockInfo::refresh()
{
	// General information:
	if( Watch::isPadawan())
	{
		str_left_top = QString("Frames[%1]").arg( tasksnum);
		if( numeric )
		{
			str_left_top += QString("( %1 - %2 ").arg( frame_first).arg( frame_last);
			if(( frame_pertask > 1 ) || ( frame_inc > 1 ))
			{
				str_left_top += QString(" : PerTask(%1)").arg( frame_pertask);
				if( frame_inc > 1 )
					str_left_top += QString(" / Increment(%1)").arg( frame_inc);
			}
			if( sequential != 1 )
				str_left_top += QString(" % Sequential(%1)").arg( sequential);
			str_left_top += ")";
		}
		else if( frame_pertask > 1)
		{
			str_left_top += QString(" * PerTask:%1").arg( frame_pertask);
		}
		else if( frame_pertask < 0)
		{
			str_left_top += QString(" / PerTask:%1").arg( -frame_pertask);
		}
	}
	else if( Watch::isJedi())
	{
		str_left_top = QString("Tasks[%1]").arg( tasksnum);
		if( numeric )
		{
			str_left_top += QString("(%1-%2").arg( frame_first).arg( frame_last);
			if(( frame_pertask > 1 ) || ( frame_inc > 1 ))
			{
				str_left_top += QString(":%1").arg( frame_pertask);
				if( frame_inc > 1 )
					str_left_top += QString("/inc(%1)").arg( frame_inc);
			}
			if( sequential != 1 )
				str_left_top += "seq(%)" + QString::number( sequential);
			str_left_top += ")";
		}
		else if( frame_pertask > 1)
		{
			str_left_top += QString("*%1").arg( frame_pertask);
		}
		else if( frame_pertask < 0)
		{
			str_left_top += QString("/%1").arg( -frame_pertask);
		}
	}
	else
	{
		str_left_top = QString("t%1").arg( tasksnum);
		if( numeric )
		{
			str_left_top += QString("(%1-%2").arg( frame_first).arg( frame_last);
			if(( frame_pertask > 1 ) || ( frame_inc > 1 ))
			{
				str_left_top += QString(":%1").arg( frame_pertask);
				if( frame_inc > 1 )
					str_left_top += QString("/%1").arg( frame_inc);
			}
			if( sequential != 1 )
				str_left_top += "%" + QString::number( sequential);
			str_left_top += ")";
		}
		else if( frame_pertask > 1)
		{
			str_left_top += QString("*%1").arg( frame_pertask);
		}
		else if( frame_pertask < 0)
		{
			str_left_top += QString("/%1").arg( -frame_pertask);
		}
	}

	str_left_top += QString(": %1").arg(m_name);

	if( false == depends.isEmpty())
		str_left_top += depends;


	// Parameters:
	str_right_top.clear();
	if( Watch::isPadawan())
	{
		if ((p_tasks_done - p_tasks_skipped) > 0) str_right_top += QString(" Render Timings: Sum:%1 / Average:%2")
			.arg(af::time2strHMS(p_tasks_sumruntime, true).c_str())
			.arg(af::time2strHMS(p_tasks_sumruntime/(p_tasks_done - p_tasks_skipped), true).c_str());

		if(( errors_avoid_host >= 0 ) || ( errors_task_same_host >= 0 ) || ( errors_retries >= 0 ))
			str_right_top += Item::generateErrorsSolvingInfo( errors_avoid_host, errors_task_same_host, errors_retries);
		if( errors_forgive_time >= 0 ) str_right_top += QString(" ErrorsForgiveTime:%1").arg( af::time2strHMS( errors_forgive_time, true).c_str());

		if(task_max_run_time > 0) str_right_top += QString(" MaxRunTime:%1").arg(af::time2strHMS(task_max_run_time, true).c_str());
		if(task_min_run_time > 0) str_right_top += QString(" MinRunTime:%1").arg(af::time2strHMS(task_min_run_time, true).c_str());

		if( max_running_tasks    != -1 ) str_right_top += QString(" MaxRunTasks:%1").arg( max_running_tasks);
		if( max_running_tasks_per_host != -1 ) str_right_top += QString(" MaxPerHost:%1").arg( max_running_tasks_per_host);
		if( false == hosts_mask.isEmpty()          ) str_right_top += QString(" HostsMask(%1)").arg( hosts_mask         );
		if( false == hosts_mask_exclude.isEmpty()  ) str_right_top += QString(" ExcludeHosts(%1)").arg( hosts_mask_exclude );
		if( false == need_properties.isEmpty()    ) str_right_top += QString(" Properties(%1)").arg( need_properties   );
		if (need_memory > 0) str_right_top += QString(" Mem>%1").arg(afqt::stoq(af::toKMG(int64_t(need_memory)*(1<<20), 1<<10)));
		if (need_gpu_mem_mb > 0) str_right_top += QString(" GPUMem>%1").arg(afqt::stoq(af::toKMG(int64_t(need_gpu_mem_mb)*1000000, 1000)));
		if (need_cpu_freq_mgz > 0) str_right_top += QString(" CPUGHz>%1").arg(afqt::stoq(af::toKMG(int64_t(need_cpu_freq_mgz)*1000000, 1000)));
		if (need_cpu_cores > 0) str_right_top += QString(" CPUCores>%1").arg(need_cpu_cores);
		if (need_cpu_freq_cores > 0) str_right_top += QString(" CPUCoresGHz>%1").arg(afqt::stoq(af::toKMG(int64_t(need_cpu_freq_cores)*1000000, 1000)));
		if (need_hdd    > 0) str_right_top += QString(" HDD>%1").arg(afqt::stoq(af::toKMG(int64_t(need_hdd   )*(1<<30), 1<<10)));
		if( need_power  > 0 ) str_right_top += QString(" Power>%1").arg( need_power);
		if( multihost )
		{
			str_right_top += QString(" Multi-Host(%1,%2)").arg( multihost_min).arg( multihost_max);
			if( multihost_samemaster) str_right_top += "SameMaster";
			if( multihost_waitmax) str_right_top += QString(":WaitMax(%1)").arg(multihost_waitmax);
			if( multihost_waitsrv) str_right_top += QString(":WaitService(%1)").arg(multihost_waitsrv);
		}

		if (skip_existing_files || check_rendered_files)
		{
			str_right_top += " Files";
			if (skip_existing_files)
				str_right_top += "Skip";
			if ((filesize_min > 0) || (filesize_max > 0))
			{
				str_right_top += QString("(");
				if (filesize_min > 0)
					str_right_top += afqt::stoq(af::toKMG(filesize_min));
				str_right_top += QString(",");
				if (filesize_max > 0)
					str_right_top += afqt::stoq(af::toKMG(filesize_max));
				str_right_top += QString(")");
			}
			if (check_rendered_files)
				str_right_top += "Check";
		}

		str_right_top += " Capacity:";
		if( varcapacity   ) str_right_top += QString("(%1-%2)*").arg( capcoeff_min).arg( capcoeff_max);
		str_right_top += QString("%1").arg( capacity);
		
		if( task_progress_change_timeout != -1) str_right_top += QString(" TasksProgessTimeout:%1").arg(af::time2strHMS( task_progress_change_timeout, true).c_str());
	}
	else if( Watch::isJedi())
	{
		if ((p_tasks_done - p_tasks_skipped) > 0) str_right_top += QString(" Timings: Sum:%1/Avg:%2")
			.arg(af::time2strHMS(p_tasks_sumruntime, true).c_str())
			.arg(af::time2strHMS(p_tasks_sumruntime/(p_tasks_done - p_tasks_skipped), true).c_str());

		if(( errors_avoid_host >= 0 ) || ( errors_task_same_host >= 0 ) || ( errors_retries >= 0 ))
			str_right_top += Item::generateErrorsSolvingInfo( errors_avoid_host, errors_task_same_host, errors_retries);
		if( errors_forgive_time >= 0 ) str_right_top += QString(" Forgive:%1").arg( af::time2strHMS( errors_forgive_time, true).c_str());

		if(task_max_run_time > 0) str_right_top += QString(" MaxTime:%1").arg(af::time2strHMS(task_max_run_time, true).c_str());
		if(task_min_run_time > 0) str_right_top += QString(" MinTime:%1").arg(af::time2strHMS(task_min_run_time, true).c_str());

		if( max_running_tasks    != -1 ) str_right_top += QString(" Max:%1").arg( max_running_tasks);
		if( max_running_tasks_per_host != -1 ) str_right_top += QString(" PerHost:%1").arg( max_running_tasks_per_host);
		if( false == hosts_mask.isEmpty()          ) str_right_top += QString(" Hosts(%1)").arg( hosts_mask         );
		if( false == hosts_mask_exclude.isEmpty()  ) str_right_top += QString(" Exclude(%1)").arg( hosts_mask_exclude );
		if( false == need_properties.isEmpty()    ) str_right_top += QString(" Props(%1)").arg( need_properties   );
		if (need_memory > 0) str_right_top += QString(" Mem>%1").arg(afqt::stoq(af::toKMG(int64_t(need_memory)*(1<<20), 1<<10)));
		if (need_gpu_mem_mb > 0) str_right_top += QString(" GMem>%1").arg(afqt::stoq(af::toKMG(int64_t(need_gpu_mem_mb)*1000000, 1000)));
		if (need_cpu_freq_mgz > 0) str_right_top += QString(" GHz>%1").arg(afqt::stoq(af::toKMG(int64_t(need_cpu_freq_mgz)*1000000, 1000)));
		if (need_cpu_cores > 0) str_right_top += QString(" Cores>%1").arg(need_cpu_cores);
		if (need_cpu_freq_cores > 0) str_right_top += QString(" CoresGHz>%1").arg(afqt::stoq(af::toKMG(int64_t(need_cpu_freq_cores)*1000000, 1000)));
		if (need_hdd    > 0) str_right_top += QString(" HDD>%1").arg(afqt::stoq(af::toKMG(int64_t(need_hdd   )*(1<<30), 1<<10)));
		if( need_power  > 0 ) str_right_top += QString(" Pow>%1").arg( need_power);
		if( multihost )
		{
			str_right_top += QString(" MH(%1,%2)").arg( multihost_min).arg( multihost_max);
			if( multihost_samemaster) str_right_top += 'S';
			if( multihost_waitmax) str_right_top += QString(":%1WM").arg(multihost_waitmax);
			if( multihost_waitsrv) str_right_top += QString(":%1WS").arg(multihost_waitsrv);
		}

		if (skip_existing_files || check_rendered_files)
		{
			str_right_top += " F";
			if (skip_existing_files)
				str_right_top += "Skp";
			if ((filesize_min > 0) || (filesize_max > 0))
			{
				str_right_top += QString("(");
				if (filesize_min > 0)
					str_right_top += afqt::stoq(af::toKMG(filesize_min));
				str_right_top += QString(",");
				if (filesize_max > 0)
					str_right_top += afqt::stoq(af::toKMG(filesize_max));
				str_right_top += QString(")");
			}
			if (check_rendered_files)
				str_right_top += "Chk";
		}

		str_right_top += " Cap[";
		if( varcapacity   ) str_right_top += QString("(%1-%2)*").arg( capcoeff_min).arg( capcoeff_max);
		str_right_top += QString("%1]").arg( capacity);
		
		if( task_progress_change_timeout != -1) str_right_top += QString(" NoProgress:%1").arg(af::time2strHMS( task_progress_change_timeout, true).c_str());
	}
	else
	{
		if ((p_tasks_done - p_tasks_skipped) > 0) str_right_top += QString(" rt:s%1/a%2")
			.arg(af::time2strHMS(p_tasks_sumruntime, true).c_str())
			.arg(af::time2strHMS(p_tasks_sumruntime/(p_tasks_done - p_tasks_skipped), true).c_str());

		if(( errors_avoid_host >= 0 ) || ( errors_task_same_host >= 0 ) || ( errors_retries >= 0 ))
			str_right_top += Item::generateErrorsSolvingInfo( errors_avoid_host, errors_task_same_host, errors_retries);
		if( errors_forgive_time >= 0 ) str_right_top += QString(" f%1").arg( af::time2strHMS( errors_forgive_time, true).c_str());

		if(task_max_run_time > 0) str_right_top += QString(" Mrt%1").arg(af::time2strHMS(task_max_run_time, true).c_str());
		if(task_min_run_time > 0) str_right_top += QString(" mrt%1").arg(af::time2strHMS(task_min_run_time, true).c_str());

		if( max_running_tasks    != -1 ) str_right_top += QString(" m%1").arg( max_running_tasks);
		if( max_running_tasks_per_host != -1 ) str_right_top += QString(" mph%1").arg( max_running_tasks_per_host);
		if( false == hosts_mask.isEmpty()          ) str_right_top += QString(" h(%1)").arg( hosts_mask         );
		if( false == hosts_mask_exclude.isEmpty()  ) str_right_top += QString(" e(%1)").arg( hosts_mask_exclude );
		if( false == need_properties.isEmpty()    ) str_right_top += QString(" p(%1)").arg( need_properties   );
		if (need_memory > 0) str_right_top += QString(" m>%1").arg(afqt::stoq(af::toKMG(int64_t(need_memory)*(1<<20), 1<<10)));
		if (need_gpu_mem_mb > 0) str_right_top += QString(" gm>%1").arg(afqt::stoq(af::toKMG(int64_t(need_gpu_mem_mb)*1000000, 1000)));
		if (need_cpu_freq_mgz > 0) str_right_top += QString(" ghz>%1").arg(afqt::stoq(af::toKMG(int64_t(need_cpu_freq_mgz)*1000000, 1000)));
		if (need_cpu_cores > 0) str_right_top += QString(" cr>%1").arg(need_cpu_cores);
		if (need_cpu_freq_cores > 0) str_right_top += QString(" cr*ghz>%1").arg(afqt::stoq(af::toKMG(int64_t(need_cpu_freq_cores)*1000000, 1000)));
		if (need_hdd    > 0) str_right_top += QString(" h>%1").arg(afqt::stoq(af::toKMG(int64_t(need_hdd   )*(1<<30), 1<<10)));
		if( need_power  > 0 ) str_right_top += QString(" p>%1").arg( need_power);
		if( multihost )
		{
			str_right_top += QString(" mh(%1,%2)").arg( multihost_min).arg( multihost_max);
			if( multihost_samemaster) str_right_top += 's';
			if( multihost_waitmax) str_right_top += QString(":%1wm").arg(multihost_waitmax);
			if( multihost_waitsrv) str_right_top += QString(":%1ws").arg(multihost_waitsrv);
		}

		if (skip_existing_files || check_rendered_files)
		{
			str_right_top += " f";
			if (skip_existing_files)
				str_right_top += "s";
			if ((filesize_min > 0) || (filesize_max > 0))
			{
				str_right_top += QString("(");
				if (filesize_min > 0)
					str_right_top += afqt::stoq(af::toKMG(filesize_min));
				str_right_top += QString(",");
				if (filesize_max > 0)
					str_right_top += afqt::stoq(af::toKMG(filesize_max));
				str_right_top += QString(")");
			}
			if (check_rendered_files)
				str_right_top += "c";
		}

		str_right_top += " [";
		if( varcapacity   ) str_right_top += QString("(%1-%2)*").arg( capcoeff_min).arg( capcoeff_max);
		str_right_top += QString("%1]").arg( capacity);
		
		if( task_progress_change_timeout != -1) str_right_top += QString(" tpt%1").arg(af::time2strHMS( task_progress_change_timeout, true).c_str());
	}


	// Progress:
	str_left_bottom = QString::number( p_percentage) + "%";
	if( Watch::isPadawan())
	{
		if (p_tasks_running ) str_left_bottom += QString(" Running:%1"         ).arg(p_tasks_running);
		if (p_capacity_total) str_left_bottom += QString(" Capacity:%1"        ).arg(af::toKMG(p_capacity_total).c_str());
		if (p_tasks_done    ) str_left_bottom += QString(" Done:%1"            ).arg(p_tasks_done);
		if (p_tasks_error   ) str_left_bottom += QString(" Errors:%1"          ).arg(p_tasks_error);
		if (p_tasks_skipped ) str_left_bottom += QString(" Skipped:%1"         ).arg(p_tasks_skipped);
		if (p_tasks_waitdep ) str_left_bottom += QString(" WaitDepends:%1"     ).arg(p_tasks_waitdep);
		if (p_tasks_warning ) str_left_bottom += QString(" Warnings:%1"        ).arg(p_tasks_warning);
		if (p_tasks_waitrec ) str_left_bottom += QString(" WaitingReconnect:%1").arg(p_tasks_waitrec);
	}
	else if( Watch::isJedi())
	{
		if (p_tasks_running ) str_left_bottom += QString(" Run:%1" ).arg(p_tasks_running);
		if (p_capacity_total) str_left_bottom += QString(" Cap:%1" ).arg(af::toKMG(p_capacity_total).c_str());
		if (p_tasks_done    ) str_left_bottom += QString(" Done:%1").arg(p_tasks_done);
		if (p_tasks_error   ) str_left_bottom += QString(" Err:%1" ).arg(p_tasks_error);
		if (p_tasks_skipped ) str_left_bottom += QString(" Skp:%1" ).arg(p_tasks_skipped);
		if (p_tasks_waitdep ) str_left_bottom += QString(" WDP:%1" ).arg(p_tasks_waitdep);
		if (p_tasks_warning ) str_left_bottom += QString(" Wrn:%1" ).arg(p_tasks_warning);
		if (p_tasks_waitrec ) str_left_bottom += QString(" WRC:%1" ).arg(p_tasks_waitrec);
	}
	else
	{
		if (p_tasks_running ) str_left_bottom += QString(" r%1"  ).arg(p_tasks_running);
		if (p_capacity_total) str_left_bottom += QString(" c%1"  ).arg(af::toKMG(p_capacity_total).c_str());
		if (p_tasks_done    ) str_left_bottom += QString(" d%1"  ).arg(p_tasks_done);
		if (p_tasks_error   ) str_left_bottom += QString(" e%1"  ).arg(p_tasks_error);
		if (p_tasks_skipped ) str_left_bottom += QString(" s%1"  ).arg(p_tasks_skipped);
		if (p_tasks_waitdep ) str_left_bottom += QString(" wdp%1").arg(p_tasks_waitdep);
		if (p_tasks_warning ) str_left_bottom += QString(" w%1"  ).arg(p_tasks_warning);
		if (p_tasks_waitrec ) str_left_bottom += QString(" wrc%1").arg(p_tasks_waitrec);
	}

	if (server_info.size()) str_left_bottom += QString(" %1").arg(server_info);

	if (m_jobid == AFJOB::SYSJOB_ID ) str_left_bottom += QString(" Ready:%1").arg( p_tasks_ready);


	// Right bottom: server info, errors&avoids
	str_right_bottom.clear();
	if(Watch::isPadawan())
	{
		if(p_error_hosts) str_right_bottom += QString(" Error Hosts:%1").arg(p_error_hosts);
		if(p_avoid_hosts) str_right_bottom += QString("/%1:Avoiding").arg(p_avoid_hosts);
	}
	else if(Watch::isJedi())
	{
		if(p_error_hosts) str_right_bottom += QString(" ErrHosts:%1").arg(p_error_hosts);
		if(p_avoid_hosts) str_right_bottom += QString("/%1:Avoid").arg(p_avoid_hosts);
	}
	else
	{
		if(p_error_hosts) str_right_bottom += QString(" eh:%1").arg(p_error_hosts);
		if(p_avoid_hosts) str_right_bottom += QString("/%1:a").arg(p_avoid_hosts);
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
			xoffset += 48;
		}
	}


	// Paint border:
	i_painter->setPen(afqt::QEnvironment::clr_outline.c );
	i_painter->setBrush(Qt::NoBrush);
	i_painter->drawRoundedRect(x+xoffset-3, y-1, w+4-xoffset, i_compact_display ? HeightCompact : Height, 3, 3);


	// Setup font size and color:
	i_painter->setFont(afqt::QEnvironment::f_info);
	QPen pen(Item::clrTextInfo(p_tasks_running, option.state & QStyle::State_Selected, m_item->isLocked()));

	// Draw tickets:
	int tk_w = 0;
	int tk_y = i_compact_display ? y+y_info : y+y_progress;
	QMapIterator<QString, int> it(tickets);
	while (it.hasNext())
	{
		it.next();

		tk_w += Item::drawTicket(i_painter, pen, x+5 + xoffset, tk_y, w-10 - xoffset - tk_w, Item::HeightTickets - 5,
				Item::TKD_RIGHT,
				it.key(), it.value());

		tk_w += 8;
	}

	i_painter->setPen(pen);

	// Paint parameters:
	QRect rect_params;
	i_painter->drawText(x, y+y_info, w-5-(i_compact_display?tk_w:0), 15, Qt::AlignRight | Qt::AlignTop, str_right_top, &rect_params);

	// Paint general information:
	i_painter->drawText(x+xoffset+5, y+y_info, w-rect_params.width()-10-xoffset, 15, Qt::AlignLeft | Qt::AlignTop, str_left_top);


	// On compact display we dont show progress, error hosts and progress bars.
	if (i_compact_display)
		return;


	// Paint error hosts:
	int right_bottom_text_width = 0;
	if (str_right_bottom.size())
	{
		if (p_avoid_hosts)
			i_painter->setPen(afqt::QEnvironment::clr_error.c);
		else if (p_error_hosts)
			i_painter->setPen(afqt::QEnvironment::clr_errorready.c);

		QRect rect;
		i_painter->drawText(x+xoffset+5, y+y_progress, w-10-xoffset-tk_w, 15, Qt::AlignRight | Qt::AlignTop, str_right_bottom, &rect);
		right_bottom_text_width = rect.width() + 10;
	}


	// Paint progress:
	i_painter->setPen(pen);
	i_painter->drawText(x+xoffset+5, y+y_progress, w-15-xoffset-right_bottom_text_width, 15, Qt::AlignLeft | Qt::AlignTop, str_left_bottom);


	xoffset += 2;
	// Paint progress bars:
	Item::drawPercent
	(
		i_painter, x+xoffset, y+y_bars, w-xoffset-2, 4,
		m_jobid == AFJOB::SYSJOB_ID ? p_tasks_running + p_tasks_ready + p_tasks_error : tasksnum,
		m_jobid == AFJOB::SYSJOB_ID ? 0 : p_tasks_done, p_tasks_error, p_tasks_running,
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
		case 'T': // STATE_TRYTHISTASKNEXT_MASK
			i_painter->setBrush(QBrush(afqt::QEnvironment::clr_tasktrynext.c, Qt::SolidPattern));
			i_painter->drawRect(x, posy, w+offset, height);
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
		case 'N': // STATE_RUNNING_MASK | STATE_WARNING_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_taskwarningrun.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'G': // STATE_DONE_MASK | STATE_WARNING_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_taskwarningdone.c, Qt::SolidPattern ));
			i_painter->drawRect( x, posy, w+offset, height);
			break;
		case 'U': // STATE_SUSPENDED_MASK | STATE_WAITDEP_MASK
			i_painter->setBrush( QBrush( afqt::QEnvironment::clr_tasksuspended.c, Qt::SolidPattern ));
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
		"restart_errors",    "Restart Errors",
		"restart_running",   "Restart Running",
		"restart_skipped",   "Restart Skipped",
		"restart_done",      "Restart Done",
		"continue",          "Continue Suspended"};

	for (int i = 0; i < operations.size(); i+=2)
	{
		ActionString * action = new ActionString(operations[i], operations[i+1], m_listitems);
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

void BlockInfo::addParam_MSS(const QString & i_name, const QString & i_label, const QString & i_tip) {
	m_params.append(new Param(Param::TMSS, Item::TAny, i_name, i_label, i_tip));}

void BlockInfo::addParam_REx(const QString & i_name, const QString & i_label, const QString & i_tip) {
	m_params.append(new Param(Param::TREx, Item::TAny, i_name, i_label, i_tip));}

void BlockInfo::addParam_Hrs(const QString & i_name, const QString & i_label, const QString & i_tip) {
	m_params.append(new Param(Param::THrs, Item::TAny, i_name, i_label, i_tip));}

void BlockInfo::addParam_MiB(const QString & i_name, const QString & i_label, const QString & i_tip, int i_min, int i_max) {
	m_params.append(new Param(Param::TMiB, Item::TAny, i_name, i_label, i_tip, i_min, i_max));}

void BlockInfo::addParam_GiB(const QString & i_name, const QString & i_label, const QString & i_tip, int i_min, int i_max) {
	m_params.append(new Param(Param::TGiB, Item::TAny, i_name, i_label, i_tip, i_min, i_max));}

void BlockInfo::addParam_Meg(const QString & i_name, const QString & i_label, const QString & i_tip, int i_min, int i_max) {
	m_params.append(new Param(Param::TMeg, Item::TAny, i_name, i_label, i_tip, i_min, i_max));}

