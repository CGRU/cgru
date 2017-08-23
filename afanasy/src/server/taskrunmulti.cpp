#include "taskrunmulti.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/taskexec.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msg.h"

#include "block.h"
#include "jobaf.h"
#include "renderaf.h"
#include "rendercontainer.h"
#include "monitorcontainer.h"
#include "task.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

TaskRunMulti::TaskRunMulti( Task * i_runningTask,
						af::TaskExec* i_taskExec,
						af::TaskProgress * i_taskProgress,
						Block * i_taskBlock,
						RenderAf * i_render,
						MonitorContainer * i_monitoring,
						int32_t * io_running_tasks_counter,
						int64_t * io_running_capacity_counter
						):
	TaskRun( i_runningTask,
			NULL,    ///< SET NO EXECUTABLE! It will be set before starting master.
			i_taskProgress,
			i_taskBlock,
			i_render,
			i_monitoring,
			io_running_tasks_counter,
			io_running_capacity_counter
		),
	m_master_running( false),
	m_stopping( false),
	m_time_last_host_added(0),
	m_time_services_started(0),
	m_time_services_stopped(0)
{
	if( i_taskExec == NULL)
	{
		AFERRAR("TaskRunMulti::TaskRunMulti: %s[%d] Task executable is NULL.",
			m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum())
		return;
	}

	// Increase capacity counter in multihost task class,
	// as TaskRun parent class do not have tasks exec here, so have no capacity info.
	(*m_running_capacity_counter) += i_taskExec->getCapResult();

	m_has_service = ( m_block->m_data->getMultiHostService().empty() == false);
	m_tasknum = i_taskExec->getTaskNum();
	m_task->v_appendLog("Starting to capture hosts:");
	m_progress->state = AFJOB::STATE_RUNNING_MASK | AFJOB::STATE_READY_MASK;
	addHost( i_taskExec, i_render, i_monitoring);
}

TaskRunMulti::~TaskRunMulti()
{
AFINFA("TaskRunMulti:: ~ TaskRunMulti: %s[%d][%d]:", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
	if( m_execs.size() != 0)
	{
		AFERRAR("TaskRunMulti:: ~ TaskRunMulti: %s[%d][%d]:",
			m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
	}
	for( std::list<af::TaskExec*>::iterator it = m_execs.begin(); it != m_execs.end(); it++) delete *it;
	m_progress->hostname = m_master_hostname;
}

int TaskRunMulti::calcWeight() const
{
	int weight = sizeof( TaskRunMulti) - sizeof(TaskRun);
	weight += TaskRun::calcWeight();
	return weight;
}

void TaskRunMulti::addHost( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring)
{
	m_task->v_appendLog( std::string("Host \"") + render->getName() + "\" added.");

	// Getting unique minimal slave number
	int number = 0;
	for(;;)
	{
		bool numberexists = false;
		for( std::list<af::TaskExec*>::const_iterator it = m_execs.begin(); it != m_execs.end(); it++)
		{
			if( number == (*it)->getNumber())
			{
				number++;
				numberexists = true;
				break;
			}
			numberexists = false;
		}
		if( numberexists == false) break;
	}
	taskexec->setNumber( number);

	// Append lists
	m_execs.push_back( taskexec);
	m_hostids.push_back( render->getId());
	m_hostnames.push_back( render->getName());

	// Set active slaves list for monitoring
	setProgressHostame();
//	m_progress->hostname = af::strJoin( m_hostnames, " ");

	// Setting task on slave, but do not starting it
	render->setTask( taskexec, monitoring, false);

	m_task->v_monitor( monitoring );
	m_task->v_store();
	m_time_last_host_added = time( NULL);

	// Setting task not be ready to take any hosts if their quantity is enough
	if( (int)m_execs.size() >= m_block->m_data->getMultiHostMax()) m_progress->state = m_progress->state & (~AFJOB::STATE_READY_MASK);
}

void TaskRunMulti::setMasterTask()
{
	if( isZombie()) return;
	m_exec = m_execs.front();
	m_hostId = m_hostids.front();
	m_execs.pop_front();
	m_hostids.pop_front();
	m_hostnames.pop_front();
}

void TaskRunMulti::startServices( RenderContainer * renders)
{
	m_time_services_started = time( NULL);
	if( m_has_service == false) return;

	m_task->v_appendLog("Starting services on slave hosts.");
	RenderContainerIt rendersIt( renders);
	std::list<int>::iterator hIt = m_hostids.begin();
	std::list<af::TaskExec*>::iterator tIt = m_execs.begin();
	std::list<af::TaskExec*>::const_iterator end = m_execs.end();
	for( ; tIt != end; hIt++, tIt++)
	{
		RenderAf * render = rendersIt.getRender( *hIt);
		if( render == NULL) continue;

		(*tIt)->setCommand( m_block->m_data->getMultiHostService());
		render->startTask(*tIt);
	}
}

void TaskRunMulti::startMaster( RenderContainer * renders, MonitorContainer * monitoring)
{
	RenderContainerIt rendersIt( renders);
	RenderAf * render = rendersIt.getRender( m_hostId);
	if( NULL == render)
	{
		AFERRAR("TaskRunMulti::startMaster: %s[%d][%d] Render id=%d is NULL.",
			m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum, m_hostId)
		return;
	}

	// Add a master to slaves list if needed
	if( m_block->m_data->canMasterRunOnSlaveHost()) m_hostnames.push_back( render->getName());

	m_exec->setHostNames( m_hostnames);

	m_master_hostname = render->getName();
	m_progress->state = AFJOB::STATE_RUNNING_MASK;
	m_progress->starts_count++;
	m_progress->time_start = time( NULL);
	m_progress->time_done = m_progress->time_start;

	render->startTask( m_exec);

	m_master_running = true;

	setProgressHostame();

	m_task->v_monitor( monitoring );
	m_task->v_store();
	m_task->v_appendLog( std::string("Starting master on \"") + render->getName() + "\"");
}

void TaskRunMulti::setProgressHostame()
{
	std::string str;
	if( m_master_running )
		str = m_master_hostname + ": ";
	str += af::strJoin( m_hostnames, " ");

	m_progress->hostname = str;
}

void TaskRunMulti::update( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost)
{
//printf("TaskRunMulti::update: [%d][%d][%d](%d):\n", taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumber());

	//
	// If this is a master update:
	if( m_exec && m_master_running )
		if( m_exec->equals( taskup))
		{
			TaskRun::update( taskup, renders, monitoring, errorHost);
			return;
		}

	//
	// This is an update from slave:

	if( isZombie() )
	{
		AFERRAR("TaskRunMulti::update: ZOMBIE %s[%d][%d]",
			m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
		return;
	}
	if((m_progress->state & AFJOB::STATE_RUNNING_MASK) == false)
	{
		AFERRAR("TaskRunMulti::update: NOT RUNNING %s[%d][%d] task is not running.",
			m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
		return;
	}

	switch ( taskup.getStatus())
	{
	case af::TaskExec::UPPercent:
	case af::TaskExec::UPStarted:
	case af::TaskExec::UPNULL:
	case af::TaskExec::UPNoTaskRunning:
	case af::TaskExec::UPNoJob:
	case af::TaskExec::UPLAST:
		return;

	case af::TaskExec::UPFinishedSuccess:
		m_task->v_appendLog("Slave host service finished.");
		releaseHost( renders, monitoring, &taskup );
		if( m_stopping == false) errorHost = true;
		break;

	case af::TaskExec::UPFailedToStart:
		m_task->v_appendLog("Failed to start slave host service.");
		releaseHost( renders, monitoring, &taskup );
		errorHost = true;
		break;

	case af::TaskExec::UPFinishedKilled:
		m_task->v_appendLog("Slave host service was killed.");
		releaseHost( renders, monitoring, &taskup );
		if( m_stopping == false) errorHost = true;
		break;

	case af::TaskExec::UPFinishedError:
		m_task->v_appendLog("Slave host service finished with error.");
		releaseHost( renders, monitoring, &taskup );
		if( m_stopping == false) errorHost = true;
		break;

	case af::TaskExec::UPEject:
		m_task->v_appendLog("Host owner ejected slave.");
		if( m_has_service && m_time_services_started )
		{
			stopSlaveService( renders, monitoring, taskup);
			return;
		}
		else
		{
			// There is no service, so there is nothing to stop.
			// We can just stop the entire task, but we should not do it in this case.
			// So lets just ignore this at all.
			m_task->v_appendLog("Ignoring, master task should be stopped to free slaves.");
/*
			// Or we should do this to restart the entire task:

			// We can't free just one slave.
			// We should restart master task without this slave:
			if( m_time_services_started && ( m_stopping == false ))
				stop("Slave eject request from host owner.\nRrestarting entire task.",
					renders, monitoring);
*/
			return;
		}
		break;

	case af::TaskExec::UPRenderDeregister:
		m_task->v_appendLog("Slave deregistered.");
		releaseHost( renders, monitoring, &taskup );
		break;

	case af::TaskExec::UPRenderExit:
		m_task->v_appendLog("Slave exited.");
		releaseHost( renders, monitoring, &taskup );
		break;

	case af::TaskExec::UPRenderZombie:
		m_task->v_appendLog("Slave became a zombie.");
		releaseHost( renders, monitoring, &taskup );
		errorHost = true;
		break;

	default:
		AFERRAR("TaskRun::updateState: %s[%d][%d]: Unknown task update status = %d",
			m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum, taskup.getStatus())
		return;
	}



	if( m_time_services_started && ( m_stopping == false ))
	{
		// We can ignore slave lost:
		if( m_block->m_data->isSlaveLostIgnore())
		{
			int numslaves = m_execs.size();
			if( m_block->m_data->canMasterRunOnSlaveHost())
				numslaves += 1;

			// Ignore host lost if there is at least 1 slave
			if( numslaves )
			{
				m_task->v_appendLog("Ignoring slave lost, current slaves count = " + af::itos( numslaves));
				return;
			}
		}

		stop("Removing slave host while task is running.\nRrestarting entire task.",
			renders, monitoring);
	}
}

bool TaskRunMulti::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
	bool changed = false;

	// There is no need to do something with zombie. It soon will be deleted.
	if( isZombie() ) return changed;

	// Slaves service stop timeout check:
	if( m_time_services_started && m_time_services_stopped &&( currentTime - m_time_services_stopped > AFJOB::TASK_STOP_TIMEOUT ))
	{
		m_task->v_appendLog("Service stop timeout.");
		releaseHost( renders, monitoring);
		if( changed == false) changed = true;
	}

	// Start services and master if time and hosts are enough
	if((m_master_running == false) && (m_stopping == false))
	{
		if( m_time_services_started == false)
			if(( (int)m_execs.size() >= m_block->m_data->getMultiHostMax()) ||
			  (( (int)m_execs.size() >= m_block->m_data->getMultiHostMin()) &&
				(currentTime-m_time_last_host_added >= m_block->m_data->getMultiHostWaitMax())))
			{
				if( m_progress->state & AFJOB::STATE_READY_MASK) m_progress->state = m_progress->state & (~AFJOB::STATE_READY_MASK);
				setMasterTask();
				m_task->v_appendLog( std::string("Finished waiting for hosts.\nCaptured slaves:\n") + af::strJoin( m_hostnames, " "));
				startServices( renders);
			}

		if( m_time_services_started )
			if((m_block->m_data->getMultiHostWaitSrv() == 0) ||
				(currentTime - m_time_services_started > m_block->m_data->getMultiHostWaitSrv()))
			{
				startMaster( renders, monitoring);
			}
	}

	if( m_master_running == false) return changed;

	if( TaskRun::refresh( currentTime, renders, monitoring, errorHostId)) changed = true;

	return changed;
}

void TaskRunMulti::stop( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("TaskRunMulti::stop: %s[%d][%d]\n\t%s\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, message.toUtf8().data());

	m_stopping = true;
	m_task->v_appendLog( message);

	// Set task not to be ready to get any hosts
	if( m_progress->state & AFJOB::STATE_READY_MASK) m_progress->state = m_progress->state & (~AFJOB::STATE_READY_MASK);

	RenderContainerIt rendersIt( renders);
	std::list<int>::iterator hIt = m_hostids.begin();
	std::list<af::TaskExec*>::iterator tIt = m_execs.begin();
	std::list<af::TaskExec*>::const_iterator end = m_execs.end();
	while( tIt != end)
	{
		RenderAf * render = rendersIt.getRender( *hIt);
		if( render == NULL) continue;

		if( m_has_service && m_time_services_started)
		{
			if( m_time_services_stopped )
			{
				AFERRAR("TaskRunMulti::stop: %s[%d][%d] Services already asked to be stopped.",
					m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_tasknum)
				return;
			}
			// Stopping service on slaves (if it was not asked to be stopped before)
			m_task->v_appendLog( std::string("Stopping service[") + af::itos((*tIt)->getNumber()) +
					"] on host \"" + render->getName() + "\"");
			render->stopTask( *tIt);
			hIt++, tIt++;
		}
		else
		{
			// Finish tasks on slaves if there is no service
			render->taskFinished( *tIt, monitoring);
			m_task->v_appendLog( std::string("Finished task[") + af::itos((*tIt)->getNumber()) +
					"] on host \"" + render->getName() + "\"");
			delete *tIt;
			tIt = m_execs.erase( tIt);
			hIt = m_hostids.erase( hIt);
		}
	}

	m_time_services_stopped = time( NULL);

	if( m_master_running)
	{
		// Stop master if it is running
		TaskRun::stop( "Stopping running master.", renders, monitoring);
	}
	else if( m_execs.size() == 0)
	{
		// Finish master if it was not running and there are no service on slaves
		TaskRun::finish( "Finish master task.", renders, monitoring);
	}
}

void TaskRunMulti::stopSlaveService(  RenderContainer * renders, MonitorContainer * monitoring, const af::MCTaskUp & taskup)
{
	RenderContainerIt rendersIt( renders);
	std::list<int>::iterator hIt = m_hostids.begin();
	std::list<af::TaskExec*>::iterator tIt = m_execs.begin();
	std::list<af::TaskExec*>::const_iterator end = m_execs.end();
	for( ; tIt != end; hIt++, tIt++)
	{
		if( false == (*tIt)->equals( taskup))
			continue;

		RenderAf * render = rendersIt.getRender( *hIt);
		if( render == NULL)
		{
			AFERROR("TaskRunMulti::stopSlaveService: no such render.")
		}
		else
		{
			render->stopTask( *tIt);
		}
		break;
	}
}

void TaskRunMulti::finish( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("TaskRunMulti::finish: %s[%d][%d]\n\t%s\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, message.toUtf8().data());

	m_master_running = false;
	if( m_stopping == false ) stop( message, renders, monitoring);
	else if( m_execs.size() == 0)
	{
		// Finish master if it was not running and there are no service on slaves
		TaskRun::finish( message, renders, monitoring);
	}
}

void TaskRunMulti::releaseHost( RenderContainer * renders, MonitorContainer * monitoring, const af::MCTaskUp * taskup)
{
//printf("TaskRunMulti::releaseHost: %s[%d][%d](%d)\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, taskup ? taskup->getNumber() : -1);

	RenderContainerIt rendersIt( renders);
	std::list<int>::iterator hIt = m_hostids.begin();
	std::list<af::TaskExec*>::iterator tIt = m_execs.begin();
	std::list<af::TaskExec*>::const_iterator end = m_execs.end();
	std::list<std::string>::iterator nIt = m_hostnames.begin();
	for( ; tIt != end; hIt++, tIt++, nIt++)
	{
		if(( taskup != NULL) && ((*tIt)->equals( *taskup) == false)) continue;

		RenderAf * render = rendersIt.getRender(*hIt);
		if( render == NULL)
		{
			AFERRAR("TaskRunMulti::releaseHost: Render[%d] is NULL %s[%d][%d](%d)",
			  *hIt, (*tIt)->getJobName().c_str(), (*tIt)->getBlockNum(), (*tIt)->getTaskNum(), (*tIt)->getNumber())
		}
		else
		{
			render->taskFinished( *tIt, monitoring);
			m_task->v_appendLog( std::string("Releasing task[") + af::itos((*tIt)->getNumber()) +
				"] on host \"" + render->getName() + "\"");
		}
		delete *tIt;
		tIt = m_execs.erase( tIt);
		hIt = m_hostids.erase( hIt);
		nIt = m_hostnames.erase( nIt);
		setProgressHostame();
		if( m_stopping && (m_master_running == false) && (m_execs.size() == 0))
			TaskRun::finish( "Finish: All slaves and master finished.", renders, monitoring);
		m_task->v_monitor( monitoring );

		// Finish stopping concrete task.
		if( taskup ) return;
	}

	if( taskup )
	{
		// Concrete task not fouded.
		AFERROR("TaskRunMulti::releaseHost: No such task:")
		taskup->v_stdOut( false);
		printf("Available tasks:\n");
		stdOut( false);
	}
}

int TaskRunMulti::v_getRunningRenderID( std::string & o_error) const
{
	if( m_progress->state & AFJOB::STATE_READY_MASK)
	{
		o_error = "Waiting for hosts.";
		return 0;
	}
	return TaskRun::v_getRunningRenderID( o_error);
}

void TaskRunMulti::stdOut( bool full) const
{
	if( m_has_service ) printf("HasSrv, ");
	if( m_master_running ) printf("MRun, ");
	if( m_stopping ) printf("Stp, ");
	printf("LHA=%d, SStarted=%d, SStopped=%d", m_time_last_host_added, m_time_services_started, m_time_services_stopped);
	printf("\n");
	std::list<int>::const_iterator hIt = m_hostids.begin();
	std::list<af::TaskExec*>::const_iterator tIt = m_execs.begin();
	std::list<af::TaskExec*>::const_iterator end = m_execs.end();
	std::list<std::string>::const_iterator nIt = m_hostnames.begin();
	for( ; tIt != end; hIt++, tIt++, nIt++)
	{
		if( full ) (*tIt)->v_stdOut( false);
		else printf("Exec=[%d][%d][%d](%d) host='%s'[%d]\n",
			(*tIt)->getJobId(), (*tIt)->getBlockNum(), (*tIt)->getTaskNum(), (*tIt)->getNumber(), (*nIt).c_str(), *hIt);
	}
}
