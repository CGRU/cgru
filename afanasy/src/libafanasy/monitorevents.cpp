#include "monitorevents.h"

#include <stdio.h>

#include "monitor.h"
#include "msg.h"
#include "name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

MonitorEvents::MonitorEvents()
{
	m_events.resize( af::Monitor::EVT_COUNT);
}

MonitorEvents::MonitorEvents( Msg * msg)
{
	m_events.resize( af::Monitor::EVT_COUNT);
	read( msg);
}

MonitorEvents::~MonitorEvents()
{
}

void MonitorEvents::v_readwrite( Msg * msg)
{
	for( int e = 0; e < m_events.size(); e++)
	{
		std::vector<int32_t> vect = m_events[e];
		rw_Int32_Vect( vect, msg);
		if( msg->isReading())
			m_events[e] = vect;
	}


	int32_t tp_size = m_tp.size();
	rw_int32_t( tp_size, msg);
	for( int t = 0; t < tp_size; t++)
	{
		if( msg->isReading())
			m_tp.push_back( MTaskProgresses());

		rw_int32_t(    m_tp[t].job_id, msg);
		rw_Int32_Vect( m_tp[t].blocks, msg);
		rw_Int32_Vect( m_tp[t].tasks,  msg);
		for( int p = 0; p < m_tp[t].tasks.size(); p++)
		{
			if( msg->isReading())
				m_tp[t].tp.push_back( TaskProgress( msg));
			else
				m_tp[t].tp[p].v_readwrite( msg);
		}
	}


	int32_t bid_size = m_bids.size();
	rw_int32_t( bid_size, msg);
	for( int b = 0; b < bid_size; b++)
	{
		if( msg->isReading())
			m_bids.push_back( MBlocksIds());

		rw_int32_t( m_bids[b].job_id,    msg);
		rw_int32_t( m_bids[b].block_num, msg);
		rw_int32_t( m_bids[b].mode,      msg);
	}


	rw_Int32_Vect( m_jobs_order_uids, msg);

	int32_t size = m_jobs_order_jids.size();
	rw_int32_t( size, msg);
	for( int i = 0; i < size; i++)
	{
		if( msg->isReading())
			m_jobs_order_jids.push_back(std::vector<int32_t>());

		rw_Int32_Vect( m_jobs_order_jids[i], msg);
	}
}

void MonitorEvents::clear()
{
	for( int e = 0; e < m_events.size(); e++)
		m_events[e].clear();

	m_tp.clear();

	m_bids.clear();

	m_jobs_order_uids.clear();
	m_jobs_order_jids.clear();
}

bool MonitorEvents::isEmpty() const
{
	for( int e = 0; e < m_events.size(); e++)
		if( m_events[e].size())
			return false;

	if( m_tp.size()) return false;

	if( m_bids.size()) return false;

	if( m_jobs_order_uids.size()) return false;
	if( m_jobs_order_jids.size()) return false;

	return true;
}

