#pragma once

#include "../libafanasy/msgqueue.h"
//#include "../libafanasy/name_af.h"
#include "../libafanasy/render.h"

//#include "../libafqt/name_afqt.h"

class Parser;
class PyRes;

//afqt::QMsg* update_handler_ptr( afqt::QMsg * msg);

class RenderHost: public af::Render
{
public:
    RenderHost( int32_t i_state, uint8_t i_priority);
    RenderHost();
    ~RenderHost();

    inline static void acceptMessage(   af::Msg * i_msg) { ms_msgAcceptQueue  ->pushMsg( i_msg);}
    inline static void dispatchMessage( af::Msg * i_msg) { ms_msgDispatchQueue->pushMsg( i_msg);}

    inline static af::Msg * acceptWait() { return ms_msgAcceptQueue->popMsg( af::AfQueue::e_wait);    }
    inline static af::Msg * acceptTry()  { return ms_msgAcceptQueue->popMsg( af::AfQueue::e_no_wait); }

    static bool isConnected() { return ms_connected;  }
    static void connectionEstablished();
    static void connectionLost();

    inline static int  getId() { return ms_obj->af::Render::getId();}
    inline static void setId( int new_id) { ms_obj->id = new_id;}

    static void setUpdateMsgType( int i_type);

    static void update();

#ifdef WINNT
    void windowsMustDie() const;
#endif

private:
    static RenderHost * ms_obj;
#ifdef WINNT
    std::vector<std::string> windowsmustdie;
#endif

    static std::vector<PyRes*> m_pyres;
//   afqt::QMsg * upmsg;

    static af::MsgQueue * ms_msgAcceptQueue;
    static af::MsgQueue * ms_msgDispatchQueue;

    static bool ms_connected;

    static int ms_updateMsgType;
};
