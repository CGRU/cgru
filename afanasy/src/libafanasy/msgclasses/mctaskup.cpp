#include "mctaskup.h"

#include <stdio.h>
#include <memory.h>

#include "../msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCTaskUp::MCTaskUp
(
	int clientId,

	int numJob,
	int numBlock,
	int numTask,
	int number,

	int UpStatus,
	int UpPercent,
	int UpFrame,
	int UpPercentFrame,
	std::string activity,
	int dataLen,
	char * data
):
	m_clientid      ( clientId),

	m_numjob        ( numJob),
	m_numblock      ( numBlock),
	m_numtask       ( numTask),
	m_number        ( number),

	m_status        ( UpStatus),
	m_percent       ( UpPercent),
	m_frame         ( UpFrame),
	m_percent_frame ( UpPercentFrame),
	m_activity      ( activity),
	m_datalen       ( dataLen ),
	m_data          ( data ),
	m_deleteData    ( false) // Don not delete data on client side, as it is not copied
{
}

MCTaskUp::MCTaskUp( Msg * msg):
	m_data ( NULL),
	m_deleteData( true)       // Delete data on server side, as it was allocated and copied from incoming message
{
	read( msg);
}

MCTaskUp::~MCTaskUp()
{
	if( m_deleteData && m_data ) delete [] m_data;
}

void MCTaskUp::v_readwrite( Msg * msg)
{
	rw_int32_t( m_clientid,       msg);

	rw_int32_t( m_numjob,         msg);
	rw_int32_t( m_numblock,       msg);
	rw_int32_t( m_numtask,        msg);
	rw_int32_t( m_number,         msg);

	rw_int8_t ( m_status,         msg);
	rw_int8_t ( m_percent,        msg);
	rw_int32_t( m_frame,          msg);
	rw_int8_t ( m_percent_frame,  msg);
	rw_String ( m_activity,      msg);
	rw_int32_t( m_datalen,        msg);

	if( m_datalen == 0 ) return;

	if( msg->isWriting() )
	{
		if( m_data == NULL )
		{
			AFERROR("MCTaskUp::readwrite: data == NULL.")
			return;
		}
		rw_data( m_data, msg, m_datalen);
	}
	else
	{
		m_data = new char[m_datalen];
		if( m_data == NULL )
		{
			AFERROR("MCTaskUp::readwrite: Can't allocate memory for data.")
			m_datalen = 0;
			return;
		}
		rw_data( m_data, msg, m_datalen);
	}
}

void MCTaskUp::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	stream << "TaskUp: client=" << m_clientid
			<< ", job="      << m_numjob
			<< ", block="    << m_numblock
			<< ", task="     << m_numtask
			<< ", number="   << m_number
			<< ", datalen="  << m_datalen
			<< ", status="   << m_status
			<< ", percent="  << m_percent
			<< ", activity=" << m_activity;

	if( full && m_datalen && m_data) stream << "\ndata:\n" << std::string( m_data, m_datalen) << std::endl;
}
