#include "mctask.h"

#include "../taskexec.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"
#include "../logger.h"

using namespace af;

const char * MCTask::TNAMES[] =
{
	"TNULL",
	"TExec",
	"TOutput",
	"TLog",
	"TErrorHosts",
	"TListen",
	"TLAST"
};

MCTask::MCTask( int i_job_id, int i_block_id, int i_task_id, int i_start_num):
	m_pos( i_job_id, i_block_id, i_task_id, i_start_num)
{
	init();
}

MCTask::MCTask( Msg * msg)
{
	init();
	read( msg);
}

void MCTask::init()
{
	m_type = TNULL;
	m_render_id = 0;
	m_exec = NULL;
}

MCTask::~MCTask()
{
	if( m_exec ) delete m_exec;
}


// Exec:
//
void MCTask::setExec( af::TaskExec * i_exec)
{
	if( m_type != TNULL ) AF_ERR << "Type is not NULL.";
	if( m_exec !=  NULL ) AF_ERR << "m_exec is not NULL.";
	if( i_exec ==  NULL ) AF_ERR << "i_exec is NULL.";

	if( m_exec ) delete m_exec;

	m_type = TExec;
	m_exec = i_exec;
}
af::TaskExec * MCTask::getExec()
{
	if( m_type != TExec ) AF_ERR << "Type is not TExec.";
	if( m_exec ==  NULL ) AF_ERR << "m_exec is NULL.";

	af::TaskExec * exec = m_exec;
	m_exec = NULL;

	return exec;
}


// Ouput:
//
void MCTask::setOutput( const std::string & i_output)
{
	if( m_type != TNULL ) AF_ERR << "Type is not NULL.";

	m_type = TOutput;
	m_data = i_output;
}
void MCTask::updateOutput( const std::string & i_output)
{
	if( m_type != TOutput ) AF_ERR << "Type is not TOutput.";

	m_data = i_output;
}
const std::string & MCTask::getOutput() const
{
	if( m_type != TOutput ) AF_ERR << "Type is not TOutput.";

	return m_data;
}


// Log:
//
void MCTask::setLog( const std::list<std::string> & i_list)
{
	if( m_type != TNULL ) AF_ERR << "Type is not NULL.";

	m_type = TLog;
	m_data = af::strJoin( i_list,"\n");
}
const std::string & MCTask::getLog() const
{
	if( m_type != TLog ) AF_ERR << "Type is not TLog.";

	return m_data;
}


// Error hosts:
//
void MCTask::setErrorHosts( const std::list<std::string> & i_list)
{
	if( m_type != TNULL ) AF_ERR << "Type is not NULL.";

	m_type = TErrorHosts;
	m_data = af::strJoin( i_list,"\n");
}
const std::string & MCTask::getErrorHosts() const
{
	if( m_type != TErrorHosts ) AF_ERR << "Type is not TLog.";

	return m_data;
}


// Listen:
//
void MCTask::setListened( const std::string & i_output)
{
	if( m_type != TNULL ) AF_ERR << "Type is not NULL.";

	m_type = TListen;
	m_data = i_output;
}
void MCTask::appendListened( const std::string & i_output)
{
	if( m_type != TListen ) AF_ERR << "Type is not TListen.";

	m_data += i_output;
}
const std::string & MCTask::getListened() const
{
	if( m_type != TListen ) AF_ERR << "Type is not TListen.";

	return m_data;
}


// I/O:
//
Msg * MCTask::generateMessage( bool i_binary)
{
	Msg * msg = new Msg();

	if( i_binary )
	{
		msg->set( Msg::TTask, this);
	}
	else
	{
		std::ostringstream str;
		str << "{\"task\":";
		jsonWrite( str);
		str << "}";
		std::string text = str.str();
		msg->setData( text.size(), text.c_str(), af::Msg::TJSON);
	}

	return msg;
}

void MCTask::v_readwrite( Msg * io_msg)
{
	rw_int32_t( m_type, io_msg);
	m_pos.v_readwrite( io_msg);
	m_progress.v_readwrite( io_msg);

	rw_int32_t( m_render_id, io_msg);

	rw_String ( m_job_name,   io_msg);
	rw_String ( m_block_name, io_msg);
	rw_String ( m_task_name,  io_msg);
	rw_String ( m_service,    io_msg);
	rw_String ( m_parser,     io_msg);

	rw_String ( m_data, io_msg);

	if( m_type == TExec )
	{
		if( io_msg->isWriting())
			m_exec->v_readwrite( io_msg);
		else
			m_exec = new af::TaskExec( io_msg);
	}
}

void MCTask::jsonWrite( std::ostringstream & o_str) const
{
	o_str << "{\"type\":\""    << TNAMES[m_type] << "\"";

	o_str << ",\"render_id\":" << m_render_id;

	o_str << ",\"job_name\":\""   << m_job_name << "\"";
	o_str << ",\"block_name\":\"" << m_block_name << "\"";
	o_str << ",\"task_name\":\""  << m_task_name << "\"";
	o_str << ",\"service\":\""    << m_service << "\"";
	o_str << ",\"parser\":\""     << m_parser << "\"";

	o_str << ",\"pos\":";
	m_pos.jsonWrite( o_str);

	o_str << ",\"progress\":";
	m_progress.jsonWrite( o_str);

	if( m_type == TExec )
	{
		o_str << ",\"exec\":";
		m_exec->jsonWrite( o_str, af::Msg::TTask);
	}

	o_str << ",\"data\":\"" << af::strEscape(m_data) << "\"}";
}

void MCTask::v_generateInfoStream( std::ostringstream & o_str, bool i_full) const
{
	if( i_full )
	{
		o_str << "Task['" << TNAMES[m_type] << "']: ";
		o_str << "Pos: " << m_pos;
		o_str << "\nProgress: " << m_progress;
		o_str << "\nJobName = '" << m_job_name << "', << BlockName = '" << m_block_name << "', TaskName = '" << m_task_name << "'";
		o_str << "\nService = '" << m_service << "', Parser  = '" << m_parser << "'";
		if( m_data.size()) o_str << "\n" << m_data;
	}
	else
	{
		o_str << "T['" << TNAMES[m_type] << "']:" << m_pos << " " << m_progress;
		o_str << " ['" << m_service << "']" << "['" << m_parser << "']";
		if( m_data.size()) o_str << "\n" << m_data;
	}
}
