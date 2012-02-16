#include "renderhost.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "pyres.h"
#include "res.h"

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
std::vector<PyRes*> RenderHost::ms_pyres;
std::vector<TaskProcess*> RenderHost::ms_tasks;

RenderHost::RenderHost( int32_t i_state, uint8_t i_priority):
   Render( i_state, i_priority)
{
//    Render = this;
    ms_obj = this;

    // Set that we are not listening any port at creation
    ms_obj->address.setPort( 0);

    ms_msgAcceptQueue   = new af::MsgQueue("Messages Accept Queue",   af::AfQueue::e_no_thread    );
    ms_msgDispatchQueue = new af::MsgQueue("Messages Dispatch Queue", af::AfQueue::e_start_thread );
    ms_msgDispatchQueue->setReturnQueue( ms_msgAcceptQueue);
    ms_msgDispatchQueue->returnNotSended();
    ms_msgDispatchQueue->setVerboseMode( af::VerboseOff);

    setOnline();

    host.os = af::Environment::getPlatform();
    GetResources( host, hres, true);

    std::list<std::string> resclasses = af::strSplit( af::Environment::getRenderResClasses(), ";");
    for( std::list<std::string>::const_iterator it = resclasses.begin(); it != resclasses.end(); it++)
    {
        if( (*it).empty() ) continue;
        printf("Adding custom resource meter '%s'\n", (*it).c_str());
        ms_pyres.push_back( new PyRes( *it, &hres));
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
    for( int i = 0; i < ms_pyres.size(); i++) ms_pyres[i]->update();

    stdOut();
    host.stdOut( true);
    hres.stdOut( true);
}

RenderHost::~RenderHost()
{
    delete ms_msgAcceptQueue;
    delete ms_msgDispatchQueue;

    for( int i = 0; i < ms_pyres.size(); i++) if( ms_pyres[i]) delete ms_pyres[i];
//   if( upmsg != NULL ) delete upmsg;
#ifdef WINNT
   windowsMustDie();
#endif

   af::Msg msg( af::Msg::TRenderDeregister, ms_obj->getId());
   msg.setAddress( af::Environment::getServerAddress());
   bool ok;
   af::msgsend( & msg, ok, af::VerboseOn);
}

void RenderHost::dispatchMessage( af::Msg * i_msg)
{
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
    ms_obj->id = i_id;
    ms_msgDispatchQueue->setVerboseMode( af::VerboseOn);
    setUpdateMsgType( af::Msg::TRenderUpdate);
    printf("Render registered.\n");
}

void RenderHost::connectionLost()
{
    if( ms_connected == false ) return;

    ms_connected = false;

    ms_obj->id = 0;

    // Stop all tasks:
//    for( int t = 0; t < tasks.size(); t++) tasks[t]->stop();

    ms_msgDispatchQueue->setVerboseMode( af::VerboseOff);

    // Begin to try to register again:
    setUpdateMsgType( af::Msg::TRenderRegister);

    printf("Render connection lost, connecting...\n");
}

void RenderHost::setUpdateMsgType( int i_type)
{
    ms_updateMsgType = i_type;
}

void RenderHost::refresh()
{
    for( std::vector<TaskProcess*>::iterator it = ms_tasks.begin(); it != ms_tasks.end(); it++)
    {
        (*it)->refresh();
    }
}

void RenderHost::update()
{
    // Do this every update time, but not the first time, as at the begininng resources are already updated
    static bool first_time = true;

    if( false == first_time )
    {
        GetResources( ms_obj->host, ms_obj->hres, false);
        for( int i = 0; i < ms_pyres.size(); i++) ms_pyres[i]->update();
    }
    else
        first_time = false;

//hres.stdOut();

#ifdef WINNT
    windowsMustDie();
#endif

    if( ms_obj->address.getPortHBO() == 0 )
    {
        // It seems that listening thread is not started to listen any port
        return;
        // Client was just started and we simple will wait
    }

    af::Msg * msg = new af::Msg( ms_updateMsgType, ms_obj);
    msg->setReceiving();
    dispatchMessage( msg);
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

void RenderHost::runTask( af::Msg * i_msg)
{
    ms_tasks.push_back( new TaskProcess( new af::TaskExec( i_msg)));
}
