#include "monitorevents.h"

#include <stdio.h>

#include "../libafanasy/blockdata.h"

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

void MonitorEvents::addInstruction( const std::string & i_str)
{
	for( int i = 0; i < m_instructions.size(); i++)
		if( i_str == m_instructions[i])
			return;

	m_instructions.push_back( i_str);
}

void MonitorEvents::addListened( MListen i_listen)
{
	for( int i = 0; i < m_listens.size(); i++)
	{
		if(( m_listens[i].job_id == i_listen.job_id ) &&
			( m_listens[i].block == i_listen.block ) &&
			( m_listens[i].task == i_listen.task ))
		{
			m_listens[i].output += i_listen.output;
			return;
		}
	}

	m_listens.push_back( i_listen);
printf("MonitorEvents::addListened: m_listens.size() = %d\n", m_listens.size());
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


	rw_Int32_Vect( m_jobs_order_ids, msg);

	rw_StringVect( m_instructions, msg);

	int32_t lis_size = m_listens.size();
	rw_int32_t( lis_size, msg);
	for( int i = 0; i < lis_size; i++)
	{
		if( msg->isReading())
			m_listens.push_back( MListen());

		rw_int32_t( m_listens[i].job_id,   msg);
		rw_int32_t( m_listens[i].block,    msg);
		rw_int32_t( m_listens[i].task,     msg);
		rw_String ( m_listens[i].hostname, msg);
		rw_String ( m_listens[i].output,   msg);
	}
}

void MonitorEvents::jsonWrite( std::ostringstream & o_str) const
{
	bool hasevents = false;
	// Nodes events:
	for( int e = 0; e < af::Monitor::EVT_COUNT; e++)
	{
		if( m_events[e].size() == 0 )
			continue;

		if( hasevents )
			o_str << ",";
		else
			o_str << "{";

		o_str << "\n\"" << af::Monitor::EVT_NAMES[e] << "\":";
		o_str << "[";

		for( int i = 0; i < m_events[e].size(); i++)
		{
			if( i )
				o_str << ",";
			o_str << m_events[e][i];
		}

		o_str << "]";
		hasevents = true;
	}

	// Tasks progress:
	if( m_tp.size())
	{
		if( hasevents )
			o_str << ",";
		else
			o_str << "{";

		o_str << "\n\"tasks_progress\":[";
		for( int j = 0; j < m_tp.size(); j++)
		{
			if( j > 0 ) o_str << ",";
			o_str << "{\"job_id\":" << m_tp[j].job_id;
			o_str << ",\"blocks\":[";
			for( int t = 0; t < m_tp[j].blocks.size(); t++)
			{
				if( t > 0 ) o_str << ",";
				o_str << m_tp[j].blocks[t];
			}
			o_str << "],\"tasks\":[";
			for( int t = 0; t < m_tp[j].tasks.size(); t++)
			{
				if( t > 0 ) o_str << ",";
				o_str << m_tp[j].tasks[t];
			}
			o_str << "],\"progress\":[";
			for( int t = 0; t < m_tp[j].tp.size(); t++)
			{
				if( t > 0 ) o_str << ",";
				m_tp[j].tp[t].jsonWrite( o_str);
			}	
			o_str << "]}";
		}
		o_str << "]";
		hasevents = true;
	}

	// Blocks ids:
	if( m_bids.size())
	{
		if( hasevents )
			o_str << ",";
		else
			o_str << "{";

		o_str << "\n\"block_ids\":{";

		o_str << "\"job_id\":[";
		for( int i = 0; i < m_bids.size(); i++)
		{
			if( i > 0 ) o_str << ",";
			o_str << m_bids[i].job_id;
		}
		o_str << "],";

		o_str << "\"block_num\":[";
		for( int i = 0; i < m_bids.size(); i++)
		{
			if( i > 0 ) o_str << ",";
			o_str << m_bids[i].block_num;
		}
		o_str << "],";

		o_str << "\"mode\":[";
		for( int i = 0; i < m_bids.size(); i++)
		{
			if( i > 0 ) o_str << ",";
			o_str << '"' << af::BlockData::DataModeFromMsgType( m_bids[i].mode) << '"';
		}
		o_str << "]}";

		hasevents = true;
	}

	if( m_jobs_order_ids.size())
	{
		if( hasevents )
			o_str << ",";
		else
			o_str << "{";

		o_str << "\"jobs_order_ids\":[";
		for( int i = 0; i < m_jobs_order_ids.size(); i++)
		{
			if( i ) o_str << ",";
			o_str << m_jobs_order_ids[i];
		}
		o_str << "]}";

		hasevents = true;
	}

	if( m_instructions.size())
	{
		if( hasevents )
			o_str << ",";
		else
			o_str << "{";

		o_str << "\"instructions\":[";
		for( int i = 0; i < m_instructions.size(); i++)
		{
			if( i ) o_str << ",";
			o_str << "\"" << m_instructions[i] << "\"";
		}
		o_str << "]}";

		hasevents = true;
	}


	if( false == hasevents )
		o_str << "\nnull";
	else
		o_str << "\n}";
}

void MonitorEvents::clear()
{
	for( int e = 0; e < m_events.size(); e++)
		m_events[e].clear();

	m_tp.clear();

	m_bids.clear();

	m_jobs_order_ids.clear();

	m_instructions.clear();

	m_listens.clear();
}

bool MonitorEvents::isEmpty() const
{
	for( int e = 0; e < m_events.size(); e++)
		if( m_events[e].size())
			return false;

	if( m_tp.size()) return false;

	if( m_bids.size()) return false;

	if( m_jobs_order_ids.size()) return false;

	if( m_instructions.size()) return false;

	return true;
}

