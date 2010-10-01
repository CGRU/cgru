#include "task.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "afcommon.h"
#include "block.h"
#include "monitorcontainer.h"
#include "msgaf.h"
#include "renderaf.h"
#include "rendercontainer.h"
#include "taskrun.h"
#include "taskrunmulti.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Task::Task( const Block * taskBlock, af::TaskProgress * taskProgress, int taskNumber):
   block( taskBlock),
   number( taskNumber),
   progress( taskProgress),
   run( NULL)
{
}

Task::~Task()
{
   if( run) delete run;
}

void Task::start( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring)
{
   if( block->data->isMultiHost())
   {
      if( run )
         ((TaskRunMulti*)(run))->addHost( taskexec, render, monitoring);
      else
         run = new TaskRunMulti( this, taskexec, progress, block, render, monitoring, &logStringList, runningtaskscounter);
      return;
   }
   if( run)
   {
      AFERROR("Task is already running");
      delete taskexec;
      return;
   }
   run = new TaskRun( this, taskexec, progress, block, render, monitoring, &logStringList, runningtaskscounter);
}

void Task::updateState( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost)
{
   if( run == NULL)
   {
      AFERRAR("Task::updatestate: Task is not running: %s[%d][%d]\n", block->job->getName().toUtf8().data(), taskup.getNumBlock(), taskup.getNumTask());
      if(( taskup.getStatus() == af::TaskExec::UPPercent) ||
         ( taskup.getStatus() == af::TaskExec::UPWarning))
            RenderAf::closeLostTask( taskup);
      return;
   }
//printf("Task::updateState:\n");
   run->update( taskup, renders, monitoring, errorHost);
   if( taskup.getDataLen() != 0 ) writeTaskOutput( taskup);
   deleteRunningZombie();
}

void Task::deleteRunningZombie()
{
   if( run == NULL ) return;
   if( false == run->isZombie()) return;
   delete run;
   run = NULL;
}

void Task::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
   bool changed = false;
   if( renders != NULL )
   {
      if( run ) changed = run->refresh( currentTime, renders, monitoring, errorHostId);
      else
      {
         // Retry errors:
         if((progress->state & AFJOB::STATE_ERROR_MASK) && (progress->errors_count <= block->getErrorsRetries()))
         {
            progress->state = progress->state |   AFJOB::STATE_READY_MASK;
            progress->state = progress->state & (~AFJOB::STATE_ERROR_MASK);
            log(QString("Automatically retrying error task %1 of %2").arg( progress->errors_count).arg( block->getErrorsRetries()));
            if( changed == false) changed = true;
         }
      }
   }
   if( changed)
   {
      monitor( monitoring);
      updateDatabase();
   }
   deleteRunningZombie();
}

void Task::restart( bool onlyRunning, const QString & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( run )
   {
      run->restart( message, renders, monitoring);
      return;
   }
   if( onlyRunning ) return;
   progress->state = AFJOB::STATE_READY_MASK;
   progress->errors_count = 0;
   updateDatabase();
   monitor( monitoring);
   log( message);
}

void Task::restartError( const QString & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( false == ( progress->state & AFJOB::STATE_ERROR_MASK )) return;
   if( run )
   {
      AFERRAR("Task::restartError: task is runnning: %s[%d][%d]\n", block->job->getName().toUtf8().data(), block->data->getBlockNum(), number);
      return;
   }
   progress->state = AFJOB::STATE_READY_MASK;
   progress->errors_count = 0;
   updateDatabase();
   monitor( monitoring);
   log( message);
}

void Task::skip( const QString & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( progress->state & AFJOB::STATE_DONE_MASK) return;
   if( run ) run->skip( message, renders, monitoring);
   else
   {
      progress->state = AFJOB::STATE_DONE_MASK | AFJOB::STATE_SKIPPED_MASK;
      progress->errors_count = 0;
      updateDatabase();
      monitor( monitoring);
      log( message);
   }
}

void Task::errorHostsAppend( const QString & hostname)
{
   int index = errorHosts.indexOf( hostname);
   if( index == -1 )
   {
      errorHosts << hostname;
      errorHostsCounts << 1;
   }
   else
   {
      errorHostsCounts[index]++;
      if( errorHostsCounts[index] >= block->getErrorsTaskSameHost() )
      {
         log( hostname + " - AVOIDING HOST !");
      }
   }
}

bool Task::avoidHostsCheck( const QString & hostname) const
{
   if( block->getErrorsTaskSameHost() < 1 ) return false;
   int index = errorHosts.indexOf( hostname);
   if( index == -1 ) return false;
   if( errorHostsCounts[index] >= block->getErrorsTaskSameHost() ) return true;
   return false;
}

bool Task::getErrorHostsList( QStringList & list, bool addTasksLabes)
{
   if( errorHosts.size() < 1 ) return false;
   for( int h = 0; h < errorHosts.size(); h++)
   {
      QString line;
      if( addTasksLabes) line = QString("Task[%1]: ").arg(number);
      line += QString("'%1': %2").arg( errorHosts[h]).arg( errorHostsCounts[h]);
      if((block->getErrorsTaskSameHost() > 0) && (errorHostsCounts[h] >= block->getErrorsTaskSameHost())) line += " - ! AVOIDING !";
      list << line;
   }
   return true;
}

void Task::monitor( MonitorContainer * monitoring) const
{
   if( monitoring ) monitoring->addTask( block->job->getId(), block->data->getBlockNum(), number, progress);
}

void Task::updateDatabase() const
{
   AFCommon::QueueDBUpdateTask( block->job->getId(), block->data->getBlockNum(), number, progress);
}

void Task::log( const QString &message)
{
   logStringList << af::time2Qstr() + " : " + message;
}

void Task::writeTaskOutput( const af::MCTaskUp& taskup) const
{
   AFCommon::QueueFileWrite( new FileData( taskup.getData(), taskup.getDataLen(), getOutputFileName( progress->starts_count)));
}

void Task::listenOutput( af::MCListenAddress & mclisten, RenderContainer * renders)
{
   if( run) run->listen( mclisten, renders);
}

const QString Task::getOutputFileName( int startcount) const
{
   QString filename = QString("b%1.t%2.s%3-%4.%5")
                        .arg( block->data->getBlockNum()).arg( number).arg( startcount)
                        .arg( block->data->getName())
                        .arg( block->data->genTaskName( number));
   af::filterFileName( filename);
   return QString("%1/%2").arg( block->job->getTasksOutputDir(), filename);
}

bool Task::getOutput( int startcount, MsgAf *msg, QString & filename, RenderContainer * renders) const
{
//printf("Task::getOutput:\n");
   if( progress->starts_count < 1 )
   {
      QString str("Task was not started.");
      msg->setString( str);
      return false;
   }
   if( startcount > progress->starts_count )
   {
      QString str("Task was started %1 times ( less than %2 times ).");
      str = str.arg(progress->starts_count).arg(startcount);
      msg->setString( str);
      return false;
   }
   if( startcount == 0 )
   {
      if( run )
      {
         return run->getOutput( startcount, msg, renders);
      }
      else
      {
         startcount = progress->starts_count;
      }
   }

   filename = getOutputFileName( startcount);
   return true;
}

int Task::calcWeight() const
{
   int weight = sizeof( Task);
   if( run ) weight += run->calcWeight();
   return weight;
}

int Task::logsWeight() const
{
   int weight = 0;
   for( int i = 0; i < logStringList.size(); i++) weight += logStringList[i].size()+1;
   return weight;
}

int Task::blackListWeight() const
{
   int weight = sizeof(int) * errorHostsCounts.size();
   for( int i = 0; i < errorHosts.size(); i++) weight += errorHosts[i].size()+1;
   return weight;
}
