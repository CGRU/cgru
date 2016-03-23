#include "receiver.h"

#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Receiver::Receiver()
{
	Watch::addReceiver( this);
}

Receiver::~Receiver()
{
	Watch::removeReceiver( this);
}

bool Receiver::processEvents( const af::MonitorEvents & i_me) { return false;}
bool Receiver::v_filesReceived( const af::MCTaskUp & i_taskup){ return false;}
void Receiver::v_connectionLost(){}
void Receiver::v_connectionEstablished(){}
