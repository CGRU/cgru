#include "renderevents.h"

#include <stdio.h>

#include "msg.h"
#include "name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

RenderEvents::RenderEvents()
{
}

RenderEvents::RenderEvents( Msg * msg)
{
	read( msg);
}

RenderEvents::~RenderEvents()
{
}

void RenderEvents::remTaskExec( const TaskExec * i_exec )
{
	for( std::vector<af::TaskExec*>::iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
	if( *it == i_exec)
	{
		m_tasks.erase( it);
		return;
	}
}

void RenderEvents::addUniqueTask( const MCTaskPos & i_tp, std::vector<MCTaskPos> & o_vec)
{
	for( int i = 0; i < o_vec.size(); i++)
		if( o_vec[i].isEqual( i_tp))
			return;

	o_vec.push_back( i_tp);
}

void RenderEvents::rw_tp_vec( std::vector<MCTaskPos> & io_vec, Msg * io_msg)
{
	int32_t len = io_vec.size();
	rw_int32_t( len, io_msg);
	for( int i = 0; i < len; i++)
	{
		if( io_msg->isReading())
			io_vec.push_back( MCTaskPos());
		io_vec[i].v_readwrite( io_msg);
	}
}

void RenderEvents::rw_texecs( std::vector<TaskExec*> & io_vec, Msg * io_msg)
{
	int32_t len = m_tasks.size();
	rw_int32_t( len, io_msg);
	for( int i = 0; i < len; i++)
	{
		if( io_msg->isReading())
			m_tasks.push_back( new TaskExec( io_msg));
		else
			m_tasks[i]->write( io_msg);
	}
}

void RenderEvents::v_readwrite( Msg * msg)
{
	rw_texecs( m_tasks,       msg);
	rw_tp_vec( m_closes,      msg);
	rw_tp_vec( m_stops,       msg);
	rw_tp_vec( m_outputs,     msg);
	rw_tp_vec( m_listens_add, msg);
	rw_tp_vec( m_listens_rem, msg);
	rw_String( m_instruction, msg);
	rw_String( m_command,     msg);
}

void RenderEvents::clear()
{
	m_tasks.clear();

	m_closes.clear();
	m_stops.clear();
	m_outputs.clear();
	m_listens_add.clear();
	m_listens_rem.clear();

	m_instruction.clear();
	m_command.clear();
}

bool RenderEvents::isEmpty() const
{
	if( m_tasks.size()) return false;

	if( m_closes.size()) return false;
	if( m_stops.size()) return false;
	if( m_outputs.size()) return false;
	if( m_listens_add.size()) return false;
	if( m_listens_rem.size()) return false;

	if( m_instruction.size()) return false;
	if( m_command.size()) return false;

	return true;
}

void RenderEvents::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	stream << " >>> RenderEvents:";

	if( m_tasks.size())
		stream << " Exec["  << m_tasks.size()  << "]";

	if( m_closes.size())
		stream << " Close[" << m_closes.size() << "]";

	if( m_stops.size())
		stream << " Stop["  << m_stops.size()  << "]";

	if( m_outputs.size())
		stream << " Out["  << m_outputs.size()  << "]";

	if( m_listens_add.size())
		stream << " Li+["  << m_listens_add.size()  << "]";

	if( m_listens_rem.size())
		stream << " Li-["  << m_listens_rem.size()  << "]";

	if( m_instruction.size())
		stream << " I[" << m_instruction << "]";
}

