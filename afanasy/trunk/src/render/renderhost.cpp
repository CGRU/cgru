#include "renderhost.h"

#include <QtNetwork/QTcpSocket>

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "../libafqt/qmsg.h"

#include "pyres.h"
#include "res.h"

#ifdef WINNT
#include <QtCore/QFile>
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

RenderHost* render;

afqt::QMsg* update_handler_ptr( afqt::QMsg * msg)
{
   return render->updateMsg( msg);
}

RenderHost::RenderHost( int32_t State, uint8_t Priority):
   Render( State, Priority),
   upmsg( NULL)
{
   setOnline();
   render = this;

   host.os = af::Environment::getPlatform();
   GetResources( host, hres, true);

   QStringList resclasses = af::Environment::getRenderResClasses().split(';');
   for( int i = 0; i < resclasses.size(); i++)
   {
      QString classname = resclasses[i].trimmed();
      if( classname.isEmpty() ) continue;
      pyres.push_back( new PyRes(classname.toUtf8().data(), &hres));
   }

#ifdef WINNT
// Windows Must Die:
   Sleep( 100);
   QString filename = QString("%1\\windowsmustdie.txt").arg( af::Environment::getAfRoot());
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
               windowsmustdie.append( line);
         }
      }
      while( len != -1);
      file.close();
      if( windowsmustdie.size())
      {
         printf("Windows Must Die:\n");
         for( int i = 0; i < windowsmustdie.size(); i++)
            printf("   %s\n", windowsmustdie[i].toUtf8().data());
      }
   }
   else printf("Unable to read '%s'.\n", filename.toUtf8().data());
#else
   usleep( 100000);
#endif

   GetResources( host, hres, false);
   for( int i = 0; i < pyres.size(); i++) pyres[i]->update();

   host.stdOut( true);
   hres.stdOut( true);
}

RenderHost::~RenderHost()
{
   for( int i = 0; i < pyres.size(); i++) if( pyres[i]) delete pyres[i];
   if( upmsg != NULL ) delete upmsg;
}

afqt::QMsg* RenderHost::updateMsg( afqt::QMsg *msg)
{
//   if( msg->type() != af::Msg::TRenderUpdate) return msg;

   GetResources( host, hres, false);
   for( int i = 0; i < pyres.size(); i++) pyres[i]->update();

//hres.stdOut();
//   msg->resetWrittenSize();
//   hres.readwrite( msg);

#ifdef WINNT
// Windows Must Die:
   for( int i = 0; i < windowsmustdie.size(); i++)
   {
      HWND WINAPI hw = FindWindow( NULL, TEXT( windowsmustdie[i].toUtf8().data()));
      if( hw != NULL )
      {
         printf("Window must die founded:\n%s\n", windowsmustdie[i].toUtf8().data());
         SendMessage( hw, WM_CLOSE, 0, 0);
      }
   }
#endif

   if( upmsg != NULL ) delete upmsg;
   upmsg = new afqt::QMsg( msg->type(), this, true);

   return upmsg;
}
