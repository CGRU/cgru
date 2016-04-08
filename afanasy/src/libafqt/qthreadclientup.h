#pragma once

#include "../libafanasy/name_af.h"

#include "name_afqt.h"
#include "qthreadclient.h"

#include <QtCore/QMutex>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>

/// This thread send message to server every \c sec seconds.
/** This is usefull to update client information on server or request some data to update client.
**/
class afqt::QThreadClientUp : public afqt::QThreadClient
{
   Q_OBJECT
public:

	QThreadClientUp( QObject * i_parent, int i_seconds, int i_numConnLost);

	~QThreadClientUp();


	/// QThread.run() method override.
	virtual void run(); 


	/// Set message for update
	void setUpMsg( af::Msg * msg);


private:
	af::Msg * getMessage();

private:
	int m_seconds;

	bool m_exiting;
};
