#include "renderupdate.h"

#include <stdio.h>

#include "msg.h"
#include "name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

RenderUpdate::RenderUpdate():
	m_hres( NULL),
	m_has_hres( false),
	m_server_side( false)
{
}

RenderUpdate::RenderUpdate( Msg * msg):
	m_server_side( true)
{
	read( msg);
}

RenderUpdate::~RenderUpdate()
{
	for( int i = 0; i < m_taskups.size(); i++)
		delete m_taskups[i];

	if( m_has_hres && m_server_side )
		delete m_hres;
}

void RenderUpdate::addTaskUp( MCTaskUp * i_tup)
{
	for( int i = 0; i < m_taskups.size(); i++)
		if( m_taskups[i]->isSameTask( *i_tup))
		{
			delete m_taskups[i];
			m_taskups[i] = i_tup;
			return;
		}

	m_taskups.push_back( i_tup);
}

void RenderUpdate::addTaskOutput( const MCTaskPos & i_tp, const std::string & i_output)
{
	for( int i = 0; i < m_outspos.size(); i++)
		if( m_outspos[i].isEqual( i_tp))
		{
			m_outputs[i] = i_output;
			return;
		}

	m_outspos.push_back( i_tp);
	m_outputs.push_back( i_output);
}

void RenderUpdate::v_readwrite( Msg * msg)
{
	// Render ID:
	rw_int32_t( m_id, msg);


	// Tasks updates:
	int32_t tups_len = m_taskups.size();
	rw_int32_t( tups_len, msg);
	for( int i = 0; i < tups_len; i++)
	{
		if( msg->isReading())
			m_taskups.push_back( new MCTaskUp( msg));
		else
			m_taskups[i]->write( msg);
	}


	// Tasks outptus:
	int32_t outs_len = m_outputs.size();
	rw_int32_t( outs_len, msg);
	for( int i = 0; i < outs_len; i++)
	{
		if( msg->isReading())
		{
			m_outspos.push_back( MCTaskPos());
			m_outputs.push_back( std::string());
		}

		m_outspos[i].v_readwrite( msg);
		rw_String( m_outputs[i], msg);
	}


	// Resources:
	rw_bool( m_has_hres, msg);
	if( m_has_hres )
	{
		if( msg->isReading())
			m_hres = new HostRes( msg);
		else
			m_hres->v_readwrite( msg);
	}
}

void RenderUpdate::clear()
{
	for( int i = 0; i < m_taskups.size(); i++)
		delete m_taskups[i];
	m_taskups.clear();

	m_outputs.clear();
	m_outspos.clear();

	m_has_hres = false;
	m_hres = NULL;
}

void RenderUpdate::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	stream << " <<< RenderUpdate[" << m_id << "]:";
	if( m_taskups.size())
		stream << " TUps[" << m_taskups.size() << "]";
	if( m_outputs.size())
		stream << " Outs[" << m_outputs.size() << "]";
	if( m_has_hres )
		stream << " HRes()";
}

