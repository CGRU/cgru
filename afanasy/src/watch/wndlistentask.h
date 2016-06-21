#pragma once

#include "../libafanasy/name_af.h"

#include "wndtext.h"
#include "receiver.h"

class WndListenTask : public WndText, public Receiver
{
public:
	WndListenTask( int i_job_id, int i_block, int i_task, const QString & i_name);
   ~WndListenTask();

	bool v_processEvents( const af::MonitorEvents & i_me);

   void v_connectionLost();

protected:
   void closeEvent( QCloseEvent * event);

private:
	void subscribe( bool i_subscribe);

private:
   int m_job_id;
   int m_block;
   int m_task;
   QString m_name;
};
