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

   std::list<std::string> resclasses = af::strSplit( af::Environment::getRenderResClasses(), ";");
   for( std::list<std::string>::const_iterator it = resclasses.begin(); it != resclasses.end(); it++)
   {
      if( (*it).empty() ) continue;
      printf("Adding custom resource meter '%s'\n", (*it).c_str());
      pyres.push_back( new PyRes( *it, &hres));
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
                  windowsmustdie.append( line);
            }
         }
         while( len != -1);
         file.close();
      }
      else printf("Unable to read '%s'.\n", filename.toUtf8().data());
   }
   if( windowsmustdie.size())
   {
      windowsmustdie.removeDuplicates();
      printf("Windows Must Die:\n");
      for( int i = 0; i < windowsmustdie.size(); i++)
         printf("   %s\n", windowsmustdie[i].toUtf8().data());
   }
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
#ifdef WINNT
   windowsMustDie();
#endif
}

afqt::QMsg* RenderHost::updateMsg( afqt::QMsg *msg)
{
//   if( msg->type() != af::Msg::TRenderUpdate) return msg;

   // Do this every update time, but not the first time, as at the begininng they are already updated
   if( upmsg != NULL )
   {
      delete upmsg;
      GetResources( host, hres, false);
      for( int i = 0; i < pyres.size(); i++) pyres[i]->update();
   }

//hres.stdOut();
//   msg->resetWrittenSize();
//   hres.readwrite( msg);

#ifdef WINNT
   windowsMustDie();
#endif

   upmsg = new afqt::QMsg( msg->type(), this, true);

   return upmsg;
}

#ifdef WINNT
void RenderHost::windowsMustDie() const
{
// Windows Must Die:
//printf("RenderHost::windowsMustDie():\n");
   for( int i = 0; i < windowsmustdie.size(); i++)
   {
      HWND WINAPI hw = FindWindow( NULL, TEXT( windowsmustdie[i].toUtf8().data()));
      if( hw != NULL )
      {
         printf("Window must die founded:\n%s\n", windowsmustdie[i].toUtf8().data());
         SendMessage( hw, WM_CLOSE, 0, 0);
      }
   }
}
#endif
