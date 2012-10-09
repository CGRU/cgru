#include "task.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "afcommon.h"
#include "block.h"
#include "jobaf.h"
#include "monitorcontainer.h"
#include "renderaf.h"
#include "rendercontainer.h"
#include "taskrun.h"
#include "taskrunmulti.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Task::Task( Block * taskBlock, af::TaskProgress * taskProgress, int taskNumber):
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
   if( block->m_data->isMultiHost())
   {
      if( run )
         ((TaskRunMulti*)(run))->addHost( taskexec, render, monitoring);
      else
         run = new TaskRunMulti( this, taskexec, progress, block, render, monitoring, runningtaskscounter);
      return;
   }
   if( run)
   {
      AFERROR("Task is already running.")
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
      stream << "Task::updatestate: Task is not running: " << block->m_job->getName();
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
            appendLog( std::string("Forgived error host \"") + *hIt + "\" since " + af::time2str(*tIt) + ".");
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
            appendLog( std::string("Automatically retrying error task") + af::itos( progress->errors_count) + " of " + af::itos( block->getErrorsRetries()) + ".");
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
   appendLog( message);
}

void Task::restartError( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( false == ( progress->state & AFJOB::STATE_ERROR_MASK )) return;
   if( run )
   {
      AFERRAR("Task::restartError: task is runnning: %s[%d][%d]", block->m_job->getName().c_str(), block->m_data->getBlockNum(), number)
      return;
   }
   progress->state = AFJOB::STATE_READY_MASK;
   progress->errors_count = 0;
   updateDatabase();
   monitor( monitoring);
   appendLog( message);
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
      appendLog( message);
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
            appendLog( hostname + " - AVOIDING HOST !");
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

void Task::getErrorHostsList( std::list<std::string> & o_list) const
{
   if( errorHosts.size())
   {
		o_list.push_back( std::string("Task[") + af::itos(number) + "] error hosts: ");
      std::list<std::string>::const_iterator hIt = errorHosts.begin();
      std::list<int>::const_iterator cIt = errorHostsCounts.begin();
      std::list<time_t>::const_iterator tIt = errorHostsTime.begin();
      for( ; hIt != errorHosts.end(); hIt++, tIt++, cIt++ )
      {
			std::string str = *hIt + ": " + af::itos( *cIt) + " at " + af::time2str( *tIt);
			if((block->getErrorsTaskSameHost() > 0) && ( *cIt >= block->getErrorsTaskSameHost())) str += " - ! AVOIDING !";
			o_list.push_back( str);
      }
   }
}

const std::string Task::getErrorHostsListString() const
{
	std::list<std::string> list;
	getErrorHostsList( list);
	return af::strJoin( list, "\n");
}

void Task::monitor( MonitorContainer * monitoring) const
{
   if( monitoring ) monitoring->addTask( block->m_job->getId(), block->m_data->getBlockNum(), number, progress);
}

void Task::updateDatabase() const
{
   AFCommon::QueueDBUpdateTask( block->m_job->getId(), block->m_data->getBlockNum(), number, progress);
}

void Task::appendLog( const std::string & message)
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
   stream << "b"  << block->m_data->getBlockNum();
   stream << ".t" << number;
   stream << ".s" << startcount;
   stream << "-" << block->m_data->getName();
   stream << "." << block->m_data->genTaskName( number);
   std::string filename = stream.str();
   af::pathFilterFileName( filename);
   filename = block->m_job->getTasksOutputDir() + "/" + filename;
   return filename;
}

af::Msg * Task::getOutput( int i_startcount, RenderContainer * i_renders, std::string & o_filename, std::string & o_error) const
{
//printf("Task::getOutput:\n");
	if( progress->starts_count < 1 )
	{
		o_error = "Task is not started.";
		return NULL;
	}
	if( i_startcount > progress->starts_count )
	{
		o_error += "Task was started "+af::itos(progress->starts_count)+" times ( less than "+af::itos(i_startcount)+" times ).";
		return NULL;
	}
	if( i_startcount == 0 )
	{
		if( run )
		{
			return run->v_getOutput( i_startcount, i_renders, o_error);
		}
		else
		{
			i_startcount = progress->starts_count;
		}
	}

	o_filename = getOutputFileName( i_startcount);
	return NULL;
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
