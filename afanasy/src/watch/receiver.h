#pragma once

#include "../libafanasy/monitorevents.h"
#include "../libafanasy/name_af.h"

#include <QTextEdit>

class Receiver
{
public:
	Receiver();
	virtual ~Receiver();

	virtual bool caseMessage( af::Msg * msg);

	virtual bool processEvents( const af::MonitorEvents & i_me);

	virtual bool v_filesReceived( const af::MCTaskUp & i_taskup);

	virtual void v_connectionLost();

	virtual void v_connectionEstablished();

private:
};
