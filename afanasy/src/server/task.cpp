#include "task.h"

#include "../include/afanasy.h"

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
#include "../libafanasy/logger.h"

Task::Task( Block * taskBlock, af::TaskProgress * taskProgress, int taskNumber):
   m_block( taskBlock),
   m_number( taskNumber),
   m_progress( taskProgress),
   m_run( NULL),
	m_listen_count( 0)
{
	// If job is not from store, it is just came from network
	// and so no we do not need to read anything
	if( false == m_block->m_job->isFromStore()) return;

	initStoreFolders();

	// Get existing files list
	if( af::pathIsFolder( m_store_dir_files))
		m_stored_files = af::getFilesList( m_store_dir_files);

	// Set thumbnail for a job if was not:
	if( m_stored_files.size() && ( false == m_block->m_job->hasThumbnail() ))
	{
		std::string filename = m_store_dir_files + AFGENERAL::PATH_SEPARATOR + m_stored_files[0];
		int size;
		char * data = af::fileRead( filename, &size);
		if( data )
		{
			m_block->m_job->setThumbnail( filename, size, data);
			delete [] data;
		}
	}

	// Read task progress
	if( false == af::pathFileExists( m_store_file_progress)) return;

	int size;
	char * data = af::fileRead( m_store_file_progress, &size);
	if( data == NULL ) return;

	rapidjson::Document document;
	char * res = af::jsonParseData( document, data, size);
	if( res == NULL )
	{
		delete [] data;
		return;
	}

	m_progress->jsonRead( document);

	delete [] data;
	delete [] res;
}

Task::~Task()
{
   if( m_run) delete m_run;
}

void Task::initStoreFolders()
{
	m_store_dir = m_block->m_job->getTasksDir() + AFGENERAL::PATH_SEPARATOR
		+ af::itos( m_block->m_data->getBlockNum())
		+ '.' + af::itos( m_number);

	m_store_dir_output = m_store_dir + AFGENERAL::PATH_SEPARATOR + "output";
	m_store_dir_files = m_store_dir + AFGENERAL::PATH_SEPARATOR + "files";
	m_store_file_progress = m_store_dir + AFGENERAL::PATH_SEPARATOR + "progress.json";
}

af::TaskExec * Task::genExec() const
{
	af::TaskExec * exec = m_block->m_data->genTask( m_number);
	exec->setParsedFiles( m_parsed_files);
	return exec;
}

void Task::v_start( af::TaskExec * i_taskexec, RenderAf * i_render, MonitorContainer * i_monitoring, int32_t * io_running_tasks_counter, int64_t * io_running_capacity_counter)
{
   if( m_block->m_data->isMultiHost())
   {
      if( m_run )
         ((TaskRunMulti*)(m_run))->addHost( i_taskexec, i_render, i_monitoring);
      else
         m_run = new TaskRunMulti( this, i_taskexec, m_progress, m_block, i_render, i_monitoring, io_running_tasks_counter, io_running_capacity_counter);
      return;
   }

   if( m_run)
   {
      AF_ERR << "Task is already running.";
      delete i_taskexec;
      return;
   }

	i_taskexec->listenOutput( m_listen_count > 0);

	m_run = new TaskRun( this, i_taskexec, m_progress, m_block, i_render, i_monitoring, io_running_tasks_counter, io_running_capacity_counter);
}

void Task::reconnect( af::TaskExec * i_taskexec, RenderAf * i_render, MonitorContainer * i_monitoring, int32_t * io_running_tasks_counter, int64_t * io_running_capacity_counter)
{
	if( m_progress->state & AFJOB::STATE_WAITRECONNECT_MASK )
	{
		v_appendLog("Reconnecting previously run...");
		AF_LOG << "Reconnecting task: \"" << *i_taskexec << "\" with\nRender: " << *i_render;
		v_start( i_taskexec, i_render, i_monitoring, io_running_tasks_counter, io_running_capacity_counter);
	}
	else
	{
		v_appendLog("Reconnection failed: task was not waiting it.");
		i_render->stopTask( i_taskexec);
		delete i_taskexec;
	}
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

	std::string log = taskup.getLog();
	if( log.size())
		v_appendLog( log);

	if( taskup.getDataLen() || log.size())
	{
		const char * data = log.c_str();
		int size = log.size();
		if( taskup.getDataLen())
		{
			data = taskup.getData();
			size = taskup.getDataLen();
		}
		v_writeTaskOutput( data, size);
	}

	if( taskup.hasListened())
	{
		af::MCTask mctask( m_block->m_job->getId(), m_block->m_data->getBlockNum(), m_number);
		m_block->m_job->fillTaskNames( mctask);
		mctask.setListened( taskup.getListened());
		mctask.m_render_id = taskup.getClientId();
		monitoring->addListened( mctask);
	}

	if( taskup.getParsedFiles().size())
		m_parsed_files = taskup.getParsedFiles();

	storeFiles( taskup);

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


	// Check reconnect timeout:
	if( m_progress->state & AFJOB::STATE_WAITRECONNECT_MASK )
	{
		if( currentTime - m_progress->time_done > af::Environment::getTaskUpdateTimeout())
		{
			v_appendLog("Reconnect timeout reached. Setting state to READY.");
			m_progress->state = AFJOB::STATE_READY_MASK;
            if( false == changed ) changed = true;
		}
	}


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
            m_progress->state = m_progress->state |   AFJOB::STATE_ERROR_READY_MASK;
            m_progress->state = m_progress->state & (~AFJOB::STATE_ERROR_MASK);
            v_appendLog( std::string("Automatically retrying error task") + af::itos( m_progress->errors_count) + " of " + af::itos( m_block->getErrorsRetries()) + ".");
            if( changed == false) changed = true;
         }
      }
   }


   if( changed)
   {
      v_monitor( monitoring);
      v_store();
   }

   
   deleteRunningZombie();
}

void Task::restart( const std::string & i_message, RenderContainer * i_renders, MonitorContainer * i_monitoring, uint32_t i_state)
{
	if( i_state != 0 )
	{
		// If request is to restart done tasks, we shoud skip skipped tasks
		// which are always done too
		if(( i_state == AFJOB::STATE_DONE_MASK ) && ( m_progress->state & AFJOB::STATE_SKIPPED_MASK ))
			return;

		if(( m_progress->state & i_state ) == 0 )
			return;
	}

	if( m_run )
	{
		m_run->restart( i_message, i_renders, i_monitoring);
		return;
	}

	m_progress->state = AFJOB::STATE_READY_MASK;
	m_progress->errors_count = 0;
	v_store();
	v_monitor( i_monitoring);
	v_appendLog( i_message);
}

void Task::skip( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( m_progress->state & AFJOB::STATE_DONE_MASK) return;
   if( m_run ) m_run->skip( message, renders, monitoring);
   else
   {
      m_progress->state = AFJOB::STATE_DONE_MASK | AFJOB::STATE_SKIPPED_MASK;
      m_progress->errors_count = 0;
      v_store();
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

void Task::v_monitor( MonitorContainer * monitoring) const
{
   if( monitoring ) monitoring->addTask( m_block->m_job->getId(), m_block->m_data->getBlockNum(), m_number, m_progress);
}

void Task::v_store()
{
	if( m_store_dir.empty())
		initStoreFolders();

	std::ostringstream str;
	m_progress->jsonWrite( str);
	AFCommon::QueueFileWrite( new FileData( str, m_store_file_progress,
		m_store_dir));
}

void Task::v_appendLog( const std::string & message)
{
   m_logStringList.push_back( af::time2str() + " : " + message);
   while( m_logStringList.size() > af::Environment::getTaskLogLinesMax() ) m_logStringList.pop_front();
}

void Task::v_writeTaskOutput( const char * i_data, int i_size) const
{
//	AFCommon::QueueFileWrite( new FileData( taskup.getData(), taskup.getDataLen(), getOutputFileName( m_progress->starts_count),
	AFCommon::QueueFileWrite( new FileData( i_data, i_size, getOutputFileName( m_progress->starts_count),
		m_store_dir_output));
}

void Task::storeFiles( const af::MCTaskUp & i_taskup)
{
	for( int i = 0; i < i_taskup.getFilesNum(); i++)
	{
		std::string filename = i_taskup.getFileName(i);

		// Store file name, if it does not stored yet:
		bool exists = false;
		for( int j = 0; j < m_stored_files.size(); j++)
			if( m_stored_files[j] == filename )
			{
				exists = true;
				break;
			}
		if( false == exists )
			m_stored_files.push_back( filename);

		filename = m_store_dir_files + AFGENERAL::PATH_SEPARATOR + filename;

		// Store first thumbnail for task job:
		if( i == 0 )
			m_block->m_job->setThumbnail( filename, i_taskup.getFileSize(i), i_taskup.getFileData(i));

		AFCommon::QueueFileWrite( new FileData( i_taskup.getFileData(i), i_taskup.getFileSize(i), filename,
			m_store_dir_files));
	}
}

af::Msg * Task::getStoredFiles() const
{
	af::MCTaskUp taskup( -1, m_block->m_job->getId(), m_block->m_data->getBlockNum(), m_number);

	for( int i = 0; i < m_stored_files.size(); i++)
	{
		std::string filename = m_store_dir_files + AFGENERAL::PATH_SEPARATOR + m_stored_files[i];

		int size = -1;
		std::string error;
		char * data = af::fileRead( filename, &size, af::Msg::SizeDataMax, &error);
		if( data )
		{
			taskup.addFile( filename, data, size);
			delete [] data;
		}
		else if( error.size())
		{
			AFCommon::QueueLogError( error);
		}
	}

	return new af::Msg( af::Msg::TTaskFiles, &taskup);
}

void Task::getStoredFiles( std::ostringstream & i_str) const
{
	std::string error;

	i_str << "\"task_files\":{";
	i_str << "\n\"job_id\":" << m_block->m_job->getId();
	i_str << ",\n\"block_id\":" << m_block->m_data->getBlockNum();
	i_str << ",\n\"task_id\":" << m_number;
	i_str << ",\n\"files\":[";

	for( int i = 0; i < m_stored_files.size(); i++)
	{
		if( i ) i_str << ",";
		
		std::string filename = m_store_dir_files + AFGENERAL::PATH_SEPARATOR + m_stored_files[i];

		i_str << "\n{\"name\":\"" << filename << "\"";
/*
		int readsize = -1;
		char * data = af::fileRead( filename, &readsize, af::Msg::SizeDataMax, &error);
		if( data )
		{
			i_str << ",\n\"data\":\"";
			i_str << af::base64encode( data, readsize);
			i_str << "\"";
			delete [] data;
		}
		else
		{
			i_str << "\n,\"error\":\"" << af::strEscape( error) << "\"";
		}

		i_str << ",\n\"size\":" << readsize;
*/
		i_str << "}";
	}

	i_str << "\n]}";
}

void Task::listenOutput(  RenderContainer * i_renders, bool i_subscribe)
{
	if( i_subscribe )
	{
		m_listen_count++;

		if( m_listen_count > 1 )
			return;
	}
	else
	{
		if( m_listen_count < 1 )
			return;

		m_listen_count--;

		if( m_listen_count > 0 )
			return;
	}

	if( m_run && m_run->notZombie())
	{
		std::string error;
		int rid = m_run->v_getRunningRenderID( error);
		if( rid < 1 )
			AFCommon::QueueLogError(error);
		else
		{
			RenderContainerIt rId( i_renders);
			RenderAf * render = rId.getRender( rid);
			if( render )
				render->listenTask(
					af::MCTaskPos( m_block->m_job->getId(), m_block->m_data->getBlockNum(), m_number), i_subscribe);
		}
	}

	return;
}

const std::string Task::getOutputFileName( int i_starts_count) const
{
	return m_store_dir_output + AFGENERAL::PATH_SEPARATOR + af::itos( i_starts_count) + ".txt";
}

void Task::getOutput( af::MCTask & io_mctask, std::string & o_error) const
{
	if( m_progress->starts_count < 1 )
	{
		o_error = "Task is not started.";
		return;
	}

	int start_num = io_mctask.getNumber();
	if( start_num > m_progress->starts_count )
	{
		o_error += "Task was started "+af::itos(m_progress->starts_count)+" times ( less than "+af::itos(start_num)+" times ).";
		return;
	}

	if( start_num == 0 )
	{
		if( m_run && m_run->notZombie())
		{
			io_mctask.m_render_id = m_run->v_getRunningRenderID( o_error);
			return;
		}
		else
		{
			start_num = m_progress->starts_count;
		}
	}

	io_mctask.setOutput( getOutputFileName( start_num));
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
