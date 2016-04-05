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

void RenderEvents::addTaskClose( const MCTaskPos & i_tp)
{
	for( int i = 0; i < m_closes.size(); i++)
		if( m_closes[i].equal( i_tp))
			return;

	m_closes.push_back( i_tp);
}

void RenderEvents::addTaskStop(  const MCTaskPos & i_tp)
{
	for( int i = 0; i < m_stops.size(); i++)
		if( m_stops[i].equal( i_tp))
			return;

	m_stops.push_back( i_tp);
}

void RenderEvents::v_readwrite( Msg * msg)
{
	int32_t len = m_tasks.size();
	rw_int32_t( len, msg);
	for( int i = 0; i < len; i++)
	{
		if( msg->isReading())
			m_tasks.push_back( new TaskExec( msg));
		else
			m_tasks[i]->write( msg);
	}

	len = m_closes.size();
	rw_int32_t( len, msg);
	for( int i = 0; i < len; i++)
	{
		if( msg->isReading())
			m_closes.push_back( MCTaskPos());
		m_closes[i].v_readwrite( msg);
	}

	len = m_stops.size();
	rw_int32_t( len, msg);
	for( int i = 0; i < len; i++)
	{
		if( msg->isReading())
			m_stops.push_back( MCTaskPos());
		m_stops[i].v_readwrite( msg);
	}

	rw_String( m_instruction, msg);
	rw_String( m_command, msg);
}

void RenderEvents::clear()
{
	m_tasks.clear();
	m_closes.clear();
	m_stops.clear();
	m_instruction.clear();
	m_command.clear();
}

bool RenderEvents::isEmpty() const
{
	if( m_tasks.size()) return false;
	if( m_closes.size()) return false;
	if( m_stops.size()) return false;
	if( m_instruction.size()) return false;
	if( m_command.size()) return false;

	return true;
}

void RenderEvents::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	stream << " >>> RenderEvents:";
	stream << " Execs["  << m_tasks.size()  << "]";
	stream << " Closes[" << m_closes.size() << "]";
	stream << " Stops["  << m_stops.size()  << "]";

	if( m_instruction.size())
		stream << " I[" << m_instruction << "]";
}

