#pragma once

#include "../libafanasy/msgqueue.h"

class RGlobal
{
public:
    RGlobal();
    ~RGlobal();

    static void acceptMessage(   af::Msg * i_msg) { ms_msgAcceptQueue  ->pushMsg( i_msg);}
    static void dispatchMessage( af::Msg * i_msg) { ms_msgDispatchQueue->pushMsg( i_msg);}

    static af::Msg * acceptWait() { return ms_msgAcceptQueue->popMsg( af::AfQueue::e_wait);    }
    static af::Msg * acceptTry()  { return ms_msgAcceptQueue->popMsg( af::AfQueue::e_no_wait); }

    static bool isConnected() { return ms_connected;  }
    static void setConnected();
    static void setDisconnected();

private:
    static af::MsgQueue * ms_msgAcceptQueue;
    static af::MsgQueue * ms_msgDispatchQueue;

    static bool ms_connected;
};
