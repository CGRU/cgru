#include "renderreconnect.h"

#include <stdio.h>

#include "msg.h"
#include "name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

RenderReconnect::RenderReconnect():
	m_server_side( false)
{
}

RenderReconnect::RenderReconnect( Msg * msg):
	m_server_side( true)
{
	read( msg);
}

RenderReconnect::~RenderReconnect()
{}

void RenderReconnect::addTaskExec( TaskExec * i_task)
{
	for( int i = 0; i < m_taskexecs.size(); i++)
		if( m_taskexecs[i]->equals( *i_task))
		{
			m_taskexecs[i] = i_task;
			return;
		}

	m_taskexecs.push_back( i_task);
}

void RenderReconnect::v_readwrite( Msg * msg)
{
	// Render ID:
	rw_int32_t( m_id, msg);

	// Tasks updates:
	int32_t n = m_taskexecs.size();
	rw_int32_t( n, msg);
	for( int i = 0; i < n; i++)
	{
		if( msg->isReading())
			m_taskexecs.push_back( new TaskExec( msg));
		else
			m_taskexecs[i]->write( msg);
	}
}

void RenderReconnect::clear()
{
	m_taskexecs.clear();
}

void RenderReconnect::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	stream << " <<< RenderReconnect[" << m_id << "]:";
	if( m_taskexecs.size())
		stream << " TExecs[" << m_taskexecs.size() << "]";
}

