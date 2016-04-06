#pragma once

#include "../libafanasy/common/dlRWLock.h"

#include "../libafanasy/msgqueue.h"
#include "../libafanasy/render.h"
#include "../libafanasy/renderupdate.h"

#include "taskprocess.h"

class Parser;
class PyRes;

class RenderHost: public af::Render
{
public:
    RenderHost();
    ~RenderHost();

    inline static int  getId() { return ms_obj->af::Render::getId();}
    inline static const std::string & getName() { return ms_obj->af::Render::getName();}

    static void setListeningPort( uint16_t i_port);

    inline static bool isListening() { return ms_listening; }

	inline static bool noOutputRedirection() { return ms_no_output_redirection; }

    inline static void acceptMessage(   af::Msg * i_msg) { ms_msgAcceptQueue->pushMsg( i_msg);}

    inline static af::Msg * acceptWait() { return ms_msgAcceptQueue->popMsg( af::AfQueue::e_wait);    }
    inline static af::Msg * acceptTry()  { return ms_msgAcceptQueue->popMsg( af::AfQueue::e_no_wait); }

    static bool isConnected() { return ms_connected;  }
    static void setRegistered( int i_id);
    static void connectionLost( bool i_any_case = false);
	static void connectionEstablished() { ms_connection_lost_count = 0; }

    static void setUpdateMsgType( int i_type);

    static void refreshTasks();

    static void update( const uint64_t & i_cycle);

    static void runTask( af::Msg * i_msg);
	static void runTask( af::TaskExec * i_task);

    static void stopTask( const af::MCTaskPos & i_taskpos);

    static void closeTask( const af::MCTaskPos & i_taskpos);

	inline static void addTaskUp( af::MCTaskUp * i_tup) { ms_up.addTaskUp( i_tup);}

    static void upTaskOutput( const af::MCTaskPos & i_taskpos);

    inline static void   lockMutex() { ms_obj->m_mutex.Lock();  }
    inline static void unLockMutex() { ms_obj->m_mutex.Unlock();}

#ifdef WINNT
    static void windowsMustDie();
#endif

	static void wolSleep( const std::string & i_str);

private:
	static void getResources();

    static void dispatchMessage( af::Msg * i_msg);

private:
    static RenderHost * ms_obj;

    static std::vector<std::string> ms_windowsmustdie;

    static std::vector<PyRes*> ms_pyres;

    static af::MsgQueue * ms_msgAcceptQueue;
    static af::MsgQueue * ms_msgDispatchQueue;

    static bool ms_connected;
	static int  ms_connection_lost_count;

    static int ms_updateMsgType;

    static std::vector<TaskProcess*> ms_tasks;

    static bool ms_listening;

	static bool ms_no_output_redirection;

	static af::RenderUpdate ms_up;

    DlMutex m_mutex;
//    DlRWLock m_mutex;
};
