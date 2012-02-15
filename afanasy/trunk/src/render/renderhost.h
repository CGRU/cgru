#pragma once

#include "../libafanasy/msgqueue.h"
#include "../libafanasy/render.h"

#include "taskprocess.h"

class Parser;
class PyRes;

//afqt::QMsg* update_handler_ptr( afqt::QMsg * msg);

class RenderHost: public af::Render
{
public:
    RenderHost( int32_t i_state, uint8_t i_priority);
    RenderHost();
    ~RenderHost();

    static void setAddressPort( uint16_t i_port) { ms_obj->address.setPort( i_port);}

    inline static void acceptMessage(   af::Msg * i_msg) { ms_msgAcceptQueue->pushMsg( i_msg);}
    inline static void dispatchMessage( af::Msg * i_msg);

    inline static af::Msg * acceptWait() { return ms_msgAcceptQueue->popMsg( af::AfQueue::e_wait);    }
    inline static af::Msg * acceptTry()  { return ms_msgAcceptQueue->popMsg( af::AfQueue::e_no_wait); }

    static bool isConnected() { return ms_connected;  }
    static void setRegistered( int i_id);
    static void connectionLost();

    inline static int  getId() { return ms_obj->af::Render::getId();}

    static void setUpdateMsgType( int i_type);

    static void update();

    static void runTask( af::Msg * i_msg);

#ifdef WINNT
    void windowsMustDie() const;
#endif

private:
    static RenderHost * ms_obj;
#ifdef WINNT
    std::vector<std::string> windowsmustdie;
#endif

    static std::vector<PyRes*> ms_pyres;

    static af::MsgQueue * ms_msgAcceptQueue;
    static af::MsgQueue * ms_msgDispatchQueue;

    static bool ms_connected;

    static int ms_updateMsgType;

    static std::vector<TaskProcess*> ms_tasks;
};
