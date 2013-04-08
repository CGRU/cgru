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
   m_block( taskBlock),
   m_number( taskNumber),
   m_progress( taskProgress),
   m_run( NULL)
{
}

Task::~Task()
{
   if( m_run) delete m_run;
}

void Task::v_start( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring)
{
   if( m_block->m_data->isMultiHost())
   {
      if( m_run )
         ((TaskRunMulti*)(m_run))->addHost( taskexec, render, monitoring);
      else
         m_run = new TaskRunMulti( this, taskexec, m_progress, m_block, render, monitoring, runningtaskscounter);
      return;
   }
   if( m_run)
   {
      AFERROR("Task is already running.")
      delete taskexec;
      return;
   }
   m_run = new TaskRun( this, taskexec, m_progress, m_block, render, monitoring, runningtaskscounter);
}

void Task::v_updateState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost)
{
   if( m_run == NULL)
   {
      std::ostringstream stream;
      stream << "Task::updatestate: Task is not running: " << m_block->m_job->getName();
      stream << "[" << taskup.getNumBlock() << "][" << taskup.getNumTask() << "]";
      AFCommon::QueueLogError( stream.str());
      if(( taskup.getStatus() == af::TaskExec::UPPercent  ) ||
         ( taskup.getStatus() == af::TaskExec::UPWarning ))
            RenderAf::closeLostTask( taskup);
      return;
   }
//printf("Task::updateState:\n");
   m_run->update( taskup, renders, monitoring, errorHost);
   if( taskup.getDataLen() != 0 ) v_writeTaskOutput( taskup);
   deleteRunningZombie();
}

void Task::deleteRunningZombie()
{
//printf("Task::deleteRunningZombie:\n");
   if( m_run == NULL ) return;
   if( false == m_run->isZombie()) return;
   delete m_run;
   m_run = NULL;
}

void Task::v_refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId)
{
//printf("Task::refresh:\n");
   bool changed = false;

   // forgive error hosts
   if(( false == m_errorHosts.empty() ) && ( m_block->getErrorsForgiveTime() > 0 ))
   {
      std::list<std::string>::iterator hIt = m_errorHosts.begin();
      std::list<int>::iterator cIt = m_errorHostsCounts.begin();
      std::list<time_t>::iterator tIt = m_errorHostsTime.begin();
      while( hIt != m_errorHosts.end() )
         if( currentTime - *tIt > m_block->getErrorsForgiveTime())
         {
            v_appendLog( std::string("Forgived error host \"") + *hIt + "\" since " + af::time2str(*tIt) + ".");
            hIt = m_errorHosts.erase( hIt);
            cIt = m_errorHostsCounts.erase( cIt);
            tIt = m_errorHostsTime.erase( tIt);
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
      if( m_run ) changed = m_run->refresh( currentTime, renders, monitoring, errorHostId);
      else
      {
         // Retry errors:
         if((m_progress->state & AFJOB::STATE_ERROR_MASK) && (m_progress->errors_count <= m_block->getErrorsRetries()))
         {
            m_progress->state = m_progress->state |   AFJOB::STATE_READY_MASK;
            m_progress->state = m_progress->state & (~AFJOB::STATE_ERROR_MASK);
            v_appendLog( std::string("Automatically retrying error task") + af::itos( m_progress->errors_count) + " of " + af::itos( m_block->getErrorsRetries()) + ".");
            if( changed == false) changed = true;
         }
      }
   }

   if( changed)
   {
      v_monitor( monitoring);
      v_updateDatabase();
   }
   deleteRunningZombie();
}

void Task::restart( bool onlyRunning, const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( m_run )
   {
      m_run->restart( message, renders, monitoring);
      return;
   }
   if( onlyRunning ) return;
   m_progress->state = AFJOB::STATE_READY_MASK;
   m_progress->errors_count = 0;
   v_updateDatabase();
   v_monitor( monitoring);
   v_appendLog( message);
}

void Task::restartError( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( false == ( m_progress->state & AFJOB::STATE_ERROR_MASK )) return;
   if( m_run )
   {
      AFERRAR("Task::restartError: task is runnning: %s[%d][%d]", m_block->m_job->getName().c_str(), m_block->m_data->getBlockNum(), m_number)
      return;
   }
   m_progress->state = AFJOB::STATE_READY_MASK;
   m_progress->errors_count = 0;
   v_updateDatabase();
   v_monitor( monitoring);
   v_appendLog( message);
}

void Task::skip( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( m_progress->state & AFJOB::STATE_DONE_MASK) return;
   if( m_run ) m_run->skip( message, renders, monitoring);
   else
   {
      m_progress->state = AFJOB::STATE_DONE_MASK | AFJOB::STATE_SKIPPED_MASK;
      m_progress->errors_count = 0;
      v_updateDatabase();
      v_monitor( monitoring);
      v_appendLog( message);
   }
}

void Task::errorHostsAppend( const std::string & hostname)
{
   std::list<std::string>::iterator hIt = m_errorHosts.begin();
   std::list<int>::iterator cIt = m_errorHostsCounts.begin();
   std::list<time_t>::iterator tIt = m_errorHostsTime.begin();
   for( ; hIt != m_errorHosts.end(); hIt++, tIt++, cIt++ )
      if( *hIt == hostname )
      {
         (*cIt)++;
         *tIt = time(NULL);
         if( *cIt >= m_block->getErrorsTaskSameHost() )
            v_appendLog( hostname + " - AVOIDING HOST !");
         return;
      }

   m_errorHosts.push_back( hostname);
   m_errorHostsCounts.push_back( 1);
   m_errorHostsTime.push_back( time(NULL));
}

bool Task::avoidHostsCheck( const std::string & hostname) const
{
   if( m_block->getErrorsTaskSameHost() < 1 ) return false;
   std::list<std::string>::const_iterator hIt = m_errorHosts.begin();
   std::list<int>::const_iterator cIt = m_errorHostsCounts.begin();
   std::list<time_t>::const_iterator tIt = m_errorHostsTime.begin();
   for( ; hIt != m_errorHosts.end(); hIt++, tIt++, cIt++ )
      if( *hIt == hostname )
      {
         if( *cIt >= m_block->getErrorsTaskSameHost() )
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
   if( m_errorHosts.size())
   {
		o_list.push_back( std::string("Task[") + af::itos(m_number) + "] error hosts: ");
      std::list<std::string>::const_iterator hIt = m_errorHosts.begin();
      std::list<int>::const_iterator cIt = m_errorHostsCounts.begin();
      std::list<time_t>::const_iterator tIt = m_errorHostsTime.begin();
      for( ; hIt != m_errorHosts.end(); hIt++, tIt++, cIt++ )
      {
			std::string str = *hIt + ": " + af::itos( *cIt) + " at " + af::time2str( *tIt);
			if((m_block->getErrorsTaskSameHost() > 0) && ( *cIt >= m_block->getErrorsTaskSameHost())) str += " - ! AVOIDING !";
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

void Task::v_monitor( MonitorContainer * monitoring) const
{
   if( monitoring ) monitoring->addTask( m_block->m_job->getId(), m_block->m_data->getBlockNum(), m_number, m_progress);
}

void Task::v_updateDatabase() const
{
   AFCommon::QueueDBUpdateTask( m_block->m_job->getId(), m_block->m_data->getBlockNum(), m_number, m_progress);
}

void Task::v_appendLog( const std::string & message)
{
   m_logStringList.push_back( af::time2str() + " : " + message);
   while( m_logStringList.size() > af::Environment::getTaskLogLinesMax() ) m_logStringList.pop_front();
}

void Task::v_writeTaskOutput( const af::MCTaskUp& taskup) const
{
   AFCommon::QueueFileWrite( new FileData( taskup.getData(), taskup.getDataLen(), getOutputFileName( m_progress->starts_count)));
}

void Task::listenOutput( af::MCListenAddress & mclisten, RenderContainer * renders)
{
   if( m_run) m_run->listen( mclisten, renders);
}

const std::string Task::getOutputFileName( int startcount) const
{
   std::ostringstream stream;
   stream << "b"  << m_block->m_data->getBlockNum();
   stream << ".t" << m_number;
   stream << ".s" << startcount;
   stream << "-" << m_block->m_data->getName();
   stream << "." << m_block->m_data->genTaskName( m_number);
   std::string filename = stream.str();
   af::pathFilterFileName( filename);
   filename = m_block->m_job->getTasksOutputDir() + "/" + filename;
   return filename;
}

af::Msg * Task::getOutput( int i_startcount, RenderContainer * i_renders, std::string & o_filename, std::string & o_error) const
{
//printf("Task::getOutput:\n");
	if( m_progress->starts_count < 1 )
	{
		o_error = "Task is not started.";
		return NULL;
	}
	if( i_startcount > m_progress->starts_count )
	{
		o_error += "Task was started "+af::itos(m_progress->starts_count)+" times ( less than "+af::itos(i_startcount)+" times ).";
		return NULL;
	}
	if( i_startcount == 0 )
	{
		if( m_run )
		{
			return m_run->v_getOutput( i_startcount, i_renders, o_error);
		}
		else
		{
			i_startcount = m_progress->starts_count;
		}
	}

	o_filename = getOutputFileName( i_startcount);
	return NULL;
}

const std::string Task::v_getInfo( bool full) const
{
   std::string info = "#";
   info += af::itos( m_number) + " ";
   info += af::state2str( m_progress->state);
   return info;
}

void Task::stdOut( bool full) const
{
   std::cout << v_getInfo( full) << std::endl;
}

int Task::calcWeight() const
{
   int weight = sizeof( Task);
   if( m_run ) weight += m_run->calcWeight();
   return weight;
}

int Task::logsWeight() const
{
   int weight = 0;
   for( std::list<std::string>::const_iterator it = m_logStringList.begin(); it != m_logStringList.end(); it++)
      weight += af::weigh( *it);
   return weight;
}

int Task::blackListWeight() const
{
   int weight = sizeof(int) * m_errorHostsCounts.size();
   weight += sizeof(time_t) * m_errorHostsTime.size();
   for( std::list<std::string>::const_iterator hIt = m_errorHosts.begin(); hIt != m_errorHosts.end(); hIt++ )
      weight += af::weigh( *hIt);
   return weight;
}
