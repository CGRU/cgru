#pragma once

#include "../libafanasy/name_af.h"

#include "name_afqt.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QTimer>

class afqt::QAfSocket: public QObject
{
Q_OBJECT
public:
	QAfSocket( QObject * i_qparent, af::Msg * i_msg_req, bool i_delete_after = true);
	~QAfSocket();

private slots:
	void slot_error( QAbstractSocket::SocketError socketError);
	void slot_connected();
	void slot_bytesWritten( qint64 i_bytes);
	void slot_readyRead();
	void slot_disconnected();

signals:
	void sig_newMsg( af::Msg * i_msg);
	void sig_error();
	void sig_zombie( QAfSocket * i_qas);

private:
	af::Msg * m_msg_req;
	bool m_delete_after;

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

	void sendMsg( af::Msg * msg, bool i_delete_after = true);

	void setUpMsg( af::Msg * i_msg, int i_seconds);

public slots:
	void slot_newMsg( af::Msg * i_msg);
	void slot_sendError();
	void slot_zombie( QAfSocket * i_qas);

signals:
	void sig_newMsg( af::Msg * i_msg);
	void sig_connectionLost();

protected:
	QList<QAfSocket*> m_qsockets_list;

	int m_numconnlost;
	int m_connlostcount;

private slots:
	void slot_up_timeout();

private:
	QTimer  * m_up_timer;
	af::Msg * m_up_msg;
	int       m_up_seconds;
};

