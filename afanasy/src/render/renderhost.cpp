#include "renderhost.h"

#ifdef WINNT
#include <fstream>
#endif

#include "../libafanasy/dlScopeLocker.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "pyres.h"
#include "res.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool AFRunning;

RenderHost * RenderHost::ms_obj = NULL;
af::MsgQueue * RenderHost::ms_msgAcceptQueue = NULL;
af::MsgQueue * RenderHost::ms_msgDispatchQueue = NULL;
int RenderHost::ms_updateMsgType = af::Msg::TRenderRegister;
bool RenderHost::ms_connected = false;
std::vector<PyRes*> RenderHost::ms_pyres;
std::vector<TaskProcess*> RenderHost::ms_tasks;
bool RenderHost::m_listening = false;
std::vector<std::string> RenderHost::windowsmustdie;

RenderHost::RenderHost( int32_t i_state, uint8_t i_priority):
   Render( i_state, i_priority)
{
    ms_obj = this;

    ms_msgAcceptQueue   = new af::MsgQueue("Messages Accept Queue",   af::AfQueue::e_no_thread    );
    ms_msgDispatchQueue = new af::MsgQueue("Messages Dispatch Queue", af::AfQueue::e_start_thread );
    ms_msgDispatchQueue->setReturnQueue( ms_msgAcceptQueue);
    ms_msgDispatchQueue->returnNotSended();
    ms_msgDispatchQueue->setVerboseMode( af::VerboseOff);

    setOnline();

    m_host.m_os = af::strJoin( af::Environment::getPlatform(), " ");
    GetResources( m_host, m_hres);

    std::vector<std::string> resclasses = af::Environment::getRenderResClasses();
    for( std::vector<std::string>::const_iterator it = resclasses.begin(); it != resclasses.end(); it++)
    {
        if( (*it).empty() ) continue;
        printf("Adding custom resource meter '%s'\n", (*it).c_str());
        ms_pyres.push_back( new PyRes( *it, &m_hres));
    }

#ifdef WINNT
    // Windows Must Die:
	HANDLE dir;
	WIN32_FIND_DATA file_data;
	if(( dir = FindFirstFile((af::Environment::getAfRoot() + "\\*").c_str(), &file_data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::string filename( file_data.cFileName);
			if( filename.find("windowsmustdie") != 0 )
				continue;

			filename = af::Environment::getAfRoot() + '\\' + filename;
			if( false == af::pathFileExists( filename))
				continue;

			std::ifstream file( filename);
			if( file.is_open())
			{
				while( file.good())
				{
					std::string line;
					std::getline( file, line);
					line = af::strStrip( line);
					if( line.size() > 0 )
						windowsmustdie.push_back( line);
				}
				file.close();
			}
			else printf("Unable to read '%s'.\n", filename.c_str());
		} while( FindNextFile( dir, &file_data));
		FindClose( dir);
	}
	else
	{
		AFERRAR("Can't open folder '%s'", af::Environment::getAfRoot().c_str())
	}
    if( windowsmustdie.size())
    {
        printf("Windows Must Die:\n");
        for( int i = 0; i < windowsmustdie.size(); i++)
            printf("   %s\n", windowsmustdie[i].c_str());
    }
#endif

	af::sleep_msec( 100);

    GetResources( m_host, m_hres);
    for( int i = 0; i < ms_pyres.size(); i++) ms_pyres[i]->update();

    v_stdOut();
    m_host.v_stdOut( true);
    m_hres.v_stdOut( true);
}

RenderHost::~RenderHost()
{
    // Delete custom python resources:
    for( int i = 0; i < ms_pyres.size(); i++)
        if( ms_pyres[i])
            delete ms_pyres[i];

    // Send deregister message if connected:
    if( ms_connected )
    {
        af::Msg msg( af::Msg::TRenderDeregister, ms_obj->getId());
        msg.setAddress( af::Environment::getServerAddress());
        bool ok;
        af::msgsend( & msg, ok, af::VerboseOn);
        ms_connected = false;
    }

    // Delete all tasks:
    for( std::vector<TaskProcess*>::iterator it = ms_tasks.begin(); it != ms_tasks.end(); )
    {
        delete *it;
        it = ms_tasks.erase( it);
    }

    // Delete queues:
    delete ms_msgAcceptQueue;
    delete ms_msgDispatchQueue;
}

void RenderHost::setListeningPort( uint16_t i_port)
{
    ms_obj->m_address.setPort( i_port);
    m_listening = true;
    if( af::Environment::isVerboseMode())
        printf("RenderHost::setListeningPort = %d\n", i_port);
}

void RenderHost::dispatchMessage( af::Msg * i_msg)
{
    if( false == AFRunning ) return;

    if( i_msg->addressIsEmpty() && ( i_msg->addressesCount() == 0 ))
    {
        // Assuming that message should be send to server if no address specified.
        i_msg->setAddress( af::Environment::getServerAddress());
    }
    ms_msgDispatchQueue->pushMsg( i_msg);
}

void RenderHost::setRegistered( int i_id)
{
    ms_connected = true;
    ms_obj->m_id = i_id;
    ms_msgDispatchQueue->setVerboseMode( af::VerboseOn);
    setUpdateMsgType( af::Msg::TRenderUpdate);
    printf("Render registered.\n");
}

void RenderHost::connectionLost()
{
    if( ms_connected == false ) return;

    ms_connected = false;

    ms_obj->m_id = 0;

    // Stop all tasks:
    for( int t = 0; t < ms_tasks.size(); t++) ms_tasks[t]->stop();

    ms_msgDispatchQueue->setVerboseMode( af::VerboseOff);

    // Begin to try to register again:
    setUpdateMsgType( af::Msg::TRenderRegister);

    printf("Render connection lost, connecting...\n");
}

void RenderHost::setUpdateMsgType( int i_type)
{
    ms_updateMsgType = i_type;
}

void RenderHost::refreshTasks()
{
    if( false == AFRunning )
        return;

    // Refresh tasks:
    for( int t = 0; t < ms_tasks.size(); t++)
    {
        ms_tasks[t]->refresh();
    }

    // Remove zombies:
    for( std::vector<TaskProcess*>::iterator it = ms_tasks.begin(); it != ms_tasks.end(); )
    {
        if((*it)->isZombie())
        {
            delete *it;
            it = ms_tasks.erase( it);
        }
        else
            it++;
    }
}

void RenderHost::update()
{
    if( false == AFRunning )
        return;

    // Do this every update time, but not the first time, as at the begininng resources are already updated
    static bool first_time = true;

    if( false == first_time )
    {
        GetResources( ms_obj->m_host, ms_obj->m_hres);
        for( int i = 0; i < ms_pyres.size(); i++) ms_pyres[i]->update();
    }
    else
        first_time = false;

//hres.stdOut();

#ifdef WINNT
    windowsMustDie();
#endif

    if( false == isListening())
    {
        // Port can't be zero!
        // It seems that listening thread is not started to listen any port.
        AFERROR("RenderHost::update(): Render is not listening any port.")
        // This error is not fatal.
        // Client was just started and we simple will wait next update.
        return;
    }

    af::Msg * msg = new af::Msg( ms_updateMsgType, ms_obj);
    msg->setReceiving();
    dispatchMessage( msg);
}

#ifdef WINNT
void RenderHost::windowsMustDie()
{
// Windows Must Die:
    AFINFO("RenderHost::windowsMustDie():");
    for( int i = 0; i < windowsmustdie.size(); i++)
    {
        HWND WINAPI hw = FindWindow( NULL, TEXT( windowsmustdie[i].c_str()));
        if( hw != NULL )
        {
            printf("Window must die founded:\n%s\n", windowsmustdie[i].c_str());
            SendMessage( hw, WM_CLOSE, 0, 0);
        }
    }
}
#endif

void RenderHost::runTask( af::Msg * i_msg)
{
    ms_tasks.push_back( new TaskProcess( new af::TaskExec( i_msg)));
}

void RenderHost::stopTask( const af::MCTaskPos & i_taskpos)
{
    for( int t = 0; t < ms_tasks.size(); t++)
    {
        if( ms_tasks[t]->is( i_taskpos))
        {
            ms_tasks[t]->stop();
            return;
        }
    }
    AFERRAR("RenderHost::stopTask: %d tasks, no such task:", int(ms_tasks.size()))
    i_taskpos.v_stdOut();
}

void RenderHost::closeTask( const af::MCTaskPos & i_taskpos)
{
    for( int t = 0; t < ms_tasks.size(); t++)
    {
        if( ms_tasks[t]->is( i_taskpos))
        {
			ms_tasks[t]->close();
//            delete ms_tasks[t];
//            ms_tasks.erase( ms_tasks.begin() + t);
            return;
        }
    }
    AFERRAR("RenderHost::closeTask: %d tasks, no such task:", int(ms_tasks.size()))
    i_taskpos.v_stdOut();
}

void RenderHost::listenTasks( const af::MCListenAddress & i_mcaddr)
{
    for( int t = 0; t < ms_tasks.size(); t++)
    {
        if( i_mcaddr.justTask())
        {
            if( ms_tasks[t]->is( i_mcaddr.getJobId(), i_mcaddr.getNumBlock(), i_mcaddr.getNumTask(), 0))
            {
                if( i_mcaddr.toListen()) ms_tasks[t]->addListenAddress(    i_mcaddr.getAddress());
                else                     ms_tasks[t]->removeListenAddress( i_mcaddr.getAddress());
                i_mcaddr.v_stdOut();
            }
        }
        else
        {
            if( ms_tasks[t]->is( i_mcaddr.getJobId()))
            {
                if( i_mcaddr.toListen()) ms_tasks[t]->addListenAddress(    i_mcaddr.getAddress());
                else                     ms_tasks[t]->removeListenAddress( i_mcaddr.getAddress());
                i_mcaddr.v_stdOut();
            }
        }
    }
}

void RenderHost::listenFailed( const af::Address & i_addr)
{
    int lasttasknum = -1;
    for( int t = 0; t < ms_tasks.size(); t++) if( (ms_tasks[t])->removeListenAddress( i_addr)) lasttasknum = t;
    if( lasttasknum != -1)
    {
        af::MCListenAddress mclass( af::MCListenAddress::FROMRENDER, i_addr, ms_tasks[lasttasknum]->exec()->getJobId());
        dispatchMessage( new af::Msg( af::Msg::TTaskListenOutput, &mclass));
    }
}

void RenderHost::getTaskOutput( const af::MCTaskPos & i_taskpos, af::Msg * o_msg)
{
    for( int t = 0; t < ms_tasks.size(); t++)
    {
        if( ms_tasks[t]->is( i_taskpos))
        {
            ms_tasks[t]->getOutput( o_msg);
            return;
        }
    }
    AFERROR("RenderHost::closeTask: No such task:\n")
    i_taskpos.v_stdOut();
}
