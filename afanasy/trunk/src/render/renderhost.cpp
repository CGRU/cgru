#include "renderhost.h"

#include <QtNetwork/QTcpSocket>

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "../libafqt/qmsg.h"

#include "pyres.h"
#include "res.h"

#ifdef WINNT
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QStringList>
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
/*
RenderHost * Render;

afqt::QMsg* update_handler_ptr( afqt::QMsg * msg)
{
   return Render->updateMsg( msg);
}
*/
RenderHost * RenderHost::ms_obj = NULL;
af::MsgQueue * RenderHost::ms_msgAcceptQueue = NULL;
af::MsgQueue * RenderHost::ms_msgDispatchQueue = NULL;
int RenderHost::ms_updateMsgType = af::Msg::TRenderRegister;
bool RenderHost::ms_connected = false;
std::vector<PyRes*> RenderHost::m_pyres;

RenderHost::RenderHost( int32_t i_state, uint8_t i_priority):
   Render( i_state, i_priority)
{
//    Render = this;
    ms_obj = this;

    ms_msgAcceptQueue   = new af::MsgQueue("Messages Accept Queue",   af::AfQueue::e_no_thread    );
    ms_msgDispatchQueue = new af::MsgQueue("Messages Dispatch Queue", af::AfQueue::e_start_thread );

    setOnline();

    host.os = af::Environment::getPlatform();
    GetResources( host, hres, true);

    std::list<std::string> resclasses = af::strSplit( af::Environment::getRenderResClasses(), ";");
    for( std::list<std::string>::const_iterator it = resclasses.begin(); it != resclasses.end(); it++)
    {
        if( (*it).empty() ) continue;
        printf("Adding custom resource meter '%s'\n", (*it).c_str());
        m_pyres.push_back( new PyRes( *it, &hres));
    }

#ifdef WINNT
    Sleep( 100);
    // Windows Must Die:
    QStringList wmdfiles = QDir(af::Environment::getAfRoot().c_str()).entryList( QStringList("windowsmustdie*.txt"), QDir::Files, QDir::Name);
    for( int i = 0; i < wmdfiles.size(); i++)
    {
        QString filename = QString("%1\\%2").arg( afqt::stoq( af::Environment::getAfRoot())).arg( wmdfiles[i]);
        QFile file( filename);
        if( file.open(QFile::ReadOnly))
        {
            char buf[4096];
            int len = 0;
            do
            {
                len = file.readLine( buf, sizeof( buf));
                if( len > 0 )
                {
                    QString line = QString( buf).simplified();
                    if( false == line.isEmpty())
                        windowsmustdie.push_back( line);
                }
            }
            while( len != -1);
            file.close();
        }
        else printf("Unable to read '%s'.\n", filename.toUtf8().data());
    }
    if( windowsmustdie.size())
    {
//        windowsmustdie.removeDuplicates();
        printf("Windows Must Die:\n");
        for( int i = 0; i < windowsmustdie.size(); i++)
            printf("   %s\n", windowsmustdie[i].c_str());
    }
#else
    usleep( 100000);
#endif

    GetResources( host, hres, false);
    for( int i = 0; i < m_pyres.size(); i++) m_pyres[i]->update();

    stdOut();
    host.stdOut( true);
    hres.stdOut( true);
}

RenderHost::~RenderHost()
{
//    qthreadClientSend->send( new afqt::QMsg( af::Msg::TRenderDeregister, render->getId()));

    delete ms_msgAcceptQueue;
    delete ms_msgDispatchQueue;

    for( int i = 0; i < m_pyres.size(); i++) if( m_pyres[i]) delete m_pyres[i];
//   if( upmsg != NULL ) delete upmsg;
#ifdef WINNT
   windowsMustDie();
#endif
}

void RenderHost::connectionEstablished()
{
    ms_connected = true;

    printf("Render connected.\n");
}

void RenderHost::connectionLost()
{
    if( ms_connected == false ) return;

    ms_connected = false;

    ms_obj->setId( 0);

    // Stop all tasks:
//    for( int t = 0; t < tasks.size(); t++) tasks[t]->stop();

    // Begin to try to register again:
    setUpdateMsgType( af::Msg::TRenderRegister);

    printf("Object: connection lost, connecting...\n");
}

void RenderHost::setUpdateMsgType( int i_type)
{
    ms_updateMsgType = i_type;
}

void RenderHost::update()
{
printf("RenderHost::update():\n");
    // Do this every update time, but not the first time, as at the begininng they are already updated
//    if( upmsg != NULL )
    static bool first_time = true;

    if( false == first_time )
    {
//        delete upmsg;
        GetResources( ms_obj->host, ms_obj->hres, false);
        for( int i = 0; i < m_pyres.size(); i++) m_pyres[i]->update();
    }
    else
        first_time = false;

//hres.stdOut();
//   msg->resetWrittenSize();
//   hres.readwrite( msg);

#ifdef WINNT
    windowsMustDie();
#endif

    dispatchMessage( new af::Msg( ms_updateMsgType, ms_obj));//, true);
}

#ifdef WINNT
void RenderHost::windowsMustDie() const
{
// Windows Must Die:
//printf("RenderHost::windowsMustDie():\n");
    for( int i = 0; i < windowsmustdie.size(); i++)
    {
        HWND WINAPI hw = FindWindow( NULL, TEXT( windowsmustdie[i].c_str());
        if( hw != NULL )
        {
            printf("Window must die founded:\n%s\n", windowsmustdie[i].c_str());
            SendMessage( hw, WM_CLOSE, 0, 0);
        }
    }
}
#endif
