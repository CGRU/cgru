#include "rglobal.h"

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::MsgQueue * RGlobal::ms_msgAcceptQueue   = NULL;
af::MsgQueue * RGlobal::ms_msgDispatchQueue = NULL;

bool RGlobal::ms_connected = false;

RGlobal::RGlobal()
{
    ms_msgAcceptQueue   = new af::MsgQueue("Messages Accept Queue",   af::AfQueue::e_no_thread    );
    ms_msgDispatchQueue = new af::MsgQueue("Messages Dispatch Queue", af::AfQueue::e_start_thread );
}

RGlobal::~RGlobal()
{
    delete ms_msgAcceptQueue;
    delete ms_msgDispatchQueue;
}

void RGlobal::setConnected()
{
    printf("Render connected.\n");
    ms_connected = true;
}

void RGlobal::setDisconnected()
{
    printf("Render disconnected.\n");
    ms_connected = false;
}
