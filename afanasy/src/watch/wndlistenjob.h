#pragma once

#include "../libafanasy/name_af.h"

#include "wndtext.h"
#include "receiver.h"

class WndListenJob : public WndText, public Receiver
{
public:
	WndListenJob( int i_job_id, const QString & i_name);
   ~WndListenJob();

	bool v_processEvents( const af::MonitorEvents & i_me);

   void v_connectionLost();

protected:
   void closeEvent( QCloseEvent * event);

private:
	void subscribe( bool i_subscribe);

private:
	int m_job_id;
	QString m_name;
};
