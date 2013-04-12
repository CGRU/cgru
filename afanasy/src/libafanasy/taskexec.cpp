#include "taskexec.h"

#include "msg.h"
#include "address.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

TaskExec::TaskExec(
		const std::string & i_name,
		const std::string & i_service_type,
		const std::string & i_parser_type,
		const std::string & i_command,
		int i_capacity,
		int i_file_size_min,
		int i_file_size_max,
		const std::string & i_files,

		long long i_start_frame,
		long long i_end_frame,
		long long i_frames_num,

		const std::string & i_working_directory,
		const std::string & i_environment,

		int i_job_id,
		int i_block_number,
		unsigned int i_block_flags,
		int i_task_number,

		int i_parser_coeff
	):

	m_name( i_name),
	m_working_directory( i_working_directory),
	m_environment(  i_environment),
	m_command( i_command),
	m_files( i_files),
	m_service( i_service_type),
	m_parser( i_parser_type),
	m_parser_coeff( i_parser_coeff),
	m_capacity( i_capacity),
	m_capacity_coeff( 0),
	m_file_size_min( i_file_size_min),
	m_file_size_max( i_file_size_max),

	m_job_id( i_job_id),
	m_block_num( i_block_number),
	m_block_flags( i_block_flags),
	m_task_num(  i_task_number),
	m_number( 0),

	m_frame_start(   i_start_frame),
	m_frame_finish(  i_end_frame),
	m_frames_num(    i_frames_num),

	m_time_start( time(NULL)),
	m_progress( NULL),
	m_on_client( false)
{
AFINFA("TaskExec::TaskExec: %s:", m_job_name.toUtf8().data(), m_block_name.toUtf8().data(), m_name.toUtf8().data())
}
/*
TaskExec::TaskExec( const std::string & Command):
	m_command( Command),
	m_parser_coeff( 0),
	m_capacity( 0),
	m_capacity_coeff( 0),
	m_file_size_min( 0),
	m_file_size_max( 0),

	m_job_id(    0),
	m_block_num( 0),
	m_task_num(  0),
	m_number(   0),

	m_frame_start(   1),
	m_frame_finish(  1),
	m_frames_num(    1),

	m_time_start( time(NULL)),

	m_on_client( true)
{
}
*/
TaskExec::~TaskExec()
{
AFINFA("TaskExec:: ~ TaskExec: %s:\n", m_job_name.toUtf8().data(), m_block_name.toUtf8().data(), m_name.toUtf8().data());
}

TaskExec::TaskExec( Msg * msg):
	m_on_client( true)
{
	read( msg);
}

void TaskExec::jsonWrite( std::ostringstream & o_str, int i_type) const
{
	if( i_type != Msg::TRendersList )
		o_str << "\"task_exec\":";

	o_str << "{\"name\":\""       << m_name       << "\"";
	o_str << ",\"service\":\""    << m_service    << "\"";
	o_str << ",\"capacity\":"     << m_capacity;
	o_str << ",\"time_start\":"   << m_time_start;

	if( m_user_name.size())
		o_str << ",\"user_name\":\""  << m_user_name  << "\"";
	if( m_block_name.size())
		o_str << ",\"block_name\":\"" << m_block_name << "\"";
	if( m_job_name.size())
		o_str << ",\"job_name\":\""   << m_job_name   << "\"";

	if( m_capacity_coeff > 0 )
		o_str << ",\"capacity_coeff\":\"" << m_capacity_coeff;
	if( m_number > 0 )
		o_str << ",\"number\":\"" << m_number;

	if( i_type != Msg::TRendersList )
	{
		o_str << ",\"command\":\"" << af::strEscape( m_command  ) << "\"";
		o_str << ",\"frames_num\":" << m_frames_num;
		o_str << ",\"frame_start\":" << m_frame_start;
		o_str << ",\"frame_finish\":" << m_frame_finish;

		if( m_parser_coeff > 1 )
			o_str << ",\"parser_coeff\":" << m_parser_coeff;
		if( m_file_size_min > 0 )
			o_str << ",\"file_size_min\":" << m_file_size_min;
		if( m_file_size_max > 0 )
			o_str << ",\"file_size_max\":" << m_file_size_max;

		if( m_parser.size())
			o_str << ",\"parser\":\"" << m_parser << "\"";
		if( m_files.size())
			o_str << ",\"files\":\"" << af::strEscape( m_files  ) << "\"";
		if( m_working_directory.size())
			o_str << ",\"working_directory\":\"" << af::strEscape( m_working_directory ) << "\"";
		if( m_custom_data_task.size())
			o_str << ",\"custom_data_task\":\"" << af::strEscape( m_custom_data_task ) << "\"";
		if( m_custom_data_block.size())
			o_str << ",\"custom_data_block\":\"" << af::strEscape( m_custom_data_block ) << "\"";
		if( m_custom_data_job.size())
			o_str << ",\"custom_data_job\":\"" << af::strEscape( m_custom_data_job ) << "\"";
		if( m_custom_data_user.size())
			o_str << ",\"custom_data_user\":\"" << af::strEscape( m_custom_data_user ) << "\"";
		if( m_custom_data_render.size())
			o_str << ",\"custom_data_render\":\"" << af::strEscape( m_custom_data_render ) << "\"";
		if( m_environment.size())
			o_str << ",\"environment\":\"" << af::strEscape( m_environment ) << "\"";
	}

	o_str << "}";
}

void TaskExec::v_readwrite( Msg * msg)
{
	switch( msg->type())
	{
	case Msg::TTask:
		rw_int32_t ( m_job_id,            msg);
		rw_int32_t ( m_block_num,         msg);
		rw_uint32_t( m_block_flags,       msg);
		rw_uint32_t( m_job_flags,         msg);
		rw_uint32_t( m_user_flags,        msg);
		rw_uint32_t( m_render_flags,      msg);
		rw_int32_t ( m_task_num,          msg);
		rw_int64_t ( m_frames_num,        msg);
		rw_int64_t ( m_frame_start,       msg);
		rw_int64_t ( m_frame_finish,      msg);
		rw_int32_t ( m_parser_coeff,      msg);
		rw_int64_t ( m_file_size_min,     msg);
		rw_int64_t ( m_file_size_max,     msg);
		rw_String  ( m_command,           msg);
		rw_String  ( m_working_directory, msg);
		rw_String  ( m_environment,       msg);
		rw_String  ( m_files,             msg);
		rw_String  ( m_parser,            msg);
		rw_String  ( m_custom_data_task,  msg);
		rw_String  ( m_custom_data_block, msg);
		rw_String  ( m_custom_data_job,   msg);
		rw_String  ( m_custom_data_user,  msg);
		rw_String  ( m_custom_data_render,msg);

		rw_StringList( m_multihost_names, msg);

	case Msg::TRendersList:
		rw_String  ( m_service,           msg);
		rw_String  ( m_name,              msg);
		rw_String  ( m_user_name,         msg);
		rw_String  ( m_block_name,        msg);
		rw_String  ( m_job_name,          msg);
		rw_int32_t ( m_number,            msg);
		rw_int32_t ( m_capacity,          msg);
		rw_int32_t ( m_capacity_coeff,    msg);
		rw_int64_t ( m_time_start,        msg);

	break;

	default:
		AFERROR("TaskExec::readwrite: Invalid message type:\n");
		msg->v_stdOut( false);
		return;
	}

	m_listen_addresses.v_readwrite( msg);
}

void TaskExec::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	stream << "[" << m_service << ":" << m_capacity << "] " << m_user_name << ": ";
	stream << m_job_name;
	stream << "[" << m_block_name << "]";
	stream << "[" << m_name << "]";
	if( m_number != 0 ) stream << "(" << m_number << ")";
	if( m_capacity_coeff) stream << "x" << m_capacity_coeff << " ";
	if( m_listen_addresses.getAddressesNum())
		m_listen_addresses.v_generateInfoStream( stream, false);

	if(full)
	{
		stream << std::endl;
		if( m_working_directory.size()) stream << "   Working directory = \"" << m_working_directory << "\".\n";
		if(  m_environment.size()) stream << "   Environment = \""       <<  m_environment << "\".\n";
		stream << m_command;
	}
}

int TaskExec::calcWeight() const
{
	int weight = sizeof( TaskExec);
	weight += weigh( m_name);
	weight += weigh( m_block_name);
	weight += weigh( m_job_name);
	weight += weigh( m_user_name);
	weight += weigh( m_working_directory);
	weight += weigh( m_environment);
	weight += weigh( m_command);
	weight += weigh( m_files);
	weight += weigh( m_service);
	weight += weigh( m_parser);
	weight += weigh( m_custom_data_block);
	weight += weigh( m_custom_data_task);
	weight += m_listen_addresses.calcWeight();
	return weight;
}

