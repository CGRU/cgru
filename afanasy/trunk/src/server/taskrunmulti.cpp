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

TaskRunMulti::TaskRunMulti( Task * runningTask,
                  af::TaskExec* taskExec,
                  af::TaskProgress * taskProgress,
                  Block * taskBlock,
                  RenderAf * render,
                  MonitorContainer * monitoring,
                  int * runningtaskscounter
                  ):
   TaskRun( runningTask,
            NULL,    ///< SET NO EXECUTABLE! It will be set before starting master.
            taskProgress,
            taskBlock,
            render,
            monitoring,
            runningtaskscounter
            ),
   masterrunning( false),
   stopping( false),
   time_lasthostadded(0),
   time_servicestarted(0),
   time_servicestopped(0)
{
   if( taskExec == NULL)
   {
      AFERRAR("TaskRunMulti::TaskRunMulti: %s[%d] Task executable is NULL.", block->job->getName().c_str(), block->data->getBlockNum())
      return;
   }
   hasservice = ( block->data->getMultiHostService().empty() == false);
   tasknum = taskExec->getTaskNum();
   task->appendLog("Starting to capture hosts:");
   progress->state = AFJOB::STATE_RUNNING_MASK | AFJOB::STATE_READY_MASK;
   addHost( taskExec, render, monitoring);
}

TaskRunMulti::~TaskRunMulti()
{
AFINFA("TaskRunMulti:: ~ TaskRunMulti: %s[%d][%d]:", block->job->getName().c_str(), block->data->getBlockNum(), tasknum)
   if( execs.size() != 0)
   {
      AFERRAR("TaskRunMulti:: ~ TaskRunMulti: %s[%d][%d]:", block->job->getName().c_str(), block->data->getBlockNum(), tasknum)
   }
   for( std::list<af::TaskExec*>::iterator it = execs.begin(); it != execs.end(); it++) delete *it;
   progress->hostname = hostnamemaster;
}

int TaskRunMulti::calcWeight() const
{
   int weight = sizeof( TaskRunMulti) - sizeof(TaskRun);
   weight += TaskRun::calcWeight();
   return weight;
}

void TaskRunMulti::addHost( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring)
{
   task->appendLog( std::string("Host \"") + render->getName() + "\" added.");

   // Getting unique minimal slave number
   int number = 0;
   for(;;)
   {
      bool numberexists = false;
      for( std::list<af::TaskExec*>::const_iterator it = execs.begin(); it != execs.end(); it++)
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
   execs.push_back( taskexec);
   hostids.push_back( render->getId());
   hostnames.push_back( render->getName());

   // Set active slaves list for monitoring
   progress->hostname = af::strJoin( hostnames, " ");

   // Setting task on slave, but do not starting it
   render->setTask( taskexec, monitoring, false);

   task->monitor( monitoring );
   task->updateDatabase();
   time_lasthostadded = time( NULL);

   // Setting task not be ready to take any hosts if their quantity is enough
   if( (int)execs.size() >= block->data->getMultiHostMax()) progress->state = progress->state & (~AFJOB::STATE_READY_MASK);
}

void TaskRunMulti::setMasterTask()
{
   exec = execs.front();
   hostId = hostids.front();
   execs.pop_front();
   hostids.pop_front();
   hostnames.pop_front();
}

void TaskRunMulti::startServices( RenderContainer * renders)
{
   time_servicestarted = time( NULL);
   if( hasservice == false) return;

   task->appendLog("Starting services on slave hosts.");
   RenderContainerIt rendersIt( renders);
   std::list<int>::iterator hIt = hostids.begin();
   std::list<af::TaskExec*>::iterator tIt = execs.begin();
   std::list<af::TaskExec*>::const_iterator end = execs.end();
   for( ; tIt != end; hIt++, tIt++)
   {
      RenderAf * render = rendersIt.getRender( *hIt);
      if( render == NULL) continue;

      (*tIt)->setCommand( block->data->getMultiHostService());
      render->startTask(*tIt);
   }
}

void TaskRunMulti::startMaster( RenderContainer * renders, MonitorContainer * monitoring)
{
   RenderContainerIt rendersIt( renders);
   RenderAf * render = rendersIt.getRender( hostId);
   if( render == NULL)
   {
      AFERRAR("TaskRunMulti::startMaster: %s[%d][%d] Render id=%d is NULL.", block->job->getName().c_str(), block->data->getBlockNum(), tasknum, hostId)
      return;
   }

   // Add a master to slaves list if needed
   if( block->data->canMasterRunOnSlaveHost()) hostnames.push_back( render->getName());

   exec->setHostNames( hostnames);

   hostnamemaster = render->getName();
   progress->state = AFJOB::STATE_RUNNING_MASK;
   progress->starts_count++;
   progress->time_start = time( NULL);
   progress->time_done = progress->time_start;
   progress->hostname = hostnamemaster + ": " + af::strJoin( hostnames, " ");
   render->startTask( exec);
   masterrunning = true;
   task->monitor( monitoring );
   task->updateDatabase();
   task->appendLog( std::string("Starting master on \"") + render->getName() + "\"");
}

void TaskRunMulti::update( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost)
{
//printf("TaskRunMulti::update: [%d][%d][%d](%d):\n", taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumber());

   if( exec && masterrunning )
      if( exec->equals( taskup))
      {
         TaskRun::update( taskup, renders, monitoring, errorHost);
         return;
      }

   if( isZombie() )
   {
      AFERRAR("TaskRunMulti::update: ZOMBIE %s[%d][%d]", block->job->getName().c_str(), block->data->getBlockNum(), tasknum)
      return;
   }
   if((progress->state & AFJOB::STATE_RUNNING_MASK) == false)
   {
      AFERRAR("TaskRunMulti::update: NOT RUNNING %s[%d][%d] task is not running.", block->job->getName().c_str(), block->data->getBlockNum(), tasknum)
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
      task->appendLog("Slave host service finished.");
      releaseHost( renders, monitoring, &taskup );
      if( stopping == false) errorHost = true;
      break;

   case af::TaskExec::UPFailedToStart:
      task->appendLog("Failed to start slave host service.");
      releaseHost( renders, monitoring, &taskup );
      errorHost = true;
      break;

   case af::TaskExec::UPFinishedCrash:
      task->appendLog("Slave host service finished crashed.");
      releaseHost( renders, monitoring, &taskup );
      if( stopping == false) errorHost = true;
      break;

   case af::TaskExec::UPFinishedError:
      task->appendLog("Slave host service finished with error.");
      releaseHost( renders, monitoring, &taskup );
      if( stopping == false) errorHost = true;
      break;

   case af::TaskExec::UPEject:
      if( time_servicestarted == false)
         releaseHost( renders, monitoring, &taskup );
      task->appendLog("Host owner ejected slave.");
      break;

   case af::TaskExec::UPRenderDeregister:
      task->appendLog("Slave deregistered.");
      releaseHost( renders, monitoring, &taskup );
      break;

   case af::TaskExec::UPRenderExit:
      task->appendLog("Slave exited.");
      releaseHost( renders, monitoring, &taskup );
      break;

   case af::TaskExec::UPRenderZombie:
      task->appendLog("Slave became a zombie.");
      releaseHost( renders, monitoring, &taskup );
      errorHost = true;
      break;

   default:
      AFERRAR("TaskRun::updateState: %s[%d][%d]: Unknown task update status = %d", block->job->getName().c_str(), block->data->getBlockNum(), tasknum, taskup.getStatus())
      return;
   }

   if( time_servicestarted && ( stopping == false ))
      stop("Removing slave host while task is running.\nRrestarting entire task.", renders, monitoring);
}

bool TaskRunMulti::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
   bool changed = false;

   // There is no need to do something with zombie. It soon will be deleted.
   if( isZombie() ) return changed;

   // Slaves service stop timeout check:
   if( time_servicestarted && time_servicestopped &&( currentTime - time_servicestopped > AFJOB::TASK_STOP_TIMEOUT ))
   {
      task->appendLog("Service stop timeout.");
      releaseHost( renders, monitoring);
      if( changed == false) changed = true;
   }

   // Start services and master if time and hosts are enough
   if((masterrunning == false) && (stopping == false))
   {
      if( time_servicestarted == false)
         if(( (int)execs.size() >= block->data->getMultiHostMax()) ||
           (( (int)execs.size() >= block->data->getMultiHostMin()) && (currentTime-time_lasthostadded >= block->data->getMultiHostWaitMax())))
         {
            if( progress->state & AFJOB::STATE_READY_MASK) progress->state = progress->state & (~AFJOB::STATE_READY_MASK);
            setMasterTask();
            task->appendLog( std::string("Finished waiting for hosts.\nCaptured slaves:\n") + af::strJoin( hostnames, " "));
            startServices( renders);
         }

      if( time_servicestarted )
         if((block->data->getMultiHostWaitSrv() == 0) ||
            (currentTime - time_servicestarted > block->data->getMultiHostWaitSrv()))
         {
            startMaster( renders, monitoring);
         }
   }

   if( masterrunning == false) return changed;

   if( TaskRun::refresh( currentTime, renders, monitoring, errorHostId)) changed = true;

   return changed;
}

void TaskRunMulti::stop( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("TaskRunMulti::stop: %s[%d][%d]\n\t%s\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, message.toUtf8().data());

   stopping = true;
   task->appendLog( message);

   // Set task not to be ready to get any hosts
   if( progress->state & AFJOB::STATE_READY_MASK) progress->state = progress->state & (~AFJOB::STATE_READY_MASK);

   RenderContainerIt rendersIt( renders);
   std::list<int>::iterator hIt = hostids.begin();
   std::list<af::TaskExec*>::iterator tIt = execs.begin();
   std::list<af::TaskExec*>::const_iterator end = execs.end();
   while( tIt != end)
   {
      RenderAf * render = rendersIt.getRender( *hIt);
      if( render == NULL) continue;

      if( hasservice && time_servicestarted)
      {
         if( time_servicestopped )
         {
            AFERRAR("TaskRunMulti::stop: %s[%d][%d] Services already asked to be stopped.", block->job->getName().c_str(), block->data->getBlockNum(), tasknum)
            return;
         }
         // Stopping service on slaves (if it was not asked to be stopped before)
         task->appendLog( std::string("Stopping service[") + af::itos((*tIt)->getNumber()) + "] on host \"" + render->getName() + "\"");
         render->stopTask( *tIt);
         hIt++, tIt++;
      }
      else
      {
         // Finish tasks on slaves if there is no service
         render->taskFinished( *tIt, monitoring);
         task->appendLog( std::string("Finished task[") + af::itos((*tIt)->getNumber()) + "] on host \"" + render->getName() + "\"");
         delete *tIt;
         tIt = execs.erase( tIt);
         hIt = hostids.erase( hIt);
      }
   }

   time_servicestopped = time( NULL);

   if( masterrunning)
   {
      // Stop master if it is running
      TaskRun::stop( "Stopping running master.", renders, monitoring);
   }
   else if( execs.size() == 0)
   {
      // Finish master if it was not running and there are no service on slaves
      TaskRun::finish( "Finish master task.", renders, monitoring);
   }
}

void TaskRunMulti::finish( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("TaskRunMulti::finish: %s[%d][%d]\n\t%s\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, message.toUtf8().data());

   masterrunning = false;
   if( stopping == false ) stop( message, renders, monitoring);
   else if( execs.size() == 0)
   {
      // Finish master if it was not running and there are no service on slaves
      TaskRun::finish( message, renders, monitoring);
   }
}

void TaskRunMulti::releaseHost( RenderContainer * renders, MonitorContainer * monitoring, const af::MCTaskUp * taskup)
{
//printf("TaskRunMulti::releaseHost: %s[%d][%d](%d)\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, taskup ? taskup->getNumber() : -1);

   RenderContainerIt rendersIt( renders);
   std::list<int>::iterator hIt = hostids.begin();
   std::list<af::TaskExec*>::iterator tIt = execs.begin();
   std::list<af::TaskExec*>::const_iterator end = execs.end();
   std::list<std::string>::iterator nIt = hostnames.begin();
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
         task->appendLog( std::string("Releasing task[") + af::itos((*tIt)->getNumber()) + "] on host \"" + render->getName() + "\"");
      }
      delete *tIt;
      tIt = execs.erase( tIt);
      hIt = hostids.erase( hIt);
      nIt = hostnames.erase( nIt);
      progress->hostname = af::strJoin( hostnames ," ");
      if( stopping && (masterrunning == false) && (execs.size() == 0))
         TaskRun::finish( "Finish: All slaves and master finished.", renders, monitoring);
      task->monitor( monitoring );

      // Finish stopping concrete task.
      if( taskup ) return;
   }

   if( taskup )
   {
      // Concrete task not fouded.
      AFERROR("TaskRunMulti::releaseHost: No such task:")
      taskup->stdOut( false);
      printf("Available tasks:\n");
      stdOut( false);
   }
}

bool TaskRunMulti::getOutput( int startcount, af::Msg *msg, RenderContainer * renders) const
{
   if( progress->state & AFJOB::STATE_READY_MASK)
   {
      msg->setString("Task is waiting for hosts.");
      return false;
   }
  return TaskRun::getOutput( startcount, msg, renders);
}

void TaskRunMulti::stdOut( bool full) const
{
   if( hasservice ) printf("HasSrv, ");
   if( masterrunning ) printf("MRun, ");
   if( stopping ) printf("Stp, ");
   printf("LHA=%d, SStarted=%d, SStopped=%d", time_lasthostadded, time_servicestarted, time_servicestopped);
   printf("\n");
   std::list<int>::const_iterator hIt = hostids.begin();
   std::list<af::TaskExec*>::const_iterator tIt = execs.begin();
   std::list<af::TaskExec*>::const_iterator end = execs.end();
   std::list<std::string>::const_iterator nIt = hostnames.begin();
   for( ; tIt != end; hIt++, tIt++, nIt++)
   {
      if( full ) (*tIt)->stdOut( false);
      else printf("Exec=[%d][%d][%d](%d) host='%s'[%d]\n",
         (*tIt)->getJobId(), (*tIt)->getBlockNum(), (*tIt)->getTaskNum(), (*tIt)->getNumber(), (*nIt).c_str(), *hIt);
   }
}
