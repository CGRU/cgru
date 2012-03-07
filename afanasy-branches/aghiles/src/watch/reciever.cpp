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

void Reciever::connectionLost(){}
void Reciever::connectionEstablished(){}
