#include "qthreadclientup.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

using namespace afqt;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

QThreadClientUp::QThreadClientUp( QObject * i_parent, int i_seconds, int i_numConnLost):
	QThreadClient( i_parent, i_numConnLost),
	m_seconds( i_seconds),
	m_exiting( false)
{
	setObjectName("QObj_QThreadClientUp");
	AFINFA("QThreadClientUp::QThreadClientUp: seconds=%d, numconnlost=%d", m_seconds, i_numConnLost)
}

QThreadClientUp::~QThreadClientUp()
{
	AFINFO("QThreadClientUp::~QThreadClientUp()")
	m_exiting = true;
	wait();
}

void QThreadClientUp::setUpMsg( af::Msg * msg)
{
	queue.push( msg);

	if( false == isRunning() )
		start();
}

af::Msg * afqt::QThreadClientUp::getMessage()
{
	while( queue.getCount() > 1)
	{
		//printf("QThreadClientUp::getMessage(): queue.getCount() > 1\n");
		delete queue.pop();
	}

	return queue.getFirst();
}

void QThreadClientUp::run()
{
	AFINFO("QThreadClientUp::run() BEGIN")

	QTcpSocket socket;

	while( false == m_exiting )
	{
		af::Msg * message = getMessage();
		if( message )
		{
			#ifdef AFOUTPUT
			printf("QThreadClientUp:"); message->v_stdOut();
			#endif

			sendMessage( message, &socket);
		}

		if( m_exiting )
			break;

		QThread::sleep( m_seconds);
	}
	AFINFO("QThreadClientUp::run() END")
}

