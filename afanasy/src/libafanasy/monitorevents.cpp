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

void MonitorEvents::addOutput( const af::MCTask & i_mctask)
{
	for( int i = 0; i < m_outputs.size(); i++)
		if( m_outputs[i].isSameTask( i_mctask))
		{
			m_outputs[i].updateOutput( i_mctask.getOutput());
			return;
		}

	m_outputs.push_back( i_mctask);
}

void MonitorEvents::addListened( const af::MCTask & i_mctask)
{
	for( int i = 0; i < m_listens.size(); i++)
	{
		if( m_listens[i].isSameTask( i_mctask))
		{
			m_listens[i].appendListened( i_mctask.getListened());
			return;
		}
	}

	m_listens.push_back( i_mctask);
}

void MonitorEvents::v_readwrite( Msg * msg)
{
	// General events:
	for( int e = 0; e < m_events.size(); e++)
	{
		std::vector<int32_t> vect = m_events[e];
		rw_Int32_Vect( vect, msg);
		if( msg->isReading())
			m_events[e] = vect;
	}


	// Tasks progresses:
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


	// Block ids with modes:
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


	// Task outputs:
	int32_t outs_size = m_outputs.size();
	rw_int32_t( outs_size, msg);
	for( int i = 0; i < outs_size; i++)
	{
		if( msg->isReading())
			m_outputs.push_back( MCTask());

		m_outputs[i].v_readwrite( msg);
	}


	// User jobs order:
	rw_Int32_Vect( m_jobs_order_ids, msg);


	// Instructions:
	rw_String( m_instruction, msg);


	// Listening:
	int32_t lis_size = m_listens.size();
	rw_int32_t( lis_size, msg);
	for( int i = 0; i < lis_size; i++)
	{
		if( msg->isReading())
			m_listens.push_back( MCTask());

		m_listens[i].v_readwrite( msg);
	}


	// Message
	rw_String( m_message, msg);
}

void MonitorEvents::jsonWrite( std::ostringstream & o_str) const
{
	bool hasevents = false;

	// Nodes events:
	for( int e = 0; e < af::Monitor::EVT_COUNT; e++)
	{
		if( m_events[e].size() == 0 ) continue;

		if( hasevents ) o_str << ","; else o_str << "{";

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
		if( hasevents ) o_str << ","; else o_str << "{";

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
		if( hasevents ) o_str << ","; else o_str << "{";

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


	// Jobs order:
	if( m_jobs_order_ids.size())
	{
		if( hasevents ) o_str << ","; else o_str << "{";

		o_str << "\n\"jobs_order_ids\":[";
		for( int i = 0; i < m_jobs_order_ids.size(); i++)
		{
			if( i ) o_str << ",";
			o_str << m_jobs_order_ids[i];
		}
		o_str << "]";

		hasevents = true;
	}


	// Instruction:
	if( m_instruction.size())
	{
		if( hasevents ) o_str << ","; else o_str << "{";

		o_str << "\n\"instruction\":\"" << m_instruction << "\"";

		hasevents = true;
	}


	// Tasks outputs:
	if( m_outputs.size())
	{
		if( hasevents ) o_str << ","; else o_str << "{";

		o_str << "\n\"tasks_outputs\":[";
		for( int i = 0; i < m_outputs.size(); i++)
		{
			if( i ) o_str << ",\n";

			m_outputs[i].jsonWrite( o_str);
		}
		o_str << "]";

		hasevents = true;
	}


	// Tasks listens:
	if( m_listens.size())
	{
		if( hasevents ) o_str << ","; else o_str << "{";

		o_str << "\n\"tasks_listens\":[";
		for( int i = 0; i < m_listens.size(); i++)
		{
			if( i ) o_str << ",\n";

			m_listens[i].jsonWrite( o_str);
		}
		o_str << "]";

		hasevents = true;
	}


	// Message:
	if( m_message.size())
	{
		if( hasevents ) o_str << ","; else o_str << "{";

		o_str << "\n\"message\":\"" << m_message << "\"";

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

	m_instruction.clear();

	m_listens.clear();

	m_outputs.clear();

	m_message.clear();
}

bool MonitorEvents::isEmpty() const
{
	for( int e = 0; e < m_events.size(); e++)
		if( m_events[e].size())
			return false;

	if( m_tp.size()) return false;

	if( m_bids.size()) return false;

	if( m_jobs_order_ids.size()) return false;

	if( m_instruction.size()) return false;

	if( m_outputs.size()) return false;

	if( m_listens.size()) return false;

	if( m_message.size()) return false;

	return true;
}

void MonitorEvents::v_generateInfoStream( std::ostringstream & o_str, bool i_full) const
{
	o_str << "MonitorEvents:";

	for( int i = 0; i < m_events.size(); i++)
	{
		if( m_events[i].size())
		{
			o_str << " '" << Monitor::EVT_NAMES[i] << "'[" << m_events[i].size() << "]:";
			for( int j = 0; j < m_events[i].size(); j++)
				o_str << " " << m_events[i][j];
		}
	}

	if( m_jobs_order_ids.size())
	{
		o_str << " JORD[" << m_jobs_order_ids.size() << "]:";
		for( int i = 0; i < m_jobs_order_ids.size(); i++)
			o_str << " " << m_jobs_order_ids[i];
	}

	if( m_tp.size())
		o_str << " TP[" << m_tp.size() << "]";

	if( m_bids.size())
		o_str << " BID[" << m_bids.size() << "]";

	if( m_outputs.size())
		o_str << " OUT[" << m_outputs.size() << "]";

	if( m_listens.size())
		o_str << " LIS[" << m_listens.size() << "]";

	if( m_instruction.size())
		o_str << " i\"" << m_instruction << "\"";

	if( m_message.size())
		o_str << " " << m_message;

	o_str << "\n";
}

