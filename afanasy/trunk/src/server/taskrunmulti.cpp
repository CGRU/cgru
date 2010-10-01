#include "taskrunmulti.h"

#include "../libafanasy/blockdata.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/taskexec.h"
#include "../libafanasy/msgclasses/mctaskpos.h"

#include "block.h"
#include "renderaf.h"
#include "rendercontainer.h"
#include "monitorcontainer.h"
#include "msgaf.h"
#include "task.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

TaskRunMulti::TaskRunMulti( Task * runningTask,
                  af::TaskExec* taskExec,
                  af::TaskProgress * taskProgress,
                  const Block * taskBlock,
                  RenderAf * render,
                  MonitorContainer * monitoring,
                  QStringList * taskLog,
                  int * runningtaskscounter
                  ):
   TaskRun( runningTask,
            NULL,    ///< SET NO EXECUTABLE! It will be set before starting master.
            taskProgress,
            taskBlock,
            render,
            monitoring,
            taskLog,
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
      AFERRAR("TaskRunMulti::TaskRunMulti: %s[%d] Task executable is NULL.\n", block->job->getName().toUtf8().data(), block->data->getBlockNum());
      return;
   }
   hasservice = (block->data->getMultiHostService().isEmpty() == false);
   tasknum = taskExec->getTaskNum();
   log("Starting to capture hosts:");
   progress->state = AFJOB::STATE_RUNNING_MASK | AFJOB::STATE_READY_MASK;
   addHost( taskExec, render, monitoring);
}

TaskRunMulti::~TaskRunMulti()
{
AFINFA("TaskRunMulti:: ~ TaskRunMulti: %s[%d][%d]:\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
   if( execs.size() != 0)
   {
      AFERRAR("TaskRunMulti:: ~ TaskRunMulti: %s[%d][%d]:\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
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
   log( QString("Host '%1' added.").arg(render->getName()));

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
   hostnames << render->getName();

   // Set active slaves list for monitoring
   progress->hostname = hostnames.join(" ");

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

   log("Starting services on slave hosts.");
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
      AFERRAR("TaskRunMulti::startMaster: %s[%d][%d] Render id=%d is NULL.\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, hostId);
      return;
   }

   // Add a master to slaves list if needed
   if( block->data->canMasterRunOnSlaveHost()) exec->setHostNames( hostnames << render->getName());
   else exec->setHostNames( hostnames);

   hostnamemaster = render->getName();
   progress->state = AFJOB::STATE_RUNNING_MASK;
   progress->starts_count++;
   progress->time_start = time( NULL);
   progress->time_done = progress->time_start;
   progress->hostname = QString("%1: %2").arg( hostnamemaster, hostnames.join(" "));
   render->startTask( exec);
   masterrunning = true;
   task->monitor( monitoring );
   task->updateDatabase();
   log(QString("Starting master on '%1'").arg(render->getName()));
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
      AFERRAR("TaskRunMulti::update: ZOMBIE %s[%d][%d]\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
      return;
   }
   if((progress->state & AFJOB::STATE_RUNNING_MASK) == false)
   {
      AFERRAR("TaskRunMulti::update: NOT RUNNING %s[%d][%d] task is not running.\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
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
      log("Slave host service finished.");
      releaseHost( renders, monitoring, &taskup );
      if( stopping == false) errorHost = true;
      break;

   case af::TaskExec::UPFailedToStart:
      log("Failed to start slave host service.");
      releaseHost( renders, monitoring, &taskup );
      errorHost = true;
      break;

   case af::TaskExec::UPFinishedCrash:
      log("Slave host service finished crashed.");
      releaseHost( renders, monitoring, &taskup );
      if( stopping == false) errorHost = true;
      break;

   case af::TaskExec::UPFinishedError:
      log("Slave host service finished with error.");
      releaseHost( renders, monitoring, &taskup );
      if( stopping == false) errorHost = true;
      break;

   case af::TaskExec::UPEject:
      if( time_servicestarted == false)
         releaseHost( renders, monitoring, &taskup );
      log("Host owner ejected slave.");
      break;

   case af::TaskExec::UPRenderDeregister:
      log("Slave deregistered.");
      releaseHost( renders, monitoring, &taskup );
      break;

   case af::TaskExec::UPRenderExit:
      log("Slave exited.");
      releaseHost( renders, monitoring, &taskup );
      break;

   case af::TaskExec::UPRenderZombie:
      log("Slave became a zombie.");
      releaseHost( renders, monitoring, &taskup );
      errorHost = true;
      break;

   default:
      AFERRAR("TaskRun::updateState: %s[%d][%d]: Unknown task update status = %d\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, taskup.getStatus());
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
      log("Service stop timeout.");
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
            log( QString("Finished waiting for hosts.\nCaptured slaves:\n%1").arg( hostnames.join(" ")));
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

void TaskRunMulti::stop( const QString & message, RenderContainer * renders, MonitorContainer * monitoring)
{
//printf("TaskRunMulti::stop: %s[%d][%d]\n\t%s\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum, message.toUtf8().data());

   stopping = true;
   log( message);

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
            AFERRAR("TaskRunMulti::stop: %s[%d][%d] Services already asked to be stopped.\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), tasknum);
            return;
         }
         // Stopping service on slaves (if it was not asked to be stopped before)
         log(QString("Stopping service[%1] on host '%2'").arg((*tIt)->getNumber()).arg(render->getName()));
         render->stopTask( *tIt);
         hIt++, tIt++;
      }
      else
      {
         // Finish tasks on slaves if there is no service
         render->taskFinished( *tIt, monitoring);
         log(QString("Finished task[%1] on host '%2'").arg((*tIt)->getNumber()).arg(render->getName()));
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

void TaskRunMulti::finish( const QString & message, RenderContainer * renders, MonitorContainer * monitoring)
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
   for( int pos = 0; tIt != end; hIt++, tIt++, pos++)
   {
      if(( taskup != NULL) && ((*tIt)->equals( *taskup) == false)) continue;

      RenderAf * render = rendersIt.getRender(*hIt);
      if( render == NULL)
      {
         AFERRAR("TaskRunMulti::releaseHost: Render[%d] is NULL %s[%d][%d](%d)\n",
           *hIt, (*tIt)->getJobName().toUtf8().data(), (*tIt)->getBlockNum(), (*tIt)->getTaskNum(), (*tIt)->getNumber());
      }
      else
      {
         render->taskFinished( *tIt, monitoring);
         log(QString("Releasing task[%1] on host '%2'").arg((*tIt)->getNumber()).arg(render->getName()));
      }
      delete *tIt;
      tIt = execs.erase( tIt);
      hIt = hostids.erase( hIt);
      hostnames.removeAt( pos);
      progress->hostname = hostnames.join(" ");
      if( stopping && (masterrunning == false) && (execs.size() == 0))
         TaskRun::finish( "Finish: All slaves and master finished.", renders, monitoring);
      task->monitor( monitoring );

      // Finish stopping concrete task.
      if( taskup ) return;
   }

   if( taskup )
   {
      // Concrete task not fouded.
      AFERROR("TaskRunMulti::releaseHost: No such task:\n");
      taskup->stdOut( false);
      printf("Available tasks:\n");
      stdOut( false);
   }
}

bool TaskRunMulti::getOutput( int startcount, MsgAf *msg, RenderContainer * renders) const
{
   if( progress->state & AFJOB::STATE_READY_MASK)
   {
      QString str("Task is waiting for hosts.");
      msg->setString( str);
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
   for( int pos = 0; tIt != end; hIt++, tIt++, pos++)
   {
      if( full ) (*tIt)->stdOut( false);
      else printf("Exec=[%d][%d][%d](%d) host='%s'[%d]\n",
         (*tIt)->getJobId(), (*tIt)->getBlockNum(), (*tIt)->getTaskNum(), (*tIt)->getNumber(), hostnames[pos].toUtf8().data(), *hIt);
   }
}
