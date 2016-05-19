#include "renderhost.h"

#ifdef WINNT
#include <fstream>
#endif

#include "../libafanasy/environment.h"
#include "../libafanasy/logger.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/taskexec.h"

#include "pyres.h"
#include "res.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool AFRunning;

RenderHost::RenderHost():
	af::Render( Client::GetEnvironment),
	m_updateMsgType( af::Msg::TRenderRegister),
	m_connected( false),
	m_connection_lost_count( 0),
	m_no_output_redirection( false)
{
	if( af::Environment::hasArgument("-nor")) m_no_output_redirection = true;

    setOnline();

    m_host.m_os = af::strJoin( af::Environment::getPlatform(), " ");
    GetResources( m_host, m_hres);

    std::vector<std::string> resclasses = af::Environment::getRenderResClasses();
    for( std::vector<std::string>::const_iterator it = resclasses.begin(); it != resclasses.end(); it++)
    {
        if( (*it).empty() ) continue;
        printf("Adding custom resource meter '%s'\n", (*it).c_str());
        m_pyres.push_back( new PyRes( *it, &m_hres));
    }

#ifdef WINNT
    // Windows Must Die:
	m_windowsmustdie = af::Environment::getRenderWindowsMustDie();
    if( m_windowsmustdie.size())
    {
        printf("Windows Must Die:\n");
        for( int i = 0; i < m_windowsmustdie.size(); i++)
            printf("   %s\n", m_windowsmustdie[i].c_str());
    }
#endif

	af::sleep_msec( 100);

    GetResources( m_host, m_hres);
    for( int i = 0; i < m_pyres.size(); i++) m_pyres[i]->update();

    v_stdOut();
    m_host.v_stdOut( true);
    m_hres.v_stdOut( true);
}

RenderHost::~RenderHost()
{
    // Delete custom python resources:
    for( int i = 0; i < m_pyres.size(); i++)
        if( m_pyres[i])
            delete m_pyres[i];

    // Send deregister message if connected:
    if( m_connected )
    {
        af::Msg msg( af::Msg::TRenderDeregister, getId());
        bool ok;
		af::Msg * answer = af::sendToServer( & msg, ok, af::VerboseOn);
		if( answer ) delete answer;
        m_connected = false;
    }

    // Delete all tasks:
    for( std::vector<TaskProcess*>::iterator it = m_tasks.begin(); it != m_tasks.end(); )
    {
        delete *it;
        it = m_tasks.erase( it);
    }
}

RenderHost * RenderHost::getInstance()
{
	// Does not return a reference, although sometimes recommanded for a
	// singleton, because we need to control when it is destroyed.
	static RenderHost * ms_obj = new RenderHost();
	return ms_obj;
}

void RenderHost::connectionEstablished()
{
	m_connection_lost_count = 0;
	
	if (m_updateMsgType == af::Msg::TRenderReconnect)
	{
		AF_LOG << "Reconnected to the server";
	}
	
	setUpdateMsgType( af::Msg::TRenderUpdate);
}

void RenderHost::setRegistered( int i_id)
{
    m_connected = true;
    m_id = i_id;
    AF_LOG << "Render registered.";
	RenderHost::connectionEstablished();
}

void RenderHost::connectionLost( bool i_any_case)
{
    if( m_connected == false )
		return;

	m_connection_lost_count++;

	if( false == i_any_case )
	{
		AF_LOG << "Connection lost count = " << m_connection_lost_count
			   << " of " << af::Environment::getRenderConnectRetries();
		if( m_connection_lost_count <= af::Environment::getRenderConnectRetries() )
		{
			return;
		}
	}


    // Stop all tasks:
    //for( int t = 0; t < m_tasks.size(); t++) m_tasks[t]->stop();
    
    // Begin to try to register again:
    if (m_tasks.empty())
    {
        // Complete reset
        m_connected = false;
        m_id = 0;
        setUpdateMsgType( af::Msg::TRenderRegister);
    }
    else
    {
        m_reco.clear();
        m_reco.setId( getId());
        for( int t = 0; t < m_tasks.size(); t++)
        {
            af::TaskExec *taskexec = m_tasks[t]->getTaskExec();
            m_reco.addTaskExec(taskexec);
        }
        setUpdateMsgType( af::Msg::TRenderReconnect);
    }

    AF_WARN << "Render connection lost, trying to reconnect...";
    AF_LOG  << "note: tasks are still running";
}

void RenderHost::setUpdateMsgType( int i_type)
{
    m_updateMsgType = i_type;
}

void RenderHost::refreshTasks()
{
    if( false == AFRunning )
        return;

    // Refresh tasks:
    for( int t = 0; t < m_tasks.size(); t++)
    {
        m_tasks[t]->refresh();
    }

    // Remove zombies:
    for( std::vector<TaskProcess*>::iterator it = m_tasks.begin(); it != m_tasks.end(); )
    {
        if((*it)->isZombie())
        {
            delete *it;
            it = m_tasks.erase( it);
        }
        else
            it++;
    }
}

void RenderHost::getResources()
{
	// Do this every update time, but not the first time, as at the begininng resources are already updated
	static bool first_time = true;
	if( first_time )
	{
		first_time = false;
		return;
	}

	GetResources( m_host, m_hres);

	for( int i = 0; i < m_pyres.size(); i++)
		m_pyres[i]->update();

	//hres.stdOut();
	m_up.setResources( &m_hres);
}

af::Msg * RenderHost::updateServer()
{
	if( false == AFRunning )
		return NULL;

	m_up.setId( getId());
	
	af::Msg * msg;

	if( m_updateMsgType == af::Msg::TRenderRegister )
	{
		msg = new af::Msg( m_updateMsgType, this);
	}
	else if( m_updateMsgType == af::Msg::TRenderUpdate )
	{
		#ifdef AFOUTPUT
		AF_LOG << m_up;
		#endif
		msg = new af::Msg( m_updateMsgType, &m_up);
	}
	else if( m_updateMsgType == af::Msg::TRenderReconnect )
	{
		#ifdef AFOUTPUT
		AF_LOG << m_reco;
		#endif
		msg = new af::Msg( m_updateMsgType, &m_reco);
	}

	#ifdef AFOUTPUT
	AF_LOG << " <<< " << msg;
	#endif

	bool ok;
	af::Msg * server_answer = af::sendToServer( msg, ok,
		msg->type() == af::Msg::TRenderRegister ? af::VerboseOff : af::VerboseOn);

	if( ok )
		connectionEstablished();
	else
		connectionLost();

	delete msg;

	m_up.clear();

	return server_answer;
}

#ifdef WINNT
void RenderHost::windowsMustDie()
{
// Windows Must Die:
    AFINFO("RenderHost::windowsMustDie():");
    for( int i = 0; i < m_windowsmustdie.size(); i++)
    {
        HWND WINAPI hw = FindWindow( NULL, TEXT( m_windowsmustdie[i].c_str()));
        if( hw != NULL )
        {
            printf("Window must die found:\n%s\n", m_windowsmustdie[i].c_str());
            SendMessage( hw, WM_CLOSE, 0, 0);
        }
    }
}
#endif

void RenderHost::runTask( af::TaskExec * i_task)
{
    for( int t = 0; t < m_tasks.size(); t++)
        if( m_tasks[t]->getTaskExec()->equals( *i_task))
            return;
    
    m_tasks.push_back( new TaskProcess( i_task, this));
}

void RenderHost::stopTask( const af::MCTaskPos & i_taskpos)
{
    for( int t = 0; t < m_tasks.size(); t++)
    {
        if( m_tasks[t]->is( i_taskpos))
        {
            m_tasks[t]->stop();
            return;
        }
    }
    AFERRAR("RenderHost::stopTask: %d tasks, no such task:", int(m_tasks.size()))
    i_taskpos.v_stdOut();
}

void RenderHost::closeTask( const af::MCTaskPos & i_taskpos)
{
    for( int t = 0; t < m_tasks.size(); t++)
    {
        if( m_tasks[t]->is( i_taskpos))
        {
			m_tasks[t]->close();
            return;
        }
    }
    AFERRAR("RenderHost::closeTask: %d tasks, no such task:", int(m_tasks.size()))
    i_taskpos.v_stdOut();
}

void RenderHost::upTaskOutput( const af::MCTaskPos & i_taskpos)
{
	std::string str;
	for( int t = 0; t < m_tasks.size(); t++)
	{
		if( m_tasks[t]->is( i_taskpos))
		{
			str = m_tasks[t]->getOutput();
			break;
		}
	}

	if( str.size() == 0 )
	{
		str = "Render has no task:";
		str += i_taskpos.v_generateInfoString();
	}

	m_up.addTaskOutput( i_taskpos, str);
}

void RenderHost::listenTask( const af::MCTaskPos & i_tp, bool i_subscribe)
{
	for( int t = 0; t < m_tasks.size(); t++)
	{
		if( m_tasks[t]->is( i_tp))
		{
			m_tasks[t]->listenOutput( i_subscribe);
			break;
		}
	}
}

void RenderHost::wolSleep( const std::string & i_str)
{
	af::Service service( af::Environment::getSysWolService(),"SLEEP", i_str);
	std::string cmd = service.getCommand();
	printf("Sleep request, executing command:\n%s\n", cmd.c_str());
	af::launchProgram( cmd);
}

