#include "qafclient.h"

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "../libafqt/qenvironment.h"

#include <QtNetwork/QTcpSocket>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

using namespace afqt;

QAfSocket::QAfSocket( QObject * i_parent, af::Msg * i_msg_req, bool i_delete_after):
	QObject( i_parent),
	m_msg_req( i_msg_req),
	m_delete_after( i_delete_after),
	m_bytes_read( 0),
	m_header_read( false),
	m_reading_finished( false)
{
	AF_DEBUG;

	m_qsocket = new QTcpSocket( this);
	m_msg_ans = new af::Msg();

	connect( m_qsocket, SIGNAL( error( QAbstractSocket::SocketError)), this, SLOT( slot_error( QAbstractSocket::SocketError)));
	connect( m_qsocket, SIGNAL( connected()), this, SLOT( slot_connected()));
	connect( m_qsocket, SIGNAL( bytesWritten( qint64)), this, SLOT( slot_bytesWritten( qint64)));
	connect( m_qsocket, SIGNAL( readyRead()), this, SLOT( slot_readyRead()));
	connect( m_qsocket, SIGNAL( disconnected()), this, SLOT( slot_disconnected()));

	m_qsocket->connectToHost( afqt::QEnvironment::getAfServerQHostAddress(), af::Environment::getServerPort());
}

void QAfSocket::slot_error( QAbstractSocket::SocketError socketError)
{
	AF_ERR << "SocketError: " << afqt::qtos( m_qsocket->errorString());

	emit sig_error();

	if( m_qsocket->state() != QAbstractSocket::UnconnectedState )
		m_qsocket->disconnectFromHost();
	else
		emit sig_zombie( this);
}

void QAfSocket::slot_connected()
{
	AF_DEBUG;

	m_bytes_written = m_qsocket->write( m_msg_req->buffer(), m_msg_req->writeSize());
}

void QAfSocket::slot_bytesWritten( qint64 i_bytes)
{
	AF_DEBUG;

	m_bytes_written += i_bytes;
}

void QAfSocket::slot_readyRead()
{
	AF_DEBUG;

	if( m_reading_finished )
		return;

	if( false == m_header_read )
	{
		m_bytes_read += m_qsocket->read( m_msg_ans->buffer() + m_bytes_read, af::Msg::SizeBuffer - m_bytes_read);

		if( m_bytes_read < af::Msg::SizeHeader )
			return;

		m_header_read = true;

		int header_offset = af::processHeader( m_msg_ans, m_bytes_read);
		if( header_offset < 0 )
		{
			emit sig_error();
			m_qsocket->disconnectFromHost();
			return;
		}

		if( m_msg_ans->type() < af::Msg::TDATA )
		{
			m_reading_finished = true;
		}
		else
		{
			m_bytes_read -= header_offset;
	
			if( m_bytes_read >= m_msg_ans->dataLen())
				m_reading_finished = true;
		}
	}

	if(( m_header_read ) && ( false == m_reading_finished ) && ( m_msg_ans->type() >= af::Msg::TDATA ))
	{
		int toread = m_msg_ans->dataLen() - m_bytes_read;

		m_bytes_read += m_qsocket->read( m_msg_ans->buffer() + af::Msg::SizeHeader + m_bytes_read, toread);

		if( m_bytes_read >= m_msg_ans->dataLen())
			m_reading_finished = true;
	}

	if( m_reading_finished )
	{
		emit sig_newMsg( m_msg_ans);
		m_msg_ans = NULL;
		m_qsocket->disconnectFromHost();
	}
}

void QAfSocket::slot_disconnected()
{
	AF_DEBUG;

	emit sig_zombie( this);
}

QAfSocket::~QAfSocket()
{
	AF_DEBUG;

	if( m_qsocket ) delete m_qsocket;
	if( m_msg_req && m_delete_after ) delete m_msg_req;
	if( m_msg_ans ) delete m_msg_ans;
}

QAfClient::QAfClient( QObject * i_qparent, int i_num_conn_lost):
	QObject( i_qparent),
	m_up_timer( NULL),
	m_up_msg( NULL),
	m_numconnlost( i_num_conn_lost),
	m_connlostcount( 0)
{
	connect( this, SIGNAL( sig_sendMsg( af::Msg*)), this, SLOT( slot_sendMsg( af::Msg*)));
}

QAfClient::~QAfClient()
{
	AFINFO("QAfClient::~QAfClient()")
}

void QAfClient::sendMsg( af::Msg * i_msg, bool i_delete_after)
{
	if( i_msg == NULL ) return;

	AF_DEBUG << i_msg;

	QAfSocket * qas = new QAfSocket( this, i_msg, i_delete_after);

	connect( qas, SIGNAL( sig_newMsg( af::Msg*)),   this, SLOT( slot_newMsg( af::Msg*)));
	connect( qas, SIGNAL( sig_error()),             this, SLOT( slot_sendError()));
	connect( qas, SIGNAL( sig_zombie( QAfSocket*)), this, SLOT( slot_zombie( QAfSocket*)));

	m_qsockets_list.push_back( qas);
}

void QAfClient::slot_newMsg( af::Msg * i_msg)
{
	m_connlostcount = 0;

	emit sig_newMsg( i_msg);
}

void QAfClient::slot_sendError()
{
	if(( m_numconnlost != 0 ) && ( m_connlostcount <= m_numconnlost )) m_connlostcount++;
	if(( m_numconnlost  > 1 ) && ( m_connlostcount <= m_numconnlost ))
	{
		AF_ERR << "Connection lost count: " << m_connlostcount << " of " << m_numconnlost;
	}
	if( m_connlostcount == m_numconnlost )
	{
		AF_ERR << "Connection Lost!";
		emit sig_connectionLost();
	}
}

void QAfClient::slot_zombie( QAfSocket * i_qas)
{
	m_qsockets_list.removeOne( i_qas);

	i_qas->deleteLater();
}

void QAfClient::setUpMsg( af::Msg * i_msg, int i_seconds)
{
	m_up_msg = i_msg;

	if( NULL == m_up_timer )
	{
		m_up_timer = new QTimer( this);
		connect( m_up_timer, SIGNAL( timeout()), this, SLOT( slot_up_timeout()));
		m_up_timer->start( 1000 * i_seconds);

		sendMsg( m_up_msg, false);
	}
	else
	{
		m_up_timer->setInterval( 1000 * i_seconds);
	}
}

void QAfClient::slot_up_timeout()
{
	sendMsg( m_up_msg, false);
}

