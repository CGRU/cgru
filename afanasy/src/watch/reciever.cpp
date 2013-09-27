#include "reciever.h"

#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Reciever::Reciever()
{
	Watch::addReciever( this);
}

Reciever::~Reciever()
{
	Watch::removeReciever( this);
}

bool Reciever::v_filesReceived( const af::MCTaskUp & i_taskup){ return false;}
void Reciever::v_connectionLost(){}
void Reciever::v_connectionEstablished(){}
