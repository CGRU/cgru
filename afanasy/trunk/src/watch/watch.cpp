#include "watch.h"

#include <QtGui/QApplication>

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "buttonmonitor.h"
#include "dialog.h"
#include "monitorhost.h"
#include "listtasks.h"
#include "reciever.h"
#include "wndlist.h"
#include "wndlistenjob.h"
#include "wndlistentask.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const QString Watch::BtnName[WLAST] = { "NULL","JOBS","USERS","RENDERS","T","M"};
const QString Watch::WndName[WLAST] = { "null","Jobs","Users","Renders","Talks","Monitors"};
WndList* Watch::opened[WLAST] = {0,0,0,0,0,0};
QLinkedList<Wnd*>      Watch::windows;
QLinkedList<Reciever*> Watch::recievers;
QLinkedList<int>       Watch::listenjobids;
QLinkedList<int>       Watch::watchtasksjobids;
QLinkedList<QWidget*>  Watch::watchtaskswindows;
QStringList Watch::previewcmds;
QApplication * Watch::app = NULL;
Dialog * Watch::d = NULL;
MonitorHost * Watch::m = NULL;

Watch::Watch( Dialog * pDialog, QApplication * pApplication)
{
   app = pApplication;
   d = pDialog;
   m = d->getMonitor();
   previewcmds = af::Environment::getPreviewCmds().split( AFWATCH::PREVIEWPROGRAMS_SEP);
}

Watch::~Watch(){}

void Watch::destroy() { d = NULL; m = NULL;}

void Watch::sendMsg(        afqt::QMsg  * msg     ){if(d) d->sendMsg(        msg    );}
void Watch::displayInfo(    const QString &message){if(d){d->displayInfo(    message);if(d->isHidden())d->show();}}
void Watch::displayWarning( const QString &message){if(d){d->displayWarning( message);if(d->isHidden())d->show();}}
void Watch::displayError(   const QString &message){if(d){d->displayError(   message);if(d->isHidden())d->show();}}
void Watch::setWindowTitle( const QString & title ){if(d){d->setWindowTitle( title  );if(d->isHidden())d->show();}}
void Watch::repaintStart()  { if(d) d->repaintStart(100); }
void Watch::repaintFinish() { if(d) d->repaintFinish();   }

bool Watch::isInitialized() { if(d) return d->isInitialized(); else return false;  }
bool Watch::isConnected()   { if(d) return d->isConnected();   else return false;  }
int  Watch::getUid()        { if(d) return d->getUid();        else return 0;      }
int  Watch::getId()         { if(m) return m->getId();         else return 0;      }

void Watch::subscribe(   const QList<int> & events) { if(m) m->  subscribe( events );}
void Watch::unsubscribe( const QList<int> & events) { if(m) m->unsubscribe( events );}
void Watch::addJobId( int jId ) { if(m) m->addJobId( jId );}
void Watch::delJobId( int jId ) { if(m) m->delJobId( jId );}
void Watch::setUid(   int uid ) { if(m) m->setUid(   uid );}

void Watch::addWindow( Wnd * wnd)
{
   if( windows.contains( wnd))
   {
      AFERROR("Watch::addWindow: Window already exists.");
   }
   else windows.append( wnd);
}
void Watch::addReciever( Reciever * reciever)
{
   if( recievers.contains( reciever))
   {
      AFERROR("Watch::addReciever: Reciever already exists.");
   }
   else recievers.append( reciever);
}
void Watch::removeWindow(   Wnd      * wnd      ) {   windows.removeAll( wnd);      }
void Watch::removeReciever( Reciever * reciever ) { recievers.removeAll( reciever); }

void Watch::caseMessage( af::Msg * msg)
{
   bool recieved = false;
   QLinkedList<Reciever*>::iterator rIt;
   for( rIt = recievers.begin(); rIt != recievers.end(); ++rIt)
   {
      msg->resetWrittenSize();
      if( (*rIt)->caseMessage( msg) && (false == recieved)) recieved = true;
   }
   if( false == recieved)
   {
      AFERROR("Watch::caseMessage: Unknown message recieved:\n");
      msg->stdOut();
   }
}

void Watch::listenJob( int id, const QString & name)
{
   if( listenjobids.contains( id))
      displayWarning("This job is already listening.");
   else
   {
      listenjobids.append( id);
      new WndListenJob( id, name);
   }
}

void Watch::listenTask( int jobid, int block, int task, const QString & name)
{
   new WndListenTask( jobid, block, task, name);
}

void Watch::watchTasks( int id, const QString & name)
{
AFINFA("Watch::watchTasks: trying to open job \"%s\"[%d] tasks window.\n", name.toUtf8().data(), id);
   QLinkedList<int>::const_iterator iIt = watchtasksjobids.begin();
   QLinkedList<QWidget*>::iterator wIt = watchtaskswindows.begin();
   while( iIt != watchtasksjobids.end())
   {
      if( *iIt == id)
      {
         raiseWindow(*wIt, &name);
         return;
      }
      iIt++;
      wIt++;
   }

   WndList * wnd = new WndList("Tasks");
   ListTasks *listtasks = new ListTasks( wnd, id, name);
   wnd->setList( listtasks);
   watchtasksjobids.append( id);
   watchtaskswindows.append( wnd);
   displayInfo(QString("Opening '%1' tasks window.").arg(name));
AFINFA("Watch::watchTasks: \"%s\" window opened.\n", name.toUtf8().data());
}

void Watch::watchTasks_rem( int id)
{
   QLinkedList<int>::iterator iIt = watchtasksjobids.begin();
   QLinkedList<QWidget*>::iterator wIt = watchtaskswindows.begin();
   while( iIt != watchtasksjobids.end())
   {
      if( *iIt == id)
      {
         iIt = watchtasksjobids.erase( iIt);
         wIt = watchtaskswindows.erase( wIt);
      }
      else
      {
         iIt++;
         wIt++;
      }
   }
}

void Watch::connectionLost()
{
   for( QLinkedList<Reciever*>::iterator rIt = recievers.begin(); rIt != recievers.end(); ++rIt)
      (*rIt)->connectionLost();
   if(m) m->connectionLost();
}

void Watch::connectionEstablished()
{
   for( QLinkedList<Reciever*>::iterator rIt = recievers.begin(); rIt != recievers.end(); ++rIt)
      (*rIt)->connectionEstablished();
   if(m) m->connectionEstablished();
}

bool Watch::openMonitor( int type, bool open)
{
   if( d == NULL ) return false;
   if( opened[type])
   {
      raiseWindow( opened[type], &WndName[type]);
      return false;
   }
   return d->openMonitor( type, open);
}

void Watch::raiseWindow( QWidget * wnd, const QString * name)
{
AFINFO("Watch::raiseWindow: trying to raise a window.\n");
   if((wnd == NULL) || (name == NULL)) return;
   if( wnd->isMinimized()) wnd->showNormal();
   wnd->activateWindow();
   wnd->raise();
   if( name ) displayInfo(QString("Raising '%1' window.").arg(*name));
AFINFA("Watch::raiseWindow: \"%s\" window raised.\n", name->toUtf8().data());
}

void Watch::repaint()
{
//printf("Watch::repaint: start\n");
   QPalette palette = app->palette();
   afqt::QEnvironment::setPalette( palette);
   afqt::QEnvironment::initFonts();
   app->setPalette( palette);
   if( d) d->repaint();
   for( int i = 0; i < WLAST; i++) if( opened[i]) opened[i]->repaintItems();
   for( QLinkedList<Wnd*>::iterator wIt = windows.begin(); wIt != windows.end(); wIt++) (*wIt)->update();
//printf("Watch::repaint: finish\n");
}
