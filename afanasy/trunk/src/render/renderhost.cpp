#include "renderhost.h"

#include <QtNetwork/QTcpSocket>

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "../libafqt/name_afqt.h"

#include "res.h"

#ifdef WINNT
#include <QtCore/QFile>
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

RenderHost* render;

af::Msg* update_handler_ptr( af::Msg * msg)
{
   return render->updateMsg( msg);
}

RenderHost::RenderHost( int32_t State, uint8_t Priority):
   Render( State, Priority)
{
   setOnline();
   render = this;
   host.os = af::Environment::getPlatform();
   GetResources( host, hres, true);
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
   host.stdOut( true);
   hres.stdOut( true);
}

RenderHost::~RenderHost()
{
}

af::Msg* RenderHost::updateMsg( af::Msg *msg)
{
   if( msg->type() != af::Msg::TRenderUpdate) return msg;

   GetResources( host, hres, false);

   msg->resetWrittenSize();

//   hres.stdOut();
   hres.readwrite( msg);

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

   return msg;
}
