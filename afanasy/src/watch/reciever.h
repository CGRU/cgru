#pragma once

#include "../libafanasy/name_af.h"

#include <QtGui/QTextEdit>

class Reciever
{
public:
	Reciever();
	virtual ~Reciever();

	virtual bool caseMessage( af::Msg * msg) = 0;

	virtual bool v_filesReceived( const af::MCTaskUp & i_taskup);

	virtual void v_connectionLost();

	virtual void v_connectionEstablished();

private:
};
