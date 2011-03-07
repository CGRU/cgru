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
         run = new TaskRunMulti( this, taskexec, progress, block, render, monitoring, runningtaskscounter);
      return;
   }
   if( run)
   {
      AFERROR("Task is already running.\n");
      delete taskexec;
      return;
   }
   run = new TaskRun( this, taskexec, progress, block, render, monitoring, runningtaskscounter);
}

void Task::updateState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost)
{
   if( run == NULL)
   {
      std::ostringstream stream;
      stream << "Task::updatestate: Task is not running: " << block->job->getName();
      stream << "[" << taskup.getNumBlock() << "][" << taskup.getNumTask() << "]";
      AFCommon::QueueLogError( stream.str());
      if(( taskup.getStatus() == af::TaskExec::UPPercent  ) ||
         ( taskup.getStatus() == af::TaskExec::UPWarning ))
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
//printf("Task::deleteRunningZombie:\n");
   if( run == NULL ) return;
   if( false == run->isZombie()) return;
   delete run;
   run = NULL;
}

void Task::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
//printf("Task::refresh:\n");
   bool changed = false;

   // forgive error hosts
   if(( false == errorHosts.empty() ) && ( block->getErrorsForgiveTime() > 0 ))
   {
      std::list<std::string>::iterator hIt = errorHosts.begin();
      std::list<int>::iterator cIt = errorHostsCounts.begin();
      std::list<time_t>::iterator tIt = errorHostsTime.begin();
      while( hIt != errorHosts.end() )
         if( currentTime - *tIt > block->getErrorsForgiveTime())
         {
            log( std::string("Forgived error host \"") + *hIt + "\" since " + af::time2str(*tIt) + ".");
            hIt = errorHosts.erase( hIt);
            cIt = errorHostsCounts.erase( cIt);
            tIt = errorHostsTime.erase( tIt);
         }
         else
         {
            hIt++;
            cIt++;
            tIt++;
         }
    }

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
            log( std::string("Automatically retrying error task") + af::itos( progress->errors_count) + " of " + af::itos( block->getErrorsRetries()) + ".");
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

void Task::restart( bool onlyRunning, const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
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

void Task::restartError( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( false == ( progress->state & AFJOB::STATE_ERROR_MASK )) return;
   if( run )
   {
      AFERRAR("Task::restartError: task is runnning: %s[%d][%d]\n", block->job->getName().c_str(), block->data->getBlockNum(), number);
      return;
   }
   progress->state = AFJOB::STATE_READY_MASK;
   progress->errors_count = 0;
   updateDatabase();
   monitor( monitoring);
   log( message);
}

void Task::skip( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
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

void Task::errorHostsAppend( const std::string & hostname)
{
   std::list<std::string>::iterator hIt = errorHosts.begin();
   std::list<int>::iterator cIt = errorHostsCounts.begin();
   std::list<time_t>::iterator tIt = errorHostsTime.begin();
   for( ; hIt != errorHosts.end(); hIt++, tIt++, cIt++ )
      if( *hIt == hostname )
      {
         (*cIt)++;
         *tIt = time(NULL);
         if( *cIt >= block->getErrorsTaskSameHost() )
            log( hostname + " - AVOIDING HOST !");
         return;
      }

   errorHosts.push_back( hostname);
   errorHostsCounts.push_back( 1);
   errorHostsTime.push_back( time(NULL));
}

bool Task::avoidHostsCheck( const std::string & hostname) const
{
   if( block->getErrorsTaskSameHost() < 1 ) return false;
   std::list<std::string>::const_iterator hIt = errorHosts.begin();
   std::list<int>::const_iterator cIt = errorHostsCounts.begin();
   std::list<time_t>::const_iterator tIt = errorHostsTime.begin();
   for( ; hIt != errorHosts.end(); hIt++, tIt++, cIt++ )
      if( *hIt == hostname )
      {
         if( *cIt >= block->getErrorsTaskSameHost() )
         {
            return true;
         }
         else
         {
            return false;
         }
      }

   return false;
}

void Task::getErrorHostsListString( std::string & str) const
{
   if( errorHosts.size())
   {
      str += "\nTask[" + af::itos(number) + "] error hosts: ";
      std::list<std::string>::const_iterator hIt = errorHosts.begin();
      std::list<int>::const_iterator cIt = errorHostsCounts.begin();
      std::list<time_t>::const_iterator tIt = errorHostsTime.begin();
      for( ; hIt != errorHosts.end(); hIt++, tIt++, cIt++ )
      {
         str += "\n";
         str += *hIt + ": " + af::itos( *cIt) + " at " + af::time2str( *tIt);
         if((block->getErrorsAvoidHost() > 0) && ( *cIt >= block->getErrorsAvoidHost())) str += " - ! AVOIDING !";
//         list << QString("%1: %2 at %3%4").arg(errorHosts[h]).arg( QString::number( errorHostsCounts[h]))
//            .arg( af::time2Qstr( errorHostsTime[h]))
//            .arg(((block->getErrorsAvoidHost() > 0) && (errorHostsCounts[h] >= block->getErrorsAvoidHost())) ? " - ! AVOIDING !" : "");
      }
   }
}

const std::string Task::getErrorHostsListString() const
{
   std::string str;
   getErrorHostsListString( str);
   return str;
}

void Task::monitor( MonitorContainer * monitoring) const
{
   if( monitoring ) monitoring->addTask( block->job->getId(), block->data->getBlockNum(), number, progress);
}

void Task::updateDatabase() const
{
   AFCommon::QueueDBUpdateTask( block->job->getId(), block->data->getBlockNum(), number, progress);
}

void Task::log( const std::string & message)
{
   logStringList.push_back( af::time2str() + " : " + message);
   while( logStringList.size() > af::Environment::getTaskLogLinesMax() ) logStringList.pop_front();
}

void Task::writeTaskOutput( const af::MCTaskUp& taskup) const
{
   AFCommon::QueueFileWrite( new FileData( taskup.getData(), taskup.getDataLen(), getOutputFileName( progress->starts_count)));
}

void Task::listenOutput( af::MCListenAddress & mclisten, RenderContainer * renders)
{
   if( run) run->listen( mclisten, renders);
}

const std::string Task::getOutputFileName( int startcount) const
{
   std::ostringstream stream;
   stream << block->job->getTasksOutputDir();
   stream << "/";
   stream << "b"  << block->data->getBlockNum();
   stream << ".t" << number;
   stream << ".s" << startcount;
   stream << "-" << block->data->getName();
   stream << "." << block->data->genTaskName( number);
//   QString filename = QString("b%1.t%2.s%3-%4.%5")
//                        .arg( block->data->getBlockNum()).arg( number).arg( startcount)
//                        .arg( block->data->getName())
//                        .arg( block->data->genTaskName( number));
   std::string filename = stream.str();
   af::pathFilterFileName( filename);
//   return QString("%1/%2").arg( block->job->getTasksOutputDir(), filename);
   return filename;
}

bool Task::getOutput( int startcount, MsgAf *msg, std::string & filename, RenderContainer * renders) const
{
//printf("Task::getOutput:\n");
   if( progress->starts_count < 1 )
   {
      msg->setString("Task is not started.");
      return false;
   }
   if( startcount > progress->starts_count )
   {
      std::ostringstream stream;
      stream << "Task was started " << progress->starts_count << " times ( less than " << startcount << " times ).";
      msg->setString( stream.str());
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

const std::string Task::getInfo( bool full) const
{
   std::string info = "#";
   info += af::itos( number) + " ";
   info += af::state2str( progress->state);
   return info;
}

void Task::stdOut( bool full) const
{
   std::cout << getInfo( full) << std::endl;
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
   for( std::list<std::string>::const_iterator it = logStringList.begin(); it != logStringList.end(); it++)
      weight += af::weigh( *it);
   return weight;
}

int Task::blackListWeight() const
{
   int weight = sizeof(int) * errorHostsCounts.size();
   weight += sizeof(time_t) * errorHostsTime.size();
   for( std::list<std::string>::const_iterator hIt = errorHosts.begin(); hIt != errorHosts.end(); hIt++ )
      weight += af::weigh( *hIt);
   return weight;
}
