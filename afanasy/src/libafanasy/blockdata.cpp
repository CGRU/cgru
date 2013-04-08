
#include "blockdata.h"

#include <memory.h>
#include <stdio.h>
#include <string.h>

#include "jobprogress.h"
#include "msg.h"
#include "taskdata.h"
#include "taskexec.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

const char   BlockData::DataMode_Progress[] = "progress";
const char   BlockData::DataMode_Properties[] = "properties";
const char   BlockData::DataMode_Full[] = "full";
const char * BlockData::DataModeFromMsgType( int i_type)
{
	switch( i_type)
	{
		case af::Msg::TBlocksProgress:   return DataMode_Progress;
		case af::Msg::TBlocksProperties: return DataMode_Properties;
		case af::Msg::TBlocks:           return DataMode_Full;
	} 
	AFERRAR("BlockData::DataModeFromMsgTyp: Invalid block write mode: %d", i_type);
	return "invalid";
}

BlockData::BlockData()
{
   initDefaults();
   construct();
}

void BlockData::initDefaults()
{
	m_job_id = 0;
	m_block_num = 0;
	m_state = 0;//AFJOB::STATE_READY_MASK;
	m_flags = 0;
	m_frame_first = 0;
	m_frame_last = 0;
	m_frames_per_task = 1;
	m_frames_inc = 1;
	m_max_running_tasks = -1;
	m_max_running_tasks_per_host = -1;
	m_tasks_max_run_time = 0;
	m_capacity = AFJOB::TASK_DEFAULT_CAPACITY;
	m_need_memory = 0;
	m_need_power = 0;
	m_need_hdd = 0;
	m_errors_retries = -1;
	m_errors_avoid_host = -1;
	m_errors_task_same_host = -1;
	m_errors_forgive_time = -1;
	m_file_size_min = -1;
	m_file_size_max = -1;
	m_capacity_coeff_min = 0;
	m_capacity_coeff_max = 0;
	m_multihost_min = 0;
	m_multihost_max = 0;
	m_multihost_max_wait = 0;
	m_multihost_service_wait = 0;
	m_parser_coeff = 1;

	p_percentage     = 0;
	p_error_hosts    = 0;
	p_avoid_hosts    = 0;
	p_tasks_ready    = 0;
	p_tasks_done     = 0;
	p_tasks_error    = 0;
	p_tasks_skipped  = 0;
	p_tasks_warning  = 0;
	p_tasks_run_time = 0;

	memset( p_progressbar, AFJOB::ASCII_PROGRESS_STATES[0], AFJOB::ASCII_PROGRESS_LENGTH);
}

BlockData::BlockData( Msg * msg)
{
   construct();
   read( msg);
}

BlockData::BlockData( int BlockNum, int JobId)
{
AFINFA("BlockData::BlockData(): JobId=%d, BlockNum=%d", m_job_id, m_block_num)
	initDefaults();
	m_job_id = JobId;
	m_block_num = BlockNum;
	construct();
}

void BlockData::construct()
{
   m_tasks_num = 0;
   m_tasks_data = NULL;
   m_running_tasks_counter = 0;

   m_depend_mask.setCaseSensitive();
   m_tasks_depend_mask.setCaseSensitive();

   m_hosts_mask.setCaseInsensitive();

   m_hosts_mask_exclude.setCaseInsensitive();
   m_hosts_mask_exclude.setExclude();

   m_need_properties.setCaseSensitive();
   m_need_properties.setContain();
}

/// Construct data from JSON:
BlockData::BlockData( const JSON & i_object, int i_num)
{
	initDefaults();
	construct();

	m_block_num = i_num;

	jsonRead( i_object);
}

void BlockData::jsonRead( const JSON & i_object, std::string * io_changes)
{
	//jr_uint32("flags",               m_flags,                 i_object);

	jr_int32 ("capacity",              m_capacity,              i_object, io_changes);
	jr_int32 ("need_memory",           m_need_memory,           i_object, io_changes);
	jr_int32 ("need_power",            m_need_power,            i_object, io_changes);
	jr_int32 ("need_hdd",              m_need_hdd,              i_object, io_changes);
	jr_regexp("depend_mask",           m_depend_mask,           i_object, io_changes);
	jr_regexp("tasks_depend_mask",     m_tasks_depend_mask,     i_object, io_changes);
	jr_regexp("hosts_mask",            m_hosts_mask,            i_object, io_changes);
	jr_regexp("hosts_mask_exclude",    m_hosts_mask_exclude,    i_object, io_changes);
	jr_regexp("need_properties",       m_need_properties,       i_object, io_changes);
	jr_string("name",                  m_name,                  i_object, io_changes);
	jr_string("service",               m_service,               i_object, io_changes);
	jr_int8  ("errors_retries",        m_errors_retries,        i_object, io_changes);
	jr_int8  ("errors_avoid_host",     m_errors_avoid_host,     i_object, io_changes);
	jr_int8  ("errors_task_same_host", m_errors_task_same_host, i_object, io_changes);
	jr_int32 ("errors_forgive_time",   m_errors_forgive_time,   i_object, io_changes);
	jr_uint32("tasks_max_run_time",    m_tasks_max_run_time,    i_object, io_changes);
	jr_string("tasks_name",            m_tasks_name,            i_object, io_changes);
	jr_string("parser",                m_parser,                i_object, io_changes);
	jr_string("working_directory",     m_working_directory,     i_object, io_changes);
	jr_string("command",               m_command,               i_object, io_changes);
	jr_string("files",                 m_files,                 i_object, io_changes);
	jr_string("command_pre",           m_command_pre,           i_object, io_changes);
	jr_string("command_post",          m_command_post,          i_object, io_changes);
	jr_int32 ("max_running_tasks",          m_max_running_tasks,          i_object, io_changes);
	jr_int32 ("max_running_tasks_per_host", m_max_running_tasks_per_host, i_object, io_changes);
	jr_string("custom_data",           m_custom_data,           i_object, io_changes);
	//jr_string("environment",         m_environment,           i_object, io_changes);
	jr_int32 ("parser_coeff",          m_parser_coeff,          i_object, io_changes);

	bool depend_sub_task = false;
	jr_bool("depend_sub_task", depend_sub_task, i_object, io_changes);
	setDependSubTask( depend_sub_task);

	bool non_sequential = false;
	jr_bool("non_sequential", non_sequential, i_object, io_changes);
	setNonSequential( non_sequential);

	bool gen_thumbnails = false;
	jr_bool("gen_thumbnails", gen_thumbnails, i_object, io_changes);
	setGenTHumbnails( gen_thumbnails);

	if( m_capacity < 1 )
		m_capacity = 1;

	// Paramers below are not editable and read only on creation
	// When use edit parameters, log provided to store changes
	if( io_changes )
		return;

	const JSON & tasks = i_object["tasks"];
	if( tasks.IsArray())
	{
		m_tasks_num = tasks.Size();
		if( m_tasks_num > 0 )
		{
			m_tasks_data = new TaskData*[m_tasks_num];
			for( int t = 0; t < m_tasks_num; t++)
			{
				m_tasks_data[t] = createTask( tasks[t]);
				if( m_tasks_data[t] == NULL)
				{
					AFERROR("BlockData::BlockData: Can not allocate memory for new task.")
					return;
				}
			}
		}
	}

	int64_t frame_first     = 0;
	int64_t frame_last      = 0;
	int64_t frames_per_task = 1;
	int64_t frames_inc      = 1;

	uint32_t flags          = 0;
	
	int32_t capacity_coeff_min = -1;
	int32_t capacity_coeff_max = -1;

	int8_t      multihost_min             = -1;
	int8_t      multihost_max             = -1;
	uint16_t    multihost_max_wait        = 0;
	std::string multihost_service;
	uint16_t    multihost_service_wait    = 0;
	bool        multihost_master_on_slave = false;

	jr_string("multihost_service",     multihost_service,       i_object);

	jr_int64 ("frame_first",     frame_first,     i_object);
	jr_int64 ("frame_last",      frame_last,      i_object);
	jr_int64 ("frames_per_task", frames_per_task, i_object);
	jr_int64 ("frames_inc",      frames_inc,      i_object);
//	jr_uint32 ("flags",          flags,           i_object);
	jr_int32 ("capacity_coeff_min", capacity_coeff_min, i_object);
	jr_int32 ("capacity_coeff_max", capacity_coeff_max, i_object);
	jr_int8  ("multihost_min",             multihost_min,             i_object);
	jr_int8  ("multihost_max",             multihost_max,             i_object);
	jr_uint16("multihost_max_wait",        multihost_max_wait,        i_object);
	jr_uint16("multihost_service_wait",    multihost_service_wait,    i_object);
	jr_bool  ("multihost_master_on_slave", multihost_master_on_slave, i_object);
	//jr_int64 ("file_size_min", m_file_size_min, i_object);
	//jr_int64 ("file_size_max", m_file_size_max, i_object);

/*
//	case Msg::TBlocksProgress:

	jr_uint32("state",                 m_state,                 i_object);
	jr_int32 ("job_id",                m_job_id,                i_object);
	jr_int32 ("block_num",             m_block_num,             i_object);
	jr_int32 ("running_tasks_counter", m_running_tasks_counter, i_object);

	jr_uint8 ("p_percentage",      p_percentage,      i_object);
	jr_int32 ("p_error_hosts",   p_error_hosts,   i_object);
	jr_int32 ("p_avoid_hosts",   p_avoid_hosts,   i_object);
	jr_int32 ("p_tasks_ready",      p_tasks_ready,      i_object);
	jr_int32 ("p_tasks_done",       p_tasks_done,       i_object);
	jr_int32 ("p_tasks_error",      p_tasks_error,      i_object);
	jr_int64 ("p_tasks_run_time", p_tasks_run_time, i_object);

	//rw_data(   (char*)p_bar_done,       i_object, AFJOB::PROGRESS_BYTES);
	//rw_data(   (char*)p_bar_running,    i_object, AFJOB::PROGRESS_BYTES);
*/
//	break;

//	default:
//		AFERRAR("BlockData::readwrite: invalid type = %s.", Msg::TNAMES[msg->type()])
//	}

	if( m_tasks_data == NULL )
		setNumeric( frame_first, frame_last, frames_per_task, frames_inc);
	else if( frames_per_task != 0 )
		m_frames_per_task = frames_per_task;

//	setFlags(flags);

	if(( capacity_coeff_min != -1 ) || ( capacity_coeff_max != -1 ))
		setVariableCapacity( capacity_coeff_min, capacity_coeff_max);

	if(( multihost_min != -1 ) || ( multihost_max != -1 ))
		setMultiHost( multihost_min, multihost_max, multihost_max_wait,
				multihost_master_on_slave, multihost_service, multihost_service_wait);
}

void BlockData::jsonWrite( std::ostringstream & o_str, const std::string & i_datamode) const
{
	int type = 0;
	if     ( i_datamode == DataMode_Progress   ) type = af::Msg::TBlocksProgress;
	else if( i_datamode == DataMode_Properties ) type = af::Msg::TBlocksProperties;
	else if( i_datamode == DataMode_Full       ) type = af::Msg::TBlocks;	
	else
		AFERRAR("BlockData::jsonWrite: Unknown data mode '%s'", i_datamode.c_str())

	jsonWrite( o_str, type);
}

void BlockData::jsonWrite( std::ostringstream & o_str, int i_type) const
{
    o_str << "{";

    switch( i_type)
	{
	case Msg::TJob:
	case Msg::TJobRegister:
	case Msg::TBlocks:

		//rw_uint32_t( m_flags,                 msg);
		//if( isNotNumeric()) rw_tasks(         msg);
		if( isNotNumeric() && ( m_tasks_data != NULL ))
		{
            o_str << "\"tasks\":[";
			for( int t = 0; t < m_tasks_num; t++ )
			{
				if( t != 0 )
                    o_str << ',';
                m_tasks_data[t]->jsonWrite( o_str);
			}
            o_str << "],";
		}

	case Msg::TBlocksProperties:
        o_str << "\"command\":\""     << af::strEscape(m_command) << "\"";
        o_str << ",\"tasks_name\":\"" << m_tasks_name             << "\"";
		if( m_parser.size())
            o_str << ",\"parser\":\"" << m_parser << "\"";
		if( m_working_directory.size())
            o_str << ",\"working_directory\":\"" << af::strEscape(m_working_directory ) << "\"";
		if( m_files.size())
            o_str << ",\"files\":\""             << af::strEscape( m_files )    << "\"";
		if( m_command_pre.size())
            o_str << ",\"cmd_pre\":\""           << af::strEscape( m_command_pre )  << "\"";
		if( m_command_post.size())
            o_str << ",\"cmd_post\":\""          << af::strEscape( m_command_post ) << "\"";
		if( m_multihost_service.size())
            o_str << ",\"multihost_service\":\"" << m_multihost_service         << "\"";
		if( m_custom_data.size())
	        o_str << ",\"custom_data\":\""       << m_custom_data               << "\"";
        //o_str << ",\"environment\":\""         << m_environment               << "\"";
        //o_str << ",\"parser_coeff\":\:"        << m_parser_coeff              << "\"";
        o_str << ',';

	case Msg::TJobsList:

        o_str << "\"name\":\""           << m_name << "\"";
        o_str << ",\"service\":\""       << m_service << "\"";
        o_str << ",\"capacity\":"        << m_capacity;
		o_str << ",\"flags\":"           << m_flags;
		o_str << ",\"numeric\":"         << (isNumeric() ? "true":"false");
		o_str << ",\"tasks_num\":"       << m_tasks_num;
		o_str << ",\"frame_first\":"     << m_frame_first;
        o_str << ",\"frame_last\":"      << m_frame_last;
        o_str << ",\"frames_per_task\":" << m_frames_per_task;
        o_str << ",\"frames_inc\":"      << m_frames_inc;
		if( isDependSubTask())
            o_str << ",\"depend_sub_task\":true";
		if( isNonSequential())
            o_str << ",\"non_sequential\":true";
		if( canVarCapacity())
		{
            o_str << ",\"capacity_coeff_min\":" << m_capacity_coeff_min;
            o_str << ",\"capacity_coeff_max\":" << m_capacity_coeff_max;
		}
		if( isMultiHost())
		{
            o_str << ",\"multihost_min\":"          << int(m_multihost_min);
            o_str << ",\"multihost_max\":"          << int(m_multihost_max);
            o_str << ",\"multihost_max_wait\":"     << int(m_multihost_max_wait);
            o_str << ",\"multihost_service_wait\":" << int(m_multihost_service_wait);
			if( canMasterRunOnSlaveHost())
                o_str << ",\"multihost_master_on_slave\":true";
		}
        //o_str << ",\"file_size_min\":"         << m_file_size_min;
        //o_str << ",\"file_size_max\":"         << m_file_size_max;
		if( m_max_running_tasks != -1 )
            o_str << ",\"max_running_tasks\":"          << m_max_running_tasks;
		if( m_max_running_tasks_per_host != -1 )
            o_str << ",\"max_running_tasks_per_host\":" << m_max_running_tasks_per_host;
		if( m_need_memory > 0 )
            o_str << ",\"need_memory\":"           << m_need_memory;
		if( m_need_power > 0 )
            o_str << ",\"need_power\":"            << m_need_power;
		if( m_need_hdd > 0 )
            o_str << ",\"need_hdd\":"              << m_need_hdd;
		if( m_errors_retries != -1 )
            o_str << ",\"errors_retries\":"        << int(m_errors_retries);
		if( m_errors_avoid_host != -1 )
            o_str << ",\"errors_avoid_host\":"     << int(m_errors_avoid_host);
		if( m_errors_task_same_host != -1 )
            o_str << ",\"errors_task_same_host\":" << int(m_errors_task_same_host);
		if( m_errors_forgive_time != -1 )
            o_str << ",\"errors_forgive_time\":"   << int(m_errors_forgive_time);
		if( m_tasks_max_run_time > 0 )
            o_str << ",\"tasks_max_run_time\":"    << int(m_tasks_max_run_time);

		if( hasDependMask())
            o_str << ",\"depend_mask\":\""        << m_depend_mask.getPattern() << "\"";
		if( hasTasksDependMask())
            o_str << ",\"tasks_depend_mask\":\""  << m_tasks_depend_mask.getPattern() << "\"";
		if( hasHostsMask())
            o_str << ",\"hosts_mask\":\""         << m_hosts_mask.getPattern() << "\"";
		if( hasHostsMaskExclude())
            o_str << ",\"hosts_mask_exclude\":\"" << m_hosts_mask_exclude.getPattern() << "\"";
		if( hasNeedProperties())
            o_str << ",\"need_properties\":\""    << m_need_properties.getPattern() << "\"";
        o_str << ',';

	case Msg::TBlocksProgress:

        o_str << "\"block_num\":"  << m_block_num;
		if( m_state != 0 )
		{
			o_str << ",";
			jw_state( m_state, o_str);
		}
		if( m_job_id != 0 )
            o_str << ",\"job_id\":" << m_job_id;
		if( m_running_tasks_counter > 0 )
            o_str << ",\"running_tasks_counter\":" << m_running_tasks_counter;

		if( p_percentage > 0 )
            o_str << ",\"p_percentage\":"     << int(p_percentage);
		if( p_error_hosts > 0 )
            o_str << ",\"p_error_hosts\":"    << p_error_hosts;
		if( p_avoid_hosts > 0 )
            o_str << ",\"p_avoid_hosts\":"    << p_avoid_hosts;
		if( p_tasks_ready > 0 )
            o_str << ",\"p_tasks_ready\":"    << p_tasks_ready;
		if( p_tasks_done > 0 )
            o_str << ",\"p_tasks_done\":"     << p_tasks_done;
		if( p_tasks_error > 0 )
            o_str << ",\"p_tasks_error\":"    << p_tasks_error;
		if( p_tasks_skipped > 0 )
            o_str << ",\"p_tasks_skipped\":"  << p_tasks_skipped;
		if( p_tasks_warning > 0 )
            o_str << ",\"p_tasks_warning\":"  << p_tasks_warning;
		if( p_tasks_run_time > 0 )
            o_str << ",\"p_tasks_run_time\":" << p_tasks_run_time;

//		if(( p_tasks_done < m_tasks_num ) ||
//		     p_tasks_error || m_running_tasks_counter )
		{
			o_str << ",\"p_progressbar\":\"";
			for( int i = 0; i < AFJOB::ASCII_PROGRESS_LENGTH; i++)
			{
				o_str << p_progressbar[i];
			}
			o_str << "\"";
		}

	break;

	default:
        AFERRAR("BlockData::readwrite: invalid type = %s.", Msg::TNAMES[i_type])
	}

    o_str << "}";
}

bool BlockData::isValid() const
{
   if( m_tasks_num == 0)
   {
      AFERRAR("BlockData::isValid: #%d block[%s] zero tasks number.", m_block_num, m_name.c_str())
      return false;
   }
   if( isNotNumeric())
   {
      if( m_tasks_data == NULL)
      {
         AFERRAR("BlockData::isValid: #%d block[%s] tasks data is null on not numeric block.", m_block_num, m_name.c_str())
         return false;
      }
      for( int t = 0; t < m_tasks_num; t++)
      {
         if(m_tasks_data[t] == NULL)
         {
            AFERRAR("BlockData::isValid: #%d block[%s] task[%d] data is null on not numeric block.", m_block_num, m_name.c_str(), t)
            return false;
         }
      }
   }
   return true;
}

BlockData::~BlockData()
{
//printf("BlockData::~BlockData()\n");
   if( m_tasks_data)
   {
      for( int t = 0; t < m_tasks_num; t++)
         if( m_tasks_data[t]) delete m_tasks_data[t];
      delete [] m_tasks_data;
   }
}

void BlockData::v_readwrite( Msg * msg)
{
//printf("BlockData::readwrite: BEGIN\n");
	switch( msg->type())
	{
	case Msg::TJob:
	case Msg::TJobRegister:
	case Msg::TBlocks:
		rw_uint32_t( m_flags,                 msg);
		if( isNotNumeric()) rw_tasks(         msg);

	case Msg::TBlocksProperties:
		rw_int32_t ( m_parser_coeff,          msg);
		rw_String  ( m_tasks_name,            msg);
		rw_String  ( m_parser,                msg);
		rw_String  ( m_working_directory,     msg);
		rw_String  ( m_environment,           msg);
		rw_String  ( m_command,               msg);
		rw_String  ( m_files,                 msg);
		rw_String  ( m_command_pre,           msg);
		rw_String  ( m_command_post,          msg);
		rw_String  ( m_multihost_service,     msg);
		rw_String  ( m_custom_data,           msg);

	case Msg::TJobsList:
		rw_uint32_t( m_flags,                 msg);
		rw_int64_t ( m_frame_first,           msg);
		rw_int64_t ( m_frame_last,            msg);
		rw_int64_t ( m_frames_per_task,       msg);
		rw_int64_t ( m_frames_inc,            msg);
		rw_int64_t ( m_file_size_min,         msg);
		rw_int64_t ( m_file_size_max,         msg);
		rw_int32_t ( m_capacity_coeff_min,    msg);
		rw_int32_t ( m_capacity_coeff_max,    msg);
		rw_uint8_t ( m_multihost_min,         msg);
		rw_uint8_t ( m_multihost_max,         msg);
		rw_uint16_t( m_multihost_service_wait,msg);
		rw_uint16_t( m_multihost_max_wait,    msg);
		rw_int32_t ( m_capacity,              msg);
		rw_int32_t ( m_max_running_tasks,     msg);
		rw_int32_t ( m_max_running_tasks_per_host,    msg);
		rw_int32_t ( m_need_memory,           msg);
		rw_int32_t ( m_need_power,            msg);
		rw_int32_t ( m_need_hdd,              msg);
		rw_RegExp  ( m_depend_mask,           msg);
		rw_RegExp  ( m_tasks_depend_mask,     msg);
		rw_RegExp  ( m_hosts_mask,            msg);
		rw_RegExp  ( m_hosts_mask_exclude,    msg);
		rw_RegExp  ( m_need_properties,       msg);
		rw_String  ( m_name,                  msg);
		rw_String  ( m_service,               msg);
		rw_int32_t ( m_tasks_num,             msg);
		rw_int8_t  ( m_errors_retries,        msg);
		rw_int8_t  ( m_errors_avoid_host,     msg);
		rw_int8_t  ( m_errors_task_same_host, msg);
		rw_int32_t ( m_errors_forgive_time,   msg);
		rw_uint32_t( m_tasks_max_run_time,    msg);

	case Msg::TBlocksProgress:

		rw_int32_t ( m_running_tasks_counter, msg);
		rw_uint8_t ( p_percentage,            msg);
		rw_int32_t ( p_error_hosts,         msg);
		rw_int32_t ( p_avoid_hosts,         msg);
		rw_int32_t ( p_tasks_ready,            msg);
		rw_int32_t ( p_tasks_done,             msg);
		rw_int32_t ( p_tasks_error,            msg);
		rw_int64_t ( p_tasks_run_time,       msg);

		rw_uint32_t( m_state,                 msg);
		rw_int32_t ( m_job_id,                msg);
		rw_int32_t ( m_block_num,             msg);

		rw_data( p_progressbar, msg, AFJOB::ASCII_PROGRESS_LENGTH);

	break;

	default:
		AFERRAR("BlockData::readwrite: invalid type = %s.", Msg::TNAMES[msg->type()])
	}
//printf("BlockData::readwrite: END\n");
}

void BlockData::rw_tasks( Msg * msg)
{
   if( isNumeric())
   {
      AFERROR("BlockData::rw_tasks: block is numeric.")
      return;
   }
   rw_int32_t( m_tasks_num, msg);
   if( m_tasks_num < 1)
   {
      AFERRAR("BlockData::rw_tasks: invalid number of tasks = %d.", m_tasks_num)
      return;
   }

   if( msg->isWriting() )
   {
      for( int b = 0; b < m_tasks_num; b++)
      {
         m_tasks_data[b]->write( msg);
      }
   }
   else
   {
      m_tasks_data = new TaskData*[m_tasks_num];
      for( int b = 0; b < m_tasks_num; b++)
      {
         m_tasks_data[b] = createTask( msg);
         if( m_tasks_data[b] == NULL)
         {
            AFERROR("BlockData::rw_tasks: Can not allocate memory for new task.")
            return;
         }
      }
   }
}

TaskData * BlockData::createTask( const JSON & i_object)
{
   return new TaskData( i_object);
}

TaskData * BlockData::createTask( Msg * msg)
{
   return new TaskData( msg);
}

bool BlockData::setCapacity( int value)
{
   if( value > 0)
   {
      m_capacity = value;
      return true;
   }
   AFERRAR("BlockData::setCapacity: invalid capacity = %d", value)
   return false;
}

void BlockData::setVariableCapacity( int i_capacity_coeff_min, int i_capacity_coeff_max)
{
	if( i_capacity_coeff_min < 0 ) i_capacity_coeff_min = 0;
	if( i_capacity_coeff_max < 0 ) i_capacity_coeff_max = 0;
	m_flags = m_flags | FVarCapacity;
	m_capacity_coeff_min = i_capacity_coeff_min;
	m_capacity_coeff_max = i_capacity_coeff_max;
}

void BlockData::setMultiHost( int i_min, int i_max, int i_waitmax,
        bool i_masterOnSlave, const std::string & i_service, int i_waitsrv)
{
   if( i_min < 1)
   {
      AFERROR("BlockData::setMultiHost: Minimum must be greater then zero.")
      return;
   }
   if( i_max < i_min)
   {
      AFERROR("BlockData::setMultiHost: Maximum must be greater or equal then minimum.")
      return;
   }
   if(( i_min > AFJOB::TASK_MULTIHOSTMAXHOSTS) || ( i_max > AFJOB::TASK_MULTIHOSTMAXHOSTS))
   {
      AFERRAR("BlockData::setMultiHost: Maximum hosts number is limited to %d.", AFJOB::TASK_MULTIHOSTMAXHOSTS)
      return;
   }
   if( i_masterOnSlave && ( false == i_service.empty() ))
   {
      AFERROR("BlockData::setMultiHost: Block can't have multihost service if master and slave can be the same host.")
      i_masterOnSlave = false;
   }

   m_flags = m_flags | FMultiHost;
   if( i_masterOnSlave) m_flags = m_flags | FMasterOnSlave;
   m_multihost_min  = i_min;
   m_multihost_max  = i_max;
   m_multihost_max_wait = i_waitmax;
   m_multihost_service_wait = i_waitsrv;
   if( false == i_service.empty()) m_multihost_service = i_service;
}

bool BlockData::setCapacityCoeffMin( int value)
{
   if( canVarCapacity() == false)
   {
      AFERROR("BlockData::setCapacityCoeffMin: Block can't variate capacity.")
      return false;
   }
   m_capacity_coeff_min = value;
   return true;
}

bool BlockData::setCapacityCoeffMax( int value)
{
   if( canVarCapacity() == false)
   {
      AFERROR("BlockData::setCapacityCoeffMax: Block can't variate capacity.")
      return false;
   }
   m_capacity_coeff_max = value;
   return true;
}

bool BlockData::setMultiHostMin( int value)
{
   if( isMultiHost() == false)
   {
      AFERROR("BlockData::setMultiHostMin: Block is not multihost.")
      return false;
   }
   if( value < 1)
   {
      AFERROR("BlockData::setMultiHostMin: Hosts minimum can't be less than one.")
      return false;
   }
   m_multihost_min = value;
   return true;
}

bool BlockData::setMultiHostMax( int value)
{
   if( isMultiHost() == false)
   {
      AFERROR("BlockData::setMultiHostMax: Block is not multihost.")
      return false;
   }
   if( value < m_multihost_min)
   {
      AFERROR("BlockData::setMultiHostMax: Hosts maximum can't be less than minimum.")
      return false;
   }
   m_multihost_max = value;
   return true;
}

bool BlockData::setNumeric( long long start, long long end, long long perTask, long long increment)
{
   if( perTask < 1)
   {
      AFERRAR("BlockData::setNumeric(): Frames per task = %lld < 1 ( setting to 1).", perTask)
      perTask = 1;
   }
   if( increment < 1)
   {
      AFERRAR("BlockData::setNumeric(): Frames increment = %lld < 1 ( setting to 1).", increment)
      increment = 1;
   }
   if( m_tasks_data)
   {
      AFERROR("BlockData::setNumeric(): this block already has tasks.")
      return false;
   }
   if( isNumeric())
   {
      AFERROR("BlockData::setNumeric(): this block is already numeric and numbers are set.")
      return false;
   }
   if( start > end)
   {
      AFERRAR("BlockData::setNumeric(): start > end ( %lld > %lld - setting end to %lld)", start, end, start)
      end = start;
   }
   m_flags = m_flags | FNumeric;

   m_frame_first    = start;
   m_frame_last     = end;
   m_frames_per_task  = perTask;
   m_frames_inc      = increment;

   long long numframes = (m_frame_last - m_frame_first) / m_frames_inc + 1;
   m_tasks_num = numframes / m_frames_per_task;
   if((numframes%perTask) != 0) m_tasks_num++;

   return true;
}
/*
bool BlockData::setFlags (unsigned int flags) {
   m_flags = m_flags | FDoPost;
   return true;
}
*/
bool BlockData::genNumbers( long long & start, long long & end, int num, long long * frames_num) const
{
   start = 0;
   end = 0;
   if( frames_num ) *frames_num = 1;

   if( num > m_tasks_num)
   {
      AFERROR("BlockData::genNumbers: n > tasksnum.")
      return false;
   }
   if( m_frames_per_task == 0)
   {
      AFERROR("BlockData::genNumbers: frame_pertask == 0.")
      return false;
   }

   if( isNotNumeric() )
   {
      if( m_frames_per_task > 0 )
      {
         start = num * m_frames_per_task;
         end = start + m_frames_per_task - 1;
         if( frames_num ) *frames_num = m_frames_per_task;
      }
      else
      {
         start = num / (-m_frames_per_task);
         end = start;//( num + 1 ) / (-frame_pertask);
         if( frames_num ) *frames_num = -m_frames_per_task;
      }
      return true;
   }

   long long offset = num * m_frames_per_task * m_frames_inc;
   if( m_frames_inc > 1 ) offset -= offset % m_frames_inc;
   start = m_frame_first + offset;

   offset = m_frames_per_task * m_frames_inc - 1;
   if(( start + offset ) > m_frame_last ) offset = m_frame_last - start;
   if( m_frames_inc > 1 ) offset -= offset % m_frames_inc;
   end = start + offset;

   if( frames_num )
   {
      if( m_frames_inc > 1 )
         *frames_num = ( end - start ) / m_frames_inc + 1;
      else
         *frames_num = end - start + 1;
   }

   return true;
}

int BlockData::calcTaskNumber( long long i_frame, bool & o_inValidRange) const
{
   int o_tasknum = i_frame - m_frame_first;

   if( m_frames_per_task > 0 ) o_tasknum = o_tasknum / m_frames_per_task;
   else o_tasknum = o_tasknum * (-m_frames_per_task);

   if( m_frames_inc > 1 ) o_tasknum = o_tasknum / m_frames_inc;

   o_inValidRange = true;
   if( o_tasknum < 0 )
   { 
      o_tasknum = 0;
      o_inValidRange = false;
   }
   if( o_tasknum >= m_tasks_num )
   {
      o_tasknum = m_tasks_num - 1;
      o_inValidRange = false;
   }
   if(( i_frame < m_frame_first ) || ( i_frame > m_frame_last ))
		o_inValidRange = false;

   return o_tasknum;
}

void BlockData::setFramesPerTask( long long perTask)
{
   if( perTask == 0)
   {
      AFERROR("BlockData::setFramesPerTask: Can't set frames per task to zero.")
      return;
   }
   if( isNumeric())
   {
      AFERROR("BlockData::setFramesPerTask: The block is numeric.")
      return;
   }
   m_frames_per_task = perTask;
}

const std::string BlockData::genCommand( int num, long long * fstart, long long * fend) const
{
   std::string str;
   if( num > m_tasks_num)
   {
      AFERROR("BlockData::getCmd: n > tasksnum.")
      return str;
   }
   if( isNumeric())
   {
      long long start, end;
      bool ok = true;
      if( fstart && fend )
      {
         start = *fstart;
         end   = *fend;
      }
      else
         ok = genNumbers( start, end, num);

      if( ok)
         str = fillNumbers( m_command, start, end);
   }
   else
   {
      str = af::replaceArgs( m_command, m_tasks_data[num]->getCommand());
   }
   return str;
}

const std::string BlockData::genFiles( int num, long long * fstart, long long * fend) const
{
   std::string str;
   if( num >= m_tasks_num)
   {
      AFERROR("BlockData::genCmdView: n >= tasksnum.")
      return str;
   }
   if( isNumeric())
   {
      if( m_files.size())
      {
         long long start, end;
         bool ok = true;
         if( fstart && fend )
         {
            start = *fstart;
            end   = *fend;
         }
         else
            ok = genNumbers( start, end, num);

         if( ok)
            str = af::fillNumbers( m_files, start, end);
      }
   }
   else
   {       
      str = af::replaceArgs( m_files, m_tasks_data[num]->getFiles() );
   }
   return str;
}

TaskExec * BlockData::genTask( int num) const
{
   if( num > m_tasks_num)
   {
      AFERROR("BlockData::genTask: n > tasksnum.")
      return NULL;
   }

   long long start = -1;
   long long end = -1;
   long long frames_num = -1;

   if( false == genNumbers( start, end, num, &frames_num)) return NULL;

	TaskExec * taskExec = new TaskExec(
			genTaskName( num, &start, &end),
			m_service,
			m_parser,
			genCommand( num, &start, &end),
			m_capacity,
			m_file_size_min,
			m_file_size_max,
			genFiles( num, &start, &end),

			start,
			end,
			frames_num,

			m_working_directory,
			m_environment,

			m_job_id,
			m_block_num,
			m_flags,
			num
		);

	taskExec->m_custom_data_block = m_custom_data;
	if( isNotNumeric())
		taskExec->m_custom_data_task = m_tasks_data[num]->getCustomData();

	return taskExec;
}

const std::string BlockData::genTaskName( int num, long long * fstart, long long * fend) const
{
   if( num > m_tasks_num)
   {
      AFERROR("BlockData::genTaskName: n > tasksnum.")
      return std::string ("> tasksnum");
   }

   if( isNumeric())
   {
      long long start, end;
      bool ok = true;
      if( fstart && fend )
      {
         start = *fstart;
         end   = *fend;
      }
      else
         ok = genNumbers( start, end, num);

      if( false == ok) return std::string("Error generating numbers.");

      if( m_tasks_name.size()) return fillNumbers( m_tasks_name, start, end);

      std::string str("frame ");
      str += itos( start);
      if( start != end )
      {
         str += std::string("-") + itos( end);
         if( m_frames_inc > 1 ) str += std::string(" / ") + itos( m_frames_inc);
      }
      return str;
   }

   return af::replaceArgs( m_tasks_name, m_tasks_data[num]->getName());
}

void BlockData::setStateDependent( bool depend)
{
   if( depend)
   {
      m_state = m_state |   AFJOB::STATE_WAITDEP_MASK;
      m_state = m_state & (~AFJOB::STATE_READY_MASK);
   }
   else
   {
      m_state = m_state & (~AFJOB::STATE_WAITDEP_MASK);
   }
}

int BlockData::calcWeight() const
{
	int weight = sizeof(BlockData);
	if( isNotNumeric() && m_tasks_data)
		for( int t = 0; t < m_tasks_num; t++)
			weight += m_tasks_data[t]->calcWeight();

	weight += weigh(m_service);
	weight += weigh(m_parser);
	weight += m_depend_mask.weigh();
	weight += m_tasks_depend_mask.weigh();
	weight += m_need_properties.weigh();
	weight += m_hosts_mask.weigh();
	weight += m_hosts_mask_exclude.weigh();
	weight += weigh(m_name);
	weight += weigh(m_working_directory);
	weight += weigh(m_environment);
	weight += weigh(m_command);
	weight += weigh(m_files);
	weight += weigh(m_command_pre);
	weight += weigh(m_command_post);
	weight += weigh(m_custom_data);

	return weight;
}

const std::string BlockData::generateInfoStringTyped( int type, bool full) const
{
   std::ostringstream stream;
   generateInfoStreamTyped( stream, type, full);
   return stream.str();
}

void BlockData::generateInfoStreamTyped( std::ostringstream & o_str, int type, bool full) const
{
   switch( type)
   {
/*   case Msg::TJob:
   case Msg::TJobRegister:
   case Msg::TBlocks:
      break;*/

   case Msg::TBlocksProperties:

      if( full ) o_str << "\nProperties:";

      if( isNumeric())
      {
         o_str << "\n Frames: " << m_frame_first << " - " << m_frame_last << ": Per Task = " << m_frames_per_task;
         if( m_frames_inc > 1 ) o_str << ", Increment = " << m_frames_inc;
      }

      if( full && ( m_parser_coeff != 1 )) o_str << "\n Parser Coefficient = " << m_parser_coeff;

      if( false == m_tasks_name.empty()) o_str << "\n Tasks Name Pattern = " << m_tasks_name;

      if( full || ( ! m_parser.empty())) o_str << "\n Parser = " << m_parser;
      if( full && (   m_parser.empty())) o_str << " is empty (no parser)";
/*
      if( false == m_working_directory.empty()) o_str << "\n Working Directory:\n" << m_working_directory;
      if( false == m_command.empty()) o_str << "\n Command:\n" << m_command;
      if( false == m_environment.empty()) o_str << "\n Environment = " << m_environment;
      if( false == m_files.empty()) o_str << "\n Files:\n" << af::strReplace( m_files, ';', '\n');
      if( false == m_command_pre.empty()) o_str << "\n Pre Command:\n" << m_command_pre;
      if( false == m_command_post.empty()) o_str << "\n Post Command:\n" << m_command_post;
      if( false == m_custom_data.empty()) o_str << "\n Custom Data:\n" << m_custom_data;
*/
//      break;
//   case Msg::TJobsList:

      if(( m_file_size_min != -1 ) && ( m_file_size_max != -1 )) o_str << "Files Size: " << m_file_size_min << " - " << m_file_size_max;

      if( full ) o_str << "\n Capacity = " << m_capacity;
      if( canVarCapacity()) o_str << " x" << m_capacity_coeff_min << " - x" << m_capacity_coeff_max;

      if( isMultiHost() )
      {
         o_str << "\n MultiHost '" << m_multihost_service << "': x" << int(m_multihost_min) << " -x" << int(m_multihost_max);
         if( canMasterRunOnSlaveHost()) o_str << "\n    Master can run on the same slave host.";
         o_str << "\n    Wait service wait = " << m_multihost_service_wait;
         o_str << "\n    Wait maximum wait = " << m_multihost_max_wait;
      }

      if( isDependSubTask() ) o_str << "\n   Sub Task Dependence.";
      if( isNonSequential() ) o_str << "\n   Non-sequential tasks solving.";

      if( full || ( m_max_running_tasks != -1 )) o_str << "\n Max Running Tasks = " << m_max_running_tasks;
      if( full && ( m_max_running_tasks == -1 )) o_str << " (no limit)";
	  if( full || ( m_max_running_tasks_per_host != -1 ))
            o_str << "\n Max Running Tasks Per Host = " << m_max_running_tasks_per_host;
      if( full && ( m_max_running_tasks_per_host == -1 )) o_str << " (no limit)";

      if( m_need_memory > 0           ) o_str << "\n Needed Memory = "   << m_need_memory;
      if( m_need_power  > 0           ) o_str << "\n Need Power = "      << m_need_power;
      if( m_need_hdd    > 0           ) o_str << "\n Need HDD = "        << m_need_hdd;
      if( m_need_properties.notEmpty()) o_str << "\n Need Properties = " << m_need_properties.getPattern();

      if(        m_depend_mask.notEmpty()) o_str << "\n Depend Mask = "         << m_depend_mask.getPattern();
      if(   m_tasks_depend_mask.notEmpty()) o_str << "\n Tasks Depend Mask = "   << m_tasks_depend_mask.getPattern();
      if(         m_hosts_mask.notEmpty()) o_str << "\n Hosts Mask = "          << m_hosts_mask.getPattern();
      if( m_hosts_mask_exclude.notEmpty()) o_str << "\n Exclude Hosts Mask = "  << m_hosts_mask_exclude.getPattern();

      if( full ) o_str << "\n Service = " << m_service;
      if( full ) o_str << "\n Tasks Number = " << m_tasks_num;

      if( full || ( m_max_running_tasks     != -1 )) o_str << "\n Maximum running tasks = " << m_max_running_tasks;
      if( full && ( m_max_running_tasks     == -1 )) o_str << " (no limit)";

      if( full ) o_str << "\nErrors solving:";
      if( full || ( m_errors_avoid_host    != -1 )) o_str << "\n Errors for block avoid host = " << int(m_errors_avoid_host);
      if( full && ( m_errors_avoid_host    == -1 )) o_str << " (user settings used)";
      if( full || ( m_errors_task_same_host != -1 )) o_str << "\n Errors for task avoid host = " << int( m_errors_task_same_host);
      if( full && ( m_errors_task_same_host == -1 )) o_str << " (user settings used)";
      if( full || ( m_errors_retries      != -1 )) o_str << "\n Error task retries = " << int( m_errors_retries);
      if( full && ( m_errors_retries      == -1 )) o_str << " (user settings used)";
      if( full || ( m_errors_forgive_time  != -1 )) o_str << "\n Errors forgive time = " << m_errors_forgive_time << " seconds";
      if( full && ( m_errors_forgive_time  == -1 )) o_str << " (infinite)";

      break;

   case Msg::TBlocksProgress:

      if( full ) o_str << "\nRunning Progress:";

      if( p_tasks_done ) o_str << "\n Run Time: Sum = " << af::time2strHMS( p_tasks_run_time, true)
            << " / Average = " << af::time2strHMS( p_tasks_run_time/p_tasks_done, true);

      if( full ) o_str << "\n Tasks Ready = " << p_tasks_ready;
      if( full ) o_str << "\n Tasks Done = " << p_tasks_done;
      if( full ) o_str << "\n Tasks Error = " << p_tasks_error;

      if( p_error_hosts ) o_str << "\n Error hosts count = " << p_error_hosts;
      if( p_avoid_hosts ) o_str << "\n Avoid hosts count = " << p_avoid_hosts;

      break;

   default:
      o_str << "Can not generate type info for type = " << Msg::TNAMES[type];
      break;
   }
}

void BlockData::generateInfoStreamTasks( std::ostringstream & o_str, bool full) const
{
	if( m_tasks_num < 1 ) return;
	if( m_tasks_data == NULL ) return;
	for( int t = 0; t < m_tasks_num; t++ )
	{
		if( t > 0 )
            o_str << std::endl;
        m_tasks_data[t]->v_generateInfoStream( o_str, full);
	}
}

void BlockData::v_generateInfoStream( std::ostringstream & o_str, bool full) const
{
   o_str << "Block[" << m_name << "] " << m_service << "[" << m_capacity << "] " << m_tasks_num << " tasks";
   generateInfoStreamTyped( o_str, Msg::TBlocksProgress,   full);
   generateInfoStreamTyped( o_str, Msg::TBlocksProperties, full);
}


// Functions to update tasks progress and block progress bar:
// (for monitoring purpoces only, no meaning for server)
bool BlockData::updateProgress( JobProgress * progress)
{
   bool changed = false;

   if( updateBars( progress))
      changed = true;

	uint32_t  new_state          = 0;
	int32_t   new_percentage     = 0;
	int32_t   new_tasks_ready    = 0;
	int32_t   new_tasks_done     = 0;
	int32_t   new_tasks_error    = 0;
	int       new_tasks_skipped  = 0;
	int       new_tasks_warning  = 0;
	long long new_tasks_run_time = 0;


   for( int t = 0; t < m_tasks_num; t++)
   {
      uint32_t task_state   = progress->tp[m_block_num][t]->state;
      int8_t   task_percent = 0;

      if( task_state & AFJOB::STATE_READY_MASK   )
      {
         new_tasks_ready++;
      }
      if( task_state & AFJOB::STATE_DONE_MASK    )
      {
         new_tasks_done++;
         task_percent = 100;
         new_tasks_run_time += progress->tp[m_block_num][t]->time_done - progress->tp[m_block_num][t]->time_start;
      }
      if( task_state & AFJOB::STATE_RUNNING_MASK )
      {
         task_percent = progress->tp[m_block_num][t]->percent;
         if( task_percent <  0 ) task_percent = 0;
         else
         if( task_percent > 99 ) task_percent = 99;
      }
      if( task_state & AFJOB::STATE_ERROR_MASK   )
      {
         new_tasks_error++;
         task_percent = 0;
         new_tasks_run_time += progress->tp[m_block_num][t]->time_done - progress->tp[m_block_num][t]->time_start;
      }
		if( task_state & AFJOB::STATE_SKIPPED_MASK )
		{
			new_tasks_skipped++;
			task_percent = 100;
		}
		if( task_state & AFJOB::STATE_WARNING_MASK )
		{
			new_tasks_warning++;
		}

      new_percentage += task_percent;
   }
   new_percentage = new_percentage / m_tasks_num;

	if(( p_tasks_ready    != new_tasks_ready    )||
	   ( p_tasks_done     != new_tasks_done     )||
	   ( p_tasks_error    != new_tasks_error    )||
	   ( p_tasks_skipped  != new_tasks_skipped  )||
	   ( p_tasks_warning  != new_tasks_warning  )||
	   ( p_percentage     != new_percentage     )||
	   ( p_tasks_run_time != new_tasks_run_time ))
		changed = true;

	p_tasks_ready    = new_tasks_ready;
	p_tasks_done     = new_tasks_done;
	p_tasks_error    = new_tasks_error;
	p_tasks_skipped  = new_tasks_skipped;
	p_tasks_warning  = new_tasks_warning;
	p_percentage     = new_percentage;
	p_tasks_run_time = new_tasks_run_time;

   if( new_tasks_ready)
   {
      new_state = new_state | AFJOB::STATE_READY_MASK;
      new_state = new_state & (~AFJOB::STATE_DONE_MASK);
   }
   else
   {
      new_state = new_state & (~AFJOB::STATE_READY_MASK);
   }

   if( m_running_tasks_counter )
   {
      new_state = new_state |   AFJOB::STATE_RUNNING_MASK;
      new_state = new_state & (~AFJOB::STATE_DONE_MASK);
   }
   else
   {
      new_state = new_state & (~AFJOB::STATE_RUNNING_MASK);
   }

	if( new_tasks_done == m_tasks_num ) new_state = new_state |   AFJOB::STATE_DONE_MASK;
	else                                new_state = new_state & (~AFJOB::STATE_DONE_MASK);

	if( new_tasks_error) new_state = new_state |   AFJOB::STATE_ERROR_MASK;
	else                 new_state = new_state & (~AFJOB::STATE_ERROR_MASK);

	if( new_tasks_skipped) new_state = new_state |   AFJOB::STATE_SKIPPED_MASK;
	else                   new_state = new_state & (~AFJOB::STATE_SKIPPED_MASK);

	if( m_state & AFJOB::STATE_WAITDEP_MASK)
	new_state = new_state & (~AFJOB::STATE_READY_MASK);

   bool depend = m_state & AFJOB::STATE_WAITDEP_MASK;
   m_state = new_state;
   if( depend ) m_state = m_state | AFJOB::STATE_WAITDEP_MASK;

   return changed;
}

bool BlockData::updateBars( JobProgress * progress)
{
   bool changed = false;

	// Set to zeros:
	for( int i = 0; i < AFJOB::ASCII_PROGRESS_LENGTH; i++)
		p_progressbar[i] = 0;

	for( int t = 0; t < m_tasks_num; t++)
	{
		// Get maximum ASCII state for this task:
		int value = 0;
		for( int i = 0; i < AFJOB::ASCII_PROGRESS_COUNT; i++)
			if(( progress->tp[m_block_num][t]->state & AFJOB::ASCII_PROGRESS_MASK ) == AFJOB::ASCII_PROGRESS_STATES[i*2+1] )
				if( value < i) // More important for monitoring value
					value = i;

		// Calculate range:
		int pos_a = AFJOB::ASCII_PROGRESS_LENGTH * ( t   ) / m_tasks_num;
		int pos_b = AFJOB::ASCII_PROGRESS_LENGTH * ( t+1 ) / m_tasks_num;
		if( pos_b > pos_a )
			pos_b--;
		if( pos_b > AFJOB::ASCII_PROGRESS_LENGTH )
			pos_b = AFJOB::ASCII_PROGRESS_LENGTH - 1;

		for( int p = pos_a; p <= pos_b; p++)
		{
			// Set new value only if it is more important:
			if( value > p_progressbar[p] )
				p_progressbar[p] = value;
		}
	}

	// Transfer values to characters:
	for( int i = 0; i < AFJOB::ASCII_PROGRESS_LENGTH; i++)
		p_progressbar[i] = AFJOB::ASCII_PROGRESS_STATES[p_progressbar[i]*2];

//for( int i = 0; i < AFJOB::ASCII_PROGRESS_LENGTH; i++)  printf("%c", p_progressbar[i]); printf("\n");
   return changed;
}

void BlockData::stdOutProgress() const
{
   std::cout << generateProgressString() << std::endl;
}

const std::string BlockData::generateProgressString() const
{
   std::ostringstream stream;
   generateProgressStream( stream);
   return stream.str();
}

void BlockData::generateProgressStream( std::ostringstream & o_str) const
{
	o_str << std::string( p_progressbar, AFJOB::ASCII_PROGRESS_LENGTH);
	o_str << std::endl;
}
