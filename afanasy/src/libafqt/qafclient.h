#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/msg.h"

#include "name_afqt.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <QtNetwork/QTcpSocket>

class afqt::QAfSocket: public QObject
{
Q_OBJECT
public:
	QAfSocket( QObject * i_qparent, af::Msg * i_msg_req, bool i_updater);
	~QAfSocket();

	inline bool isUpdater() const { return m_updater; }
	inline const af::Msg * getReqestMsg() const { return m_msg_req; }
	inline void delRequestMsg() { if( m_msg_req ) delete m_msg_req; m_msg_req = NULL; }
	inline const std::string getSocketErrorStr() const { return afqt::qtos( m_qsocket->errorString()); }

private slots:
	void slot_error( QAbstractSocket::SocketError socketError);
	void slot_connected();
	void slot_bytesWritten( qint64 i_bytes);
	void slot_readyRead();
	void slot_disconnected();

signals:
	void sig_newMsg( af::Msg * i_msg);
	void sig_error( QAfSocket * i_qas);
	void sig_zombie( QAfSocket * i_qas);

private:
	af::Msg * m_msg_req;
	bool m_updater;

	af::Msg * m_msg_ans;

	QTcpSocket * m_qsocket;

	int m_bytes_written;
	int m_bytes_read;
	bool m_header_read;
	bool m_reading_finished;
	bool m_zombie;
};

class afqt::QAfClient : public QObject
{
Q_OBJECT
public:
	QAfClient( QObject * i_qparent, int i_num_conn_lost = 1 );
	~QAfClient();

	void sendMsg( af::Msg * msg, bool i_updater = false);

	// Set message to send to server periodically.
	void setUpMsg( af::Msg * i_msg, int i_seconds);

	// Client stops to send any messages, but still process existing.
	// After last messsage has sent sig_finished() emitted.
	void setClosing();

public slots:
	void slot_newMsg( af::Msg * i_msg);
	void slot_sendError( QAfSocket * i_qas);
	void slot_zombie( QAfSocket * i_qas);

signals:
	void sig_newMsg( af::Msg * i_msg);
	void sig_connectionLost();
	void sig_finished();

private slots:
	void slot_up_timeout();

private:
	QList<QAfSocket*> m_qafsockets_list;

	int  m_numconnlost;
	int  m_connlostcount;
	bool m_closing;

	QTimer  * m_up_timer;
	af::Msg * m_up_msg;
	int       m_up_seconds;
	bool      m_up_processing;
};

