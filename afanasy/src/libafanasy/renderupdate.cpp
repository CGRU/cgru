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
		if( m_taskups[i]->sameTask( *i_tup))
		{
			delete m_taskups[i];
			m_taskups[i] = i_tup;
			return;
		}

	m_taskups.push_back( i_tup);
}

void RenderUpdate::v_readwrite( Msg * msg)
{
	rw_int32_t( m_id, msg);


	int32_t tups_len = m_taskups.size();
	rw_int32_t( tups_len, msg);
	for( int i = 0; i < tups_len; i++)
	{
		if( msg->isReading())
			m_taskups.push_back( new MCTaskUp( msg));
		else
			m_taskups[i]->write( msg);
	}


	rw_bool( m_has_hres, msg);
	if( false == m_has_hres )
		return;

	if( msg->isReading())
		m_hres = new HostRes( msg);
	else
		m_hres->v_readwrite( msg);
}

void RenderUpdate::clear()
{
	for( int i = 0; i < m_taskups.size(); i++)
		delete m_taskups[i];
	m_taskups.clear();

	m_has_hres = false;
	m_hres = NULL;
}

void RenderUpdate::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	stream << " <<< RenderUpdate[" << m_id << "]:";
	stream << " TUps[" << m_taskups.size() << "]";
	stream << " h" << ( m_has_hres ? "1" : "0");
	stream << " s" << ( m_server_side ? "1" : "0");
}

