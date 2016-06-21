#include "mctaskoutput.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"
#include "../logger.h"

using namespace af;

MCTaskOutput::MCTaskOutput( int i_job_id, int i_block_id, int i_task_id, int i_start_num):
	m_job_id(    i_job_id    ),
	m_block_id(  i_block_id  ),
	m_task_id(   i_task_id   ),
	m_start_num( i_start_num ),
	m_render_id(0)
{
}

MCTaskOutput::MCTaskOutput( Msg * msg){ read( msg);}

MCTaskOutput::~MCTaskOutput(){}

bool MCTaskOutput::isSameTask( const MCTaskOutput & i_other) const
{
	if( i_other.m_job_id   != m_job_id   ) return false;
	if( i_other.m_block_id != m_block_id ) return false;
	if( i_other.m_task_id  != m_task_id  ) return false;

	return true;
}

bool MCTaskOutput::isSameTask( const MCTaskPos & i_tp) const
{
	if( m_job_id   != i_tp.getJobId()    ) return false;
	if( m_block_id != i_tp.getBlockNum() ) return false;
	if( m_task_id  != i_tp.getTaskNum()  ) return false;

	return true;
}

Msg * MCTaskOutput::generateMessage( bool i_binary)
{
	Msg * msg = new Msg();

	if( i_binary )
	{
		msg->set( Msg::TTaskOutput, this);
	}
	else
	{
		std::ostringstream str;
		str << "{\"task_output\":";
		jsonWrite( str);
		str << "}";
		std::string text = str.str();
		msg->setData( text.size(), text.c_str(), af::Msg::TJSON);
	}

	return msg;
}

void MCTaskOutput::v_readwrite( Msg * io_msg)
{
	rw_int32_t( m_job_id,    io_msg);
	rw_int32_t( m_block_id,  io_msg);
	rw_int32_t( m_task_id,   io_msg);
	rw_int32_t( m_start_num, io_msg);
	rw_int32_t( m_render_id, io_msg);

	rw_String ( m_job_name,   io_msg);
	rw_String ( m_block_name, io_msg);
	rw_String ( m_task_name,  io_msg);
	rw_String ( m_service,    io_msg);
	rw_String ( m_parser,     io_msg);

	rw_String ( m_output, io_msg);
}

void MCTaskOutput::jsonWrite( std::ostringstream & o_str) const
{
	o_str << "{\"job_id\":"    << m_job_id;
	o_str << ",\"block_id\":"  << m_block_id;
	o_str << ",\"task_id\":"   << m_task_id;
	o_str << ",\"start_num\":" << m_start_num;
	o_str << ",\"render_id\":" << m_render_id;

	o_str << ",\"job_name\":\""   << m_job_name << "\"";
	o_str << ",\"block_name\":\"" << m_block_name << "\"";
	o_str << ",\"task_name\":\""  << m_task_name << "\"";
	o_str << ",\"service\":\""    << m_service << "\"";
	o_str << ",\"parser\":\""     << m_parser << "\"";
	o_str << ",\"filename\":\""   << m_filename << "\"";

	o_str << ",\"output\":\"" << af::strEscape(m_output) << "\"}";
}

void MCTaskOutput::v_generateInfoStream( std::ostringstream & o_str, bool i_full) const
{
	if( i_full )
	{
		o_str << "Task Ouput: JobID=" << m_job_id << ", Block=" << m_block_id << ", Task=" << m_task_id << ", Start=" << m_start_num;
		o_str << "\nJobName = '" << m_job_name << "', << BlockName = '" << m_block_name << "', TaskName = '" << m_task_name << "'";
		o_str << "\nService = '" << m_service << "', Parser  = '" << m_parser << "'";
		o_str << "\n" << m_output;
	}
	else
	{
		o_str << "TO";
		o_str << "[" << m_job_id << "][" << m_block_id << "][" << m_task_id << "](" << m_start_num << ")";
		o_str << "['" << m_service << "']";
		o_str << "['" << m_parser << "']";
		o_str << "\n" << m_output;
	}
}
